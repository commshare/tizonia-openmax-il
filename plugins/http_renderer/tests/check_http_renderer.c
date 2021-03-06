/**
 * Copyright (C) 2011-2015 Aratelia Limited - Juan A. Rubio
 *
 * This file is part of Tizonia
 *
 * Tizonia is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Tizonia is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Tizonia.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   check_http_renderer.c
 * @author Juan A. Rubio <juan.rubio@aratelia.com>
 *
 * @brief  Tizonia OpenMAX IL - HTTP Renderer unit tests
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>

#include "OMX_Component.h"
#include "OMX_Types.h"
#include "OMX_TizoniaExt.h"

#include "tizplatform.h"
#include "tizfsm.h"
#include "tizkernel.h"

#ifdef TIZ_LOG_CATEGORY_NAME
#undef TIZ_LOG_CATEGORY_NAME
#define TIZ_LOG_CATEGORY_NAME "tiz.http_renderer.check"
#endif

char *pg_rmd_path;
pid_t g_rmd_pid;
pid_t g_mplayer_pid;
bool g_manual_run = false;

#define COMPONENT_NAME "OMX.Aratelia.audio_renderer.http"
#define CHECK_HTTP_RENDERER_CURL_CMD "/bin/bash -c \"/usr/bin/curl -vs http://localhost:8011/ 2>&1 > /dev/null\""


/* TODO: Move these two to the rc file */
#define RATE_FILE1 48000
#define RATE_FILE2 44100

static const char *pg_files[] = {
  NULL,
  NULL
};

static const OMX_U32 pg_rates[] = {
  RATE_FILE1,
  RATE_FILE2
};

#define HTTP_RENDERER_TEST_TIMEOUT 360
#define INFINITE_WAIT 0xffffffff
/* duration of event timeout in msec when we expect event to be set */
#define TIMEOUT_EXPECTING_SUCCESS 500
/* duration of event timeout in msec when we don't expect event to be set */
#define TIMEOUT_EXPECTING_FAILURE 2000

typedef void *cc_ctx_t;
typedef struct check_common_context check_common_context_t;
struct check_common_context
{
  OMX_BOOL signaled;
  tiz_mutex_t mutex;
  tiz_cond_t cond;
  OMX_STATETYPE state;
  OMX_BUFFERHEADERTYPE *p_hdr;
};

static bool
refresh_rm_db (void)
{
  bool rv = false;
  const char *p_rmdb_path = NULL;
  const char *p_sqlite_path = NULL;
  const char *p_init_path = NULL;
  const char *p_rmd_path = NULL;

  p_rmdb_path = tiz_rcfile_get_value("resource-management", "rmdb");
  p_sqlite_path = tiz_rcfile_get_value("resource-management",
                                       "rmdb.sqlite_script");
  p_init_path = tiz_rcfile_get_value("resource-management",
                                     "rmdb.init_script");

  p_rmd_path = tiz_rcfile_get_value("resource-management", "rmd.path");

  if (!p_rmdb_path || !p_sqlite_path || !p_init_path || !p_rmd_path)

    {
      TIZ_LOG(TIZ_PRIORITY_TRACE, "Test data not available...");
    }
  else
    {
      pg_rmd_path = strndup (p_rmd_path, PATH_MAX);

      TIZ_LOG(TIZ_PRIORITY_TRACE, "RM daemon [%s] ...", pg_rmd_path);

      /* Re-fresh the rm db */
      size_t total_len = strlen (p_init_path)
        + strlen (p_sqlite_path)
        + strlen (p_rmdb_path) + 4;
      char *p_cmd = tiz_mem_calloc (1, total_len);
      if (p_cmd)
        {
          snprintf(p_cmd, total_len -1, "%s %s %s",
                  p_init_path, p_sqlite_path, p_rmdb_path);
          if (-1 != system (p_cmd))
            {
              TIZ_LOG(TIZ_PRIORITY_TRACE, "Successfully run [%s] script...", p_cmd);
              rv = true;
            }
          else
            {
              TIZ_LOG(TIZ_PRIORITY_TRACE, 
                      "Error while executing db init shell script...");
            }
          tiz_mem_free (p_cmd);
        }
    }

  return rv;
}

static void
setup (void)
{
  int error = 0;

  fail_if (!refresh_rm_db());

  /* Start the rm daemon */
  g_rmd_pid = fork ();
  fail_if (g_rmd_pid == -1);

  if (g_rmd_pid)
    {
      sleep (1);
    }
  else
    {
      TIZ_LOG (TIZ_PRIORITY_TRACE, "Starting the RM Daemon");
      const char *arg0 = "";
      error = execlp (pg_rmd_path, arg0, (char *) NULL);
      fail_if (error == -1);
    }
}

static void
teardown (void)
{
  int error = 0;

  if (g_rmd_pid)
    {
      error = kill (g_rmd_pid, SIGTERM);
      fail_if (error == -1);
    }
  tiz_mem_free (pg_rmd_path);
}

static OMX_ERRORTYPE
_ctx_init (cc_ctx_t * app_ctx)
{
  check_common_context_t *p_ctx =
    tiz_mem_alloc (sizeof (check_common_context_t));

  if (!p_ctx)
    {
      return OMX_ErrorInsufficientResources;
    }

  p_ctx->signaled = OMX_FALSE;

  if (tiz_mutex_init (&p_ctx->mutex))
    {
      tiz_mem_free (p_ctx);
      return OMX_ErrorInsufficientResources;
    }

  if (tiz_cond_init (&p_ctx->cond))
    {
      tiz_mutex_destroy (&p_ctx->mutex);
      tiz_mem_free (p_ctx);
      return OMX_ErrorInsufficientResources;
    }

  * app_ctx = p_ctx;

  return OMX_ErrorNone;

}

static OMX_ERRORTYPE
_ctx_destroy (cc_ctx_t * app_ctx)
{
  check_common_context_t *p_ctx = NULL;
  assert (app_ctx);
  p_ctx = * app_ctx;

  if (tiz_mutex_lock (&p_ctx->mutex))
    {
      return OMX_ErrorBadParameter;
    }

  tiz_cond_destroy (&p_ctx->cond);
  tiz_mutex_unlock (&p_ctx->mutex);
  tiz_mutex_destroy (&p_ctx->mutex);

  tiz_mem_free (p_ctx);

  return OMX_ErrorNone;

}

static OMX_ERRORTYPE
_ctx_signal (cc_ctx_t * app_ctx)
{
  check_common_context_t *p_ctx = NULL;
  assert (app_ctx);
  p_ctx = * app_ctx;

  if (tiz_mutex_lock (&p_ctx->mutex))
    {
      return OMX_ErrorBadParameter;
    }

  p_ctx->signaled = OMX_TRUE;
  tiz_cond_signal (&p_ctx->cond);
  tiz_mutex_unlock (&p_ctx->mutex);

  return OMX_ErrorNone;
}

static OMX_ERRORTYPE
_ctx_wait (cc_ctx_t * app_ctx, OMX_U32 a_millis, OMX_BOOL * ap_has_timedout)
{
  int retcode;
  check_common_context_t *p_ctx = NULL;
  assert (app_ctx);
  p_ctx = * app_ctx;

  TIZ_LOG (TIZ_PRIORITY_TRACE, "a_millis [%u]", a_millis);

  * ap_has_timedout = OMX_FALSE;

  if (tiz_mutex_lock (&p_ctx->mutex))
    {
      return OMX_ErrorBadParameter;
    }

  if (0 == a_millis)
    {
      if (!p_ctx->signaled)
        {
          * ap_has_timedout = OMX_TRUE;
        }
    }

  else if (INFINITE_WAIT == a_millis)
    {
      while (!p_ctx->signaled)
        {
          tiz_cond_wait (&p_ctx->cond, &p_ctx->mutex);
        }
    }

  else
    {
      while (!p_ctx->signaled)
        {
          retcode = tiz_cond_timedwait (&p_ctx->cond,
                                          &p_ctx->mutex, a_millis);

          /* TODO: Change this to OMX_ErrorTimeout */
          if (retcode == OMX_ErrorUndefined && !p_ctx->signaled)
            {
              * ap_has_timedout = OMX_TRUE;
              break;
            }
        }
    }

  tiz_mutex_unlock (&p_ctx->mutex);

  return OMX_ErrorNone;

}

static OMX_ERRORTYPE
_ctx_reset (cc_ctx_t * app_ctx)
{
  check_common_context_t *p_ctx = NULL;
  assert (app_ctx);
  p_ctx = * app_ctx;

  if (tiz_mutex_lock (&p_ctx->mutex))
    {
      return OMX_ErrorBadParameter;
    }

  p_ctx->signaled = OMX_FALSE;
  tiz_mutex_unlock (&p_ctx->mutex);

  return OMX_ErrorNone;
}


OMX_ERRORTYPE
check_EventHandler (OMX_HANDLETYPE ap_hdl,
                    OMX_PTR ap_app_data,
                    OMX_EVENTTYPE eEvent,
                    OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData)
{
  check_common_context_t *p_ctx = NULL;
  cc_ctx_t *pp_ctx = NULL;
  assert (ap_app_data);
  pp_ctx = (cc_ctx_t *) ap_app_data;
  p_ctx = *pp_ctx;

  TIZ_LOG (TIZ_PRIORITY_TRACE, "Component Event [%s]", tiz_evt_to_str (eEvent));

  if (OMX_EventCmdComplete == eEvent)
    {
      switch ((OMX_COMMANDTYPE) (nData1))
        {
        case OMX_CommandStateSet:
          {
            TIZ_LOG (TIZ_PRIORITY_TRACE, "Component transitioned to [%s]",
                       tiz_state_to_str ((OMX_STATETYPE) (nData2)));
            p_ctx->state = (OMX_STATETYPE) (nData2);
            _ctx_signal (pp_ctx);
            break;
          }

        case OMX_CommandPortDisable:
        case OMX_CommandPortEnable:
        default:
          {
            assert (0);
          }

        };
    }

  if (OMX_EventBufferFlag == eEvent)
    {
      if (nData2 & OMX_BUFFERFLAG_EOS)
        {
          TIZ_LOG (TIZ_PRIORITY_TRACE, "Received EOS from [%s] port[%i]",
                     COMPONENT_NAME, nData1);
        }
      else
        {
          fail_if (0);
        }
    }

  return OMX_ErrorNone;

}

OMX_ERRORTYPE check_EmptyBufferDone
  (OMX_HANDLETYPE ap_hdl,
   OMX_PTR ap_app_data, OMX_BUFFERHEADERTYPE * ap_buf)
{
  check_common_context_t *p_ctx = NULL;
  cc_ctx_t *pp_ctx = NULL;

  TIZ_LOG (TIZ_PRIORITY_TRACE, "EmptyBufferDone: BUFFER [%p]", ap_buf);

  assert (ap_app_data);
  assert (ap_buf);
  pp_ctx = (cc_ctx_t *) ap_app_data;
  p_ctx = *pp_ctx;

  p_ctx->p_hdr = ap_buf;
  _ctx_signal (pp_ctx);

  return OMX_ErrorNone;

}

OMX_ERRORTYPE check_FillBufferDone
  (OMX_HANDLETYPE ap_hdl,
   OMX_PTR ap_app_data, OMX_BUFFERHEADERTYPE * ap_buf)
{
  return OMX_ErrorNone;
}


static OMX_CALLBACKTYPE _check_cbacks = {
  check_EventHandler,
  check_EmptyBufferDone,
  check_FillBufferDone
};

static bool
init_test_data()
{
  bool rv = false;
  const char *p_testfile1 = NULL;
  const char *p_testfile2 = NULL;

  p_testfile1 = tiz_rcfile_get_value("plugins-data",
                                     "OMX.Aratelia.audio_renderer.http.testfile1_uri");
  p_testfile2 = tiz_rcfile_get_value("plugins-data",
                                     "OMX.Aratelia.audio_renderer.http.testfile2_uri");

  if (!p_testfile1 || !p_testfile2)

    {
      TIZ_LOG(TIZ_PRIORITY_TRACE, "Test data not available...");
    }
  else
    {
      pg_files[0] = p_testfile1; pg_files[1] = p_testfile2;
      TIZ_LOG(TIZ_PRIORITY_TRACE, "Test data available [%s]", pg_files[0]);
      TIZ_LOG(TIZ_PRIORITY_TRACE, "Test data available [%s]", pg_files[1]);
      rv = true;
    }

  return rv;
}

static int
exec_mplayer (void)
{

  /* NOTE: This return statment is here to manually run this test by connecting
     an external streaming client like vlc, mplayer, etc...*/
  /* e.g.: */
  /*  */
  /* $ cd ~/work/tizonia/plugins/http_renderer */
  /* $ tests/.libs/check_http_renderer whatever */
  /*  */

  if (g_manual_run)
    {
      return 1;
    }

  g_mplayer_pid = fork ();
  fail_if (g_mplayer_pid == -1);

  if (g_mplayer_pid)
    {
      return g_mplayer_pid;
    }
  else
    {
      char cmd [128];
      TIZ_LOG (TIZ_PRIORITY_TRACE, "Connecting curl...");
      sleep (1);
      snprintf (cmd, strlen (CHECK_HTTP_RENDERER_CURL_CMD) + 1, "%s",
                CHECK_HTTP_RENDERER_CURL_CMD);
      TIZ_LOG (TIZ_PRIORITY_TRACE, "cmd = [%s]", cmd);
      fail_if (-1 == system (cmd));
    }

  return 0;
}


/*
 * Unit tests
 */

START_TEST (test_http_streaming)
{
  OMX_ERRORTYPE error = OMX_ErrorNone;
  OMX_HANDLETYPE p_hdl = NULL;
  OMX_COMMANDTYPE cmd = OMX_CommandStateSet;
  OMX_STATETYPE state = OMX_StateIdle;
  cc_ctx_t ctx;
  check_common_context_t *p_ctx = NULL;
  OMX_BOOL timedout = OMX_FALSE;
  OMX_PARAM_PORTDEFINITIONTYPE port_def;
  OMX_TIZONIA_HTTPSERVERTYPE httpsrv;
  OMX_AUDIO_PARAM_MP3TYPE mp3type;
  OMX_BUFFERHEADERTYPE **p_hdrlst;
  OMX_U32 i = 0;
  FILE *p_file = 0;
  int err = 0;
  fail_if (!init_test_data());

  TIZ_LOG (TIZ_PRIORITY_TRACE, "Init", p_hdl);

  error = _ctx_init (&ctx);
  fail_if (OMX_ErrorNone != error);

  p_ctx = (check_common_context_t *) (ctx);

  error = OMX_Init ();
  fail_if (OMX_ErrorNone != error);

  /* ------------------------- */
  /* Instantiate the component */
  /* ------------------------- */
  error = OMX_GetHandle (&p_hdl, COMPONENT_NAME, (OMX_PTR *) (&ctx),
                         &_check_cbacks);
  fail_if (OMX_ErrorNone != error);

  TIZ_LOG (TIZ_PRIORITY_TRACE, "p_hdl [%p]", p_hdl);

  /* -------------------------------- */
  /* Obtain the port def from port #0 */
  /* -------------------------------- */
  port_def.nSize = sizeof (OMX_PARAM_PORTDEFINITIONTYPE);
  port_def.nVersion.nVersion = OMX_VERSION;
  port_def.nPortIndex = 0;
  error = OMX_GetParameter (p_hdl, OMX_IndexParamPortDefinition, &port_def);
  fail_if (OMX_ErrorNone != error);

  TIZ_LOG (TIZ_PRIORITY_TRACE, "nBufferSize [%d]", port_def.nBufferSize);
  TIZ_LOG (TIZ_PRIORITY_TRACE, "nBufferCountActual [%d]",
             port_def.nBufferCountActual);

  /* -------------------------------- */
  /* Retrieve the http server params  */
  /* -------------------------------- */
  httpsrv.nSize = sizeof (OMX_TIZONIA_HTTPSERVERTYPE);
  httpsrv.nVersion.nVersion = OMX_VERSION;
  error = OMX_GetParameter (p_hdl, OMX_TizoniaIndexParamHttpServer, &httpsrv);
  TIZ_LOG (TIZ_PRIORITY_TRACE, "OMX_GetParameter(OMX_TizoniaIndexParamHttpServer) = [%s]",
           tiz_err_to_str (error));
  fail_if (OMX_ErrorNone != error);

  /* ---------------------------------------- */
  /* Now set the desired http server settings */
  /* ---------------------------------------- */
  httpsrv.nSize = sizeof (OMX_TIZONIA_HTTPSERVERTYPE);
  httpsrv.nVersion.nVersion = OMX_VERSION;
  httpsrv.nListeningPort = 8011;
  httpsrv.nMaxClients = 5;
  error = OMX_SetParameter (p_hdl, OMX_TizoniaIndexParamHttpServer, &httpsrv);
  fail_if (OMX_ErrorNone != error);

  /* --------------------------- */
  /* Retrieve the mp3 settings   */
  /* --------------------------- */
  mp3type.nSize             = sizeof (OMX_AUDIO_PARAM_MP3TYPE);
  mp3type.nVersion.nVersion = OMX_VERSION;
  mp3type.nPortIndex        = 0;
  error = OMX_GetParameter (p_hdl, OMX_IndexParamAudioMp3, &mp3type);
  fail_if (OMX_ErrorNone != error);

  /* -------------------- */
  /* Set some mp3 params  */
  /* -------------------- */
  mp3type.nSize             = sizeof (OMX_AUDIO_PARAM_MP3TYPE);
  mp3type.nVersion.nVersion = OMX_VERSION;
  mp3type.nPortIndex        = 0;
  mp3type.nBitRate          = 320000;
  mp3type.nSampleRate       = 48000;
  mp3type.nChannels         = 2;
  error = OMX_SetParameter (p_hdl, OMX_IndexParamAudioMp3, &mp3type);
  fail_if (OMX_ErrorNone != error);

  /* --------------------------- */
  /* Initiate transition to IDLE */
  /* --------------------------- */
  error = OMX_SendCommand (p_hdl, cmd, state, NULL);
  fail_if (OMX_ErrorNone != error);

  TIZ_LOG (TIZ_PRIORITY_TRACE, "Allocating buffers");

  /* ---------------- */
  /* Allocate buffers */
  /* ---------------- */
  p_hdrlst = (OMX_BUFFERHEADERTYPE **)
    tiz_mem_calloc (port_def.nBufferCountActual,
                    sizeof (OMX_BUFFERHEADERTYPE *));

  for (i = 0; i < port_def.nBufferCountActual; ++i)
    {
      error = OMX_AllocateBuffer (p_hdl, &p_hdrlst[i], 0,    /* input port */
                                  0, port_def.nBufferSize);
      fail_if (OMX_ErrorNone != error);
      fail_if (p_hdrlst[i] == NULL);
      TIZ_LOG (TIZ_PRIORITY_TRACE, "p_hdrlst[%i] =  [%p]", i, p_hdrlst[i]);
      TIZ_LOG (TIZ_PRIORITY_TRACE, "p_hdrlst[%d]->nAllocLen [%d]", i,
                 p_hdrlst[i]->nAllocLen);
      TIZ_LOG (TIZ_PRIORITY_TRACE, "p_hdrlst[%d]->nFilledLen [%d]", i,
                 p_hdrlst[i]->nFilledLen);
      TIZ_LOG (TIZ_PRIORITY_TRACE, "p_hdrlst[%d]->nOffset [%d]", i,
                 p_hdrlst[i]->nOffset);
      TIZ_LOG (TIZ_PRIORITY_TRACE, "p_hdrlst[%d]->nOutputPortIndex [%d]", i,
                 p_hdrlst[i]->nOutputPortIndex);
      TIZ_LOG (TIZ_PRIORITY_TRACE, "p_hdrlst[%d]->nInputPortIndex [%d]", i,
                 p_hdrlst[i]->nInputPortIndex);
      TIZ_LOG (TIZ_PRIORITY_TRACE, "p_hdrlst[%d]->nFlags [%X]", i,
                 p_hdrlst[i]->nFlags);
      fail_if (port_def.nBufferSize > p_hdrlst[i]->nAllocLen);

    }

  /* ------------------------- */
  /* Await transition callback */
  /* ------------------------- */
  error = _ctx_wait (&ctx, TIMEOUT_EXPECTING_SUCCESS, &timedout);
  fail_if (OMX_ErrorNone != error);
  fail_if (OMX_TRUE == timedout);
  TIZ_LOG (TIZ_PRIORITY_TRACE, "p_ctx->state [%s]",
             tiz_state_to_str (p_ctx->state));
  fail_if (OMX_StateIdle != p_ctx->state);

  /* ------------------------------ */
  /* Check state transition success */
  /* ------------------------------ */
  error = OMX_GetState (p_hdl, &state);
  TIZ_LOG (TIZ_PRIORITY_TRACE, "state [%s]", tiz_state_to_str (state));
  fail_if (OMX_ErrorNone != error);
  fail_if (OMX_StateIdle != state);

  /* -------------------------- */
  /* Initiate transition to EXE */
  /* -------------------------- */
  error = _ctx_reset (&ctx);
  state = OMX_StateExecuting;
  error = OMX_SendCommand (p_hdl, cmd, state, NULL);
  fail_if (OMX_ErrorNone != error);

  /* ------------------------- */
  /* Await transition callback */
  /* ------------------------- */
  error = _ctx_wait (&ctx, TIMEOUT_EXPECTING_SUCCESS, &timedout);
  fail_if (OMX_ErrorNone != error);
  fail_if (OMX_TRUE == timedout);
  TIZ_LOG (TIZ_PRIORITY_TRACE, "p_ctx->state [%s]",
             tiz_state_to_str (p_ctx->state));
  fail_if (OMX_StateExecuting != p_ctx->state);

  /* fork */
  if (exec_mplayer ())
    {
      /* -------------------- */
      /* buffer transfer loop */
      /* -------------------- */
      fail_if ((p_file = fopen (pg_files[_i], "r")) == 0);

      i = 0;
      while (i < port_def.nBufferCountActual)
        {
          TIZ_LOG (TIZ_PRIORITY_TRACE, "Reading from file [%s]", pg_files[_i]);
          if (!
              (err =
               fread (p_hdrlst[i]->pBuffer, 1, port_def.nBufferSize, p_file)))
            {
              if (feof (p_file))
                {
                  TIZ_LOG (TIZ_PRIORITY_TRACE, "End of file reached for [%s]",
                           pg_files[_i]);
                }
              else
                {
                  TIZ_LOG (TIZ_PRIORITY_TRACE,
                           "An error occurred while reading [%s]",
                           pg_files[_i]);
                  fail_if (0);
                }
            }

          /* --------------- */
          /* Transfer buffer */
          /* --------------- */
          error = _ctx_reset (&ctx);
          p_hdrlst[i]->nFilledLen = port_def.nBufferSize;
          TIZ_LOG (TIZ_PRIORITY_TRACE, "Emptying buffer with len [%d]", p_hdrlst[i]->nFilledLen);

          error = OMX_EmptyThisBuffer (p_hdl, p_hdrlst[i]);
          fail_if (OMX_ErrorNone != error);

          /* ------------------------- */
          /* Await BufferDone callback */
          /* ------------------------- */
          do
            {
              error = _ctx_wait (&ctx, TIMEOUT_EXPECTING_SUCCESS, &timedout);
              fail_if (OMX_ErrorNone != error);
              /*       fail_if (OMX_TRUE == timedout); */
              if (error == OMX_ErrorNone && timedout == OMX_FALSE)
                {
                  fail_if (p_ctx->p_hdr != p_hdrlst[i]);
                }
              else
                {
                  timedout = OMX_FALSE;
                  error = OMX_ErrorTimeout;
                }
            } 
          while (error != OMX_ErrorNone);

          i++;
          i %= port_def.nBufferCountActual;

          if (0 == err)
            {
              /* EOF */
              break;
            }

        }

      fclose (p_file);

      /* --------------------------- */
      /* Initiate transition to IDLE */
      /* --------------------------- */
      error = _ctx_reset (&ctx);
      state = OMX_StateIdle;
      error = OMX_SendCommand (p_hdl, cmd, state, NULL);
      fail_if (OMX_ErrorNone != error);

      /* ------------------------- */
      /* Await transition callback */
      /* ------------------------- */
      error = _ctx_wait (&ctx, TIMEOUT_EXPECTING_SUCCESS, &timedout);
      fail_if (OMX_ErrorNone != error);
      fail_if (OMX_TRUE == timedout);
      TIZ_LOG (TIZ_PRIORITY_TRACE, "p_ctx->state [%s]",
               tiz_state_to_str (p_ctx->state));
      fail_if (OMX_StateIdle != p_ctx->state);

      /* ----------------------------- */
      /* Initiate transition to LOADED */
      /* ----------------------------- */
      error = _ctx_reset (&ctx);
      state = OMX_StateLoaded;
      error = OMX_SendCommand (p_hdl, cmd, state, NULL);
      fail_if (OMX_ErrorNone != error);

      /* ------------------ */
      /* Deallocate buffers */
      /* ------------------ */
      fail_if (OMX_ErrorNone != error);
      for (i = 0; i < port_def.nBufferCountActual; ++i)
        {
          error = OMX_FreeBuffer (p_hdl, 0,      /* input port */
                                  p_hdrlst[i]);
          fail_if (OMX_ErrorNone != error);
        }

      /* ------------------------- */
      /* Await transition callback */
      /* ------------------------- */
      error = _ctx_wait (&ctx, TIMEOUT_EXPECTING_SUCCESS, &timedout);
      fail_if (OMX_ErrorNone != error);
      fail_if (OMX_TRUE == timedout);
      fail_if (OMX_StateLoaded != p_ctx->state);

      /* ------------------------------ */
      /* Check state transition success */
      /* ------------------------------ */
      error = OMX_GetState (p_hdl, &state);
      TIZ_LOG (TIZ_PRIORITY_TRACE, "state [%s]", tiz_state_to_str (state));
      fail_if (OMX_ErrorNone != error);
      fail_if (OMX_StateLoaded != state);

      error = OMX_FreeHandle (p_hdl);
      fail_if (OMX_ErrorNone != error);

      tiz_mem_free (p_hdrlst);

      error = OMX_Deinit ();
      fail_if (OMX_ErrorNone != error);

      _ctx_destroy (&ctx);
    }
  else
    {
      /* TODO: Compare dumped file with original */
    }

}
END_TEST

Suite *
httpr_suite (void)
{

  TCase *tc_httpr;
  Suite *s = suite_create ("libtizhttpr");

  /* test case */
  tc_httpr = tcase_create ("HTTP Renderer");
  tcase_add_unchecked_fixture (tc_httpr, setup, teardown);
  tcase_set_timeout (tc_httpr, HTTP_RENDERER_TEST_TIMEOUT);
  tcase_add_loop_test (tc_httpr, test_http_streaming, 0, 1);
  suite_add_tcase (s, tc_httpr);

  return s;

}

int
main (int argc, char **argv)
{
  int number_failed;
  SRunner *sr = srunner_create (httpr_suite ());

  if (argc > 1)
    {
      g_manual_run = true;
    }

  tiz_log_init();

  TIZ_LOG (TIZ_PRIORITY_TRACE, "Tizonia OpenMAX IL - HTTP Renderer unit tests");

  srunner_run_all (sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);

  tiz_log_deinit ();

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* Local Variables: */
/* c-default-style: gnu */
/* fill-column: 79 */
/* indent-tabs-mode: nil */
/* compile-command: "make check" */
/* End: */

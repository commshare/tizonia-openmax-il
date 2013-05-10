/**
 * Copyright (C) 2011-2013 Aratelia Limited - Juan A. Rubio
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
 * @file   arprc.c
 * @author Juan A. Rubio <juan.rubio@aratelia.com>
 *
 * @brief  Audio Renderer Component processor class
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "arprc.h"
#include "arprc_decls.h"
#include "tizkernel.h"
#include "tizscheduler.h"
#include "tizosal.h"

#include <assert.h>

#ifdef TIZ_LOG_CATEGORY_NAME
#undef TIZ_LOG_CATEGORY_NAME
#define TIZ_LOG_CATEGORY_NAME "tiz.audio_renderer.prc"
#endif

#define TIZ_AR_ALSA_PCM_DEVICE "null"

/*
 * arprc
 */

static void *
ar_proc_ctor (void *ap_obj, va_list * app)
{
  ar_prc_t *p_obj = super_ctor (arprc, ap_obj, app);
  p_obj->p_playback_hdl = NULL;
  p_obj->p_hw_params = NULL;
  p_obj->p_alsa_pcm_ = NULL;
  return p_obj;
}

static void *
ar_proc_dtor (void *ap_obj)
{
  ar_prc_t *p_obj = ap_obj;

  tiz_mem_free (p_obj->p_alsa_pcm_);

  if (NULL != p_obj->p_hw_params)
    {
      snd_pcm_hw_params_free (p_obj->p_hw_params);
    }

  if (NULL != p_obj->p_playback_hdl)
    {
      snd_pcm_close (p_obj->p_playback_hdl);
    }

  return super_dtor (arprc, ap_obj);
}

static OMX_ERRORTYPE
ar_proc_render_buffer (const void *ap_obj, OMX_BUFFERHEADERTYPE * p_hdr)
{
  const ar_prc_t *p_obj = ap_obj;
  int err, offset = 0;
  int samples, step;

  TIZ_LOG (TIZ_TRACE,
           "Rendering HEADER [%p]...nFilledLen[%d] !!!", p_hdr,
           p_hdr->nFilledLen);

  step = (p_obj->pcmmode.nBitPerSample / 8) * p_obj->pcmmode.nChannels;
  samples = p_hdr->nFilledLen / step;
  TIZ_LOG (TIZ_TRACE, "step [%d], samples [%d]", step, samples);

  while (samples)
    {
      err = snd_pcm_writei (p_obj->p_playback_hdl,
                            p_hdr->pBuffer + p_hdr->nOffset + offset,
                            samples);
      TIZ_LOG (TIZ_TRACE,
               "Rendering HEADER [%p]..." "err [%d] samples [%d]",
               p_hdr, err, samples);
      if (-EAGAIN == err)
        {
          TIZ_LOG (TIZ_TRACE, "Rendering HEADER [%p]...-EAGAIN");
          continue;
        }

      if (err < 0)
        {
          TIZ_LOG (TIZ_TRACE, "Rendering HEADER [%p]...underflow");
          err = snd_pcm_recover (p_obj->p_playback_hdl, err, 0);
          if (err < 0)
            {
              TIZ_LOG (TIZ_ERROR, "snd_pcm_recover error: %s",
                       snd_strerror (err));
              break;
            }
        }

      offset += err * step;
      samples -= err;

    }

  p_hdr->nFilledLen = 0;

  return OMX_ErrorNone;
}

/*
 * from tiz_srv class
 */
static char *
get_alsa_device (void *ap_obj)
{
  ar_prc_t *p_obj = ap_obj;
  const char *p_alsa_pcm = NULL;

  assert (ap_obj);

  assert (NULL == p_obj->p_alsa_pcm_);

  p_alsa_pcm
    = tiz_rcfile_get_value ("plugins-data", "OMX.Aratelia.audio_renderer.pcm.alsa_device");

  if (NULL != p_alsa_pcm)
    {
      TIZ_LOG (TIZ_TRACE, "Using ALSA pcm [%s]...", p_alsa_pcm);
      p_obj->p_alsa_pcm_ = strndup (p_alsa_pcm, OMX_MAX_STRINGNAME_SIZE);
    }
  else
    {
      TIZ_LOG (TIZ_TRACE,
               "No alsa device found in config file. Using [%s]...",
               TIZ_AR_ALSA_PCM_DEVICE);
    }

  return (NULL != p_obj->p_alsa_pcm_) ? p_obj->p_alsa_pcm_
    : TIZ_AR_ALSA_PCM_DEVICE;
}

static OMX_ERRORTYPE
ar_proc_allocate_resources (void *ap_obj, OMX_U32 a_pid)
{
  ar_prc_t *p_obj = ap_obj;
  int err;
  assert (ap_obj);

  if (!(p_obj->p_playback_hdl))
    {
      if ((err =
           snd_pcm_open (&p_obj->p_playback_hdl, get_alsa_device (p_obj),
                         SND_PCM_STREAM_PLAYBACK, 0)) < 0)
        {
          TIZ_LOG (TIZ_ERROR, "cannot open audio device %s (%s)",
                   TIZ_AR_ALSA_PCM_DEVICE, snd_strerror (err));
          return OMX_ErrorInsufficientResources;
        }

      if ((err = snd_pcm_hw_params_malloc (&p_obj->p_hw_params)) < 0)
        {
          TIZ_LOG (TIZ_ERROR,
                   "cannot allocate hardware parameter structure" " (%s)",
                   snd_strerror (err));
          return OMX_ErrorInsufficientResources;
        }
    }

  if ((err = snd_pcm_hw_params_any (p_obj->p_playback_hdl,
                                    p_obj->p_hw_params)) < 0)
    {
      TIZ_LOG (TIZ_ERROR, "cannot initialize hardware parameter "
               "structure (%s)", snd_strerror (err));
      return OMX_ErrorInsufficientResources;
    }

  TIZ_LOG (TIZ_TRACE, "Resource allocation complete... "
           "arprc = [%p]!!!", p_obj);

  return OMX_ErrorNone;
}

static OMX_ERRORTYPE
ar_proc_deallocate_resources (void *ap_obj)
{
  ar_prc_t *p_obj = ap_obj;
  assert (ap_obj);

  if (p_obj->p_hw_params)
    {
      snd_pcm_hw_params_free (p_obj->p_hw_params);
      snd_pcm_close (p_obj->p_playback_hdl);
      p_obj->p_playback_hdl = NULL;
      p_obj->p_hw_params = NULL;
    }

  tiz_mem_free (p_obj->p_alsa_pcm_);
  p_obj->p_alsa_pcm_ = NULL;

  TIZ_LOG (TIZ_TRACE, "Resource deallocation complete...");

  return OMX_ErrorNone;
}

static OMX_ERRORTYPE
ar_proc_prepare_to_transfer (void *ap_obj, OMX_U32 a_pid)
{
  ar_prc_t *p_obj = ap_obj;
  const tiz_srv_t *p_parent = ap_obj;
  OMX_ERRORTYPE ret_val = OMX_ErrorNone;
  void *p_krn = tiz_get_krn (p_parent->p_hdl_);
  int err;
  snd_pcm_format_t snd_pcm_format;
  assert (ap_obj);

  if (NULL != p_obj->p_playback_hdl)
    {
      /* Retrieve pcm params from port */
      p_obj->pcmmode.nSize = sizeof (OMX_AUDIO_PARAM_PCMMODETYPE);
      p_obj->pcmmode.nVersion.nVersion = OMX_VERSION;
      p_obj->pcmmode.nPortIndex = 0;    /* port index */
      if (OMX_ErrorNone != (ret_val = tiz_api_GetParameter
                            (p_krn,
                             p_parent->p_hdl_,
                             OMX_IndexParamAudioPcm, &p_obj->pcmmode)))
        {
          TIZ_LOG (TIZ_ERROR, "Error retrieving pcm params from port");
          return ret_val;
        }

      TIZ_LOG (TIZ_NOTICE, "nChannels = [%d] nBitPerSample = [%d] "
               "nSamplingRate = [%d] eNumData = [%d] eEndian = [%d] "
               "bInterleaved = [%s] ePCMMode = [%d]",
               p_obj->pcmmode.nChannels,
               p_obj->pcmmode.nBitPerSample,
               p_obj->pcmmode.nSamplingRate,
               p_obj->pcmmode.eNumData,
               p_obj->pcmmode.eEndian,
               p_obj->pcmmode.bInterleaved ? "OMX_TRUE" : "OMX_FALSE",
               p_obj->pcmmode.ePCMMode);

      /* TODO : Add function to encode properly encode snd_pcm_format */
      snd_pcm_format = p_obj->pcmmode.eEndian == OMX_EndianLittle ?
        SND_PCM_FORMAT_S16 : SND_PCM_FORMAT_S16_BE;

      if ((err = snd_pcm_set_params (p_obj->p_playback_hdl,
                                     snd_pcm_format,
                                     SND_PCM_ACCESS_RW_INTERLEAVED,
                                     p_obj->pcmmode.nChannels,
                                     p_obj->pcmmode.nSamplingRate,
                                     1, 100 * 1000)) < 0)
        {
          TIZ_LOG (TIZ_TRACE, "Didn' work...p_obj = [ERROR]!!!");
        }

      if ((err = snd_pcm_prepare (p_obj->p_playback_hdl)) < 0)
        {
          TIZ_LOG (TIZ_ERROR, "Cannot prepare audio interface for use "
                   "(%s)", snd_strerror (err));
          return OMX_ErrorInsufficientResources;
        }
    }

  return OMX_ErrorNone;
}

static OMX_ERRORTYPE
ar_proc_transfer_and_process (void *ap_obj, OMX_U32 a_pid)
{
  ar_prc_t *p_obj = ap_obj;
  assert (ap_obj);

  TIZ_LOG (TIZ_TRACE, "Awaiting buffers...p_obj = [%p]!!!", p_obj);

  return OMX_ErrorNone;

}

static OMX_ERRORTYPE
ar_proc_stop_and_return (void *ap_obj)
{
  ar_prc_t *p_obj = ap_obj;
  assert (ap_obj);

  TIZ_LOG (TIZ_TRACE, "Stopped buffer transfer...p_obj = [%p]!!!", p_obj);

  return OMX_ErrorNone;
}

/*
 * from tiz_prc class
 */

static OMX_ERRORTYPE
ar_proc_buffers_ready (const void *ap_obj)
{
  const tiz_srv_t *p_parent = ap_obj;
  tiz_pd_set_t ports;
  void *p_krn = tiz_get_krn (p_parent->p_hdl_);
  OMX_BUFFERHEADERTYPE *p_hdr = NULL;

  TIZ_PD_ZERO (&ports);

  tiz_check_omx_err (tiz_krn_select (p_krn, 1, &ports));

  if (TIZ_PD_ISSET (0, &ports))
    {
      tiz_check_omx_err (tiz_krn_claim_buffer (p_krn, 0, 0, &p_hdr));
      TIZ_LOG (TIZ_TRACE, "Claimed HEADER [%p]...", p_hdr);
      tiz_check_omx_err (ar_proc_render_buffer (ap_obj, p_hdr));
      if (p_hdr->nFlags & OMX_BUFFERFLAG_EOS)
        {
          TIZ_LOG (TIZ_DEBUG, "OMX_BUFFERFLAG_EOS in HEADER [%p]", p_hdr);
          tiz_srv_issue_event ((OMX_PTR) ap_obj,
                                  OMX_EventBufferFlag,
                                  0, p_hdr->nFlags, NULL);
        }
      tiz_krn_release_buffer (p_krn, 0, p_hdr);
    }

  return OMX_ErrorNone;
}

/*
 * initialization
 */

const void *arprc;

void
ar_prc_init (void)
{
  if (!arprc)
    {
      tiz_prc_init ();
      arprc =
        factory_new
        (tizprc_class,
         "arprc",
         tizprc,
         sizeof (ar_prc_t),
         ctor, ar_proc_ctor,
         dtor, ar_proc_dtor,
         tiz_srv_allocate_resources, ar_proc_allocate_resources,
         tiz_srv_deallocate_resources, ar_proc_deallocate_resources,
         tiz_srv_prepare_to_transfer, ar_proc_prepare_to_transfer,
         tiz_srv_transfer_and_process, ar_proc_transfer_and_process,
         tiz_srv_stop_and_return, ar_proc_stop_and_return,
         tiz_prc_buffers_ready, ar_proc_buffers_ready, 0);
    }
}
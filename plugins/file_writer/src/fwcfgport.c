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
 * @file   fwcfgport.c
 * @author Juan A. Rubio <juan.rubio@aratelia.com>
 *
 * @brief  Tizonia OpenMAX IL - Binary Writer config port implementation
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "fwcfgport.h"
#include "fwcfgport_decls.h"
#include "tizosal.h"

#include <assert.h>
#include <string.h>
#include <limits.h>

#ifdef TIZ_LOG_CATEGORY_NAME
#undef TIZ_LOG_CATEGORY_NAME
#define TIZ_LOG_CATEGORY_NAME "tiz.file_writer.fwcfgport"
#endif

#define TIZ_FILE_WRITER_DEFAULT_AUDIO_URI "OMX.Aratelia.file_writer.binary.default_audio_uri"

static char *
find_default_uri ()
{
  char *p_rv = NULL;
  const char *p_uri = NULL;

  p_uri = tiz_rcfile_get_value ("plugins-data",
                                TIZ_FILE_WRITER_DEFAULT_AUDIO_URI);

  assert (NULL != p_uri
          &&
          "OMX.Aratelia.file_writer.binary.default_audio_uri not present in tizrc...");

  TIZ_LOG (TIZ_TRACE, "Default URI [%s]...", p_uri);

  p_rv = strndup (p_uri, PATH_MAX);

  return p_rv;
}


/*
 * fwcfgport class
 */

static void *
fw_cfgport_ctor (void *ap_obj, va_list * app)
{
  fw_cfgport_t *p_obj = super_ctor (fwcfgport, ap_obj, app);
  p_obj->p_uri_ = find_default_uri ();
  tiz_port_register_index (p_obj, OMX_IndexParamContentURI);
  return p_obj;
}

static void *
fw_cfgport_dtor (void *ap_obj)
{
  fw_cfgport_t *p_obj = (fw_cfgport_t *) ap_obj;
  tiz_mem_free (p_obj->p_uri_);
  return super_dtor (fwcfgport, ap_obj);
}

/*
 * from tiz_api
 */

static OMX_ERRORTYPE
fw_cfgport_GetParameter (const void *ap_obj,
                         OMX_HANDLETYPE ap_hdl,
                         OMX_INDEXTYPE a_index, OMX_PTR ap_struct)
{
  const fw_cfgport_t *p_obj = ap_obj;

  switch (a_index)
    {
    case OMX_IndexParamContentURI:
      {
        OMX_PARAM_CONTENTURITYPE *p_uri
          = (OMX_PARAM_CONTENTURITYPE *) ap_struct;
        OMX_U32 uri_buf_size =
          p_uri->nSize - sizeof (OMX_U32) - sizeof (OMX_VERSIONTYPE);
        OMX_U32 uri_len = strlen (p_obj->p_uri_);
        TIZ_LOG (TIZ_TRACE, "uri_buf_size [%d]...", uri_buf_size);
        TIZ_LOG (TIZ_TRACE, "p_obj->p_uri_ [%08x]...", p_obj->p_uri_);

        if (uri_buf_size < (uri_len + 1))
          {
            return OMX_ErrorBadParameter;
          }

        p_uri->nVersion.nVersion = OMX_VERSION;
        strncpy ((char *) p_uri->contentURI, p_obj->p_uri_, uri_len + 1);
        if (p_uri->contentURI)
          {
            p_uri->contentURI[uri_len] = '\0';
          }

      }
      break;

    default:
      {
        /* Delegate to the base port */
        return super_GetParameter (fwcfgport,
                                   ap_obj, ap_hdl, a_index, ap_struct);
      }
    };

  return OMX_ErrorNone;

}

static OMX_ERRORTYPE
fw_cfgport_SetParameter (const void *ap_obj,
                         OMX_HANDLETYPE ap_hdl,
                         OMX_INDEXTYPE a_index, OMX_PTR ap_struct)
{
  fw_cfgport_t *p_obj = (fw_cfgport_t *) ap_obj;

  TIZ_LOG (TIZ_TRACE, "SetParameter [%s]...", tiz_idx_to_str (a_index));

  switch (a_index)
    {

    case OMX_IndexParamContentURI:
      {
        OMX_PARAM_CONTENTURITYPE *p_uri
          = (OMX_PARAM_CONTENTURITYPE *) ap_struct;
        OMX_U32 uri_size =
          p_uri->nSize - sizeof (OMX_U32) - sizeof (OMX_VERSIONTYPE);

        tiz_mem_free (p_obj->p_uri_);
        p_obj->p_uri_ = tiz_mem_calloc (1, uri_size);
        strncpy (p_obj->p_uri_, (char *) p_uri->contentURI, uri_size);
        if (p_obj->p_uri_)
          {
            p_uri->contentURI[uri_size - 1] = '\0';
          }

        TIZ_LOG (TIZ_TRACE, "Set URI [%s]...", p_obj->p_uri_);
      }
      break;

    default:
      {
        /* Delegate to the base port */
        return super_SetParameter (fwcfgport,
                                   ap_obj, ap_hdl, a_index, ap_struct);
      }
    };

  return OMX_ErrorNone;

}

/*
 * initialization
 */

const void *fwcfgport;

void
fw_cfgport_init (void)
{
  if (!fwcfgport)
    {
      tiz_configport_init ();
      fwcfgport =
        factory_new
        (tizconfigport_class,
         "fwcfgport",
         tizconfigport,
         sizeof (fw_cfgport_t),
         ctor, fw_cfgport_ctor,
         dtor, fw_cfgport_dtor,
         tiz_api_GetParameter, fw_cfgport_GetParameter,
         tiz_api_SetParameter, fw_cfgport_SetParameter, 0);
    }
}
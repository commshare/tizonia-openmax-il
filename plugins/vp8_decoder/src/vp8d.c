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
 * @file   vp8d.c
 * @author Juan A. Rubio <juan.rubio@aratelia.com>
 * 
 * @brief  Tizonia OpenMAX IL - VP8 Decoder component
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "OMX_Core.h"
#include "OMX_Component.h"
#include "OMX_Types.h"

#include "tizosal.h"
#include "tizscheduler.h"
#include "tizvp8port.h"
#include "tizconfigport.h"
#include "vp8dprc.h"

#include <assert.h>
#include <string.h>

#ifdef TIZ_LOG_CATEGORY_NAME
#undef TIZ_LOG_CATEGORY_NAME
#define TIZ_LOG_CATEGORY_NAME "tiz.vp8_decoder"
#endif

#define ARATELIA_VP8_DECODER_DEFAULT_FRAME_WIDTH  176
#define ARATELIA_VP8_DECODER_DEFAULT_FRAME_HEIGHT 144

#define ARATELIA_VP8_DECODER_DEFAULT_ROLE "video_decoder.vp8"
#define ARATELIA_VP8_DECODER_COMPONENT_NAME "OMX.Aratelia.video_decoder.vp8"

/* With libtizonia, port indexes must start at index 0 */
#define ARATELIA_VP8_DECODER_INPUT_PORT_INDEX  0
#define ARATELIA_VP8_DECODER_OUTPUT_PORT_INDEX 1

#define ARATELIA_VP8_DECODER_PORT_MIN_BUF_COUNT 2
/* 38016 = (width * height) + ((width * height)/2) */
#define ARATELIA_VP8_DECODER_PORT_MIN_INPUT_BUF_SIZE 38016
#define ARATELIA_VP8_DECODER_PORT_MIN_OUTPUT_BUF_SIZE 345600
#define ARATELIA_VP8_DECODER_PORT_NONCONTIGUOUS OMX_FALSE
#define ARATELIA_VP8_DECODER_PORT_ALIGNMENT 0
#define ARATELIA_VP8_DECODER_PORT_SUPPLIERPREF OMX_BufferSupplyInput

static OMX_VERSIONTYPE vp8_decoder_version = { {1, 0, 0, 0} };

static OMX_PTR
instantiate_input_port (OMX_HANDLETYPE ap_hdl)
{
  OMX_VIDEO_PORTDEFINITIONTYPE portdef;
  OMX_VIDEO_PARAM_VP8TYPE vp8type;
  OMX_VIDEO_CODINGTYPE encodings[] = {
    OMX_VIDEO_CodingVP8,
    OMX_VIDEO_CodingMax
  };
  OMX_COLOR_FORMATTYPE formats[] = {
    OMX_COLOR_FormatUnused,
    OMX_COLOR_FormatMax
  };
  tiz_port_options_t vp8_port_opts = {
    OMX_PortDomainVideo,
    OMX_DirInput,
    ARATELIA_VP8_DECODER_PORT_MIN_BUF_COUNT,
    ARATELIA_VP8_DECODER_PORT_MIN_INPUT_BUF_SIZE,
    ARATELIA_VP8_DECODER_PORT_NONCONTIGUOUS,
    ARATELIA_VP8_DECODER_PORT_ALIGNMENT,
    ARATELIA_VP8_DECODER_PORT_SUPPLIERPREF,
    {ARATELIA_VP8_DECODER_INPUT_PORT_INDEX, NULL, NULL, NULL},
    1                           /* slave port */
  };
  OMX_VIDEO_VP8LEVELTYPE levels[] = {
    OMX_VIDEO_VP8Level_Version0,
    OMX_VIDEO_VP8Level_Version1,
    OMX_VIDEO_VP8Level_Version2,
    OMX_VIDEO_VP8Level_Version3,
    OMX_VIDEO_VP8LevelMax
  };

  /* This figures are based on the defaults defined in the standard for the VP8
   * decoder component */
  portdef.pNativeRender         = NULL;
  portdef.nFrameWidth           = 176;
  portdef.nFrameHeight          = 144;
  portdef.nStride               = 0;
  portdef.nSliceHeight          = 0;
  portdef.nBitrate              = 64000;
  portdef.xFramerate            = 15;
  portdef.bFlagErrorConcealment = OMX_FALSE;
  portdef.eCompressionFormat    = OMX_VIDEO_CodingVP8;
  portdef.eColorFormat          = OMX_COLOR_FormatUnused;
  portdef.pNativeWindow         = NULL;

  vp8type.nSize               = sizeof (OMX_VIDEO_PARAM_VP8TYPE);
  vp8type.nVersion.nVersion   = OMX_VERSION;
  vp8type.nPortIndex          = ARATELIA_VP8_DECODER_INPUT_PORT_INDEX;
  vp8type.eProfile            = OMX_VIDEO_VP8ProfileMain;
  vp8type.eLevel              = OMX_VIDEO_VP8Level_Version0;
  vp8type.nDCTPartitions      = 0; /* 1 DCP partitiion */
  vp8type.bErrorResilientMode = OMX_FALSE;

  tiz_vp8port_init ();
  return factory_new (tizvp8port, &vp8_port_opts, &portdef,
                      &encodings, &formats, &vp8type, &levels,
                      NULL  /* OMX_VIDEO_PARAM_BITRATETYPE */);
}

static OMX_PTR
instantiate_output_port (OMX_HANDLETYPE ap_hdl)
{
  OMX_VIDEO_PORTDEFINITIONTYPE portdef;
  OMX_VIDEO_CODINGTYPE encodings[] = {
    OMX_VIDEO_CodingUnused,
    OMX_VIDEO_CodingMax
  };
  OMX_COLOR_FORMATTYPE formats[] = {
    OMX_COLOR_FormatYUV420Planar,
    OMX_COLOR_FormatMax
  };
  tiz_port_options_t rawvideo_port_opts = {
    OMX_PortDomainVideo,
    OMX_DirOutput,
    ARATELIA_VP8_DECODER_PORT_MIN_BUF_COUNT,
    ARATELIA_VP8_DECODER_PORT_MIN_OUTPUT_BUF_SIZE,
    ARATELIA_VP8_DECODER_PORT_NONCONTIGUOUS,
    ARATELIA_VP8_DECODER_PORT_ALIGNMENT,
    ARATELIA_VP8_DECODER_PORT_SUPPLIERPREF,
    {ARATELIA_VP8_DECODER_OUTPUT_PORT_INDEX, NULL, NULL, NULL},
    0                           /* Master port */
  };

  /* This figures are based on the defaults defined in the standard for the VP8
   * decoder component */
  portdef.pNativeRender         = NULL;
  portdef.nFrameWidth           = 176;
  portdef.nFrameHeight          = 144;
  portdef.nStride               = 0;
  portdef.nSliceHeight          = 0;
  portdef.nBitrate              = 64000;
  portdef.xFramerate            = 15;
  portdef.bFlagErrorConcealment = OMX_FALSE;
  portdef.eCompressionFormat    = OMX_VIDEO_CodingUnused;
  portdef.eColorFormat          = OMX_COLOR_FormatYUV420Planar;
  portdef.pNativeWindow         = NULL;

  tiz_videoport_init ();
  return factory_new (tizvideoport, &rawvideo_port_opts, &portdef,
                      &encodings, &formats);
}

static OMX_PTR
instantiate_config_port (OMX_HANDLETYPE ap_hdl)
{
  tiz_configport_init ();
  return factory_new (tizconfigport, NULL,   /* this port does not take options */
                      ARATELIA_VP8_DECODER_COMPONENT_NAME,
                      vp8_decoder_version);
}

static OMX_PTR
instantiate_processor (OMX_HANDLETYPE ap_hdl)
{
  /* Instantiate the processor */
  vp8d_prc_init ();
  return factory_new (vp8dprc, ap_hdl);
}

OMX_ERRORTYPE
OMX_ComponentInit (OMX_HANDLETYPE ap_hdl)
{
  tiz_role_factory_t role_factory;
  const tiz_role_factory_t *rf_list[] = { &role_factory };

  assert (ap_hdl);

  TIZ_LOG (TIZ_TRACE, "OMX_ComponentInit: "
           "Inititializing [%s]", ARATELIA_VP8_DECODER_COMPONENT_NAME);

  strcpy ((OMX_STRING) role_factory.role, ARATELIA_VP8_DECODER_DEFAULT_ROLE);
  role_factory.pf_cport   = instantiate_config_port;
  role_factory.pf_port[0] = instantiate_input_port;
  role_factory.pf_port[1] = instantiate_output_port;
  role_factory.nports     = 2;
  role_factory.pf_proc    = instantiate_processor;

  tiz_check_omx_err (tiz_comp_init (ap_hdl, ARATELIA_VP8_DECODER_COMPONENT_NAME));
  tiz_check_omx_err (tiz_comp_register_roles (ap_hdl, rf_list, 1));

  return OMX_ErrorNone;
}
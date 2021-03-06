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
 * @file   tizbinaryport_decls.h
 * @author Juan A. Rubio <juan.rubio@aratelia.com>
 *
 * @brief  Tizonia OpenMAX IL - binaryport class declarations
 *
 *
 */

#ifndef TIZBINARYPORT_DECLS_H
#define TIZBINARYPORT_DECLS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "tizport_decls.h"

  typedef struct tiz_binaryport tiz_binaryport_t;
  struct tiz_binaryport
  {
    /* Object */
    const tiz_port_t _;
    void *p_port_;
  };

  typedef struct tiz_binaryport_class tiz_binaryport_class_t;
  struct tiz_binaryport_class
  {
    /* Class */
    const tiz_port_class_t _;
    /* NOTE: Class methods might be added in the future */
  };

#ifdef __cplusplus
}
#endif

#endif                          /* TIZBINARYPORT_DECLS_H */

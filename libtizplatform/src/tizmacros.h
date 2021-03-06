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
 * @file   tizmacros.h
 * @author Juan A. Rubio <juan.rubio@aratelia.com>
 *
 * @brief  Tizonia Platform - Macro utilities
 *
 *
 */

#ifndef TIZMACROS_H
#define TIZMACROS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tizomxutils.h"
#include "tizomxutils.h"

#include <stddef.h>

#undef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#undef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define TIZ_LIKELY(expr) (__builtin_expect (expr, 1))
#define TIZ_UNLIKELY(expr) (__builtin_expect (expr, 0))
#else
#define TIZ_LIKELY(expr) (expr)
#define TIZ_UNLIKELY(expr) (expr)
#endif

/* NOTE : This is WORK IN PROGRESS !!! : When the TIZ_DISABLE_CHECKS feature is
   implemented correctly, it WILL NOT DISABLE ALL CHECKS, only those that have
   to do with out-of-memory conditions. */
#ifdef TIZ_DISABLE_CHECKS

/**
 * tiz_check_omx_err:
 * @expr: the expression to check
 *
 * Verifies that the expression evaluates to OMX_ErrorNone.  Otherwise an error
 * message is logged and the current function returns the resulting openmax il
 * error.
 *
 * If TIZ_DISABLE_CHECKS is defined then the check is not performed but the
 * expression is still evaluated.
 */
#define tiz_check_omx_err \
  (expr) do               \
  {                       \
    (void)(expr);         \
  }                       \
  while (0)

/**
 * tiz_check_omx_err_ret_oom:
 * @expr: the expression to check
 *
 * Verifies that the expression evaluates to OMX_ErrorNone. Otherwise an error
 * message is logged and the current function returns
 * OMX_ErrorInsufficientResources.
 *
 * If TIZ_DISABLE_CHECKS is defined then the check is not performed but the
 * expression is still evaluated.
 */
#define tiz_check_omx_err_ret_oom \
  (expr) do                       \
  {                               \
    (void)(expr);                 \
  }                               \
  while (0)

/**
 * tiz_check_null_ret_oom:
 * @expr: the expression to check
 *
 * Verifies that the expression evaluates to non-NULL. Otherwise an error
 * message is logged and the current function returns
 * OMX_ErrorInsufficientResources.
 *
 * If TIZ_DISABLE_CHECKS is defined then the check is not performed but the
 * expression is still evaluated.
 */
#define tiz_check_omx_err_ret_oom \
  (expr) do                       \
  {                               \
    (void)(expr);                 \
  }                               \
  while (0)

/**
 * tiz_check_omx_err_ret_null:
 * @expr: the expression to check
 *
 * Verifies that the expression evaluates to OMX_ErrorNone. Otherwise an error
 * message is logged and the current function returns NULL.
 *
 * If TIZ_DISABLE_CHECKS is defined then the check is not performed but the
 * expression is still evaluated.
 */
#define tiz_check_omx_err_ret_null \
  (expr) do                        \
  {                                \
    (void)(expr);                  \
  }                                \
  while (0)

/**
 * tiz_check_omx_err_ret_val:
 * @expr: the expression to check
 *
 * Verifies that the expression evaluates to OMX_ErrorNone. Otherwise an error
 * message is logged and the current function returns @val (an openmax il
 * error).
 *
 * If TIZ_DISABLE_CHECKS is defined then the check is not performed but the
 * expression is still evaluated.
 */
#define tiz_check_omx_err_ret_val \
  (expr) do                       \
  {                               \
    (void)(expr);                 \
  }                               \
  while (0)

/**
 * tiz_check_err:
 * @expr: the expression to check
 *
 * Verifies that the expression evaluates to true.  If the expression evaluates
 * to false, an error message is logged and the current function returns.  This
 * is to be used only in functions that do not return a value.
 *
 * If TIZ_DISABLE_CHECKS is defined then the check is not performed but the
 * expression is still evaluated.
 */
#define tiz_check_err \
  (expr) do           \
  {                   \
    (void)(expr);     \
  }                   \
  while (0)

/**
 * tiz_ret_val_on_err:
 * @expr: the expression to check
 * @val: the value to return if the expression is not true
 *
 * Verifies that the expression evaluates to true.  If the expression evaluates
 * to false, an error message is logged and @val is returned from the current
 * function.
 *
 * If TIZ_DISABLE_CHECKS is defined then the check is not performed but the
 * expression is still evaluated.
 */
#define tiz_check_ret_ret_val \
  (expr, val) do              \
  {                           \
    (void)(expr);             \
  }                           \
  while (0)

#else /* !TIZ_DISABLE_CHECKS */

#ifdef __GNUC__

#define tiz_check_omx_err(expr)                                           \
  do                                                                      \
    {                                                                     \
      OMX_ERRORTYPE _err = expr;                                          \
      if                                                                  \
        TIZ_LIKELY (OMX_ErrorNone == _err)                                \
        {                                                                 \
        }                                                                 \
      else                                                                \
        {                                                                 \
          TIZ_LOG (TIZ_PRIORITY_ERROR, "[%s]...", tiz_err_to_str (_err)); \
          return _err;                                                    \
        }                                                                 \
    }                                                                     \
  while (0)

#define tiz_check_omx_err_ret_oom(expr)                \
  do                                                   \
    {                                                  \
      OMX_ERRORTYPE _err = expr;                       \
      if                                               \
        TIZ_LIKELY (OMX_ErrorNone == _err)             \
        {                                              \
        }                                              \
      else                                             \
        {                                              \
          TIZ_LOG (TIZ_PRIORITY_ERROR,                 \
                   "[OMX_ErrorInsufficientResources] " \
                   "was [%s]...",                      \
                   tiz_err_to_str (_err));             \
          return OMX_ErrorInsufficientResources;       \
        }                                              \
    }                                                  \
  while (0)

#define tiz_check_null_ret_oom(expr)                                        \
  do                                                                        \
    {                                                                       \
      if                                                                    \
        TIZ_LIKELY (NULL != (expr))                                         \
        {                                                                   \
        }                                                                   \
      else                                                                  \
        {                                                                   \
          TIZ_LOG (TIZ_PRIORITY_ERROR, "[OMX_ErrorInsufficientResources]"); \
          return OMX_ErrorInsufficientResources;                            \
        }                                                                   \
    }                                                                       \
  while (0)

#define tiz_check_omx_err_ret_null(expr)                       \
  do                                                           \
    {                                                          \
      OMX_ERRORTYPE _err = expr;                               \
      if                                                       \
        TIZ_LIKELY (OMX_ErrorNone == _err)                     \
        {                                                      \
        }                                                      \
      else                                                     \
        {                                                      \
          TIZ_LOG (TIZ_PRIORITY_ERROR, "[NULL] : was [%s]...", \
                   tiz_err_to_str (_err));                     \
          return NULL;                                         \
        }                                                      \
    }                                                          \
  while (0)

#define tiz_check_omx_err_ret_val(expr, val)                     \
  do                                                             \
    {                                                            \
      OMX_ERRORTYPE _err = expr;                                 \
      if                                                         \
        TIZ_LIKELY (OMX_ErrorNone == _err)                       \
        {                                                        \
        }                                                        \
      else                                                       \
        {                                                        \
          TIZ_LOG (TIZ_PRIORITY_ERROR, "[%s] : was [%s]",        \
                   tiz_err_to_str (val), tiz_err_to_str (_err)); \
          return (val);                                          \
        }                                                        \
    }                                                            \
  while (0)

#define tiz_ret_on_err(expr)                                        \
  do                                                                \
    {                                                               \
      if                                                            \
        TIZ_LIKELY (expr)                                           \
        {                                                           \
        }                                                           \
      else                                                          \
        {                                                           \
          TIZ_LOG (TIZ_PRIORITY_ERROR, "Check '%s' failed", #expr); \
          return;                                                   \
        }                                                           \
    }                                                               \
  while (0)

#define tiz_ret_val_on_err(expr, val)                               \
  do                                                                \
    {                                                               \
      if                                                            \
        TIZ_LIKELY (expr)                                           \
        {                                                           \
        }                                                           \
      else                                                          \
        {                                                           \
          TIZ_LOG (TIZ_PRIORITY_ERROR, "Check '%s' failed", #expr); \
          return val;                                               \
        }                                                           \
    }                                                               \
  while (0)

#else /* !__GNUC__ */

#define tiz_check_omx_err(expr)                                           \
  do                                                                      \
    {                                                                     \
      OMX_ERRORTYPE _err = expr;                                          \
      if (OMX_ErrorNone != _err)                                          \
        {                                                                 \
          TIZ_LOG (TIZ_PRIORITY_ERROR, "[%s]...", tiz_err_to_str (_err)); \
          return _err;                                                    \
        }                                                                 \
    }                                                                     \
  while (0)

#define tiz_check_omx_err_ret_oom(expr)                \
  do                                                   \
    {                                                  \
      OMX_ERRORTYPE _err = expr;                       \
      if (OMX_ErrorNone != _err)                       \
        {                                              \
          TIZ_LOG (TIZ_PRIORITY_ERROR,                 \
                   "[OMX_ErrorInsufficientResources] " \
                   "was [%s]...",                      \
                   tiz_err_to_str (_err));             \
          return OMX_ErrorInsufficientResources;       \
        }                                              \
    }                                                  \
  while (0)

#define tiz_check_null_ret_oom(expr)                                        \
  do                                                                        \
    {                                                                       \
      if (NULL != (expr))                                                   \
        {                                                                   \
        }                                                                   \
      else                                                                  \
        {                                                                   \
          TIZ_LOG (TIZ_PRIORITY_ERROR, "[OMX_ErrorInsufficientResources]"); \
          return OMX_ErrorInsufficientResources;                            \
        }                                                                   \
    }                                                                       \
  while (0)

#define tiz_check_omx_err_ret_null(expr)                       \
  do                                                           \
    {                                                          \
      OMX_ERRORTYPE _err = expr;                               \
      if (OMX_ErrorNone != _err)                               \
        {                                                      \
          TIZ_LOG (TIZ_PRIORITY_ERROR, "[NULL] : was [%s]...", \
                   tiz_err_to_str (_err));                     \
          return NULL;                                         \
        }                                                      \
    }                                                          \
  while (0)

#define tiz_check_omx_err_ret_val(expr, val)                     \
  do                                                             \
    {                                                            \
      OMX_ERRORTYPE _err = expr;                                 \
      if (OMX_ErrorNone != _err)                                 \
        {                                                        \
          TIZ_LOG (TIZ_PRIORITY_ERROR, "[%s] : was [%s]",        \
                   tiz_err_to_str (val), tiz_err_to_str (_err)); \
          return (val);                                          \
        }                                                        \
    }                                                            \
  while (0)

#define tiz_ret_on_err                                            \
  (expr) do                                                       \
  {                                                               \
    if (expr)                                                     \
      {                                                           \
      }                                                           \
    else                                                          \
      {                                                           \
        TIZ_LOG (TIZ_PRIORITY_ERROR, "Check '%s' failed", #expr); \
        return;                                                   \
      }                                                           \
  }                                                               \
  while (0)

#define tiz_ret_val_on_err                                        \
  (expr, val) do                                                  \
  {                                                               \
    if (expr)                                                     \
      {                                                           \
      }                                                           \
    else                                                          \
      {                                                           \
        TIZ_LOG (TIZ_PRIORITY_ERROR, "Check '%s' failed", #expr); \
        return (val);                                             \
      }                                                           \
  }                                                               \
  while (0)

#endif /* !__GNUC__ */

#endif /* !TIZ_DISABLE_CHECKS */

  /* Avoid unused variables warnings */

#ifdef TIZ_UNUSED
#elif defined(__GNUC__)
#define TIZ_UNUSED(x) UNUSED_##x __attribute__ ((unused))
#elif defined(__LCLINT__)
#define TIZ_UNUSED(x) /*@unused@*/ x
#elif defined(__cplusplus)
#define TIZ_UNUSED(x)
#else
#define TIZ_UNUSED(x) x
#endif

  /* Turn off ASAN (Address Sanitazer) */

#if defined(__clang__) || defined (__GNUC__)
# define ATTRIBUTE_NO_SANITIZE_ADDRESS __attribute__((no_sanitize_address))
#else
# define ATTRIBUTE_NO_SANITIZE_ADDRESS
#endif

#ifdef __cplusplus
}
#endif

#endif /* TIZMACROS_H */

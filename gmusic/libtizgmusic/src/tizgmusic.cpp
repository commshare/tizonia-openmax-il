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
 * @file   tizgmusic.cpp
 * @author Juan A. Rubio <juan.rubio@aratelia.com>
 *
 * @brief  Tizonia - Simple Google Music client library
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <boost/lexical_cast.hpp>

#include "tizgmusic.hpp"

namespace bp = boost::python;

/* This macro assumes the existence of an "int rc" local variable */
#define try_catch_wrapper(expr)                                  \
  do                                                             \
    {                                                            \
      try                                                        \
        {                                                        \
          (expr);                                                \
        }                                                        \
      catch (bp::error_already_set & e)                          \
        {                                                        \
          PyErr_PrintEx (0);                                     \
          rc = 1;                                                \
        }                                                        \
      catch (const std::exception &e)                            \
        {                                                        \
          std::cerr << e.what ();                                \
        }                                                        \
      catch (...)                                                \
        {                                                        \
          std::cerr << std::string ("Unknown exception caught"); \
        }                                                        \
    }                                                            \
  while (0)

namespace
{
  void init_gmusic (boost::python::object &py_main,
                    boost::python::object &py_global)
  {
    Py_Initialize ();

    // Import the Google Music proxy module
    py_main = bp::import ("tizgmusicproxy");

    // Retrieve the main module's namespace
    py_global = py_main.attr ("__dict__");
  }

  void start_gmusic (boost::python::object &py_global,
                     boost::python::object &py_gm_proxy,
                     const std::string &user, const std::string &pass,
                     const std::string &device_id)
  {
    bp::object pygmusicproxy = py_global["tizgmusicproxy"];
    py_gm_proxy
        = pygmusicproxy (user.c_str (), pass.c_str (), device_id.c_str ());
    py_gm_proxy.attr ("update_local_lib")();
  }
}

tizgmusic::tizgmusic (const std::string &user, const std::string &pass,
                      const std::string &device_id)
  : user_ (user), pass_ (pass), device_id_ (device_id)
{
}

tizgmusic::~tizgmusic ()
{
}

int tizgmusic::init ()
{
  int rc = 0;
  try_catch_wrapper (init_gmusic (py_main_, py_global_));
  return rc;
}

int tizgmusic::start ()
{
  int rc = 0;
  try_catch_wrapper (
      start_gmusic (py_global_, py_gm_proxy_, user_, pass_, device_id_));
  return rc;
}

void tizgmusic::stop ()
{
  int rc = 0;
  try_catch_wrapper (py_gm_proxy_.attr ("logout")());
  (void)rc;
}

void tizgmusic::deinit ()
{
  // boost::python doesn't support Py_Finalize() yet!
}

int tizgmusic::enqueue_album (const std::string &album)
{
  int rc = 0;
  try_catch_wrapper (py_gm_proxy_.attr ("enqueue_album")(bp::object (album)));
  return rc;
}

int tizgmusic::enqueue_artist (const std::string &artist)
{
  int rc = 0;
  try_catch_wrapper (py_gm_proxy_.attr ("enqueue_artist")(bp::object (artist)));
  return rc;
}

int tizgmusic::enqueue_playlist (const std::string &playlist)
{
  int rc = 0;
  try_catch_wrapper (py_gm_proxy_.attr ("enqueue_playlist")(bp::object (playlist)));
  return rc;
}

const char *tizgmusic::get_next_url ()
{
  current_url_.clear ();
  try
    {
      const char *p_next_url
          = bp::extract< char const * >(py_gm_proxy_.attr ("next_url")());
      if (p_next_url && !get_current_song ())
        {
          current_url_.assign (p_next_url);
        }
    }
  catch (bp::error_already_set &e)
    {
      PyErr_PrintEx (0);
    }
  catch (...)
    {
    }
  return current_url_.empty () ? NULL : current_url_.c_str ();
}

const char *tizgmusic::get_prev_url ()
{
  current_url_.clear ();
  try
    {
      const char *p_prev_url
          = bp::extract< char const * >(py_gm_proxy_.attr ("prev_url")());
      if (p_prev_url && !get_current_song ())
        {
          current_url_.assign (p_prev_url);
        }
    }
  catch (bp::error_already_set &e)
    {
      PyErr_PrintEx (0);
    }
  catch (...)
    {
    }
  return current_url_.empty () ? NULL : current_url_.c_str ();
}

const char *tizgmusic::get_current_song_artist ()
{
  return current_artist_.empty () ? NULL : current_artist_.c_str ();
}

const char *tizgmusic::get_current_song_title ()
{
  return current_title_.empty () ? NULL : current_title_.c_str ();
}

const char *tizgmusic::get_current_song_album ()
{
  return current_album_.empty () ? NULL : current_album_.c_str ();
}

const char *tizgmusic::get_current_song_duration ()
{
  return current_duration_.empty () ? NULL : current_duration_.c_str ();
}

const char *tizgmusic::get_current_song_track_number ()
{
  return current_track_num_.empty () ? NULL : current_track_num_.c_str ();
}

const char *tizgmusic::get_current_song_tracks_in_album ()
{
  return current_song_tracks_total_.empty ()
             ? NULL
             : current_song_tracks_total_.c_str ();
}

void tizgmusic::clear_queue ()
{
  int rc = 0;
  try_catch_wrapper (py_gm_proxy_.attr ("clear_queue")());
  (void)rc;
}

int tizgmusic::get_current_song ()
{
  int rc = 1;
  current_artist_.clear ();
  current_title_.clear ();

  const bp::tuple &info1 = bp::extract< bp::tuple >(
      py_gm_proxy_.attr ("current_song_title_and_artist")());
  const char *p_artist = bp::extract< char const * >(info1[0]);
  const char *p_title = bp::extract< char const * >(info1[1]);

  if (p_artist)
    {
      current_artist_.assign (p_artist);
    }
  if (p_title)
    {
      current_title_.assign (p_title);
    }

  const bp::tuple &info2 = bp::extract< bp::tuple >(
      py_gm_proxy_.attr ("current_song_album_and_duration")());
  const char *p_album = bp::extract< char const * >(info2[0]);
  int duration = bp::extract< int >(info2[1]);

  if (p_album)
    {
      current_album_.assign (p_album);
    }

  int seconds = 0;
  current_duration_.clear ();
  if (duration)
    {
      duration /= 1000;
      seconds = duration % 60;
      int minutes = (duration - seconds) / 60;
      int hours = 0;
      if (minutes >= 60)
        {
          int total_minutes = minutes;
          minutes = total_minutes % 60;
          hours = (total_minutes - minutes) / 60;
        }

      if (hours > 0)
        {
          current_duration_.append (boost::lexical_cast< std::string >(hours));
          current_duration_.append ("h:");
        }

      if (minutes > 0)
        {
          current_duration_.append (
              boost::lexical_cast< std::string >(minutes));
          current_duration_.append ("m:");
        }
    }

  char seconds_str[3];
  sprintf (seconds_str, "%02i", seconds);
  current_duration_.append (seconds_str);
  current_duration_.append ("s");

  const bp::tuple &info3 = bp::extract< bp::tuple >(
      py_gm_proxy_.attr ("current_song_track_number_and_total_tracks")());
  const int track_num = bp::extract< int >(info3[0]);
  const int total_tracks = bp::extract< int >(info3[1]);

  current_track_num_.assign (boost::lexical_cast< std::string >(track_num));
  current_song_tracks_total_.assign (boost::lexical_cast< std::string >(total_tracks));

  if (p_artist || p_title)
    {
      rc = 0;
    }

  return rc;
}

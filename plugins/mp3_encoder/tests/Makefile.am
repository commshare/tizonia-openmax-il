# Copyright (C) 2011-2015 Aratelia Limited - Juan A. Rubio
#
# This file is part of Tizonia
#
# Tizonia is free software: you can redistribute it and/or modify it under the
# terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
#
# Tizonia is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
# more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with Tizonia.  If not, see <http://www.gnu.org/licenses/>.

TESTS = check_mp3_encoder

check_PROGRAMS = check_mp3_encoder

check_mp3_encoder_SOURCES = check_mp3_encoder.c

check_mp3_encoder_CFLAGS = \
	-I$(top_srcdir)/../../include/omxil-1.2 \
	-I$(top_srcdir)/../../libtizplatform/src \
	-I$(top_srcdir)/../../libtizonia/src/ \
	@CHECK_CFLAGS@

check_mp3_encoder_LDADD = \
	$(top_builddir)/../../libtizplatform/src/libtizplatform.la \
	$(top_builddir)/../../libtizcore/src/libtizcore.la \
	$(top_builddir)/../../libtizonia/src/libtizonia.la \
	@CHECK_LIBS@


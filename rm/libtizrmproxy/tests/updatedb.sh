#!/bin/bash
#
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

SQLITE3=$(which sqlite3)
BASEDIR=$(dirname $0)
TIZ_RM_DB_NAME="../../tizrmd/data/tizrm.db"
COMMANDS=${BASEDIR}"/"$1

echo -e "Updating $TIZ_RM_DB_NAME using $COMMANDS..."
cat $COMMANDS
$SQLITE3 $TIZ_RM_DB_NAME < $COMMANDS
echo -e "Done."


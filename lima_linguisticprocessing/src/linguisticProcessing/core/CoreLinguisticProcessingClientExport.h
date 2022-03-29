/*
    Copyright 2004-2021 CEA LIST

    This file is part of LIMA.

    LIMA is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LIMA is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with LIMA.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef LIMA_CORELINGUISTICPROCESSINGCLIENTEXPORT_H
#define LIMA_CORELINGUISTICPROCESSINGCLIENTEXPORT_H

#include <common/LimaCommon.h>

#ifdef WIN32

#ifdef LIMA_CORELINGUISTICPROCESSINGCLIENT_EXPORTING
   #define LIMA_CORELINGUISTICPROCESSINGCLIENT_EXPORT    __declspec(dllexport)
#else
   #define LIMA_CORELINGUISTICPROCESSINGCLIENT_EXPORT    __declspec(dllimport)
#endif


#else // Not WIN32

#define LIMA_CORELINGUISTICPROCESSINGCLIENT_EXPORT

#endif

#endif

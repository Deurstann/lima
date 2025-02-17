# SPDX-FileCopyrightText: 2022 CEA LIST <gael.de-chalendar@cea.fr>
#
# SPDX-License-Identifier: MIT

# - Find LimaLinguisticdata

set(LIMA_VERSION_MAJOR "3")
set(LIMA_VERSION_MINOR "0")
set(LIMA_VERSION_RELEASE @LIMA_VERSION_RELEASE@)
set(LIMA_VERSION "${LIMA_VERSION_MAJOR}.${LIMA_VERSION_MINOR}.${LIMA_VERSION_RELEASE}")

set(LIMA_GENERIC_LIB_VERSION ${LIMA_VERSION})
set(LIMA_GENERIC_LIB_SOVERSION ${LIMA_VERSION_MAJOR})

message(STATUS "Lima version  ${LIMA_VERSION_MAJOR}.${LIMA_VERSION_MINOR}.${LIMA_VERSION_RELEASE} - LIMA_GENERIC_LIB_VERSION  ${LIMA_GENERIC_LIB_VERSION} ")

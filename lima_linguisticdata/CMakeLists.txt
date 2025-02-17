# Copyright 2002-2020 CEA LIST
# SPDX-FileCopyrightText: 2022 CEA LIST <gael.de-chalendar@cea.fr>
#
# SPDX-License-Identifier: MIT

project(LimaLinguisticData)

if (ENVPATH)
  set(ENV{PATH} "${ENVPATH}:$ENV{PATH}")
endif()
if (ENVPERL5LIB)
  set(ENV{PERL5LIB} "${ENVPERL5LIB}:$ENV{PERL5LIB}")
endif()
message("lima_linguisticdata PERL5LIB=$ENV{PERL5LIB}")
message("lima_linguisticdata PATH=$ENV{PATH}")

# message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
# set(CMAKE_VERBOSE_MAKEFILE on)

#cmake_policy
# if(POLICY CMP0046)
# cmake_policy(SET CMP0046 OLD)
# endif()

enable_testing()

include(Languages.cmake)
include(cmake/LinguisticData.cmake)
# include(${CMAKE_SOURCE_DIR}/../SystemSpecificInformations.cmake)
# for add_component macro (require 2.8.5)
include(CPackComponent)

configure_file(LIMALDATAConfig-src.cmake LIMALDATAConfig.cmake @ONLY)

LIMA_GENERIC_CONFIGENV(eng)
LIMA_GENERIC_CONFIGENV(fre)
LIMA_GENERIC_CONFIGENV(por)
LIMA_GENERIC_CONFIGENV(ud)
LIMA_GENERIC_CONFIGENV(ud-eng)

add_custom_target(pretest-execEnv)

add_subdirectory(cmake)
add_subdirectory(scratch)
add_subdirectory(analysisDictionary)
add_subdirectory(disambiguisationMatrices)
add_subdirectory(rules-idiom)
add_subdirectory(SpecificEntities)
add_subdirectory(syntacticAnalysis)
add_subdirectory(SRLIntegration)

LIMA_PRETEST_CONFIGENV(eng)
LIMA_PRETEST_CONFIGENV(fre)
LIMA_PRETEST_CONFIGENV(por)

########### install files ###############
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/LIMALDATAConfig.cmake DESTINATION share/apps/lima/cmake/Modules
        COMPONENT devel)


# Copyright 2002-2019 CEA LIST
# SPDX-FileCopyrightText: 2022 CEA LIST <gael.de-chalendar@cea.fr>
#
# SPDX-License-Identifier: MIT

include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src )

########### main target ###############

SET(lima-lp-specificentities_LIB_SRCS
   SpecificEntitiesXmlLogger.cpp
   SpecificEntitiesConstraints.cpp
   NormalizationUtils.cpp
   NormalizeDateTimeResources.cpp
   MicrosForNormalization.cpp
   SpecificEntitiesMicros.cpp
#   SpecificEntitiesLoader.cpp # Not used anywhere. Should it be deprecated?
   SpecificEntitiesCompletion.cpp
)

DECLARE_LIMA_PLUGIN(lima-lp-specificentities)

target_compile_definitions(lima-lp-specificentities PRIVATE "-DLIMA_SPECIFICENTITIES_EXPORTING")

target_link_libraries(lima-lp-specificentities
  lima-lp-linguisticprocessors
  lima-lp-linguisticresources
  lima-lp-annotationgraph
  lima-lp-automaton
  lima-lp-linguisticanalysisstructure
  lima-lp-syntacticanalysis
)

set_target_properties(lima-lp-specificentities PROPERTIES VERSION ${LIMA_LP_LIB_VERSION} SOVERSION ${LIMA_LP_LIB_SOVERSION})

install(TARGETS lima-lp-specificentities DESTINATION ${LIB_INSTALL_DIR}
  COMPONENT runtime)

########### specific targets for normalization actions ###############

SET(lima-lp-se-datetime_LIB_SRCS
  NormalizeDateTime.cpp
)

DECLARE_LIMA_PLUGIN(lima-lp-se-datetime)

target_compile_definitions(lima-lp-se-datetime PRIVATE "-DLIMA_SPECIFICENTITIES_DATETIME_EXPORTING")

target_link_libraries(lima-lp-se-datetime
  lima-lp-specificentities
)

set_target_properties(lima-lp-se-datetime PROPERTIES
    VERSION ${LIMA_LP_LIB_VERSION}
    SOVERSION ${LIMA_LP_LIB_SOVERSION})

install(TARGETS lima-lp-se-datetime DESTINATION ${LIB_INSTALL_DIR}
  COMPONENT runtime)

SET(lima-lp-se-number_LIB_SRCS
  NormalizeNumber.cpp
)

DECLARE_LIMA_PLUGIN(lima-lp-se-number)

target_compile_definitions(lima-lp-se-number PRIVATE "-DLIMA_SPECIFICENTITIES_NUMBER_EXPORTING")

target_link_libraries(lima-lp-se-number
  lima-lp-specificentities
)

set_target_properties(lima-lp-se-number PROPERTIES VERSION ${LIMA_LP_LIB_VERSION} SOVERSION ${LIMA_LP_LIB_SOVERSION})

install(TARGETS lima-lp-se-number DESTINATION ${LIB_INSTALL_DIR}
  COMPONENT runtime)

SET(lima-lp-se-person_LIB_SRCS
  NormalizePersonName.cpp
  NormalizePerson.cpp
)

DECLARE_LIMA_PLUGIN(lima-lp-se-person)

target_compile_definitions(lima-lp-se-person PRIVATE "-DLIMA_SPECIFICENTITIES_PERSON_EXPORTING")

target_link_libraries(lima-lp-se-person
  lima-lp-specificentities
)

set_target_properties(lima-lp-se-person PROPERTIES VERSION ${LIMA_LP_LIB_VERSION} SOVERSION ${LIMA_LP_LIB_SOVERSION})

install(TARGETS lima-lp-se-person DESTINATION ${LIB_INSTALL_DIR}
  COMPONENT runtime)

########### install files ###############
install(FILES
    NormalizeDateTimeResources.h
    NormalizationUtils.h
    NormalizeNumber.h
    SpecificEntitiesXmlLogger.h
    NormalizeDateTime.h
    SpecificEntitiesConstraints.h
    SpecificEntitiesRecognizer.h
    NormalizePersonName.h
    NormalizePerson.h
    #SpecificEntitiesLoader.h # Not used anywhere. Should it be deprecated?
    SpecificEntitiesCompletion.h
    SpecificEntitiesExport.h
  DESTINATION include/linguisticProcessing/core/SpecificEntities
  COMPONENT devel
)

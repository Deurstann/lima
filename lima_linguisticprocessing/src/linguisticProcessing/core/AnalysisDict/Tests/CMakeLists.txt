# Copyright 2002-2019 CEA LIST
# SPDX-FileCopyrightText: 2022 CEA LIST <gael.de-chalendar@cea.fr>
#
# SPDX-License-Identifier: MIT

SET(EnhancedAnalysisDictionaryTest0_SRCS
   EnhancedAnalysisDictionaryTest0.cpp
)

add_executable(EnhancedAnalysisDictionaryTest0 ${EnhancedAnalysisDictionaryTest0_SRCS})

target_link_libraries(EnhancedAnalysisDictionaryTest0)

add_test(NAME EnhancedAnalysisDictionaryTest0 COMMAND EnhancedAnalysisDictionaryTest0)

########### next test ###############

# generate EnhancedAnalysisDictionaryTest1.cpp if it does not exist
configure_file(EnhancedAnalysisDictionaryTest1-src.cpp EnhancedAnalysisDictionaryTest1.cpp @ONLY)

# generate EnhancedAnalysisDictionaryTest1.cpp when EnhancedAnalysisDictionaryTest1-src.cpp is modified
add_custom_command(OUTPUT EnhancedAnalysisDictionaryTest1.cpp
  COMMAND configure_file(${CMAKE_CURRENT_SOURCE_DIR}/EnhancedAnalysisDictionaryTest1-src.cpp EnhancedAnalysisDictionaryTest1.cpp @ONLY)
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/EnhancedAnalysisDictionaryTest1-src.cpp
  COMMENT "generate EnhancedAnalysisDictionaryTest1.cpp"
  VERBATIM)
add_custom_target(EnhancedAnalysisDictionaryTest1.cpp ALL
  DEPENDS EnhancedAnalysisDictionaryTest1-src.cpp)

configure_file(lima-lp-res-Enhanced.xml lima-lp-res-Enhanced.xml COPYONLY )

SET(EnhancedAnalysisDictionaryTest1_SRCS
   ${CMAKE_CURRENT_BINARY_DIR}/EnhancedAnalysisDictionaryTest1.cpp
)

add_executable(EnhancedAnalysisDictionaryTest1 ${EnhancedAnalysisDictionaryTest1_SRCS})

target_link_libraries(EnhancedAnalysisDictionaryTest1)

add_test(NAME EnhancedAnalysisDictionaryTest1 COMMAND EnhancedAnalysisDictionaryTest1)

########### next test ###############

# generate DictionaryDataTest1.cpp
configure_file(DictionaryDataTest1-src.cpp DictionaryDataTest1.cpp @ONLY)

#generate DictionaryDataTest1.cpp when DictionaryDataTest1-src.cpp is modified
add_custom_command(OUTPUT DictionaryDataTest1.cpp
  COMMAND configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DictionaryDataTest1-src.cpp DictionaryDataTest1.cpp @ONLY)
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/DictionaryDataTest1-src.cpp
  VERBATIM)
add_custom_target(DictionaryDataTest1.cpp ALL
  DEPENDS DictionaryDataTest1-src.cpp)

SET(DictionaryDataTest1_SRCS
   ${CMAKE_CURRENT_BINARY_DIR}/DictionaryDataTest1.cpp
)

add_executable(DictionaryDataTest1 ${DictionaryDataTest1_SRCS})

target_link_libraries(DictionaryDataTest1)

add_test(NAME DictionaryDataTest1 COMMAND DictionaryDataTest1)


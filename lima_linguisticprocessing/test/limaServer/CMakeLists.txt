# SPDX-FileCopyrightText: 2022 CEA LIST <gael.de-chalendar@cea.fr>
#
# SPDX-License-Identifier: MIT

########### next target ###############
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/test ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

SET(limaserver_SRCS
  LimaServer.cpp
  main.cpp
  analysisthread.cpp
)

add_executable(limaserver ${limaserver_SRCS})

target_link_libraries(limaserver
  lima-common
  lima-linguisticprocessing
  ${QHTTPSERVERCPP_LIBRARY}
  ${Qt_LIBRARIES}
  ${Boost_LIBRARIES}
)

install(TARGETS limaserver
  DESTINATION bin
  COMPONENT runtime)

install(PROGRAMS
  ${CMAKE_CURRENT_SOURCE_DIR}/analyzeText.sh
  DESTINATION bin
  COMPONENT runtime)

########### next target ###############

# SET(testQsLogger_SRCS
#    testQHttpServer.cpp
# )

# add_executable(testQsLogger ${testQsLogger_SRCS})

# target_link_libraries(testQHttpServer
#     lima-common
#     ${QHTTPSERVER_NAMES}
#     ${Qt_LIBRARIES}
#     ${Boost_LIBRARIES})

# install(TARGETS testQHttpServer DESTINATION bin)

########### next target ###############
# if (NOT QtDBus_FOUND)
#   message(WARNING "QtDbus not found: cannot build Lima Dbus server.")
# else ()
#
#   qt_generate_dbus_interface(
#     LimaDBusServer.h
#     fr.cea.limadbusserver.xml
#     OPTIONS -a -M
#   )
#
#   SET(limadbusserver_SRCS
#     LimaDBusServer.cpp
#     mainLimaDBusServer.cpp
#     AnalysisWrapper.cpp
#     ${CMAKE_CURRENT_BINARY_DIR}/fr.cea.limadbusserver.xml
#   )
#
#   qt_add_dbus_adaptor(limadbusserver_SRCS
#       ${CMAKE_CURRENT_BINARY_DIR}/fr.cea.limadbusserver.xml
#       LimaDBusServer.h LimaDBusServer)
#   # qt5_add_resources(limadbusserver limaserver.qrc)
#
#   add_executable(limadbusserver ${limadbusserver_SRCS})
#
#   target_link_libraries(limadbusserver
#     lima-common
#     lima-linguisticprocessing
#     ${Boost_LIBRARIES}
#     ${Qt_LIBRARIES}
#     ${QHTTPSERVERCPP_LIBRARY}
#   )
#
#   install(TARGETS limadbusserver
#     DESTINATION bin
#     COMPONENT runtime)
#
# endif()

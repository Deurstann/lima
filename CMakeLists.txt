# SPDX-FileCopyrightText: 2022 CEA LIST <gael.de-chalendar@cea.fr>
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)
project(Lima)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(WITH_GUI ON)
set(WITH_DEBUG_MESSAGES ON)
set(WITH_ARCH OFF)
set(WITH_ASAN OFF)
set(SHORTEN_POR_CORPUS_FOR_SVMLEARN ON)

enable_testing()
include(GNUInstallDirs)

include(${CMAKE_SOURCE_DIR}/SystemSpecificInformations.cmake)
message("System name is ${CMAKE_SYSTEM_NAME}")

message (STATUS "CMAKE_SYSTEM_PREFIX_PATH ${CMAKE_SYSTEM_PREFIX_PATH}")
# Find*.cmake will be searched in the following dirs if not found in system dirs
set(CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/cmake/Modules" # for those available in this project
    "$ENV{LIMA_EXTERNALS}/share/apps/cmake/Modules" # for those available in the externals dir
    "$ENV{LIMA_EXTERNALS}/share/cmake/Modules" # for those available in the externals dir
    "/usr/share/cmake/Modules" # for those available on the system
    "/usr/local/share/cmake/Modules" # for those available on the system
)

# find_path and find_library will search in these dirs too
set(CMAKE_PREFIX_PATH
    "${CMAKE_PREFIX_PATH}"
    "$ENV{LIMA_EXTERNALS}"
/Users/gael/Qt/5.15.2/clang_64/
)

set(ENV{CTEST_OUTPUT_ON_FAILURE} "1")
include(${CMAKE_SOURCE_DIR}/SetCompilerFlags.cmake)
set(ENV{PATH} "${CMAKE_SOURCE_DIR}/lima_pelf/evalPosTagging/SVM/SVMTool-1.3.1/bin:$ENV{PATH}")
set(ENV{PERL5LIB} "${CMAKE_SOURCE_DIR}/lima_pelf/evalPosTagging/SVM/SVMTool-1.3.1/lib:$ENV{PERL5LIB}")
message("PERL5LIB=$ENV{PERL5LIB}")
message("PATH=$ENV{PATH}")

if(APPLE)
    set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
elseif(UNIX)
  # Try to find architecture
  execute_process(COMMAND uname -m OUTPUT_VARIABLE CPACK_PACKAGE_ARCHITECTURE)
  string(STRIP "${CPACK_PACKAGE_ARCHITECTURE}" CPACK_PACKAGE_ARCHITECTURE)
  # Try to find distro name and distro-specific arch
  execute_process(COMMAND lsb_release -is OUTPUT_VARIABLE LSB_ID)
  execute_process(COMMAND lsb_release -rs OUTPUT_VARIABLE LSB_RELEASE)
  string(STRIP "${LSB_ID}" LSB_ID)
  string(STRIP "${LSB_RELEASE}" LSB_RELEASE)
  set(LSB_DISTRIB "${LSB_ID}${LSB_RELEASE}")
  if(NOT LSB_DISTRIB)
    set(LSB_DISTRIB "unix")
  endif(NOT LSB_DISTRIB)
  set(CPACK_SYSTEM_NAME "${LSB_DISTRIB}-${CPACK_PACKAGE_ARCHITECTURE}")
endif()

# assume built-in pthreads on MacOS
if(APPLE)
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(CMAKE_HAVE_THREADS_LIBRARY 1)
    set(CMAKE_USE_WIN32_THREADS_INIT 0)
    set(CMAKE_USE_PTHREADS_INIT 1)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

if(${CMAKE_HOST_SYSTEM_NAME} MATCHES Windows)
  if(CMAKE_CL_64)
    set(CPACK_SYSTEM_NAME win64)
  else(CMAKE_CL_64)
    set(CPACK_SYSTEM_NAME win32)
  endif(CMAKE_CL_64)
endif()

message("CPACK_SYSTEM_NAME is ${CPACK_SYSTEM_NAME}")

# Workaround for MSVC 2017 bug:
# See details in https://bugreports.qt.io/browse/QTBUG-72073
# ATTN: add_compile_definitions is available since cmake 3.12
if(WIN32)
add_compile_definitions(QT_NO_FLOAT16_OPERATORS)
endif()

# In Linux singletons don't work as expected in shared libraries.
# Multiple instances are created instead. "-rdynamic" allows to identify
# static template members between executable and shared library.
if(NOT WIN32)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
endif()

# use openmp
find_package(OpenMP)
if(OPENMP_FOUND)
  message("OpenMP FOUND")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
else (OPENMP_FOUND)
  message("OpenMP NOT FOUND")
endif()

find_package(TensorFlow)
if (TensorFlow_FOUND)
  find_package(Eigen)
  find_package(Protobuf)
endif()

if (TensorFlow_FOUND)
  message(STATUS "Found TensorFlow. ${TensorFlow_INCLUDE_DIRS} ${TensorFlow_LIBRARIES} ")
else ()
  message(STATUS "TensorFlow not found.")
endif ()

if (Eigen_FOUND)
  message(STATUS "Found Eigen. ${Eigen_INCLUDE_DIRS} ${Eigen_LIBRARIES} ")
else ()
  message(STATUS "Eigen not found.")
endif ()

if (Protobuf_FOUND)
  message(STATUS "Found Protobuf. ${Protobuf_INCLUDE_DIRS} ${Protobuf_LIBRARIES} ")
else ()
  message(STATUS "Protobuf not found.")
endif ()

if (TensorFlow_FOUND AND Eigen_FOUND)
  message(STATUS "Found TensorFlow. ${TensorFlow_INCLUDE_DIRS} ${TensorFlow_LIBRARIES} ")

  include_directories(SYSTEM ${TensorFlow_INCLUDE_DIRS} ${Eigen_INCLUDE_DIRS} ${Protobuf_INCLUDE_DIRS})
  if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    install(FILES ${TensorFlow_LIBRARIES} ${TensorFlow_LIBRARIES_DYN}
      ${Eigen_LIBRARIES}
          DESTINATION ${LIB_INSTALL_DIR}
          COMPONENT runtime)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
      ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
      ${Eigen_LIBRARIES}
      ${TensorFlow_LIBRARIES}
      ${TensorFlow_LIBRARIES_DYN})
  endif ()
else()
  message(STATUS "TensorFlow and/or Eigen modules NOT Found. TensorFlow based modules will no be built.")
endif()

include(${CMAKE_SOURCE_DIR}/manageQt.cmake)
addQtModules(Core Gui Qml Quick Test Xml Widgets )
message("\n\n\n{Qt_LIBRARIES}=${Qt_LIBRARIES} ")

# Search Qt optional components
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS DBus)
if (QtDBus_FOUND)
  addQtModules(DBus)
  get_target_property(QtDBus_location Qt${QT_VERSION_MAJOR}::DBus LOCATION)
endif()

if ("${QT_DBUSCPP2XML_EXECUTABLE}" STREQUAL "QT_DBUSCPP2XML_EXECUTABLE-NOTFOUND")
  message(WARNING "Dbus tools not found ${QT_DBUSCPP2XML_EXECUTABLE}")
else ()
endif()


add_definitions( -DBoost_DEBUG=ON )
add_definitions( -DBOOST_ALL_NO_LIB )
add_definitions( -DBOOST_ALL_DYN_LINK )
set( Boost_USE_STATIC_LIBS OFF )
set( Boost_USE_MULTITHREADED ON )
set( Boost_USE_STATIC_RUNTIME OFF )
set( BOOST_LIB_DIAGNOSTIC )
find_package(Boost 1.58 REQUIRED
  COMPONENTS regex filesystem program_options system thread unit_test_framework)


message("Boost_INCLUDE_DIR= ${Boost_INCLUDE_DIR}")
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set (Boost_SYSTEM_LIBRARY ${Boost_SYSTEM_LIBRARY_DEBUG})
    string(REPLACE ".lib" ".dll" Boost_SYSTEM_LIBRARY_dll ${Boost_SYSTEM_LIBRARY_DEBUG})
    string(REPLACE ".lib" ".dll" Boost_UNIT_TEST_FRAMEWORK_LIBRARY_dll ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY_DEBUG})
    string(REPLACE ".lib" ".dll" Boost_REGEX_LIBRARY_dll ${Boost_REGEX_LIBRARY_DEBUG})
    string(REPLACE ".lib" ".dll" Boost_FILESYSTEM_LIBRARY_dll ${Boost_FILESYSTEM_LIBRARY_DEBUG})
    string(REPLACE ".lib" ".dll" Boost_PROGRAM_OPTIONS_LIBRARY_dll ${Boost_PROGRAM_OPTIONS_LIBRARY_DEBUG})
    string(REPLACE ".lib" ".dll" Boost_THREAD_LIBRARY_dll ${Boost_THREAD_LIBRARY_DEBUG})
  else()
    set (Boost_SYSTEM_LIBRARY ${Boost_SYSTEM_LIBRARY_RELEASE})
    string(REPLACE ".lib" ".dll" Boost_SYSTEM_LIBRARY_dll ${Boost_SYSTEM_LIBRARY_RELEASE})
    string(REPLACE ".lib" ".dll" Boost_UNIT_TEST_FRAMEWORK_LIBRARY_dll ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY_RELEASE})
    string(REPLACE ".lib" ".dll" Boost_REGEX_LIBRARY_dll ${Boost_REGEX_LIBRARY_RELEASE})
    string(REPLACE ".lib" ".dll" Boost_FILESYSTEM_LIBRARY_dll ${Boost_FILESYSTEM_LIBRARY_RELEASE})
    string(REPLACE ".lib" ".dll" Boost_PROGRAM_OPTIONS_LIBRARY_dll ${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE})
    string(REPLACE ".lib" ".dll" Boost_THREAD_LIBRARY_dll ${Boost_THREAD_LIBRARY_RELEASE})
  endif()
  install(FILES ${Boost_SYSTEM_LIBRARY_dll}
                ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY_dll}
                ${Boost_REGEX_LIBRARY_dll}
                ${Boost_FILESYSTEM_LIBRARY_dll}
                ${Boost_PROGRAM_OPTIONS_LIBRARY_dll}
                ${Boost_THREAD_LIBRARY_dll}
          DESTINATION ${LIB_INSTALL_DIR}
          COMPONENT runtime)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
      ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
      ${Boost_REGEX_LIBRARY_dll}
      ${Boost_FILESYSTEM_LIBRARY_dll}
      ${Boost_PROGRAM_OPTIONS_LIBRARY_dll}
      ${Boost_THREAD_LIBRARY_dll})
endif ()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/execEnv/config/plugins)
file(WRITE ${CMAKE_BINARY_DIR}/execEnv/config/plugins/${CMAKE_PROJECT_NAME} "")

macro (ADD_DLL_TO_PATH _lib)
    message("ADD_DLL_TO_PATH ${_lib}")
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        set(ADD_DLL_TO_PATH_LIB_NAME lib${_lib}.so)
    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        set(ADD_DLL_TO_PATH_LIB_NAME lib${_lib}.dylib)
    elseif (WIN32)
      set(ADD_DLL_TO_PATH_LIB_NAME ${_lib}.dll)
    endif()
    message("Will copy $<TARGET_FILE_DIR:${_lib}>/${ADD_DLL_TO_PATH_LIB_NAME} to ${CMAKE_BINARY_DIR}/execEnv/lib/")
    add_custom_command(
      OUTPUT ${CMAKE_BINARY_DIR}/execEnv/lib/${ADD_DLL_TO_PATH_LIB_NAME}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/execEnv/lib
      COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE_DIR:${_lib}>/${ADD_DLL_TO_PATH_LIB_NAME}
        ${CMAKE_BINARY_DIR}/execEnv/lib/${ADD_DLL_TO_PATH_LIB_NAME}
      DEPENDS
        ${_lib}
      COMMENT "Copy $<TARGET_FILE_DIR:${_lib}>/${ADD_DLL_TO_PATH_LIB_NAME} to ${CMAKE_BINARY_DIR}/execEnv/lib/${ADD_DLL_TO_PATH_LIB_NAME}"
      VERBATIM
    )
    add_custom_target(
      ${_lib}_dll
      ALL
      DEPENDS ${CMAKE_BINARY_DIR}/execEnv/lib/${ADD_DLL_TO_PATH_LIB_NAME}
    )
    add_dependencies(rules-configEnv ${_lib}_dll)
endmacro()

macro (DECLARE_LIMA_PLUGIN _plugin)
  file (APPEND ${CMAKE_BINARY_DIR}/execEnv/config/plugins/${CMAKE_PROJECT_NAME} "${_plugin}\n")
  add_library(${_plugin} SHARED ${${_plugin}_LIB_SRCS} ${${_plugin}_MOC_OUTFILES})
  ADD_DLL_TO_PATH(${_plugin})
endmacro ()

SET(optionalLibs)

if (DEFINED ENV{PYTHON_VERSION})
  set(Python_ADDITIONAL_VERSIONS $ENV{PYTHON_VERSION})
else ()
  set(Python_ADDITIONAL_VERSIONS 3.5 3.6 3.7)
endif ()

find_package(PythonLibs 3)
if (${PYTHONLIBS_FOUND})
  include_directories(${PYTHON_INCLUDE_DIRS})
  if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(PYTHON_DLLS "")
    foreach(PYTHON_LIB ${PYTHON_LIBRARIES})
      message("Python lib: ${PYTHON_LIB}")
      if (NOT EXISTS ${PYTHON_LIB})
        continue()
      endif()
#      string(REPLACE "/" "\\" PYTHON_LIB ${PYTHON_LIB})
#      get_filename_component(PYTHON_LIB_FILE_NAME, ${PYTHON_LIB}, NAME)
#      it looks like get_Filename_component doesn't work (cmake 3.13.3 - 3.13.4)
      string(REGEX MATCH "[^/\\]+$" PYTHON_LIB_FILE_NAME ${PYTHON_LIB})
      string(LENGTH "${PYTHON_LIB_FILE_NAME}" PYTHON_LIB_FILE_NAME_LEN)
#      message("SRC: ${PYTHON_LIB} FN: ${PYTHON_LIB_FILE_NAME} LEN: ${PYTHON_LIB_FILE_NAME_LEN}")
      if ("${PYTHON_LIB_FILE_NAME_LEN}" LESS 6)
        continue()
      endif()
      message("Python lib name: ${PYTHON_LIB_FILE_NAME}")
      string(SUBSTRING ${PYTHON_LIB_FILE_NAME} 0, 6, FIRST_SIX_CHARS_OF_LIB_NAME)
#      message("First six chars: ${FIRST_SIX_CHARS_OF_LIB_NAME}")
      if (NOT ${FIRST_SIX_CHARS_OF_LIB_NAME} STREQUAL "python")
         continue()
      endif()
      string(REPLACE ".lib" ".dll" PYTHON_DLL ${PYTHON_LIB})
      string(REPLACE "libs" "" PYTHON_DLL ${PYTHON_DLL})
      message("Python dll: ${PYTHON_DLL} / Python lib: ${PYTHON_LIB}")
      if(EXISTS ${PYTHON_DLL})
        string(REPLACE "\\" "\\\\" ESCAPED_PATH_TO_PYTHON_DLL ${PYTHON_DLL})
        list(APPEND PYTHON_DLLS ${ESCAPED_PATH_TO_PYTHON_DLL})
        break()
      else()
        message(FATAL_ERROR "Could not find Python ( ${PYTHON_DLL} / ${PYTHON_LIB} )")
      endif()
    endforeach()

    message("PYTHON_DLLS = ${PYTHON_DLLS} / PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")

    string(LENGTH "${PYTHON_DLLS}" PYTHON_DLLS_LEN)
    if (${PYTHON_DLLS_LEN} LESS 1)
      message(FATAL_ERROR "Could not find Python")
    endif()

    install(FILES ${PYTHON_DLLS}
            DESTINATION ${LIB_INSTALL_DIR}
            COMPONENT runtime)

    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
        ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
        ${PYTHON_DLLS})

    install(FILES ${PYTHON_LIB}
            DESTINATION ${LIB_INSTALL_DIR}
            COMPONENT runtime)

    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
        ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
        ${PYTHON_LIB})

  endif ()
endif ()

find_package(SVMTOOL++)
if(SVMTOOL++_FOUND)
  set(optionalLibs ${optionalLibs} ${SVMTOOL++_NAMES})
  include_directories(${SVMTOOL++CPP_INCLUDE_DIR})
  link_directories(${SVMTOOL++CPP_LIBRARY_DIRS})
  message(STATUS "Hi, Found SVMTool++ ${SVMTOOL++CPP_INCLUDE_DIR}.")
  if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    install(FILES ${SVMTOOL++_NAMES}
          DESTINATION ${LIB_INSTALL_DIR}
          COMPONENT runtime)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
      ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
      ${SVMTOOL++_NAMES})
  endif()
else ()
  message(STATUS "Hi, Could not find SVMTool++. SVM-base PoS tagger will not be available.")
endif()

find_package (Enchant)
if (ENCHANT_FOUND)
  set(optionalLibs ${optionalLibs} ${ENCHANT_LIBRARIES})
  include_directories(${ENCHANT_INCLUDE_DIRS})
  if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    install(FILES ${ENCHANT_LIBRARIES}
          DESTINATION ${LIB_INSTALL_DIR}
          COMPONENT runtime)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
      ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
      ${ENCHANT_LIBRARIES})
  endif ()

else ()
  message(STATUS "Could not find Enchant ${ENCHANT_INCLUDE_DIRS}. Spelling correction will not be built.")
endif ()

find_package (Tre)
if (TRE_FOUND)
  set(optionalLibs ${optionalLibs} ${TRECPP_LIBRARIES})
  include_directories(${TRECPP_INCLUDE_DIRS})
  message(STATUS "Found tre library. Module ApproxStringMatcher will be built")
  if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    install(FILES ${TRECPP_LIBRARIES}
          DESTINATION ${LIB_INSTALL_DIR}
          COMPONENT runtime)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
      ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
      ${TRECPP_LIBRARIES})
  endif ()
else (TRECPP_FOUND)
  message(STATUS "Could not find tre ${TRECPP_INCLUDE_DIRS}. ApproxStringMatcher module will not be built.")
endif (TRE_FOUND)

# QHttpServer is necessary for limaserver HTTP server
find_package(QHttpServer QUIET)
if (NOT QHTTPSERVER_FOUND)
  message(STATUS "QHttpServer Not found. Lima HTTP server will NOT be built")
else ()
  message(STATUS "Found QHttpServer. lima HTTP server will be built with ${QHTTPSERVERCPP_INCLUDE_DIR} and ${QHTTPSERVERCPP_LIBRARY}")
  include_directories(${QHTTPSERVERCPP_INCLUDE_DIR})
  if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    string(REPLACE ".lib" ".dll" QHTTPSERVERCPP_LIBRARY_DLL ${QHTTPSERVERCPP_LIBRARY})
    install(FILES ${QHTTPSERVERCPP_LIBRARY} ${QHTTPSERVERCPP_LIBRARY_DLL}
          DESTINATION ${LIB_INSTALL_DIR}
          COMPONENT runtime)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
      ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
      ${QHTTPSERVERCPP_LIBRARY})
  endif ()
endif ()

message(STATUS "TF_SOURCES_PATH = ${TF_SOURCES_PATH}")

  # Here we are looking up for a system-wide installation of Eigen
message("Looking up for Eigen3")
find_package(Eigen3 3.3 NO_MODULE)

message("EIGEN_FOUND: ${EIGEN_FOUND}")
message("EIGEN3_FOUND: ${EIGEN3_FOUND}")
if(WIN32 AND EIGEN3_FOUND)
include_directories("${EIGEN3_INCLUDE_DIR}/..")
message("Eigen3 include: ${EIGEN3_INCLUDE_DIR}/..")
message("Eigen3 root: ${EIGEN3_USE_FILE}")
endif()

find_package(Torch)
find_package(ICU 60.0 COMPONENTS uc)
message("ICU_INCLUDE_DIR= ${ICU_INCLUDE_DIR}")
include_directories(${ICU_INCLUDE_DIR})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  install(DIRECTORY  ${ICU_INCLUDE_DIR}/../bin64/
          DESTINATION ${LIB_INSTALL_DIR}
          COMPONENT runtime
          FILES_MATCHING PATTERN "icu*.dll")
  FILE(GLOB ICU_LIBRARIES "${ICU_INCLUDE_DIR}/../bin64/icu*.dll")
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
      ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
      ${ICU_LIBRARIES})
      message("icu: ${ICU_LIBRARIES}")
endif()

include_directories(${CMAKE_SOURCE_DIR}/extern)

# Set the default component name before any call to install
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "runtime")

# Allow to set LIMA_VERSION variables necessary to setup packages names
configure_file(lima_common/LIMACOMMONConfig-src.cmake LIMACOMMONConfig.cmake @ONLY)
find_package(LIMACOMMON PATHS ${CMAKE_CURRENT_BINARY_DIR} REQUIRED)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/LIMACOMMONConfig.cmake
  DESTINATION share/apps/lima/cmake/Modules
  COMPONENT devel)

set(LIMA_LP_LIB_VERSION ${LIMA_GENERIC_LIB_VERSION})
set(LIMA_LP_LIB_SOVERSION ${LIMA_GENERIC_LIB_SOVERSION})


set(CPACK_PACKAGE_VENDOR "CEA LIST")
set(CPACK_PACKAGE_CONTACT "Gaël de Chalendar <Gael.de-Chalendar@cea.fr>")
SET(CPACK_PACKAGE_VERSION_MAJOR "${LIMA_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${LIMA_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${LIMA_VERSION_RELEASE}")
SET(CPACK_PACKAGE_VERSION "${LIMA_VERSION_MAJOR}.${LIMA_VERSION_MINOR}.${LIMA_VERSION_RELEASE}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Lima-${LIMA_VERSION_MAJOR}.${LIMA_VERSION_MINOR}")

include(${CMAKE_SOURCE_DIR}/lima_linguisticdata/cmake/LinguisticData.cmake)

set(ENV{PATH} "$ENV{PATH};${CMAKE_BINARY_DIR}/execEnv/lib")

SPECIFICENTITIES_GENERIC_CONFIGENV()

add_subdirectory(cmake)

include_directories(
  ${CMAKE_BINARY_DIR}/lima_common
  ${CMAKE_BINARY_DIR}/lima_common/src
  lima_common
  lima_common/src
)
add_subdirectory(lima_common)

include_directories(
  ${CMAKE_BINARY_DIR}/lima_linguisticprocessing
  ${CMAKE_BINARY_DIR}/lima_linguisticprocessing/src
  ${CMAKE_BINARY_DIR}/lima_linguisticprocessing/src/linguisticProcessing
  lima_linguisticprocessing
  lima_linguisticprocessing/src
  lima_linguisticprocessing/src/linguisticProcessing
)
add_subdirectory(lima_linguisticprocessing)

# if ("x${LIMA_RESOURCES}" STREQUAL "xbuild")
  add_subdirectory(lima_linguisticdata)
# endif()

add_subdirectory(lima_pelf)

# add_subdirectory(lima_annoqt)

if (WITH_GUI)
  if (Qt${QT_VERSION_MAJOR}Core_FOUND)
    if (Qt${QT_VERSION_MAJOR}Core_VERSION VERSION_GREATER 5.5.0)
      message("Build with lima_gui")
      add_subdirectory(lima_gui)
    else()
      message("Minimum supported Qt version for lima_gui is 5.5.0. You have Qt ${QtCore_VERSION}. lima_gui will no be built.")
    endif()
  else()
    message("QtCore was not found. Build without lima_gui.")
  endif()
else()
  message("Build without lima_gui")
endif()

add_subdirectory(extern)

if (TORCH_FOUND AND EIGEN3_FOUND)
  message("Torch and Eigen found. deeplima will be built.")
  message("TORCH_CXX_FLAGS = ${TORCH_CXX_FLAGS}")
  message("TORCH_LIBRARIES = ${TORCH_LIBRARIES}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
  add_subdirectory(deeplima)
endif()

########### documentation ###############

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(doc
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)


########### packaging ###############



message("CPACK_INSTALL_CMAKE_PROJECTS=${CPACK_INSTALL_CMAKE_PROJECTS}")

# Specify additional runtime libraries that may not be detected. After
# inclusion any detected libraries will be appended to this.
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
  ${QtCore_location}
  ${QtXml_location}
  ${QtTest_location}
  ${Boost_SYSTEM_LIBRARY_dll}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY_dll})

set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT "runtime")
set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
set(MAKE_INSTALL_UCRT_LIBRARIES TRUE)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
endif()
include(InstallRequiredSystemLibraries)

if (TensorFlow_FOUND AND Eigen_FOUND)
  set(CPACK_PACKAGE_NAME "lima-tf")
else()
  set(CPACK_PACKAGE_NAME "lima")
endif()
message("Lima package name will be ${CPACK_PACKAGE_NAME}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "LIMA Linguistic Analyzer")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")

# set of commands to define component based packaging
# notify cpack of component all names
SET(CPACK_COMPONENTS_ALL devel runtime common fre eng por ud)
set(CPACK_RPM_COMPONENT_INSTALL ON)
SET(CPACK_RPM_USE_DISPLAY_NAME_IN_FILENAME ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)
SET(CPACK_DEBIAN_USE_DISPLAY_NAME_IN_FILENAME ON)

# Cpack Bundle Mac OS X installer commands
# See https://cmake.org/cmake/help/latest/cpack_gen/bundle.html for more commands
# Particularly those for signing the generated package


#CPACK_BUNDLE_NAME
#    The name of the generated bundle. This appears in the macOS Finder as the bundle name. Required.
set(CPACK_BUNDLE_NAME ${CPACK_PACKAGE_FILE_NAME})

#CPACK_BUNDLE_ICON
#    Path to an macOS icon file that will be used as the icon for the generated bundle. This is the icon that
#    appears in the macOS Finder for the bundle, and in the macOS dock when the bundle is opened. Required.
set(CPACK_BUNDLE_ICON "${CMAKE_SOURCE_DIR}/lima.icns")

install(PROGRAMS "${CMAKE_SOURCE_DIR}/lima-bundle-startup.sh"
    DESTINATION bin
    COMPONENT runtime)

#set(CPACK_BUNDLE_STARTUP_COMMAND "/Volumes/lima-2.1.-Darwin/lima-2.1.-Darwin.app/Contents/MacOS/lima-2.1.-Darwin")
set(CPACK_BUNDLE_STARTUP_COMMAND "${CMAKE_SOURCE_DIR}/lima-bundle-startup.sh")


set(ICON_FILE_NAME "${CPACK_BUNDLE_NAME}")

#CFBundleGetInfoString
set(MACOSX_BUNDLE_INFO_STRING "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} Version ${${PROJECT_NAME}_VERSION}, Copyright ${CPACK_PACKAGE_VENDOR}")
set(MACOSX_BUNDLE_ICON_FILE ${ICON_FILE_NAME})
set(MACOSX_BUNDLE_GUI_IDENTIFIER "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
#CFBundleLongVersionString
set(MACOSX_BUNDLE_LONG_VERSION_STRING "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} Version ${${PROJECT_NAME}_VERSION}")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${${PROJECT_NAME}_VERSION})

#CPACK_BUNDLE_PLIST
#    Path to an macOS Property List [.plist] file that will be used for the generated bundle. This assumes
#    that the caller has generated or specified their own Info.plist file. Required.
set(CONFIGURED_PLIST_PATH ${CMAKE_BINARY_DIR}/packaging/Info.plist)

configure_file(${CMAKE_SOURCE_DIR}/lima.plist ${CONFIGURED_PLIST_PATH} @ONLY)
set(CPACK_BUNDLE_PLIST ${CONFIGURED_PLIST_PATH})



# Cpack NSIS Windows installer commands

SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
SET(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
SET(CPACK_NSIS_MODIFY_PATH "ON")
SET(CPACK_STRIP_FILES "")
SET(CPACK_SOURCE_STRIP_FILES "")
if ("x$ENV{LIMA_DISABLE_CPACK_DEBIAN_PACKAGE_SHLIBDEPS}" STREQUAL "x")
  set(CPACK_DEBIAN_runtime_PACKAGE_SHLIBDEPS ON)
endif()
set(CPACK_GENERATOR ${SPECIFIC_SYSTEM_PREFERED_CPACK_GENERATOR})
message("Cpack generator is ${CPACK_GENERATOR}")

list(APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "!define env_hklm 'HKLM \\\"SYSTEM\\\\CurrentControlSet\\\\Control\\\\Session Manager\\\\Environment\\\"'")
list(APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "WriteRegExpandStr \\\${env_hklm} LIMA_DIST $INSTDIR")
list(APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "WriteRegExpandStr \\\${env_hklm} LIMA_CONF $INSTDIR\\\\share\\\\config\\\\lima")
list(APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "WriteRegExpandStr \\\${env_hklm} LIMA_RESOURCES $INSTDIR\\\\share\\\\apps\\\\lima\\\\resources")
list(APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "SendMessage \\\${HWND_BROADCAST} \\\${WM_WININICHANGE} 0 \\\"STR:Environment\\\" /TIMEOUT=5000")
string (REPLACE ";" "\n" CPACK_NSIS_EXTRA_INSTALL_COMMANDS "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}")

list(APPEND CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "DeleteRegValue \\\${env_hklm} LIMA_DIST")
list(APPEND CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "DeleteRegValue \\\${env_hklm} LIMA_CONF")
list(APPEND CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "DeleteRegValue \\\${env_hklm} LIMA_RESOURCES")
list(APPEND CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "SendMessage \\\${HWND_BROADCAST} \\\${WM_WININICHANGE} 0 \\\"STR:Environment\\\" /TIMEOUT=5000")
string (REPLACE ";" "\n" CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS}")

include(CPack)


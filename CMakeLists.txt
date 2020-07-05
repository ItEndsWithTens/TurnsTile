### Basic info ###

cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)

# For simplicity of testing, spit everything out into the same directories,
# thus allowing only one set of binaries (the current build) to exist at once.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/artifacts/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/artifacts/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/artifacts/build/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/artifacts/build/bin)

project(TurnsTile CXX)

set(TURNSTILE_MAJOR 0)
set(TURNSTILE_MINOR 3)
set(TURNSTILE_PATCH 2)
set(TURNSTILE_VERSION ${TURNSTILE_MAJOR}.${TURNSTILE_MINOR}.${TURNSTILE_PATCH})
set(TURNSTILE_AUTHOR "Rob Martens")
set(TURNSTILE_AUTHOR_EMAIL "<robert.martens@gmail.com>")

if(CMAKE_GENERATOR_PLATFORM STREQUAL "" OR
   CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
  set(TURNSTILE_ARCH "x64")
elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
  set(TURNSTILE_ARCH "x86")
endif()
set(TURNSTILE_HOST "AviSynth+ ${TURNSTILE_ARCH}")

set(TURNSTILE_DESCRIPTION "TurnsTile plugin for ${TURNSTILE_HOST}")



### Find AviSynth ###

# Check current user's plugins directory first, if it exists...
get_filename_component(
    HOST_PLUGIN_DIR
    [HKEY_CURRENT_USER\\SOFTWARE\\AviSynth;plugindir+] ABSOLUTE
)

# ...otherwise, use the local machine's folder.
if(${HOST_PLUGIN_DIR} STREQUAL "/registry")
  get_filename_component(
      HOST_PLUGIN_DIR
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\AviSynth;plugindir+] ABSOLUTE
  )
endif()



### Source files ###

set(AVISYNTHPLUS_INCLUDE_DIR ${HOST_PLUGIN_DIR}/../FilterSDK/include)

include_directories(${AVISYNTHPLUS_INCLUDE_DIR})

# Add the AviSynth+ header to the project for easy reference.
set(SRCS
  ${AVISYNTHPLUS_INCLUDE_DIR}/avisynth.h
)

list(APPEND SRCS
  src/interface.h
  src/TurnsTile.h
  src/TurnsTileBmpSource.h
  src/CLUTer.h
  src/interface.cpp
  src/TurnsTile.cpp
  src/TurnsTileBmpSource.cpp
  src/CLUTer.cpp
)


configure_file(
    src/TurnsTile.rc.in ${CMAKE_SOURCE_DIR}/src/TurnsTile.rc
)
list(APPEND SRCS src/TurnsTile.rc)
if(MSVC)
  # CMake handles the Source and Header Files filters automatically, but not
  # Resource Files, the way VS does. This is just cosmetic, really.
  source_group("Resource Files" FILES src/TurnsTile.rc)
endif()

string(TOUPPER ${TURNSTILE_HOST} TURNSTILE_HOST_DEFINE)
string(REPLACE "+" "PLUS" TURNSTILE_HOST_DEFINE ${TURNSTILE_HOST_DEFINE})
string(REPLACE "." "" TURNSTILE_HOST_DEFINE ${TURNSTILE_HOST_DEFINE})
string(REPLACE " " "_" TURNSTILE_HOST_DEFINE ${TURNSTILE_HOST_DEFINE})

add_library(${PROJECT_NAME} MODULE ${SRCS})
set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
  COMPILE_DEFINITIONS "TURNSTILE_HOST_${TURNSTILE_HOST_DEFINE}"
)



### CMake options ###
if(MSVC)

  # Dynamic linking is better from a security point of view, but static linking
  # is somewhat more convenient. The option is here should one feel the urge.
  option(
    TURNSTILE_STATIC_CRT "Link statically with the C Run-Time libraries." FALSE
  )

  if(TURNSTILE_STATIC_CRT)

    foreach(flag_var
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endforeach()

  else()

    foreach(flag_var
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      string(REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
    endforeach()

  endif()

endif()



### Testing ###

option(TURNSTILE_TESTS "Enable testing with Catch framework." TRUE)
if(TURNSTILE_TESTS)

  set(SRCS_TEST
    test/include/catch/catch.hpp

    test/include/md5/md5.h
    test/include/md5/md5.c

    ${TURNSTILE_HOST_HDR}
    test/src/avsavx/errors.cpp
    test/src/avsavx/main.cpp
    test/src/avsavx/output.cpp
    test/src/avsavx/util_avsavx.h
    test/src/avsavx/util_avsavx.cpp
    test/src/util_common.h
    test/src/util_common.cpp
  )

  set_source_files_properties(test/include/md5/md5.c PROPERTIES LANGUAGE CXX)

  set(
    TURNSTILE_TEST_DESCRIPTION
    "Test executable for the ${TURNSTILE_DESCRIPTION}"
  )

  configure_file(
    test/src/TurnsTile-test.rc.in
    ${CMAKE_SOURCE_DIR}/test/src/TurnsTile-test.rc
  )
  list(APPEND SRCS_TEST test/src/TurnsTile-test.rc)
  if(MSVC)
    source_group("Resource Files" FILES test/src/TurnsTile-test.rc)
  endif()

  add_executable(${PROJECT_NAME}-test ${SRCS_TEST})

  set_target_properties(
    ${PROJECT_NAME}-test
    PROPERTIES
    COMPILE_DEFINITIONS "TURNSTILE_HOST_${TURNSTILE_HOST_DEFINE}"
  )

endif()

### Packaging options ###

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)

set(CPACK_SOURCE_IGNORE_FILES
  # Directories
  "\\\\.git"
  "artifacts"
  "build"
  "scratch"
  # Project files
  "gitignore$"
  "\\\\.db$"
  "\\\\.kdev"
  "\\\\.ffindex$"
  # Source
  "\\\\.rc$"
  "~$"
  # Packages
  "zip$"
)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/artifacts/install CACHE INTERNAL "")

install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION .
  LIBRARY DESTINATION .
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)

install(
  FILES
  README.txt
  DESTINATION .
  RENAME ${PROJECT_NAME}.txt
)

# The binary package can be built with the PACKAGE target in the Visual Studio
# solution, either in the IDE or on the command line. The source package, on the
# other hand, must be built manually. Change to artifacts/pack and run
#
#   cpack --config ..\..\build\CPackSourceConfig.cmake
#
# replacing the path if necessary for your particular circumstances.
set(CPACK_SOURCE_GENERATOR "ZIP")
set(TURNSTILE_HOST_ABBR "AVS+")
set(CPACK_GENERATOR "ZIP")
set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_SOURCE_DIR}/artifacts/pack)

set(CPACK_PACKAGE_VERSION_MAJOR ${TURNSTILE_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${TURNSTILE_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${TURNSTILE_PATCH})

set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${TURNSTILE_VERSION}-${TURNSTILE_HOST_ABBR}${TURNSTILE_HOST_VERSION}-${CMAKE_SYSTEM_NAME}-${TURNSTILE_ARCH})

include(CPack)

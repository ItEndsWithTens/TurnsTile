### Basic info ###

# A built-in abstraction to set MSVC runtime linking flags was added in 3.15.
cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED ON)

# For simplicity of testing, spit everything out into the same directories,
# thus allowing only one set of binaries (the current build) to exist at once.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/artifacts/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/artifacts/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/artifacts/build/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/artifacts/build/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/artifacts/build/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/artifacts/build/bin)

project("TurnsTile" CXX)

set(TURNSTILE_MAJOR 1)
set(TURNSTILE_MINOR 0)
set(TURNSTILE_PATCH 0)
set(TURNSTILE_VERSION ${TURNSTILE_MAJOR}.${TURNSTILE_MINOR}.${TURNSTILE_PATCH})
set(TURNSTILE_AUTHOR "Rob Martens")
set(TURNSTILE_AUTHOR_EMAIL "<robert.martens@gmail.com>")

string(TIMESTAMP TURNSTILE_YEAR "%Y")

string(TOLOWER "${CMAKE_GENERATOR_PLATFORM}" CMAKE_GENERATOR_PLATFORM_LOWER)
if(CMAKE_GENERATOR_PLATFORM_LOWER STREQUAL "" OR
   CMAKE_GENERATOR_PLATFORM_LOWER STREQUAL "x64")
  set(TURNSTILE_ARCH "x64")
elseif(CMAKE_GENERATOR_PLATFORM_LOWER STREQUAL "win32")
  set(TURNSTILE_ARCH "x86")
endif()
set(TURNSTILE_HOST "Avisynth+ ${TURNSTILE_ARCH}")

set(TURNSTILE_DESCRIPTION "TurnsTile plugin for ${TURNSTILE_HOST}")



### CMake options ###
if(MSVC)

  # Dynamic linking is better from a security point of view, but static linking
  # is somewhat more convenient. The option is here should one feel the urge.
  option(
    TURNSTILE_STATIC_CRT "Link statically with the C Run-Time libraries." FALSE
  )

  # First-class support for static linking was introduced in CMake 3.15; the
  # variable set here applies to all subsequently defined targets.
  if(TURNSTILE_STATIC_CRT)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  endif()

endif()



### Find Avisynth header ###

set(AVISYNTHPLUS_HDR_DOCSTRING "The copy of avisynth.h TurnsTile should compile with.")

if(WIN32)
  # Check current user's plugins directory first, if it exists...
  get_filename_component(
      HOST_PLUGIN_DIR
      [HKEY_CURRENT_USER\\SOFTWARE\\AviSynth;plugindir+] ABSOLUTE
  )

  # ...otherwise, use the local machine's folder...
  if(${HOST_PLUGIN_DIR} STREQUAL "/registry")
    get_filename_component(
        HOST_PLUGIN_DIR
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\AviSynth;plugindir+] ABSOLUTE
    )
  endif()

  # ...but fall back to the 2.5 directories if all else fails.
  if(${HOST_PLUGIN_DIR} STREQUAL "/registry")
    get_filename_component(
        HOST_PLUGIN_DIR
        [HKEY_CURRENT_USER\\SOFTWARE\\AviSynth;plugindir2_5] ABSOLUTE
    )
  endif()

  if(${HOST_PLUGIN_DIR} STREQUAL "/registry")
    get_filename_component(
        HOST_PLUGIN_DIR
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\AviSynth;plugindir2_5] ABSOLUTE
    )
  endif()
else()
  set(AVISYNTHPLUS_HDR "/usr/local/include/avisynth/avisynth.h" CACHE FILEPATH "The copy of avisynth.h TurnsTile should compile with.")
endif()

# If the header location is undefined, create a baseline to start with.
if(NOT DEFINED AVISYNTHPLUS_HDR)
  set(AVISYNTHPLUS_HDR "" CACHE FILEPATH ${AVISYNTHPLUS_HDR_DOCSTRING})
endif()

# If the header option is blank, it's either the baseline empty string or a
# user's choice to clear a previously set value, so set a sensible default,
# cleaning up directory traversal dots and symlinks in the process.
if(AVISYNTHPLUS_HDR STREQUAL "")
  get_filename_component(AVISYNTHPLUS_HDR_REALPATH ${HOST_PLUGIN_DIR}/../FilterSDK/include/avisynth.h REALPATH BASE_DIR)
else()
  get_filename_component(AVISYNTHPLUS_HDR_REALPATH ${AVISYNTHPLUS_HDR} REALPATH BASE_DIR)
endif()

# Now that the path has been cleaned up, overwrite any existing header path with
# a friendlier version. One of the rare occasions where FORCE is appropriate.
set(AVISYNTHPLUS_HDR ${AVISYNTHPLUS_HDR_REALPATH} CACHE FILEPATH ${AVISYNTHPLUS_HDR_DOCSTRING} FORCE)

# Grabbing the directory component of a "realpath" means no cleanup necessary.
get_filename_component(AVISYNTHPLUS_INCLUDE_DIR ${AVISYNTHPLUS_HDR} DIRECTORY)

include_directories(${AVISYNTHPLUS_INCLUDE_DIR})



### Check Avisynth version ###

include(CheckCXXSymbolExists)
set(CMAKE_REQUIRED_INCLUDES ${AVISYNTHPLUS_INCLUDE_DIR})
unset(AVISYNTHPLUS_FOUND CACHE)
check_cxx_symbol_exists(__AVISYNTH_8_H__ avisynth.h AVISYNTHPLUS_FOUND)

if(NOT AVISYNTHPLUS_FOUND)
  # The space before \ is necessary to prevent getting extra line breaks, for
  # some reason beyond the veil of human understanding.
  message(FATAL_ERROR " \
Suitable Avisynth header not found!\n \
\tThis plugin needs the header for Avisynth+ (plugin interface >= version 8).\n \
\n \
\tOnce built, TurnsTile will run in Avisynth 2.6 without issue, but\n \
\tit needs the plus version of the header to successfully compile.")
endif()



### Source files ###

# Export compile_commands.json, so VS Code Intellisense can find the AVS header.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR}/test/include/catch)
include_directories(${CMAKE_SOURCE_DIR}/test/include/md5)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add the Avisynth+ header to the project for easy reference.
set(SRCS ${AVISYNTHPLUS_HDR})

list(APPEND SRCS
  include/lodepng/lodepng.h
  include/lodepng/lodepng.cpp)

list(APPEND SRCS
  src/interface.h
  src/TurnsTile.h
  src/TurnsTileTestSource.h
  src/CLUTer.h
  src/interface.cpp
  src/TurnsTile.cpp
  src/TurnsTileTestSource.cpp
  src/CLUTer.cpp)

configure_file(src/TurnsTile.rc.in ${CMAKE_SOURCE_DIR}/src/TurnsTile.rc)
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

add_library(TurnsTile SHARED ${SRCS})

# In Windows, defining a library as SHARED means it's considered runtime output,
# and will end up in the runtime output directory unless forced elsewhere. In
# Linux and macOS, on the other hand, SHARED libraries are considered library
# output, and will go where one expects with no special property overrides.
set_target_properties(
  TurnsTile
  PROPERTIES
  COMPILE_DEFINITIONS "TURNSTILE_HOST_${TURNSTILE_HOST_DEFINE}"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/artifacts/build/lib
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/artifacts/build/lib
  LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/artifacts/build/lib
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/artifacts/build/lib
  OUTPUT_NAME $<IF:$<BOOL:${WIN32}>,TurnsTile,turnstile>
)



### Testing ###

option(TURNSTILE_TESTS "Enable testing with Catch framework." TRUE)
if(TURNSTILE_TESTS)

  set(SRCS_TEST
    test/include/catch/catch.hpp

    test/include/md5/md5.h
    test/include/md5/md5.c

    ${AVISYNTHPLUS_HDR}
    test/src/avs/errors.cpp
    test/src/avs/main.cpp
    test/src/avs/output.cpp
    test/src/avs/util_avs.h
    test/src/avs/util_avs.cpp
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

  add_executable(TurnsTile-test ${SRCS_TEST})

  set_target_properties(
    TurnsTile-test
    PROPERTIES
    COMPILE_DEFINITIONS "TURNSTILE_HOST_${TURNSTILE_HOST_DEFINE}"
    OUTPUT_NAME $<IF:$<BOOL:${WIN32}>,TurnsTile-test,turnstile-test>
  )

  if(NOT WIN32)
    find_path(DL_INCLUDE_DIR NAMES dlfcn.h)
    find_library(DL_LIBRARIES NAMES dl)
    include_directories(${DL_INCLUDE_DIR})

    # Ugly workaround that it is, it's necessary for some reason to link against
    # libpthreads, at least in Linux. For details, see the following:
    #   https://stackoverflow.com/questions/51209268/using-stdthread-in-a-library-loaded-with-dlopen-leads-to-a-sigsev
    #   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=67791
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
  
    target_link_libraries(TurnsTile-test PRIVATE ${DL_LIBRARIES} Threads::Threads)
  endif()

endif()



### Packaging options ###

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)

set(CPACK_SOURCE_IGNORE_FILES
  # Directories
  "\\\\.git"
  "artifacts"
  "build"
  "include"
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
  TARGETS TurnsTile
  RUNTIME DESTINATION .
  LIBRARY DESTINATION .
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)

# To keep documentation easily accessible, the readme filename should match the
# plugin filename, but GitHub won't show docs up front unless they're named a
# certain way. To deal with that, README.txt gets renamed on install; the need
# for a separate variable here is thanks to CMake for some reason not allowing
# the use of generator expressions with the 'install' command's RENAME option.
set(TURNSTILE_OUTPUT_NAME TurnsTile)
if(NOT WIN32)
  set(TURNSTILE_OUTPUT_NAME libturnstile)
endif()

install(
  FILES README.md
  DESTINATION .
  RENAME ${TURNSTILE_OUTPUT_NAME}.txt
)

# The binary package can be built with the PACKAGE target in the Visual Studio
# solution, either in the IDE or on the command line. The source package, on the
# other hand, must be built manually. Change to artifacts/pack and run
#
#   cpack --config ..\..\build\CPackSourceConfig.cmake
#
# replacing the path if necessary for your particular circumstances.
set(CPACK_SOURCE_GENERATOR "ZIP")
if(WIN32)
  set(CPACK_GENERATOR "ZIP")
else()
  set(CPACK_GENERATOR "TGZ")
endif()
set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_SOURCE_DIR}/artifacts/pack)

set(CPACK_PACKAGE_VERSION_MAJOR ${TURNSTILE_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${TURNSTILE_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${TURNSTILE_PATCH})

set(CPACK_PACKAGE_FILE_NAME TurnsTile-${TURNSTILE_VERSION}-${CMAKE_SYSTEM_NAME}-${TURNSTILE_ARCH}-${CMAKE_CXX_COMPILER_ID})

# These need to be defined before including CPack, otherwise they take on values
# intended for packaging source files. The vagaries of CPack, it would seem.
set(TURNSTILE_BINARY_PACKAGE_NAME ${CPACK_PACKAGE_FILE_NAME})
set(TURNSTILE_BINARY_PACKAGE_GENERATOR ${CPACK_GENERATOR})

include(CPack)

# Needs to be defined after including CPack, to get the right value of CPACK_SYSTEM_NAME.
set(TURNSTILE_CPACK_TEMP_DIR "${CMAKE_BINARY_DIR}/_CPack_Packages/${CPACK_SYSTEM_NAME}/${TURNSTILE_BINARY_PACKAGE_GENERATOR}/${TURNSTILE_BINARY_PACKAGE_NAME}")

# The content of the extras directory is potentially helpful and instructive for
# users, but doesn't really belong in the install location. Unfortunately, CPack
# will only package things specified by an 'install' command. Luckily the CODE
# signature of 'install' allows arbitrary CMake script like this, allowing the
# 'extras' files to stay out of the install destination while remaining in the
# binary distribution. Users then don't have to download anything else.
#
# This is a variation on Jeremy Tan's source package customization technique:
#
#   https://jtanx.github.io/2019/08/22/cmake-dist-customisation/
#
install(
  CODE "
    file(
      INSTALL ${CMAKE_SOURCE_DIR}/extras
      DESTINATION ${TURNSTILE_CPACK_TEMP_DIR})
  "
)

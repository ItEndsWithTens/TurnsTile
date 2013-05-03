### Basic info ###

cmake_minimum_required(VERSION 2.8.4 FATAL_ERROR)

if(WIN32)
  project(TurnsTile CXX)
else()
  project(turnstile CXX)
endif()

set(TURNSTILE_MAJOR 0)
set(TURNSTILE_MINOR 3)
set(TURNSTILE_PATCH 2)
set(TURNSTILE_VERSION ${TURNSTILE_MAJOR}.${TURNSTILE_MINOR}.${TURNSTILE_PATCH})
set(TURNSTILE_AUTHOR "Robert Martens")
set(TURNSTILE_AUTHOR_EMAIL "<robert.martens@gmail.com>")

if(WIN32)
  set(HOSTS "Avisynth 2.5" "Avisynth 2.6")
else()
  set(HOSTS "Avxsynth")
endif()

list(SORT HOSTS)
list(GET HOSTS 0 HOST_DEFAULT)
set(TURNSTILE_HOST ${HOST_DEFAULT} CACHE STRING "Host application to target.")
set_property(CACHE TURNSTILE_HOST PROPERTY STRINGS ${HOSTS})
set(TURNSTILE_DESCRIPTION "TurnsTile plugin for ${TURNSTILE_HOST}")


### Source files ###

if(${TURNSTILE_HOST} STREQUAL "Avxsynth")
  set(TURNSTILE_HOST_HDR include/avx/avxplugin.h)
  include_directories(
    include/avx/
    include/avx/utils
    include/avx/windowsports
  )
elseif(${TURNSTILE_HOST} STREQUAL "Avisynth 2.6")
  set(TURNSTILE_HOST_HDR include/avs/avisynth-260MT-x86.h)
else()
  set(TURNSTILE_HOST_HDR include/avs/avisynth-258MT-x86.h)
endif()

set(SRCS
  ${TURNSTILE_HOST_HDR}
  src/interface.h
  src/TurnsTile.h
  src/CLUTer.h
  src/interface.cpp
  src/TurnsTile.cpp
  src/CLUTer.cpp
)

if(WIN32)
  configure_file(
      src/TurnsTile.rc.in ${CMAKE_CURRENT_SOURCE_DIR}/src/TurnsTile.rc
  )
  list(APPEND SRCS src/TurnsTile.rc)
  if(MSVC)
    # CMake handles the Source and Header Files filters automatically, but not
    # Resource Files, the way VS does. This is just cosmetic, really.
    source_group("Resource Files" FILES src/TurnsTile.rc)
  endif()
endif()

string(TOUPPER ${TURNSTILE_HOST} TURNSTILE_HOST_DEFINE)
string(REPLACE "." "" TURNSTILE_HOST_DEFINE ${TURNSTILE_HOST_DEFINE})
string(REPLACE " " "_" TURNSTILE_HOST_DEFINE ${TURNSTILE_HOST_DEFINE})

# When defined as SHARED, the plugin builds correctly in Windows and Linux, but
# it is technically a MODULE, and that distinction is important in OS X.
add_library(${PROJECT_NAME} MODULE ${SRCS})
set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
  COMPILE_DEFINITIONS "TURNSTILE_HOST_${TURNSTILE_HOST_DEFINE}"
)



### CMake options ###
if(MSVC)
  
  # Static linking is more convenient for a plugin like this, allowing all
  # dependencies to be packaged into one file; drop it into your Avisynth
  # plugins folder and you're ready to go! Since it's not necessary, however,
  # I've made it an option for anyone who'd rather link dynamically.
  option(
    TURNSTILE_STATIC_CRT "Link statically with the C Run-Time libraries." TRUE
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

if(WIN32)
  get_filename_component(
    HOST_PLUGIN_DIR
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\AviSynth;plugindir2_5] ABSOLUTE
  )
else()
  set(HOST_PLUGIN_DIR /usr/local/lib/avxsynth)
endif()

set(
  TURNSTILE_PLUGINS_DIR ${HOST_PLUGIN_DIR} CACHE FILEPATH
  "Path to your plugin autoload directory (for more convenient debugging)."
)



### Testing ###

# This custom target basically duplicates the result of the CMake-provided
# INSTALL target, but saves you some annoying little extra effort. By setting
# the ALL dependency, the standard ALL_BUILD target now depends on this one,
# so that every time you build the project, the resulting library will be copied
# to the plugins directory, even if the plugin is up to date, and even if you're
# only switching between configurations, making debugging a one key affair.
add_custom_target(
  ${PROJECT_NAME}-copy ALL
  COMMAND ${CMAKE_COMMAND} -E copy
  $<TARGET_FILE:${PROJECT_NAME}> ${TURNSTILE_PLUGINS_DIR}
)



### Packaging options ###

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)

set(CPACK_SOURCE_IGNORE_FILES
  # Directories
  "\\\\.git"
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

set(CMAKE_INSTALL_PREFIX ${TURNSTILE_PLUGINS_DIR} CACHE INTERNAL "")

install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION .
  LIBRARY DESTINATION .
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)

# As far as I can tell, CMake doesn't make the output filename of a target
# readily available, so I need to pull it out of the full path.
get_target_property(TURNSTILE_OUTPUT_PATH ${PROJECT_NAME} LOCATION)
get_filename_component(TURNSTILE_OUTPUT_NAME ${TURNSTILE_OUTPUT_PATH} NAME_WE)
install(
  FILES
  README.txt
  DESTINATION .
  RENAME ${TURNSTILE_OUTPUT_NAME}.txt
)

# Build the source package by changing to your build directory and
# running: cpack --config CPackSourceConfig.cmake
set(CPACK_SOURCE_GENERATOR "ZIP")
if(WIN32)
  set(TURNSTILE_HOST_ABBR "AVS")
  set(CPACK_GENERATOR "ZIP")
else()
  set(TURNSTILE_HOST_ABBR "AVX")
  set(CPACK_GENERATOR "TGZ")
endif()
set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/dist)

set(CPACK_PACKAGE_VERSION_MAJOR ${TURNSTILE_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${TURNSTILE_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${TURNSTILE_PATCH})
if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  set(TURNSTILE_ARCH x64)
else()
  set(TURNSTILE_ARCH x86)
endif()

set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${TURNSTILE_VERSION}-${TURNSTILE_HOST_ABBR}${TURNSTILE_HOST_VERSION}-${CMAKE_SYSTEM_NAME}-${TURNSTILE_ARCH})

include(CPack)

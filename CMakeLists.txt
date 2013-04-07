### Basic info ###

cmake_minimum_required(VERSION 2.8.4 FATAL_ERROR)

project(TurnsTile CXX)

set(TURNSTILE_MAJOR 0)
set(TURNSTILE_MINOR 3)
set(TURNSTILE_PATCH 1)
set(TURNSTILE_VERSION ${TURNSTILE_MAJOR}.${TURNSTILE_MINOR}.${TURNSTILE_PATCH})
set(TURNSTILE_AUTHOR "Robert Martens <robert.martens@gmail.com>")
set(TURNSTILE_AVS_VERSION "2.5")



### Source files ###

set(SRCS
  src/avisynth.h
  src/TurnsTile.h
  src/CLUTer.h
  src/interface.cpp
  src/TurnsTile.cpp
  src/CLUTer.cpp
  src/TurnsTile.rc
)

if(MSVC)
  # CMake handles the Source and Header Files filters automatically, but not
  # Resource Files, the way VS does. This is just cosmetic, really.
  source_group("Resource Files" FILES src/TurnsTile.rc)
endif()

add_library(${PROJECT_NAME} SHARED ${SRCS})



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

get_filename_component(
  AVISYNTH_AUTOLOAD
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\AviSynth;plugindir2_5] ABSOLUTE
)
set(
  TURNSTILE_PLUGINS_DIR ${AVISYNTH_AUTOLOAD} CACHE FILEPATH
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
  # Project files
  "gitignore$"
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
)

install(
  FILES
  TurnsTile.txt
  DESTINATION .
)

# Build the source package by changing to your build directory and
# running: cpack --config CPackSourceConfig.cmake
set(CPACK_SOURCE_GENERATOR "ZIP")
set(TURNSTILE_AVS_ABBR "AVS")
set(CPACK_GENERATOR "ZIP")

set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/dist)

set(CPACK_PACKAGE_VERSION_MAJOR ${TURNSTILE_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${TURNSTILE_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${TURNSTILE_PATCH})
set(TURNSTILE_ARCH x86)

set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${TURNSTILE_VERSION}-${TURNSTILE_AVS_ABBR}${TURNSTILE_AVS_VERSION}-${TURNSTILE_ARCH})

include(CPack)

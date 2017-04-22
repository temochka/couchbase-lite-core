cmake_minimum_required (VERSION 2.6)
cmake_policy(VERSION 3.1)
project (PlatformUtil)
if(NOT MSVC AND NOT ANDROID)
  return()
endif()

set(COMPILE_FLAGS   "${COMPILE_FLAGS}   -Wall -Werror")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Debug/Release builds
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DDEBUG)
endif()

if(MSVC)
  set(UTIL_SRC "src/PlatformIO.cc" "src/arc4random-msvc.cc" "src/asprintf.c" "src/mkdtemp.cc" "src/mkstemp.cc" "src/strlcat.c" "src/vasprintf-msvc.c")
else()
  set(UTIL_SRC "src/arc4random-android.cc" "src/strlcat.c")
endif()

include_directories("include")
add_library(PlatformUtil        SHARED  ${UTIL_SRC})

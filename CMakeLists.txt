cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(SC-Bot)

if (${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
  add_compile_definitions(SHOW_CONSOLE)
endif()

include(cmake/CPM.cmake)

CPMAddPackage("gh:matcool/mat-dash#cdc9f70")
CPMAddPackage("gh:HJfod/cocos-headers#cbf79fc")
CPMAddPackage("gh:SChaotyx/gd.h#2ee1f0b")
CPMAddPackage("gh:TsudaKageyu/minhook#4a45552")
CPMAddPackage("gh:btzy/nativefiledialog-extended#06a5c1f")

file(
  GLOB_RECURSE SOURCE_FILES
  src/*.cpp
  ${nativefiledialog-extended_SOURCE_DIR}/src/nfd_common.c
  ${nativefiledialog-extended_SOURCE_DIR}/src/nfd_win.cpp
    )

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

execute_process(
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(
  "${CMAKE_SOURCE_DIR}/src/Version.h.in"
  "${CMAKE_SOURCE_DIR}/src/Version.h"
)

target_compile_definitions(SC-Bot PRIVATE CC_DLL=)

target_include_directories(SC-Bot PRIVATE
    ${gd.h_SOURCE_DIR}/include ${gd.h_SOURCE_DIR}
    ${minhook_SOURCE_DIR}/include
    ${nativefiledialog-extended_SOURCE_DIR}/src/include
)

target_link_libraries(SC-Bot 
  mat-dash
  cocos2d
  minhook
  opengl32
  ${CMAKE_SOURCE_DIR}/libs/glew32.lib
)

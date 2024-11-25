cmake_minimum_required(VERSION 3.2)

set(CMAKE_TOOLCHAIN_FILE $ENV{DEVKITPRO}/cmake/WiiU.cmake)
set(BUILD_DIR "${CMAKE_SOURCE_DIR}/build")

if(NOT EXISTS "${CMAKE_TOOLCHAIN_FILE}")
    message(FATAL_ERROR  "Toolchain file: ´${CMAKE_TOOLCHAIN_FILE}´ does not exist. Run devkitPro/msys2/usr/bin/pacman -Syu --needed wiiu-dev")
    endif()

message(STATUS "Toolchain path: ${CMAKE_TOOLCHAIN_FILE}")

project(MungPlex-Gecko LANGUAGES C CXX)

add_executable(MungPlex-Gecko main.cpp)

target_compile_options(MungPlex-Gecko PRIVATE -Wall -Wextra -pedantic)

wut_create_rpx(MungPlex-Gecko)
wut_create_wuhb(MungPlex-Gecko)



#ToDo: make this call make so there's no need to do this manually
#message(STATUS "Processing Makefile...")

#add_custom_command(
#    OUTPUT ${BUILD_DIR}/dummy
#    COMMAND make -C ${BUILD_DIR}
#    COMMAND ${CMAKE_COMMAND} -E touch ${BUILD_DIR}/dummy
#    COMMENT "Building all targets with external Makefile"
#)

#add_custom_target(
#    run_make ALL
#    DEPENDS ${BUILD_DIR}/dummy
#)
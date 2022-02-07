cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
add_subdirectory(../pico_i2c_slave/i2c_slave ../pico_i2c_slave/i2c_slave/build)
add_subdirectory(common)

project(pico_firmwares C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory(epanel-io)

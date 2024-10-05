cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0048 NEW)
project (GDDPSMeter)

#x64-bit is disabled by default, pass in DEFINE_X64 to enable it
option(DEFINE_X64 "x64 build option" OFF)

#only build RelWithDebInfo and Release
set(CMAKE_CONFIGURATION_TYPES RelWithDebInfo Release)

if(DEFINE_X64)
  set(CMAKE_GENERATOR_PLATFORM x64)
  add_definitions(-DX64)
endif()

add_subdirectory(GDDPSMeterLoader)
add_subdirectory(DPSMeter)


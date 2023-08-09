cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0048 NEW)
project (GDDPSMeter)

#set(CMAKE_GENERATOR_PLATFORM x64)

add_subdirectory(GDDPSMeterLoader)
add_subdirectory(DPSMeter)


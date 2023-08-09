add_executable(GDDPSMeterLoader
GDDPSMeterLoader.cpp
proc.cpp
proc.h
GDDPSMeterLoader.rc resource.h)

set_target_properties(GDDPSMeterLoader PROPERTIES OUTPUT_NAME "GDDPSMeterLoader")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_RELEASE")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common)

file (GLOB SRC_FILES *.cpp *.h )
source_group ("src" FILES ${SRC_FILES} )
file (GLOB COMMON_FILES ../common/*.cpp ../common/*.h )
source_group ("common" FILES ${COMMON_FILES} )

add_executable(GDDPSMeterLoader ${SRC_FILES} ${COMMON_FILES}
GDDPSMeterLoader.rc resource.h)

set_target_properties(GDDPSMeterLoader PROPERTIES OUTPUT_NAME "GDDPSMeterLoader")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_RELEASE")

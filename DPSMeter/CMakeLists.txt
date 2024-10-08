
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Detours/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/imgui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/srcdetour)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/srcrenderer)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/srcmain)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common)

include_directories($ENV{WindowsSDK_IncludePath})
include_directories($ENV{DXSDK_DIR}/include)

if(DEFINE_X64)
  link_directories("$ENV{DXSDK_DIR}/lib/x64")
  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Detours/lib.X64)
else()
  link_directories("$ENV{DXSDK_DIR}/lib/x86")
  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Detours/lib.X86)
endif()

#include_directories(${WindowsSDK_IncludePath})
#include_directories(${DXSDK_DIR}/include)

file (GLOB MAIN_FILES srcmain/*.cpp srcmain/*.h )
source_group ("main" FILES ${MAIN_FILES} )

file (GLOB IMGUI_FILES imgui/*.cpp imgui/*.h )
source_group ("imgui" FILES ${IMGUI_FILES} )

file (GLOB DETOUR_FILES srcdetour/*.cpp srcdetour/*.h )
source_group ("srcdetour" FILES ${DETOUR_FILES} )

file (GLOB RENDERER_FILES srcrenderer/*.cpp srcrenderer/*.h )
source_group ("srcrenderer" FILES ${RENDERER_FILES} )

file (GLOB COMMON_FILES ../common/*.cpp ../common/*.h )
source_group ("common" FILES ${COMMON_FILES} )

add_library(DPSMeter SHARED
resource.h
DPSMeter.rc
${MAIN_FILES} ${IMGUI_FILES} ${DETOUR_FILES} ${RENDERER_FILES} ${COMMON_FILES})

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_RELEASE")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -D_DEBINFO")
set_target_properties(DPSMeter PROPERTIES OUTPUT_NAME "DPSMeter")
set_target_properties(DPSMeter PROPERTIES LINK_FLAGS "/ignore:4099")
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/GDDPSMeterLoader)

target_link_libraries(DPSMeter detours.lib winmm.lib d3dx9d.lib dbghelp.lib)



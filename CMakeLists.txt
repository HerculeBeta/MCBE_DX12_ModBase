cmake_minimum_required(VERSION 3.20)
project(MCBE_DX12_ModBase LANGUAGES CXX)

file(GLOB_RECURSE MCBE_DX12_MODBASE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE MCBE_DX12_MODBASE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_subdirectory(external/imgui)

set(MCBE_DX12_MODBASE_VENDOR_SOURCES
    external/kiero/kiero.cpp
)

add_library(MCBE_DX12_ModBase SHARED
    ${MCBE_DX12_MODBASE_SOURCES}
    ${MCBE_DX12_MODBASE_HEADERS}
    ${MCBE_DX12_MODBASE_VENDOR_SOURCES}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Source" FILES
    ${MCBE_DX12_MODBASE_SOURCES}
    ${MCBE_DX12_MODBASE_HEADERS}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/external" PREFIX "External" FILES
    ${MCBE_DX12_MODBASE_VENDOR_SOURCES}
)

target_include_directories(MCBE_DX12_ModBase PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/external"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/kiero"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/minhook"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/d3d12"
)

target_link_directories(MCBE_DX12_ModBase PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/minhook/lib"
)

target_link_libraries(MCBE_DX12_ModBase PRIVATE
    imgui
    Minhook
    d3d12
    dxgi
    Psapi
)

target_compile_definitions(MCBE_DX12_ModBase PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

set_target_properties(MCBE_DX12_ModBase PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "MCBE_DX12_ModBase"
)

if(MSVC)
    target_compile_options(MCBE_DX12_ModBase PRIVATE /W4 /permissive- /utf-8 /FIWindows.h)
endif()

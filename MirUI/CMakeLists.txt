cmake_minimum_required(VERSION 3.28)

project(MirUI
    VERSION 0.4.0
    DESCRIPTION "M1R 4D C++ UI core and engine interop"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MirUI is the C++ presentation/interop layer.
# SwiftUI sources are compiled separately by Package.swift/Xcode.
add_library(MirUI STATIC
    Core/Transform/TransformAdapter.cpp
    Interop/SelectionEventBridge.cpp
    Interop/SelectionInspectorBridge.cpp
    Widgets/PropertyGrid/RenderSelectionPropertyAdapter.cpp
    Widgets/PropertyGrid/SelectionInspectorModel.cpp
)

target_include_directories(MirUI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(MirUI PUBLIC MirEngine)
target_compile_features(MirUI PUBLIC cxx_std_23)

set_target_properties(MirUI PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

message(STATUS "MirUI v${PROJECT_VERSION}")
message(STATUS "  C++ presentation + engine interop: ON")
message(STATUS "  SwiftUI sources: Package.swift / Xcode")

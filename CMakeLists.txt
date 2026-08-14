cmake_minimum_required(VERSION 3.28)

project(MIR4D
    VERSION 0.1.0
    DESCRIPTION "MIR 4D Engineering CAD"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MIR_ENABLE_OCCT "Enable OpenCASCADE-backed STEP/IGES IO" OFF)
option(MIR_ENABLE_ASSIMP "Enable optional Assimp asset/mesh importer" OFF)

add_library(MirEngine INTERFACE)
target_include_directories(MirEngine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(MirEngine INTERFACE cxx_std_23)
target_compile_definitions(MirEngine INTERFACE MIR_ENABLE_OCCT=$<BOOL:${MIR_ENABLE_OCCT}>)

add_library(MirEngineBRep INTERFACE)
target_link_libraries(MirEngineBRep INTERFACE MirEngine)
target_include_directories(MirEngineBRep INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/MirEngine/BRep
    ${CMAKE_CURRENT_SOURCE_DIR}/MirEngine/BRep/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/MirEngine/BRep/Topology
    ${CMAKE_CURRENT_SOURCE_DIR}/MirEngine/BRep/Geometry
    ${CMAKE_CURRENT_SOURCE_DIR}/MirEngine/BRep/Builders
    ${CMAKE_CURRENT_SOURCE_DIR}/MirEngine/BRep/Tessellator
    ${CMAKE_CURRENT_SOURCE_DIR}/MirEngine/BRep/Boolean
    ${CMAKE_CURRENT_SOURCE_DIR}/MirEngine/BRep/Converters
    ${CMAKE_CURRENT_SOURCE_DIR}/MirEngine/BRep/Commands
    ${CMAKE_CURRENT_SOURCE_DIR}/MirEngine/BRep/Validator
)

after=""

if(APPLE)
    target_compile_definitions(MirEngine INTERFACE MIR_PLATFORM_MACOS=1)

    # The native macOS OpenGL bridge is compiled only on Apple platforms.
    # SwiftUI talks to this library through the stable C ABI in
    # MirEngine/Exports/MirEngineExports.h.
    enable_language(OBJCXX)

    find_library(MIR4D_OPENGL_FRAMEWORK OpenGL REQUIRED)
    find_library(MIR4D_COCOA_FRAMEWORK Cocoa REQUIRED)

    add_library(MirEngineMacOpenGL STATIC
        MirEngine/Exports/MirEngineExports.cpp
        MirEngine/Rendering/OpenGL/OpenGLRenderer.cpp
        MirEngine/Platform/macOS/OpenGL/MacOpenGLContext.mm
    )

    target_include_directories(MirEngineMacOpenGL PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    # The native C ABI creates B-Rep-backed scene content, so the OpenGL
    # bridge must inherit the complete B-Rep include graph, not only the
    # base MirEngine include root.
    target_link_libraries(MirEngineMacOpenGL PUBLIC
        MirEngineBRep
        ${MIR4D_OPENGL_FRAMEWORK}
        ${MIR4D_COCOA_FRAMEWORK}
    )
    target_compile_features(MirEngineMacOpenGL PUBLIC cxx_std_23)
endif()

add_library(MirEngineRuntime INTERFACE)
target_link_libraries(MirEngineRuntime INTERFACE MirEngineBRep)
target_include_directories(MirEngineRuntime INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(MirEngineRuntime INTERFACE cxx_std_23)

# Explicit domain targets keep header-only modules in the build graph.
add_library(MirEngineViewport INTERFACE)
target_link_libraries(MirEngineViewport INTERFACE MirEngine)
target_include_directories(MirEngineViewport INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(MirEngineGeometry INTERFACE)
target_link_libraries(MirEngineGeometry INTERFACE MirEngine)
target_include_directories(MirEngineGeometry INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(MirEngineSimulation INTERFACE)
target_link_libraries(MirEngineSimulation INTERFACE MirEngine)
target_include_directories(MirEngineSimulation INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(MirEngineDomains INTERFACE)
target_link_libraries(MirEngineDomains INTERFACE MirEngine)
target_include_directories(MirEngineDomains INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(MirEngineRender STATIC
    MirEngine/Render/RenderCamera.cpp
)
target_link_libraries(MirEngineRender PUBLIC MirEngine)
target_include_directories(MirEngineRender PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(MirEngineRender PUBLIC cxx_std_23)

add_library(MirEngineIO STATIC
    MirEngine/IO/Mesh/StlImporter.cpp
    MirEngine/IO/Mesh/StlExporter.cpp
    MirEngine/IO/ImportService.cpp
    MirEngine/IO/ExportService.cpp
    MirEngine/IO/Commands/MeshImportCommandHandler.cpp
    MirEngine/IO/Step/OcctBridge.cpp
    MirEngine/IO/Step/StepImporter.cpp
    MirEngine/IO/Step/StepExporter.cpp
    MirEngine/Rendering/DocumentSceneRenderBridge.cpp
)
target_link_libraries(MirEngineIO PUBLIC MirEngineRuntime MirEngineBRep)
target_include_directories(MirEngineIO PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(MirEngineIO PUBLIC cxx_std_23)

if(MIR_ENABLE_OCCT)
    message(STATUS "MIR_ENABLE_OCCT=ON: OpenCASCADE discovery/linking is required before STEP mapping is enabled.")
endif()

if(MIR_ENABLE_ASSIMP)
    find_package(assimp CONFIG REQUIRED)
    add_library(MirEngineAssimpIO STATIC
        MirEngine/IO/Mesh/AssimpImporter.cpp
    )
    target_link_libraries(MirEngineAssimpIO PUBLIC MirEngineIO assimp::assimp)
    target_include_directories(MirEngineAssimpIO PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_features(MirEngineAssimpIO PUBLIC cxx_std_23)
endif()

add_subdirectory(MirUI)

if(APPLE)
    # MirUI is the C++ presentation/interop library consumed by SwiftUI.
    # Link the native OpenGL implementation into the same dependency graph so
    # Xcode cannot accidentally build the UI without the real renderer.
    target_link_libraries(MirUI PUBLIC MirEngineMacOpenGL)
endif()

add_executable(MIR4D_BuildCheck MirEngine/Tests/Core/BuildCheck.cpp)
target_link_libraries(MIR4D_BuildCheck PRIVATE MirEngine)

add_executable(MIR4D_Integration4D MirEngine/Tests/Integration/Integration4D.cpp)
target_link_libraries(MIR4D_Integration4D PRIVATE MirEngine)

add_executable(MIR4D_StlRoundTrip MirEngine/Tests/IO/StlRoundTrip.cpp)
target_link_libraries(MIR4D_StlRoundTrip PRIVATE MirEngineIO)

add_executable(MIR4D_StepImporter MirEngine/Tests/IO/Step/StepImporterTest.cpp)
target_link_libraries(MIR4D_StepImporter PRIVATE MirEngineIO)

add_executable(MIR4D_DocumentRenderBridge MirEngine/Tests/Render/DocumentRenderBridge.cpp)
target_link_libraries(MIR4D_DocumentRenderBridge PRIVATE MirEngineIO)

add_executable(MIR4D_RenderCamera MirEngineRender PRIVATE MirEngineRender)

target_link_libraries(MIR4D_RenderCamera PRIVATE MirEngineRender)

add_executable(MIR4D_CreateBox MirEngine/Tests/Document/CreateBox.cpp)
target_link_libraries(MIR4D_CreateBox PRIVATE MirEngineBRep)

add_executable(MIR4D_ObjectStore MirEngine/Tests/Document/ObjectStoreTests.cpp)
target_link_libraries(MIR4D_ObjectStore PRIVATE MirEngine)

add_executable(MIR4D_EngineDocumentBridge MirEngine/Tests/Document/EngineDocumentBridge.cpp)
target_link_libraries(MIR4D_EngineDocumentBridge PRIVATE MirUI)

add_executable(MIR4D_BRepBox MirEngine/Tests/BRep/BRepBox.cpp)
target_link_libraries(MIR4D_BRepBox PRIVATE MirEngineBRep)

add_executable(MIR4D_BRepTopology MirEngine/Tests/BRep/BRepTopology.cpp)
target_link_libraries(MIR4D_BRepTopology PRIVATE MirEngineBRep)

add_executable(MIR4D_BRepProfileExtrude MirEngine/Tests/BRep/BRepProfileExtrude.cpp)
target_link_libraries(MIR4D_BRepProfileExtrude PRIVATE MirEngineBRep)

add_executable(MIR4D_BRepHeaderCheck MirEngine/Tests/BRep/BRepHeaderCheck.cpp)
target_link_libraries(MIR4D_BRepHeaderCheck PRIVATE MirEngineBRep)

add_executable(MIR4D_SketchRectangleProfile MirEngine/Tests/Sketch/SketchRectangleProfileTest.cpp)
target_link_libraries(MIR4D_SketchRectangleProfile PRIVATE MirEngine)

add_executable(MIR4D_SketchRectangleSolver MirEngine/Tests/Sketch/SketchRectangleSolverTest.cpp)
target_link_libraries(MIR4D_SketchRectangleSolver PRIVATE MirEngine)

add_executable(MIR4D_ViewportFoundation MirEngine/Tests/Render/ViewportFoundation.cpp)
target_link_libraries(MIR4D_ViewportFoundation PRIVATE MirEngineViewport)

add_executable(MIR4D_ViewportRuntime MirEngine/Tests/Render/ViewportRuntime.cpp)
target_link_libraries(MIR4D_ViewportRuntime PRIVATE MirEngineViewport)

add_executable(MIR4D_ViewportController MirEngine/Tests/Viewport/ViewportControllerTest.cpp)
target_link_libraries(MIR4D_ViewportController PRIVATE MirEngineViewport)

add_executable(MIR4D_Edge MirEngine/Tests/Geometry/EdgeTest.cpp)
target_link_libraries(MIR4D_Edge PRIVATE MirEngineGeometry)

add_executable(MIR4D_Count MirEngine/Tests/Core/CountTest.cpp)
target_link_libraries(MIR4D_Count PRIVATE MirEngine)

add_executable(MIR4D_Simulation MirEngine/Tests/Simulation/SimulationTest.cpp)
target_link_libraries(MIR4D_Simulation PRIVATE MirEngineSimulation)

add_executable(MIR4D_ExtendedDomains MirEngine/Tests/Domains/ExtendedDomainsTest.cpp)
target_link_libraries(MIR4D_ExtendedDomains PRIVATE MirEngineDomains)

add_executable(MIR4D_SelectionState MirEngine/Tests/Interaction/SelectionStateTests.cpp)
target_link_libraries(MIR4D_SelectionState PRIVATE MirEngine)

enable_testing()
add_test(NAME MIR4D_BuildCheck COMMAND MIR4D_BuildCheck)
add_test(NAME MIR4D_Integration4D COMMAND MIR4D_Integration4D)
add_test(NAME MIR4D_StlRoundTrip COMMAND MIR4D_StlRoundTrip)
add_test(NAME MIR4D_StepImporter COMMAND MIR4D_StepImporter)
add_test(NAME MIR4D_DocumentRenderBridge COMMAND MIR4D_DocumentRenderBridge)
add_test(NAME MIR4D_RenderCamera COMMAND MIR4D_RenderCamera)
add_test(NAME MIR4D_CreateBox COMMAND MIR4D_CreateBox)
add_test(NAME MIR4D_ObjectStore COMMAND MIR4D_ObjectStore)
add_test(NAME MIR4D_EngineDocumentBridge COMMAND MIR4D_EngineDocumentBridge)
add_test(NAME MIR4D_BRepBox COMMAND MIR4D_BRepBox)
add_test(NAME MIR4D_BRepTopology COMMAND MIR4D_BRepTopology)
add_test(NAME MIR4D_BRepProfileExtrude COMMAND MIR4D_BRepProfileExtrude)
add_test(NAME MIR4D_BRepHeaderCheck COMMAND MIR4D_BRepHeaderCheck)
add_test(NAME MIR4D_SketchRectangleProfile COMMAND MIR4D_SketchRectangleProfile)
add_test(NAME MIR4D_SketchRectangleSolver COMMAND MIR4D_SketchRectangleSolver)
add_test(NAME MIR4D_ViewportFoundation COMMAND MIR4D_ViewportFoundation)
add_test(NAME MIR4D_ViewportRuntime COMMAND MIR4D_ViewportRuntime)
add_test(NAME MIR4D_ViewportController COMMAND MIR4D_ViewportController)
add_test(NAME MIR4D_Edge COMMAND MIR4D_Edge)
add_test(NAME MIR4D_Count COMMAND MIR4D_Count)
add_test(NAME MIR4D_Simulation COMMAND MIR4D_Simulation)
add_test(NAME MIR4D_ExtendedDomains COMMAND MIR4D_ExtendedDomains)
add_test(NAME MIR4D_SelectionState COMMAND MIR4D_SelectionState)

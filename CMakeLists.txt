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

if(APPLE)
    target_compile_definitions(MirEngine INTERFACE MIR_PLATFORM_MACOS=1)

    enable_language(OBJCXX)

    find_library(MIR4D_OPENGL_FRAMEWORK OpenGL REQUIRED)
    find_library(MIR4D_COCOA_FRAMEWORK Cocoa REQUIRED)

    add_library(MirEngineMacOpenGL STATIC
        MirEngine/Core/Exports/MirEngineExports.cpp
        MirEngine/Core/Logging/Logger.cpp
        MirEngine/Rendering/OpenGL/OpenGLRenderer.cpp
        MirEngine/Rendering/OpenGL/OpenGLCapabilities.cpp
        MirEngine/Rendering/OpenGL/OpenGLDebug.cpp
        MirEngine/Rendering/OpenGL/OpenGLDevice.cpp
        MirEngine/Rendering/OpenGL/OpenGLIndexBuffer.cpp
        MirEngine/Rendering/OpenGL/OpenGLShader.cpp
        MirEngine/Rendering/OpenGL/OpenGLState.cpp
        MirEngine/Rendering/OpenGL/OpenGLTexture.cpp
        MirEngine/Rendering/OpenGL/OpenGLFrameBuffer.cpp
        MirEngine/Rendering/OpenGL/OpenGLVertexArray.cpp
        MirEngine/Rendering/OpenGL/OpenGLVertexBuffer.cpp
        MirEngine/Rendering/Passes/GeometryPass.cpp
        MirEngine/Rendering/Passes/GizmoPass.cpp
        MirEngine/Rendering/Passes/GridPass.cpp
        MirEngine/Rendering/Passes/OverlayPass.cpp
        MirEngine/Rendering/Passes/SelectionPass.cpp
        MirEngine/Rendering/Resources/ShaderLibrary.cpp
        MirEngine/Rendering/Camera/RenderCamera.cpp
        MirEngine/Rendering/OpenGL/OpenGLShaderProgram.cpp
        MirEngine/Rendering/OpenGL/OpenGLFrameRenderer.cpp
        MirEngine/Rendering/OpenGL/OpenGLMeshRenderer.cpp
        MirEngine/Rendering/OpenGL/OpenGLRenderMeshUploader.cpp
        MirEngine/Rendering/Selection/OpenGLSelectionRenderer.cpp
        MirEngine/Rendering/Selection/RenderScreenRay.cpp
        MirEngine/Rendering/Selection/RenderSelectionController.cpp
        MirEngine/Rendering/Selection/RenderSelectionProperties.cpp
        MirEngine/Rendering/Selection/RenderSelectionPropertiesFormatter.cpp
        MirEngine/Rendering/Viewport/RenderViewport.cpp
        MirEngine/Platform/macOS/OpenGL/MacOpenGLContext.mm
    )

    # MirEngineExports.cpp constructs B-Rep-backed scene objects. Keep the
    # native OpenGL target independent from the declaration order of the
    # MirEngineBRep interface target by publishing the required include roots
    # directly on this target.
    target_include_directories(MirEngineMacOpenGL PUBLIC
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
    target_compile_definitions(MirEngineMacOpenGL PUBLIC
        GL_SILENCE_DEPRECATION=1
    )
    target_link_libraries(MirEngineMacOpenGL PUBLIC
        MirEngine
        MirEngineIO
        ${MIR4D_OPENGL_FRAMEWORK}
        ${MIR4D_COCOA_FRAMEWORK}
    )
    target_compile_features(MirEngineMacOpenGL PUBLIC cxx_std_23)

    add_library(MirEngineRender INTERFACE)
    target_link_libraries(MirEngineRender INTERFACE MirEngineMacOpenGL)
    target_include_directories(MirEngineRender INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

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
    target_sources(MirEngineIO PRIVATE MirEngine/IO/Mesh/AssimpImporter.cpp)
    target_link_libraries(MirEngineIO PUBLIC assimp::assimp)
endif()

add_subdirectory(MirUI)

if(APPLE)
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

add_executable(MIR4D_AssimpImport MirEngine/Tests/IO/AssimpImport.cpp)
target_include_directories(MIR4D_AssimpImport PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(MIR4D_AssimpImport PRIVATE MirEngineIO)

if(MIR_ENABLE_ASSIMP)
    # AssimpImporter registers itself through a static initializer in AssimpImporter.cpp.
    # Force the whole archive into the executable so the registrar is not dropped by the linker.
    foreach(_t MIR4D_StlRoundTrip MIR4D_StepImporter MIR4D_AssimpImport MIR4D_DocumentRenderBridge)
        if(TARGET ${_t})
            target_link_options(${_t} PRIVATE -Wl,-force_load,$<TARGET_FILE:MirEngineIO>)
        endif()
    endforeach()
endif()

add_executable(MIR4D_DocumentRenderBridge MirEngine/Tests/Render/DocumentRenderBridge.cpp)
target_link_libraries(MIR4D_DocumentRenderBridge PRIVATE MirEngineIO)

add_executable(MIR4D_RenderCamera MirEngine/Tests/Render/RenderCameraTest.cpp)
target_link_libraries(MIR4D_RenderCamera PRIVATE MirEngineRender)

if(APPLE)
    # Автономный отладочный стенд viewport'а (окно + мышь + клавиатура + диагностика),
    # работает вне MirUI/SwiftUI через тот же C ABI.
    add_executable(MIR4D_DebugViewport MirEngine/Tests/Render/DebugViewport.mm)
    target_include_directories(MIR4D_DebugViewport PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(MIR4D_DebugViewport PRIVATE MirEngineMacOpenGL)
endif()

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

add_executable(MIR4D_BRepBoolean MirEngine/Tests/BRep/BRepBoolean.cpp)
target_link_libraries(MIR4D_BRepBoolean PRIVATE MirEngineBRep)

add_executable(MIR4D_GeometryQuery MirEngine/Tests/Geometry/GeometryQueryTest.cpp)
target_link_libraries(MIR4D_GeometryQuery PRIVATE MirEngineGeometry)

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
add_test(NAME MIR4D_AssimpImport COMMAND MIR4D_AssimpImport)
add_test(NAME MIR4D_DocumentRenderBridge COMMAND MIR4D_DocumentRenderBridge)
add_test(NAME MIR4D_RenderCamera COMMAND MIR4D_RenderCamera)
add_test(NAME MIR4D_CreateBox COMMAND MIR4D_CreateBox)
add_test(NAME MIR4D_ObjectStore COMMAND MIR4D_ObjectStore)
add_test(NAME MIR4D_EngineDocumentBridge COMMAND MIR4D_EngineDocumentBridge)
add_test(NAME MIR4D_BRepBox COMMAND MIR4D_BRepBox)
add_test(NAME MIR4D_BRepTopology COMMAND MIR4D_BRepTopology)
add_test(NAME MIR4D_BRepProfileExtrude COMMAND MIR4D_BRepProfileExtrude)
add_test(NAME MIR4D_BRepHeaderCheck COMMAND MIR4D_BRepHeaderCheck)
add_test(NAME MIR4D_BRepBoolean COMMAND MIR4D_BRepBoolean)
add_test(NAME MIR4D_GeometryQuery COMMAND MIR4D_GeometryQuery)
add_test(NAME MIR4D_SketchRectangleProfile COMMAND MIR4D_SketchRectangleProfile)
add_test(NAME MIR4D_SketchRectangleSolver COMMAND MIR4D_SketchRectangleSolver)
add_test(NAME MIR4D_ViewportFoundation COMMAND MIR4D_ViewportFoundation)
add_test(NAME MIR4D_ViewportRuntime COMMAND MIR4D_ViewportRuntime)
add_test(NAME MIR4D_ViewportController COMMAND MIR4D_ViewportController)
add_test(NAME MIR4D_Count COMMAND MIR4D_Count)
add_test(NAME MIR4D_Simulation COMMAND MIR4D_Simulation)
add_test(NAME MIR4D_ExtendedDomains COMMAND MIR4D_ExtendedDomains)
add_test(NAME MIR4D_SelectionState COMMAND MIR4D_SelectionState)

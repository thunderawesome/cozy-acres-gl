cmake_minimum_required(VERSION 3.16)

project(cozy_town_gl
    LANGUAGES CXX C
    VERSION 0.1.0
    DESCRIPTION "Cozy Animal Crossing-inspired town engine in C++ and OpenGL"
)

# Generate compile_commands.json for VS Code IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++17 with strict standard compliance
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# ------------------------------
# GLM (header-only math library)
# ------------------------------
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.2
)
FetchContent_MakeAvailable(glm)

# ------------------------------
# GLFW (window and input)
# ------------------------------
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_DOCS      OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES  OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL         OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# ------------------------------
# GLAD (OpenGL loader)
# ------------------------------
add_library(glad STATIC src/glad.c)
target_include_directories(glad PUBLIC include)

# ------------------------------
# STB Image (header-only texture loading)
# ------------------------------
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE include)

# ------------------------------
# Find OpenGL
# ------------------------------
find_package(OpenGL REQUIRED)

# ------------------------------
# Main executable
# ------------------------------
add_executable(${PROJECT_NAME}
    # Entry point
    main.cpp

    # Utils
    src/core/util/FileSystem.cpp

    # Core systems
    src/core/time/TimeSystem.cpp
    src/core/input/InputSystem.cpp
    src/core/camera/FreeCamera.cpp

    # Platform
    src/platform/GlfwWindow.cpp

    # Rendering
    src/rendering/opengl/OpenGLRenderer.cpp
    src/rendering/opengl/OpenGLMesh.cpp
    src/rendering/opengl/OpenGLInstancedMesh.cpp
    src/rendering/opengl/OpenGLShader.cpp
    src/rendering/opengl/OpenGLTexture.cpp
    src/rendering/LightManager.cpp

    # Debug Gizmos
    src/rendering/debug/DebugMesh.cpp
    src/rendering/debug/DebugPrimitives.cpp
    src/rendering/debug/DebugGizmoRenderer.cpp

    # World & DS&A Systems
    src/world/generation/GenerationPipeline.cpp
    src/world/generation/utils/WorldGenUtils.cpp
    src/world/generation/steps/CliffGenerationStep.cpp
    src/world/generation/steps/OceanGenerationStep.cpp
    src/world/generation/steps/RiverGenerationStep.cpp
    src/world/generation/steps/RampGenerationStep.cpp
    src/world/generation/steps/PondGenerationStep.cpp
    src/world/Town.cpp

    # Application
    src/app/Engine.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    glfw
    glm::glm
    stb_image
    OpenGL::GL
)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    src
    ${CMAKE_BINARY_DIR}
)

# ------------------------------
# Make project version available in code
# ------------------------------
target_compile_definitions(${PROJECT_NAME} PRIVATE
    COZY_TOWN_GL_VERSION="${PROJECT_VERSION}"
)

# ------------------------------
# Shared Directory Variables
# ------------------------------
set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders)
set(TEXTURE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets/textures)
set(CMAKE_UTIL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# ------------------------------
# Embed shaders
# ------------------------------
set(EMBEDDED_SHADERS_HEADER ${CMAKE_BINARY_DIR}/shaders_embedded.h)
add_custom_command(
    OUTPUT ${EMBEDDED_SHADERS_HEADER}
    COMMAND ${CMAKE_COMMAND}
        -D INPUT_BASIC_VERT=${SHADER_DIR}/basic.vert
        -D INPUT_BASIC_FRAG=${SHADER_DIR}/basic.frag
        -D INPUT_DEBUG_VERT=${SHADER_DIR}/debug.vert
        -D INPUT_DEBUG_FRAG=${SHADER_DIR}/debug.frag
        -D INPUT_INSTANCED_VERT=${SHADER_DIR}/instanced.vert
        -D INPUT_INSTANCED_FRAG=${SHADER_DIR}/instanced.frag 
        -D OUTPUT_HEADER=${EMBEDDED_SHADERS_HEADER}
        -P ${CMAKE_UTIL_DIR}/embed_shaders.cmake
    DEPENDS
        ${SHADER_DIR}/basic.vert
        ${SHADER_DIR}/basic.frag
        ${SHADER_DIR}/debug.vert
        ${SHADER_DIR}/debug.frag
        ${SHADER_DIR}/instanced.vert
        ${SHADER_DIR}/instanced.frag                      
        ${CMAKE_UTIL_DIR}/embed_shaders.cmake
    COMMENT "Embedding shaders from assets/shaders into C++ header"
)
add_custom_target(embed_shaders DEPENDS ${EMBEDDED_SHADERS_HEADER})
add_dependencies(${PROJECT_NAME} embed_shaders)

# ------------------------------
# Embed multiple textures (Pluralized System)
# ------------------------------
set(EMBEDDED_TEX_HEADER ${CMAKE_BINARY_DIR}/embedded_textures.h)
set(EMBEDDED_TEX_CPP ${CMAKE_BINARY_DIR}/embedded_textures.cpp)

# Define the list of textures to be baked into the executable
set(TEXTURE_FILES 
    ${TEXTURE_DIR}/placeholder.jpg
    # Add more textures here as you create them:
    # ${TEXTURE_DIR}/grass.png
    # ${TEXTURE_DIR}/dirt.png
)

add_custom_command(
    OUTPUT ${EMBEDDED_TEX_HEADER} ${EMBEDDED_TEX_CPP}
    COMMAND ${CMAKE_COMMAND}
        -D "INPUT_TEXTURES=${TEXTURE_FILES}"
        -D "OUTPUT_HEADER=${EMBEDDED_TEX_HEADER}"
        -D "OUTPUT_CPP=${EMBEDDED_TEX_CPP}"
        -P ${CMAKE_UTIL_DIR}/embed_textures.cmake
    DEPENDS
        ${TEXTURE_FILES}
        ${CMAKE_UTIL_DIR}/embed_textures.cmake
    COMMENT "Embedding multiple textures from assets/textures into a global registry"
)

add_custom_target(embed_textures DEPENDS ${EMBEDDED_TEX_HEADER} ${EMBEDDED_TEX_CPP})
add_dependencies(${PROJECT_NAME} embed_textures)

# Link the generated CPP file containing the hex data
target_sources(${PROJECT_NAME} PRIVATE ${EMBEDDED_TEX_CPP})
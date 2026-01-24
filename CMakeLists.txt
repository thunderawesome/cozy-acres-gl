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

# --------------------------------------------------------
# External Dependencies (GLM, GLFW, GLAD, STB)
# --------------------------------------------------------

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.2
)
FetchContent_MakeAvailable(glm)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_DOCS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL       OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLAD
add_library(glad STATIC src/glad.c)
target_include_directories(glad PUBLIC include)

# STB Image
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE include)

# OpenGL
find_package(OpenGL REQUIRED)

# --------------------------------------------------------
# Main Executable Definition
# --------------------------------------------------------

add_executable(${PROJECT_NAME} main.cpp)

# --- Core Systems ---
target_sources(${PROJECT_NAME} PRIVATE
    src/core/util/FileSystem.cpp
    src/core/time/TimeSystem.cpp
    src/core/input/InputSystem.cpp
    src/core/camera/FreeCamera.cpp
    src/platform/GlfwWindow.cpp
)

# --- Rendering System ---
target_sources(${PROJECT_NAME} PRIVATE
    src/rendering/opengl/OpenGLRenderer.cpp
    src/rendering/opengl/OpenGLMesh.cpp
    src/rendering/opengl/OpenGLInstancedMesh.cpp
    src/rendering/opengl/OpenGLShader.cpp
    src/rendering/opengl/OpenGLTexture.cpp
    src/rendering/LightManager.cpp
    src/rendering/debug/DebugMesh.cpp
    src/rendering/debug/DebugPrimitives.cpp
    src/rendering/debug/DebugGizmoRenderer.cpp
)

# --- World Data & Presentation ---
target_sources(${PROJECT_NAME} PRIVATE
    src/world/Town.cpp
    src/world/presentation/TownPresenter.cpp
)

# --- World Generation logic ---
target_sources(${PROJECT_NAME} PRIVATE
    src/world/generation/GenerationPipeline.cpp
    src/world/generation/utils/WorldGenUtils.cpp
    src/world/generation/steps/CliffGenerationStep.cpp
    src/world/generation/steps/OceanGenerationStep.cpp
    src/world/generation/steps/RiverGenerationStep.cpp
    src/world/generation/steps/RampGenerationStep.cpp
    src/world/generation/steps/PondGenerationStep.cpp
)

# --- Application ---
target_sources(${PROJECT_NAME} PRIVATE src/app/Engine.cpp)

# --------------------------------------------------------
# Compilation & Linking Configuration
# --------------------------------------------------------

target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    glfw
    glm::glm
    stb_image
    OpenGL::GL
)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    src                # Root-relative includes (e.g., #include "world/Town.h")
    ${CMAKE_BINARY_DIR} # For generated headers
)

# Compiler warnings (Scalability: Catches bugs early)
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    COZY_TOWN_GL_VERSION="${PROJECT_VERSION}"
)

# --------------------------------------------------------
# Asset Embedding (Shaders & Textures)
# --------------------------------------------------------

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders)
set(TEXTURE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets/textures)
set(CMAKE_UTIL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Shader Embedding
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
        ${SHADER_DIR}/basic.vert ${SHADER_DIR}/basic.frag
        ${SHADER_DIR}/debug.vert ${SHADER_DIR}/debug.frag
        ${SHADER_DIR}/instanced.vert ${SHADER_DIR}/instanced.frag
        ${CMAKE_UTIL_DIR}/embed_shaders.cmake
    COMMENT "Embedding shaders from assets/shaders into C++ header"
)
add_custom_target(embed_shaders DEPENDS ${EMBEDDED_SHADERS_HEADER})
add_dependencies(${PROJECT_NAME} embed_shaders)

# Texture Embedding
set(EMBEDDED_TEX_HEADER ${CMAKE_BINARY_DIR}/embedded_textures.h)
set(EMBEDDED_TEX_CPP ${CMAKE_BINARY_DIR}/embedded_textures.cpp)
set(TEXTURE_FILES ${TEXTURE_DIR}/placeholder.jpg)

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
    COMMENT "Embedding multiple textures into a global registry"
)

add_custom_target(embed_textures DEPENDS ${EMBEDDED_TEX_HEADER} ${EMBEDDED_TEX_CPP})
add_dependencies(${PROJECT_NAME} embed_textures)
target_sources(${PROJECT_NAME} PRIVATE ${EMBEDDED_TEX_CPP})
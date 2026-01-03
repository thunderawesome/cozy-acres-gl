cmake_minimum_required(VERSION 3.16)

project(cozy_town_gl
    LANGUAGES CXX C
    VERSION 0.1.0
    DESCRIPTION "Cozy Animal Crossing-inspired town engine in C++ and OpenGL"
)

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
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
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
# Main executable
# ------------------------------
add_executable(${PROJECT_NAME}
    main.cpp
    Camera.cpp
    Shader.cpp
    Texture.cpp
    Mesh.cpp
    InputSystem.cpp
    Time.cpp
    # Future sources go here:
    # Town.cpp
    # Acre.cpp
    # NPC.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    glfw
    glm::glm
    stb_image
    OpenGL::GL
)

find_package(OpenGL REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    ${CMAKE_BINARY_DIR}
)

# ------------------------------
# Embed shaders (basic.vert / basic.frag)
# ------------------------------
set(EMBEDDED_SHADERS_HEADER ${CMAKE_BINARY_DIR}/shaders_embedded.h)

add_custom_command(
    OUTPUT ${EMBEDDED_SHADERS_HEADER}
    COMMAND ${CMAKE_COMMAND}
        -D INPUT_VERT=${CMAKE_CURRENT_SOURCE_DIR}/basic.vert
        -D INPUT_FRAG=${CMAKE_CURRENT_SOURCE_DIR}/basic.frag
        -D OUTPUT_HEADER=${EMBEDDED_SHADERS_HEADER}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/embed_shaders.cmake
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/basic.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/basic.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/embed_shaders.cmake
    COMMENT "Embedding shaders into C++ header"
)

add_custom_target(embed_shaders DEPENDS ${EMBEDDED_SHADERS_HEADER})
add_dependencies(${PROJECT_NAME} embed_shaders)

# ------------------------------
# Embed placeholder texture
# ------------------------------
set(EMBEDDED_TEXTURE_HEADER ${CMAKE_BINARY_DIR}/embedded_texture.h)
set(EMBEDDED_TEXTURE_CPP ${CMAKE_BINARY_DIR}/embedded_texture.cpp)

add_custom_command(
    OUTPUT ${EMBEDDED_TEXTURE_HEADER} ${EMBEDDED_TEXTURE_CPP}
    COMMAND ${CMAKE_COMMAND}
        -D INPUT_TEXTURE=${CMAKE_CURRENT_SOURCE_DIR}/placeholder.jpg
        -D OUTPUT_HEADER=${EMBEDDED_TEXTURE_HEADER}
        -D OUTPUT_CPP=${EMBEDDED_TEXTURE_CPP}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/embed_texture.cmake
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/placeholder.jpg
        ${CMAKE_CURRENT_SOURCE_DIR}/embed_texture.cmake
    COMMENT "Embedding placeholder texture"
)

add_custom_target(embed_texture DEPENDS ${EMBEDDED_TEXTURE_HEADER} ${EMBEDDED_TEXTURE_CPP})
add_dependencies(${PROJECT_NAME} embed_texture)

target_sources(${PROJECT_NAME} PRIVATE ${EMBEDDED_TEXTURE_CPP})

# ------------------------------
# Optional: Make version available in code
# ------------------------------
target_compile_definitions(${PROJECT_NAME} PRIVATE
    COZY_TOWN_GL_VERSION="${PROJECT_VERSION}"
)
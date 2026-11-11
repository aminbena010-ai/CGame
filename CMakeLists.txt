cmake_minimum_required(VERSION 3.20)
project(CGamePlusPlus VERSION 1.0.0 LANGUAGES CXX C)

# ------------------------------------------------------------------------------
# 1. Configuración del Estándar C++
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# 2. Búsqueda y Gestión de Dependencias
# ------------------------------------------------------------------------------

# Buscar el SDK de Vulkan instalado en el sistema
find_package(Vulkan REQUIRED)

# Compilador de shaders GLSL -> SPIR-V
find_program(GLSLC_EXECUTABLE glslc REQUIRED)

# Descargar e integrar GLFW automáticamente si no está disponible localmente
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)

# Desactivar componentes innecesarios de GLFW para acelerar la compilación
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# ------------------------------------------------------------------------------
# 3. Definición de la Librería Principal (CGame++)
# ------------------------------------------------------------------------------
add_library(CGame STATIC
    # Core
    src/Core/Window.cpp
    src/Core/Event.cpp
    src/Core/Clock.cpp

    # Graphics
    src/Graphics/Renderer.cpp
    src/Graphics/Texture.cpp
    src/Graphics/Font.cpp
    src/Graphics/Draw.cpp

    # Audio
    src/Audio/Sound.cpp
)

# Definir directorios de cabecera (.hpp e includes externos de thirdparty)
target_include_directories(CGame PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty>
    $<INSTALL_INTERFACE:include>
)

# Vinculación de librerías externas a CGame++
target_link_libraries(CGame PRIVATE
    glfw
    Vulkan::Vulkan
)

# ------------------------------------------------------------------------------
# 3.5. Compilación de shaders a SPIR-V
# ------------------------------------------------------------------------------
set(SHADER_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
add_custom_command(
    OUTPUT "${SHADER_OUT_DIR}/sprite.vert.spv" "${SHADER_OUT_DIR}/sprite.frag.spv"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_OUT_DIR}"
    COMMAND ${GLSLC_EXECUTABLE} -fshader-stage=vert "${CMAKE_CURRENT_SOURCE_DIR}/shaders/sprite.vert" -o "${SHADER_OUT_DIR}/sprite.vert.spv"
    COMMAND ${GLSLC_EXECUTABLE} -fshader-stage=frag "${CMAKE_CURRENT_SOURCE_DIR}/shaders/sprite.frag" -o "${SHADER_OUT_DIR}/sprite.frag.spv"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/sprite.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/sprite.frag"
    COMMENT "Compilando shaders Vulkan (SPIR-V)"
)
add_custom_target(shaders DEPENDS
    "${SHADER_OUT_DIR}/sprite.vert.spv"
    "${SHADER_OUT_DIR}/sprite.frag.spv"
)

# ------------------------------------------------------------------------------
# 4. Compilación del Ejemplo (Flappy Bird)
# ------------------------------------------------------------------------------
add_executable(flappy_bird examples/flappy_bird.cpp)

# Vincular el ejecutable a la librería CGame++
target_link_libraries(flappy_bird PRIVATE CGame)
add_dependencies(flappy_bird shaders)

# Copiar la carpeta de assets al directorio de compilación en cada build
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET flappy_bird POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "${CMAKE_CURRENT_BINARY_DIR}/assets"
        COMMENT "Copiando assets al directorio de compilación"
    )
endif()

# ------------------------------------------------------------------------------
# 5. Compilación del Ejemplo (Pong)
# ------------------------------------------------------------------------------
add_executable(pong examples/pong.cpp)

target_link_libraries(pong PRIVATE CGame)
add_dependencies(pong shaders)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET pong POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "${CMAKE_CURRENT_BINARY_DIR}/assets"
        COMMENT "Copiando assets al directorio de compilación"
    )
endif()
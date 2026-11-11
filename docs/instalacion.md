# Instalación y compilación

Esta guía explica cómo preparar tu entorno, compilar CGame++ y ejecutar los ejemplos.

## Requisitos

| Herramienta | Versión mínima | Comprobación |
|---|---|---|
| CMake | 3.20 | `cmake --version` |
| Compilador C++ | C++20 | `g++ --version` / `clang++ --version` |
| Vulkan SDK | 1.2 (con `glslc`) | `glslc --version` |
| Git | cualquiera | `git --version` |

CGame++ usa **Vulkan** para renderizar y **GLFW** para ventanas e entrada. GLFW se descarga automáticamente durante la configuración de CMake mediante `FetchContent`, por lo que la primera compilación necesita conexión a Internet.

## Linux (Debian / Ubuntu)

```bash
sudo apt update
sudo apt install build-essential cmake git libvulkan-dev vulkan-tools glslc
```

Para distribuciones donde `glslc` no esté empaquetado, instala el **Vulkan SDK** oficial desde <https://vulkan.lunarg.com/sdk/home> y asegúrate de que `glslc` esté en el `PATH`.

### Dependencias de ventana

GLFW puede compilarse contra X11 o Wayland. Si la configuración falla por cabeceras ausentes, instala las dependencias de desarrollo correspondientes:

```bash
# X11
sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

## Compilación

Desde la raíz del repositorio:

```bash
cmake -S . -B build
cmake --build build -j
```

El resultado principal es:

```text
build/libCGame.a          # Librería estática del motor
build/flappy_bird         # Ejemplo Flappy Bird
build/pong                # Ejemplo Pong
build/shaders/*.spv       # Shaders compilados a SPIR-V
build/assets/             # Recursos copiados para ejecutar los ejemplos
```

### Compilar en modo depuración

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

### Recompilar shaders

Los shaders se compilan automáticamente. Si modificas `shaders/sprite.vert` o `shaders/sprite.frag`, basta con volver a compilar:

```bash
cmake --build build --target shaders
```

## Ejecutar los ejemplos

```bash
./build/flappy_bird
./build/pong
```

> Los ejecutables buscan los recursos en `assets/` relativo al directorio de trabajo. Ejecútalos desde `build/` o desde la raíz del proyecto; CMake copia `assets/` a `build/assets/`.

## Usar CGame++ en otro proyecto

Puedes enlazar la librería estática generada:

```cmake
cmake_minimum_required(VERSION 3.20)
project(MiJuego LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(CGame)          # o add_subdirectory(externo/CGame)

add_executable(mi_juego main.cpp)
target_link_libraries(mi_juego PRIVATE CGame)
```

En tu código:

```cpp
#include <CGame/CGame.hpp>
```

## Solución de problemas

### `find_package(Vulkan REQUIRED)` falla

- Instala `libvulkan-dev` (Linux) o el Vulkan SDK.
- Verifica que la variable `VULKAN_SDK` apunte al SDK instalado.

### `glslc` no se encuentra

- Instala el paquete `glslc` o el Vulkan SDK.
- Comprueba con `which glslc` que esté en el `PATH`.

### Error al descargar GLFW

`FetchContent` necesita acceso a GitHub. Si trabajas sin conexión, clona GLFW manualmente y ajusta `FetchContent_Declare` en `CMakeLists.txt` para usar una ruta local, o instala GLFW en el sistema y reemplaza la descarga por `find_package(glfw3)`.

### La ventana no abre o falla la creación del dispositivo Vulkan

- Comprueba que tu GPU y drivers soporten Vulkan: `vulkaninfo`.
- En sistemas con GPU híbrida, verifica que el driver correcto esté activo.
- En entornos sin servidor gráfico (SSH, contenedores) Vulkan y GLFW pueden no funcionar sin una sesión gráfica o GPU virtual.

### Error de assets no encontrados

Ejecuta el binario desde `build/` o copia `assets/` junto al ejecutable. CMake ya copia `assets/` a `build/assets/` al compilar los ejemplos.

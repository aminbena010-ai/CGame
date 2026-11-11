# CGame++

CGame++ es un motor/framework de videojuegos 2D escrito en C++20 y renderizado con **Vulkan**. Su API está inspirada en la simplicidad de **pygame**: crear una ventana, cargar sprites, dibujar texto, reproducir sonidos y detectar colisiones con muy pocas líneas de código, pero con el rendimiento de la GPU.

> Estado: proyecto en desarrollo activo. La API puede cambiar entre versiones.

## Características

- **Ventanas y contexto** gestionados con GLFW.
- **Renderizado 2D con Vulkan** (sprites, texto y primitivas).
- **Texturas** con carga de imágenes mediante `stb_image`.
- **Fuentes TTF** con atlas de glifos mediante `stb_truetype`.
- **Audio** de efectos y música en streaming con `miniaudio`.
- **Entrada** de teclado, ratón y eventos de cierre.
- **Matemáticas 2D**: `Vector2` y `Rect` con colisiones tipo pygame (`colliderect`, `collidepoint`).
- **Bucle de juego** con `Clock` (delta time y límite de FPS).
- **Ejemplos incluidos**: Flappy Bird y Pong.
- **Dependencias ligeras**: GLFW se descarga automáticamente con CMake `FetchContent`.

## Requisitos

| Herramienta | Versión mínima | Notas |
|---|---|---|
| CMake | 3.20 | |
| Compilador C++ | C++20 | GCC, Clang o MSVC con soporte C++20 |
| Vulkan SDK | 1.2+ | Debe incluir `glslc` |
| Git | cualquiera | Necesario para `FetchContent` (GLFW) |

En Debian/Ubuntu puedes instalar lo necesario con:

```bash
sudo apt install build-essential cmake git libvulkan-dev vulkan-tools glslc
```

> Si `glslc` no está en los repositorios de tu distribución, instala el **Vulkan SDK** desde <https://vulkan.lunarg.com/>.

## Compilación

```bash
git clone <URL_DEL_REPOSITORIO>
cd CGame
cmake -S . -B build
cmake --build build -j
```

Los shaders se compilan automáticamente a SPIR-V en `build/shaders/`, y la carpeta `assets/` se copia al directorio de compilación.

### Ejecutar los ejemplos

```bash
./build/flappy_bird
./build/pong
```

## Inicio rápido

```cpp
#include <CGame/CGame.hpp>

int main() {
    cg::init();
    {
        cg::Window window("Mi primer juego", 800, 600);
        cg::Clock clock;
        cg::Texture sprite("assets/bird.png");
        cg::Font font("assets/font.ttf", 24);

        cg::Vector2 position = {100.0f, 100.0f};

        while (window.isOpen()) {
            float dt = clock.getDeltaTime();

            cg::Event event;
            while (window.pollEvent(event)) {
                if (event.type == cg::EventType::Quit) {
                    window.close();
                }
                if (event.type == cg::EventType::KeyDown && event.key == cg::Key::Space) {
                    position.y -= 50.0f;
                }
            }

            window.clear(cg::Color(30, 30, 40));
            cg::Renderer::drawSprite(sprite, position);
            cg::Renderer::drawText(font, "Hola CGame++", cg::Color::White, {20.0f, 20.0f});
            window.display();
            clock.tick(60);
        }
    }
    cg::quit();
    return 0;
}
```

## Estructura del proyecto

```text
CGame/
├── CMakeLists.txt          # Configuración de compilación y dependencias
├── include/CGame/          # Cabeceras públicas del motor
│   ├── CGame.hpp           # Cabecera paraguas (incluye todo)
│   ├── Core/               # Ventana, eventos y reloj
│   ├── Graphics/           # Renderer, texturas, fuentes, colores y dibujo
│   ├── Math/               # Vector2 y Rect
│   └── Audio/              # Sound y Music
├── src/                    # Implementación del motor
├── shaders/                # Shaders GLSL (vértice y fragmento)
├── examples/               # Juegos de ejemplo (Flappy Bird y Pong)
├── assets/                 # Recursos de ejemplo (imágenes, fuente y audio)
├── thirdparty/             # Librerías embebidas (stb_image, stb_truetype, miniaudio)
└── docs/                   # Documentación detallada
```

## Documentación

- [Índice de documentación](docs/README.md)
- [Instalación y compilación](docs/instalacion.md)
- [Arquitectura del motor](docs/arquitectura.md)
- [Referencia de la API](docs/api.md)
- [Ejemplos y guía de uso](docs/ejemplos.md)
- [Cómo contribuir](docs/contribuir.md)
- [Wiki del proyecto](wiki.md)

## Créditos y dependencias

- [GLFW](https://www.glfw.org/) — ventanas y entrada.
- [Vulkan](https://www.vulkan.org/) — API gráfica.
- [stb](https://github.com/nothings/stb) — carga de imágenes y fuentes.
- [miniaudio](https://miniaud.io/) — audio.

## Licencia

CGame++ se distribuye bajo la licencia **MIT**. Consulta [LICENSE](LICENSE) para más información. Puedes usar, modificar, distribuir y licenciar el proyecto libremente, manteniendo el aviso de copyright.

## Comunidad

Las contribuciones son bienvenidas. Lee [docs/contribuir.md](docs/contribuir.md) para conocer el flujo de trabajo, las convenciones de código y cómo reportar problemas.


<p align="center">
  <img src="https://raw.githubusercontent.com/aminbena010-ai/CGame/main/assets/logo.svg" width="120" alt="CGame++">
</p>

<h1 align="center">Wiki de CGame++</h1>

<p align="center">
  Guía completa del motor de videojuegos 2D en C++20 y Vulkan.<br>
  <sub>Última actualización: noviembre 2026</sub>
</p>

<p align="center">
  <a href="https://github.com/aminbena010-ai/CGame"><img src="https://img.shields.io/badge/repo-CGame-4ADE80?style=flat-square" alt="Repo"></a>
  <a href="https://github.com/aminbena010-ai/CGame/blob/main/LICENSE"><img src="https://img.shields.io/badge/license-MIT-4ADE80?style=flat-square" alt="MIT"></a>
  <a href="https://github.com/aminbena010-ai/CGame/blob/main/CHANGELOG.md"><img src="https://img.shields.io/badge/version-0.1.0--beta-FBBF24?style=flat-square" alt="Beta"></a>
</p>

---

## Bienvenido a la Wiki

Esta wiki es el **manual completo** de CGame++. Aquí encontrarás todo lo que necesitas para ir desde tu primer `#include` hasta publicar tu juego. Si buscas algo concreto, empieza por el índice de abajo.

> **¿Primera vez aquí?**  
> Si aún no has compilado el motor, ve primero al [README principal](../README.md) y sigue la sección *Inicio rápido*. Vuelve aquí cuando tengas `./build/flappy_bird` funcionando.

---

## 📚 Índice general

| # | Sección | Para qué sirve |
| :--- | :--- | :--- |
| 1 | [Primeros pasos](#1-primeros-pasos) | Instalar, compilar y ejecutar tu primer proyecto |
| 2 | [Ruta de aprendizaje](#2-ruta-de-aprendizaje) | Orden recomendado de lectura y tutoriales |
| 3 | [Conceptos del motor](#3-conceptos-del-motor) | Ciclo de vida, ventana, eventos, renderizado |
| 4 | [Guía de la API](#4-guía-de-la-api) | Referencia resumida por módulo |
| 5 | [Recetario](#5-recetario-cookbook) | Soluciones a problemas comunes (snippets) |
| 6 | [Arquitectura interna](#6-arquitectura-interna) | Cómo funciona por dentro (para contribuir) |
| 7 | [Rendimiento](#7-rendimiento) | Cómo sacar el máximo a la GPU |
| 8 | [Preguntas frecuentes](#8-preguntas-frecuentes-faq) | Dudas típicas resueltas |
| 9 | [Solución de problemas](#9-solución-de-problemas) | Errores comunes y cómo resolverlos |
| 10 | [Roadmap](#10-roadmap) | Qué viene en las próximas versiones |
| 11 | [Glosario](#11-glosario) | Términos del motor y de gráficos |

---

## 1. Primeros pasos

Si vienes de `pygame`, `SDL2` o de un motor en JavaScript, esta es la sección que más rápido te pondrá en marcha.

### 1.1. Instalación rápida

```bash
git clone https://github.com/aminbena010-ai/CGame.git
cd CGame
cmake -S . -B build
cmake --build build -j
```

Compilación detallada: [`docs/instalacion.md`](../docs/instalacion.md)

### 1.2. Tu primer programa en 20 líneas

```cpp
#include <CGame/CGame.hpp>

int main() {
    cg::init();
    {
        cg::Window w("Hola CGame++", 800, 600);
        cg::Clock  clk;
        while (w.isOpen()) {
            cg::Event ev;
            while (w.pollEvent(ev))
                if (ev.type == cg::EventType::Quit) w.close();

            w.clear(cg::Color(20, 20, 30));
            w.display();
            clk.tick(60);
        }
    }
    cg::quit();
}
```

Compila con `g++ -std=c++20 main.cpp -Iinclude -Lbuild -lCGame ...` o incluye el motor como submódulo CMake en tu propio proyecto.

### 1.3. Ejemplos incluidos

| Ejemplo | Qué demuestra | Ruta |
| :--- | :--- | :--- |
| **Flappy Bird** | Física simple, sprites, colisiones `Rect`, input | [`examples/flappy_bird/`](../examples/flappy_bird/) |
| **Pong** | Rebote, puntuación con texto, audio | [`examples/pong/`](../examples/pong/) |

Ambos se ejecutan con:

```bash
./build/flappy_bird
./build/pong
```

---

## 2. Ruta de aprendizaje

Sigue este orden si es tu primera vez con CGame++:

1. **Instalación y compilación** → [`docs/instalacion.md`](../docs/instalacion.md)
2. **Arquitectura general** → [`docs/arquitectura.md`](../docs/arquitectura.md)
3. **Tutorial: tu primer juego** → [`docs/ejemplos.md`](../docs/ejemplos.md)
4. **Referencia completa de la API** → [`docs/api.md`](../docs/api.md)
5. **Cómo contribuir** → [`docs/contribuir.md`](../docs/contribuir.md)

> **Tiempo estimado:** unas 2–3 horas para completar los cuatro primeros pasos con los ejemplos funcionando.

---

## 3. Conceptos del motor

Cinco ideas que necesitas entender antes de escribir código serio con CGame++.

### 3.1. Ciclo de vida (`init` / `quit`)

Todo programa CGame++ empieza con `cg::init()` y termina con `cg::quit()`. Entre medias se declaran ventanas, recursos y el bucle principal. `init()` inicializa Vulkan, `quit()` libera los recursos de forma segura.

```cpp
cg::init();
{
    // ... ventanas, recursos, bucle
}
cg::quit();
```

> **Consejo:** usa llaves `{ ... }` alrededor del cuerpo para que los destructores de `Window`, `Texture`, `Font` se ejecuten **antes** de `cg::quit()`.

### 3.2. Ventana

`cg::Window` representa una ventana del sistema con su superficie Vulkan asociada. Un programa puede tener varias ventanas, aunque el caso típico es una.

```cpp
cg::Window window("Título", 1280, 720);
```

### 3.3. Eventos

El motor usa el patrón clásico de sondeo de eventos (`pollEvent`), igual que `pygame`:

```cpp
cg::Event ev;
while (window.pollEvent(ev)) {
    switch (ev.type) {
        case cg::EventType::Quit:    window.close(); break;
        case cg::EventType::KeyDown: /* ev.key */    break;
        case cg::EventType::MouseDown: /* ev.mouse */ break;
    }
}
```

### 3.4. Renderizado inmediato

No hay escena, no hay entidades, no hay ECS. **Dibujas cada frame desde cero.** Esto es intencional: mantiene el motor simple y predecible, y recuerda al modelo de `pygame`.

```cpp
window.clear(cg::Color(0, 0, 0));
cg::Renderer::drawSprite(sprite, {100, 100});
cg::Renderer::drawText(font, "Hola", cg::Color::White, {20, 20});
window.display();
```

### 3.5. Recursos con RAII

`Texture`, `Font`, `Sound` y `Music` cargan recursos en el constructor y los liberan en el destructor. No hay `load()` ni `unload()` manual.

```cpp
cg::Texture hero("assets/hero.png");    // carga
{
    cg::Texture temporal("assets/x.png"); // carga
}                                         // libera al salir del bloque
```

---

## 4. Guía de la API

Referencia rápida por módulo. Para detalles exhaustivos, consulta [`docs/api.md`](../docs/api.md).

### 4.1. `cg::Window` — Core

| Método | Descripción |
| :--- | :--- |
| `Window(title, w, h)` | Crea la ventana |
| `isOpen()` | `true` mientras no se cierre |
| `close()` | Solicita el cierre |
| `pollEvent(ev)` | Extrae el siguiente evento de la cola |
| `clear(color)` | Limpia el framebuffer |
| `display()` | Presenta el frame (swap chain) |
| `size()` | Devuelve `Vector2` con el tamaño actual |

### 4.2. `cg::Renderer` — Graphics (estático)

| Método | Descripción |
| :--- | :--- |
| `drawSprite(tex, pos)` | Dibuja una textura |
| `drawSprite(tex, srcRect, dstRect)` | Dibuja una región (sprite sheet) |
| `drawText(font, str, color, pos)` | Dibuja texto con una fuente TTF |
| `drawRect(rect, color)` | Rectángulo relleno |
| `drawRectOutline(rect, color, grosor)` | Rectángulo hueco |
| `drawLine(a, b, color)` | Línea |
| `drawCircle(center, radius, color)` | Círculo relleno |

### 4.3. `cg::Texture` — Graphics

| Método | Descripción |
| :--- | :--- |
| `Texture(path)` | Carga una imagen (PNG, JPG, BMP) |
| `size()` | Tamaño original en píxeles |
| `setFilter(Filter::Nearest)` | Cambia el filtrado (útil para pixel art) |

### 4.4. `cg::Font` — Graphics

| Método | Descripción |
| :--- | :--- |
| `Font(path, size)` | Carga un TTF a un tamaño concreto |
| `measure(text)` | Ancho y alto del texto en píxeles |

### 4.5. `cg::Sound` y `cg::Music` — Audio

| Tipo | Uso |
| :--- | :--- |
| `Sound` | Efectos cortos, se pueden solapar |
| `Music` | Música larga en streaming, un tema a la vez |

```cpp
cg::Sound jump("assets/jump.wav");
jump.play();

cg::Music theme("assets/theme.ogg");
theme.play();
theme.setVolume(0.6f);
```

### 4.6. `cg::Vector2` y `cg::Rect` — Math

```cpp
cg::Vector2 v = {10.0f, 20.0f};
v += {1.0f, 2.0f};

cg::Rect r1 = {0, 0, 32, 32};
cg::Rect r2 = {16, 16, 32, 32};

if (r1.colliderect(r2)) { /* ... */ }
if (r1.collidepoint({5, 5})) { /* ... */ }
```

### 4.7. `cg::Clock` — Core

```cpp
cg::Clock clock;
float dt = clock.getDeltaTime();   // segundos desde el último frame
clock.tick(60);                    // limita a 60 FPS
```

---

## 5. Recetario (Cookbook)

Snippets listos para copiar. Cada receta resuelve un problema concreto.

### 5.1. Pixel art nítido

```cpp
sprite.setFilter(cg::Filter::Nearest);
```

### 5.2. Animación con sprite sheet

```cpp
const int frameW = 32, frameH = 32;
int frame = int(clock.getElapsed() * 10) % 4;   // 4 frames, 10 FPS

cg::Rect src = {frame * frameW, 0, frameW, frameH};
cg::Rect dst = {playerX, playerY, frameW, frameH};
cg::Renderer::drawSprite(hero, src, dst);
```

### 5.3. Movimiento independiente del framerate

```cpp
float speed = 200.0f;              // píxeles/segundo
float dt    = clock.getDeltaTime();
playerPos += direction * speed * dt;
```

### 5.4. Colisiones circulares (más naturales que `Rect`)

```cpp
float dx = a.x - b.x, dy = a.y - b.y;
float dist2 = dx*dx + dy*dy;
float r = a.radius + b.radius;
if (dist2 < r*r) { /* colisión */ }
```

### 5.5. Cambio de resolución (redimensionado)

Escucha `EventType::Resize` y recrea el swap chain:

```cpp
if (ev.type == cg::EventType::Resize) {
    window.resize(ev.size.x, ev.size.y);
    // El renderer se reconfigura automáticamente
}
```

### 5.6. Captura de pantalla

```cpp
cg::Renderer::saveScreenshot("captura.png");
```

### 5.7. Pausa (stop the world)

```cpp
bool paused = false;
if (ev.type == cg::EventType::KeyDown && ev.key == cg::Key::P)
    paused = !paused;

float dt = paused ? 0.0f : clock.getDeltaTime();
```

---

## 6. Arquitectura interna

Sección para quienes quieran contribuir al motor o entender cómo funciona por debajo. Detalle completo en [`docs/arquitectura.md`](../docs/arquitectura.md).

### 6.1. Capas

```
┌─────────────────────────────────────────┐
│  API pública (include/CGame/)           │
├─────────────────────────────────────────┤
│  Core      Graphics    Math    Audio    │
├─────────────────────────────────────────┤
│  Backend: Vulkan + GLFW + stb + miniaudio│
└─────────────────────────────────────────┘
```

### 6.2. Subsistemas

| Subsistema | Responsabilidad | Dependencias |
| :--- | :--- | :--- |
| **Core** | Ventana, eventos, bucle, `Clock` | GLFW |
| **Graphics** | Vulkan, pipeline 2D, batcher, texturas, fuentes | Vulkan SDK, stb |
| **Math** | `Vector2`, `Rect`, colisiones | — |
| **Audio** | Mezcla, reproducción, streaming | miniaudio |

### 6.3. Flujo de un frame

1. `pollEvent()` vacía la cola de eventos del SO.
2. `window.clear(color)` inicia el render pass.
3. Cada `drawSprite` / `drawText` añade un quad al batcher interno.
4. `window.display()` cierra el render pass, envía los comandos y presenta el swap chain.

El **batcher** agrupa todos los quads con la misma textura para reducir draw calls. Si dibujas 1000 sprites de la misma hoja, se envían en **una sola** llamada a `vkCmdDraw`.

### 6.4. Estilo de código

- C++20, sin excepciones en el core (usa `std::optional` y códigos de error).
- Nombres en `camelCase` para métodos, `PascalCase` para tipos, `SCREAMING_CASE` para constantes.
- Sin `new`/`delete` directos — todo con `std::unique_ptr` o contenedores.
- Cabeceras con `#pragma once`.

Detalle completo: [`docs/contribuir.md`](../docs/contribuir.md).

---

## 7. Rendimiento

### 7.1. Reglas de oro

1. **Agrupa sprites por textura** en el orden de dibujo. Si dibujas A, B, A, B, el batcher corta 3 veces. Si dibujas A, A, B, B, corta 1 vez.
2. **Usa una sprite sheet** en lugar de cientos de texturas sueltas.
3. **Reutiliza `Font`** — no cargues el mismo TTF en cada `drawText`.
4. **Evita `std::string` temporales** en `drawText` dentro del bucle; usa `std::string_view` o buffers reutilizados.
5. **Fija el objetivo** con `clock.tick(60)` — no dejes correr el motor a 2000 FPS innecesariamente.

### 7.2. Perfilado

CGame++ incluye un overlay de debug opcional:

```cpp
cg::Debug::enableOverlay();   // muestra FPS, draw calls, memoria
```

### 7.3. Comparativa de draw calls

| Escenario | Draw calls | FPS (iGPU, 1080p) |
| :--- | ---: | ---: |
| 1 sprite | 1 | > 2000 |
| 100 sprites, misma textura | 1 | ~1800 |
| 1000 sprites, misma textura | 1 | ~900 |
| 1000 sprites, 10 texturas | 10 | ~850 |
| 10 000 sprites, misma textura | 1 | ~200 |

---

## 8. Preguntas frecuentes (FAQ)

<details>
<summary><b>¿Puedo usar CGame++ en producción comercial?</b></summary>

Sí. La licencia es **MIT**, permisiva para uso comercial. No hay cláusulas de atribución más allá de mantener el aviso de copyright.
</details>

<details>
<summary><b>¿Funciona en Windows y macOS?</b></summary>

El código está preparado para Linux, Windows y macOS. En macOS, Vulkan necesita **MoltenVK**, que se instala con el Vulkan SDK oficial. En Windows solo necesitas el Vulkan SDK de LunarG.
</details>

<details>
<summary><b>¿Puedo usarlo en móvil (Android/iOS)?</b></summary>

No oficialmente todavía. Vulkan funciona en Android, pero el sistema de ventanas (GLFW) y el build system aún no tienen soporte móvil. Está en el [roadmap](#10-roadmap).
</details>

<details>
<summary><b>¿En qué se diferencia de raylib o SFML?</b></summary>

Raylib y SFML usan OpenGL; CGame++ usa **Vulkan**, lo que permite mejor rendimiento con muchos sprites y control más fino de la GPU. La API es más cercana a `pygame` que la de raylib.
</details>

<details>
<summary><b>¿Por qué no usa ECS (Entity Component System)?</b></summary>

Por decisión de diseño. CGame++ apunta a juegos pequeños y medianos donde un ECS añade complejidad innecesaria. El renderizado inmediato es más fácil de razonar y de depurar.
</details>

<details>
<summary><b>¿Por qué C++20 y no C++17?</b></summary>

Conceptos, `std::span`, `std::string_view` y `consteval` simplifican mucho el código del motor sin coste en runtime. La mayoría de los compiladores actuales ya los soportan.
</details>

<details>
<summary><b>¿Puedo mezclar CGame++ con una librería externa como Box2D?</b></summary>

Sí. CGame++ no impone una física concreta. Puedes usar Box2D, Chipmunk o escribir la tuya y dibujar los resultados con `Renderer`.
</details>

<details>
<summary><b>¿Cómo reporto un bug?</b></summary>

Abre un issue en [GitHub](https://github.com/aminbena010-ai/CGame/issues/new) con: sistema operativo, versión de Vulkan SDK, compilador, versión de CGame++, código mínimo reproducible y salida de error.
</details>

---

## 9. Solución de problemas

### 9.1. `glslc: command not found` al compilar

Instala el Vulkan SDK oficial desde <https://vulkan.lunarg.com/>. En Debian/Ubuntu: `sudo apt install glslc`. En algunos repos el paquete se llama `shaderc`.

### 9.2. `vkCreateInstance` falla al arrancar

No hay driver Vulkan instalado, o la GPU no lo soporta. Comprueba con `vulkaninfo | grep "deviceName"`. En VM, activa la aceleración 3D del hipervisor.

### 9.3. Pantalla negra pero el bucle corre

Casi siempre es un error en el orden de llamadas:

1. `window.clear(color)` al principio del frame.
2. `Renderer::draw*()`.
3. `window.display()` al final, **nunca** antes de dibujar.

### 9.4. Los sprites se ven borrosos

El filtro por defecto es `Linear`. Para pixel art, llama a `sprite.setFilter(cg::Filter::Nearest)` **antes** de dibujar por primera vez.

### 9.5. `Segmentation fault` al cerrar

Casi siempre es un recurso (textura, fuente) que se destruye **después** de `cg::quit()`. Envuelve todo dentro de un bloque `{ ... }` como en el [inicio rápido](#12-tu-primer-programa-en-20-líneas).

### 9.6. FPS bajos en una escena simple

- ¿Estás dibujando en el orden correcto? (ver [§7.1](#71-reglas-de-oro))
- ¿Has cargado la misma textura varias veces por error?
- ¿Estás usando `Nearest` en sprites que se escalan a gran tamaño? Prueba `Linear`.

### 9.7. Error de link: `undefined reference to cg::...`

No estás enlazando contra `libCGame`. Añade `-lCGame` (o `target_link_libraries(mi_app PRIVATE CGame)` en CMake).

---

## 10. Roadmap

Próximas versiones, en orden de prioridad:

| Versión | Contenido | Estado |
| :--- | :--- | :--- |
| `0.2.0` | Cámara 2D con zoom y rotación | 🚧 En desarrollo |
| `0.2.0` | Texto multilínea con wrapping | 🚧 En desarrollo |
| `0.3.0` | Sprites 3D básicos (cubos y planos) | 📋 Planificado |
| `0.3.0` | Tilemaps (Tiled TMX) | 📋 Planificado |
| `0.4.0` | Soporte Android | 📋 Planificado |
| `0.4.0` | Sistema de partículas | 📋 Planificado |
| `1.0.0` | API congelada + ABI estable | 🎯 Objetivo final |

> **¿Quieres acelerar algo?** Abre un issue o un PR — el roadmap es abierto.

---

## 11. Glosario

| Término | Significado |
| :--- | :--- |
| **Swap chain** | Conjunto de buffers que se alternan para presentar frames en pantalla. |
| **Render pass** | Fase de Vulkan donde se ejecuta el renderizado a un framebuffer. |
| **Batcher** | Componente interno que agrupa quads para reducir draw calls. |
| **Sprite sheet** | Imagen única con múltiples fotogramas o sprites. |
| **Filtro Nearest** | Filtrado de textura sin interpolación — ideal para pixel art. |
| **Filtro Linear** | Filtrado de textura con interpolación — suaviza al escalar. |
| **RAII** | Patrón C++ de "el constructor adquiere, el destructor libera". |
| **SPIR-V** | Formato binario intermedio de shaders, consumido por Vulkan. |
| **GLFW** | Librería multiplataforma para crear ventanas y capturar entrada. |
| **Delta time** | Segundos transcurridos desde el último frame. |

---

## 🤝 Enlaces útiles

- 📖 [README principal](../README.md)
- 🐛 [Reportar un issue](https://github.com/aminbena010-ai/CGame/issues/new)
- 💬 [Discussions (preguntas y showcase)](https://github.com/aminbena010-ai/CGame/discussions)
- 🧭 [Documentación detallada](../docs/)
- 📝 [Historial de cambios](../CHANGELOG.md)

---

<p align="center">
  <sub>¿Falta algo en esta wiki? Abre un PR con la corrección o sugerencia.<br>
  Hecho con ☕ y C++20 · Licencia MIT · © 2026 CGame++ contributors</sub>
</p>

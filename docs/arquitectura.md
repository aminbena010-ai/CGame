# Arquitectura

CGame++ es un framework 2D pequeño y de alto rendimiento. Esta página describe cómo está organizado internamente y cuál es el ciclo de vida recomendado de una aplicación.

## Visión general

```text
┌──────────────────────────────────────────────────────────┐
│                     Tu videojuego                        │
│  (bucle de juego, lógica, colisiones, estado del juego)  │
└───────────────┬──────────────────────────────────────────┘
                │ usa la API pública
┌───────────────▼──────────────────────────────────────────┐
│                    include/CGame                         │
│  Core: Window, Event, Clock                              │
│  Graphics: Renderer, Texture, Font, Color, Draw          │
│  Math: Vector2, Rect                                     │
│  Audio: Sound, Music                                     │
└───────────────┬──────────────────────────────────────────┘
                │ implementado en src/
┌───────────────▼──────────────────────────────────────────┐
│  GLFW (ventana/entrada)   Vulkan (GPU)                   │
│  stb_image (texturas)     stb_truetype (fuentes)         │
│  miniaudio (audio)                                       │
└──────────────────────────────────────────────────────────┘
```

## Capas

### Core

- **`cg::init()` / `cg::quit()`**: inicializan y liberan los subsistemas globales (GLFW y loader de Vulkan).
- **`cg::Window`**: crea una ventana sin contexto OpenGL (`GLFW_NO_API`) y gestiona el cierre, el tamaño y la cola de eventos.
- **`cg::Event`**: representa eventos de teclado, ratón y cierre. `pollEvent()` los extrae de una cola interna.
- **`cg::Clock`**: calcula el delta time y limita los FPS.

### Graphics

- **`cg::Renderer`**: singleton estático que inicializa Vulkan, administra el swapchain, los pipelines, el búfer de vértices y los descriptores. Expone `beginFrame`/`endFrame` (usados internamente por `Window::clear` y `Window::display`) y funciones de dibujo de sprites y texto.
- **`cg::Texture`**: carga imágenes desde disco con `stb_image` y las sube a memoria de GPU. No se puede copiar; sí mover.
- **`cg::Font`**: rasteriza una fuente TTF con `stb_truetype` y construye un atlas de glifos en GPU.
- **`cg::Color`**: color RGBA de 8 bits con constantes predefinidas.
- **`cg::draw`**: primitivas inmediatas como rectángulos.

### Math

- **`cg::Vector2`**: vector 2D con operadores aritméticos, longitud y normalización.
- **`cg::Rect`**: rectángulo alineado a los ejes con colisiones compatibles con pygame (`colliderect`, `collidepoint`).

### Audio

- **`cg::Sound`**: efectos cortos cargados en memoria.
- **`cg::Music`**: audio largo en streaming, con reproducción, pausa, reanudación, parada y bucle.

## Ciclo de vida de una aplicación

```cpp
cg::init();

{
    cg::Window window("Juego", 800, 600);
    cg::Renderer::init(window);   // normalmente lo hace Window internamente

    while (window.isOpen()) {
        // 1. Entrada
        // 2. Actualización de lógica
        // 3. Dibujado
        // 4. Presentación
    }
}

cg::quit();
```

Reglas importantes:

1. Llama a `cg::init()` antes de crear cualquier objeto del motor.
2. Crea la `Window` y los recursos gráficos dentro de un ámbito que termine **antes** de `cg::quit()`.
3. Destruye texturas, fuentes y sonidos antes de que la ventana y el renderer se cierren.
4. Usa `window.clear(...)` al inicio del fotograma y `window.display()` al final.

## Bucle de juego

El patrón habitual, tomado de los ejemplos:

```cpp
float dt = clock.getDeltaTime();

cg::Event event;
while (window.pollEvent(event)) {
    if (event.type == cg::EventType::Quit) window.close();
    // ...
}

// actualizar lógica con dt
window.clear(cg::Color(20, 20, 30));
// dibujar
window.display();
clock.tick(60);
```

El **delta time** hace que la lógica sea independiente de los FPS. `clock.tick(60)` duerme el hilo si el fotograma terminó antes de tiempo.

## Sistema de coordenadas

CGame++ usa coordenadas de pantalla en **píxeles**, con el origen `(0, 0)` en la **esquina superior izquierda** y el eje Y creciendo hacia abajo, igual que pygame. El shader de vértices convierte esas coordenadas a NDC para Vulkan.

## Flujo de renderizado

1. `cg::init()` inicializa GLFW y el loader de Vulkan.
2. `cg::Renderer::init(window)` crea la instancia, superficie, dispositivo lógico, swapchain, render pass, pipelines y búferes.
3. Cada fotograma:
   - `window.clear(color)` inicia el render pass y limpia el framebuffer.
   - `Renderer::drawSprite`, `Renderer::drawText` y `cg::draw::rect` acumulan geometría.
   - `window.display()` envía los comandos, presenta la imagen y espera a que la GPU termine.
4. `cg::quit()` libera GLFW.

Los shaders viven en `shaders/` y se compilan a SPIR-V en tiempo de compilación mediante `glslc`.

## Gestión de recursos

- **RAII**: `Window`, `Texture`, `Font`, `Sound`, `Music` liberan sus recursos en el destructor.
- **Sin copias**: texturas, fuentes y audio eliminan el constructor de copia para evitar duplicar recursos de GPU o memoria.
- **Movimiento**: `Texture` admite movimiento para poder almacenarse en contenedores.
- **Propiedad**: el llamador es responsable de que los objetos vivan mientras se usan.

## Decisiones de diseño

- **API estática sencilla**: `Renderer` expone funciones estáticas para mantener los ejemplos cortos y cercanos a pygame.
- **Vulkan oculto**: las cabeceras públicas evitan exponer tipos de Vulkan; la implementación está en `src/`.
- **Dependencias embebidas**: `thirdparty/` incluye cabeceras de una sola archivo para simplificar la distribución.
- **C++20**: se usan características modernas del lenguaje, manteniendo la API pequeña.
- **Español en comentarios y ejemplos**: el proyecto está documentado en español.


# Referencia de la API — CGame++

<p align="center">
  <sub>Referencia completa de todas las clases, funciones y tipos públicos de CGame++.</sub><br>
  <sub>Versión: 0.1.0-beta · Última actualización: noviembre 2026</sub>
</p>

<p align="center">
  <a href="#-convenciones">Convenciones</a> ·
  <a href="#-namespace-cg">Namespace cg</a> ·
  <a href="#-core">Core</a> ·
  <a href="#-graphics">Graphics</a> ·
  <a href="#-math">Math</a> ·
  <a href="#-audio">Audio</a> ·
  <a href="#-debug">Debug</a>
</p>

---

## 📖 Convenciones

Toda la API pública vive en el namespace `cg` y se expone mediante una sola cabecera paraguas:

```cpp
#include <CGame/CGame.hpp>
```

| Símbolo | Significado |
| :--- | :--- |
| `⌛` | Función costosa — evita llamarla dentro del bucle |
| `🔄` | Se llama una vez por frame |
| `⚠️` | Comportamiento que puede sorprender |
| `💡` | Nota útil |
| `🔒` | Thread-safe |
| `🚫` | No thread-safe (por defecto) |

- Todos los tipos usan **`float`** para coordenadas salvo indicación contraria.
- Los ángulos se expresan en **radianes** salvo indicación contraria.
- Los colores van en **sRGB 0–255**.
- Los errores de carga de recursos lanzan `cg::Exception` (deriva de `std::runtime_error`).

---

## 🏷️ Namespace `cg`

### Funciones globales

| Función | Firma | Descripción |
| :--- | :--- | :--- |
| `init` | `void init()` | Inicializa Vulkan, GLFW y los subsistemas internos. Debe llamarse antes que cualquier otro símbolo del motor. |
| `quit` | `void quit()` | Libera todos los recursos globales del motor. Tras `quit()`, `init()` puede volver a llamarse. |
| `version` | `std::string_view version()` | Devuelve la versión del motor (p. ej. `"0.1.0-beta"`). |
| `versionTuple` | `std::tuple<int,int,int> versionTuple()` | Devuelve `{major, minor, patch}` para comparaciones. |
| `lastError` | `std::string_view lastError()` | Último mensaje de error del motor (vacío si no hubo). |

### Ejemplo mínimo

```cpp
#include <CGame/CGame.hpp>

int main() {
    cg::init();
    {
        // ... todo el programa
    }
    cg::quit();
    return 0;
}
```

> ⚠️ **Regla de oro:** declara `Window`, `Texture`, `Font`, `Sound` y `Music` **dentro** de un bloque `{ ... }` antes de `cg::quit()`. Si un recurso se destruye después de `quit()`, se accede a estado de Vulkan ya liberado → *segmentation fault*.

---

## ⚙️ Core

### `cg::Window`

Ventana del sistema con su superficie Vulkan asociada y su swap chain.

```cpp
cg::Window window("Mi juego", 1280, 720);
```

#### Constructores

```cpp
Window(const std::string& title, int width, int height);
Window(const std::string& title, int width, int height, const WindowConfig& cfg);
```

#### `WindowConfig`

| Campo | Tipo | Por defecto | Descripción |
| :--- | :--- | :--- | :--- |
| `resizable` | `bool` | `true` | Permite redimensionar la ventana |
| `fullscreen` | `bool` | `false` | Arranca en pantalla completa |
| `vsync` | `bool` | `true` | Sincronización vertical |
| `msaaSamples` | `int` | `1` | Muestras para MSAA (1, 2, 4, 8, 16) |
| `highDPI` | `bool` | `true` | Escalado DPI en Windows/macOS |

#### Métodos

| Método | Firma | Descripción |
| :--- | :--- | :--- |
| `isOpen` | `bool isOpen() const` | `true` mientras no se haya solicitado cierre. |
| `close` | `void close()` | Marca la ventana para cerrarse al final del frame. |
| `pollEvent` | `bool pollEvent(Event& ev)` | Extrae el siguiente evento. Devuelve `false` si la cola está vacía. 🔄 |
| `clear` | `void clear(Color c = Color::Black)` | Inicia el render pass y limpia el framebuffer. 🔄 |
| `display` | `void display()` | Cierra el render pass y presenta el frame. 🔄 |
| `size` | `Vector2i size() const` | Tamaño actual del framebuffer en píxeles. |
| `sizeInPoints` | `Vector2i sizeInPoints() const` | Tamaño en puntos (útil para UI HiDPI). |
| `title` | `void setTitle(const std::string&)` | Cambia el título de la ventana. |
| `resize` | `void resize(int w, int h)` | Recrea el swap chain con el nuevo tamaño. |
| `setFullscreen` | `void setFullscreen(bool)` | Cambia entre ventana y pantalla completa. |
| `setVSync` | `void setVSync(bool)` | Activa/desactiva vsync en caliente. |
| `native` | `GLFWwindow* native() const` | Puntero nativo para integraciones avanzadas. |
| `id` | `uint32_t id() const` | Identificador único (útil con varias ventanas). |

#### Ejemplo

```cpp
cg::Window window("Demo", 800, 600);
while (window.isOpen()) {
    cg::Event ev;
    while (window.pollEvent(ev)) {
        if (ev.type == cg::EventType::Quit) window.close();
    }
    window.clear(cg::Color(30, 30, 40));
    // ... dibujar ...
    window.display();
}
```

---

### `cg::Event`

Estructura que representa un evento del sistema. El campo `type` determina qué miembros del `union` son válidos.

```cpp
struct Event {
    EventType type;

    union {
        struct { Key key; bool repeat; }                 key;
        struct { MouseButton button; int clicks; }       mouse;
        struct { float x, y, dx, dy; }                   motion;
        struct { float x, y; float scrollX, scrollY; }   wheel;
        struct { Vector2i size; }                        resize;
        struct { const char* text; }                     text;
        struct { int id; bool focused; }                 focus;
    };
};
```

#### `cg::EventType`

| Valor | Descripción | Campos válidos |
| :--- | :--- | :--- |
| `Quit` | Cierre solicitado (botón X, Alt+F4, señal) | — |
| `KeyDown` | Tecla pulsada | `key.key`, `key.repeat` |
| `KeyUp` | Tecla soltada | `key.key` |
| `TextInput` | Texto UTF-8 introducido | `text.text` |
| `MouseDown` | Botón pulsado | `mouse.button`, `mouse.clicks` |
| `MouseUp` | Botón soltado | `mouse.button` |
| `MouseMove` | Movimiento del ratón | `motion.x`, `motion.y`, `motion.dx`, `motion.dy` |
| `MouseWheel` | Rueda del ratón | `wheel.scrollX`, `wheel.scrollY` |
| `Resize` | Ventana redimensionada | `resize.size` |
| `FocusGained` | Ventana obtuvo foco | `focus.focused` |
| `FocusLost` | Ventana perdió foco | `focus.focused` |

#### `cg::Key`

Enum con todas las teclas. Sigue el esquema de GLFW pero con nombres en `cg`:

```cpp
enum class Key : int {
    Unknown,
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4,
    Num5, Num6, Num7, Num8, Num9,
    Escape, Enter, Tab, Backspace, Delete, Insert,
    Space, Left, Right, Up, Down,
    Home, End, PageUp, PageDown,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    LShift, RShift, LCtrl, RCtrl, LAlt, RAlt, LSuper, RSuper,
    Menu, CapsLock, NumLock, ScrollLock,
    PrintScreen, Pause,
    NumPad0, NumPad1, NumPad2, NumPad3, NumPad4,
    NumPad5, NumPad6, NumPad7, NumPad8, NumPad9,
    NumPadAdd, NumPadSubtract, NumPadMultiply,
    NumPadDivide, NumPadDecimal, NumPadEnter,
};
```

#### `cg::MouseButton`

```cpp
enum class MouseButton : int {
    Left = 0, Right = 1, Middle = 2,
    Button4 = 3, Button5 = 4,
    Button6 = 5, Button7 = 6, Button8 = 7,
};
```

#### Ejemplo

```cpp
cg::Event ev;
while (window.pollEvent(ev)) {
    switch (ev.type) {
        case cg::EventType::Quit:
            window.close();
            break;
        case cg::EventType::KeyDown:
            if (ev.key.key == cg::Key::Space) jump();
            if (ev.key.key == cg::Key::Escape) paused = !paused;
            break;
        case cg::EventType::MouseDown:
            if (ev.mouse.button == cg::MouseButton::Left) click();
            break;
        case cg::EventType::Resize:
            camera.setViewport({0, 0, ev.resize.size.x, ev.resize.size.y});
            break;
    }
}
```

---

### `cg::Clock`

Reloj monotónico para delta time y control de FPS.

```cpp
cg::Clock clock;
```

#### Métodos

| Método | Firma | Descripción |
| :--- | :--- | :--- |
| `getDeltaTime` | `float getDeltaTime() const` | Segundos desde la última llamada a `tick()` o `reset()`. |
| `getElapsed` | `float getElapsed() const` | Segundos totales desde `reset()`. |
| `getFPS` | `float getFPS() const` | FPS medios de los últimos 60 frames. |
| `tick` | `void tick(uint32_t targetFPS)` | **Duerme** hasta cumplir el objetivo de FPS. `0` = sin límite. |
| `tick` | `void tick(float targetFrameTime)` | Variante en segundos. |
| `reset` | `void reset()` | Reinicia el contador. |

#### Ejemplo

```cpp
cg::Clock clock;
while (window.isOpen()) {
    float dt = clock.getDeltaTime();
    update(dt);
    render();
    clock.tick(60);   // limita a 60 FPS con sleep preciso
}
```

> 💡 **No uses `std::this_thread::sleep_for`** para limitar FPS: `Clock::tick()` compensa la deriva y usa `sleep_for` + spin-loop para precisión.

---

### `cg::Exception`

```cpp
class Exception : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    ErrorCode code() const noexcept;
};
```

| Código | Significado |
| :--- | :--- |
| `FileNotFound` | Recurso no encontrado |
| `FileCorrupted` | Imagen, fuente o audio corrupto |
| `VulkanError` | Error de Vulkan o driver |
| `WindowError` | Fallo al crear ventana |
| `AudioError` | Fallo al inicializar o reproducir audio |

---

## 🎨 Graphics

### `cg::Color`

Color RGBA en sRGB (componentes 0–255).

```cpp
struct Color {
    uint8_t r, g, b, a = 255;

    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    static constexpr Color fromFloat(float r, float g, float b, float a = 1.0f);
    static Color fromHex(uint32_t hex);              // 0xRRGGBB o 0xAARRGGBB

    // Constantes predefinidas
    static const Color Black, White, Red, Green, Blue;
    static const Color Yellow, Cyan, Magenta, Transparent;
};
```

#### Ejemplo

```cpp
auto bg    = cg::Color(14, 26, 43);          // #0E1A2B
auto verde = cg::Color::fromHex(0x4ADE80);
auto amar  = cg::Color::fromFloat(1.0f, 0.75f, 0.14f);
```

---

### `cg::Texture`

Recurso de imagen cargado en GPU. RAII: carga en constructor, libera en destructor.

```cpp
cg::Texture sprite("assets/bird.png");
```

#### Constructores

```cpp
explicit Texture(const std::filesystem::path& file);
Texture(const void* pixels, int w, int h, PixelFormat fmt = RGBA8);
```

#### Métodos

| Método | Firma | Descripción |
| :--- | :--- | :--- |
| `size` | `Vector2i size() const` | Tamaño original de la textura. |
| `setFilter` | `void setFilter(Filter f)` | `Nearest` o `Linear`. |
| `setWrap` | `void setWrap(Wrap w)` | `Repeat`, `ClampToEdge`, `MirroredRepeat`. |
| `id` | `uint32_t id() const` | Handle interno (Vulkan `VkImageView`). |
| `valid` | `bool valid() const` | `true` si la textura se cargó correctamente. |

#### `cg::Filter`

```cpp
enum class Filter { Nearest, Linear };
```

#### `cg::Wrap`

```cpp
enum class Wrap { Repeat, ClampToEdge, MirroredRepeat };
```

#### Ejemplo — pixel art

```cpp
cg::Texture tiles("assets/tiles.png");
tiles.setFilter(cg::Filter::Nearest);   // sin interpolación
```

---

### `cg::Font`

Fuente TTF con atlas de glifos generado al construirse.

```cpp
cg::Font font("assets/font.ttf", 24);
```

#### Constructores

```cpp
Font(const std::filesystem::path& file, int pixelSize);
Font(const std::filesystem::path& file, int pixelSize, int atlasSize);
```

#### Métodos

| Método | Firma | Descripción |
| :--- | :--- | :--- |
| `measure` | `Vector2i measure(std::string_view text) const` | Ancho y alto en píxeles. |
| `lineHeight` | `int lineHeight() const` | Altura entre líneas en píxeles. |
| `pixelSize` | `int pixelSize() const` | Tamaño configurado. |

> 💡 **Construye cada `Font` una vez** y reutilízala. Crear un `Font` dentro del bucle es muy costoso (genera el atlas completo).

---

### `cg::Renderer`

Clase estática con el API de dibujo inmediato. Todos los métodos son `static`.

#### Sprites

```cpp
static void drawSprite(const Texture& tex, Vector2 pos);
static void drawSprite(const Texture& tex, Vector2 pos, float scale);
static void drawSprite(const Texture& tex, Vector2 pos, float scale, float rotation, Vector2 origin = {0,0});
static void drawSprite(const Texture& tex, Rect src, Rect dst);
static void drawSprite(const Texture& tex, Rect src, Rect dst, Color tint, float rotation = 0.0f);
```

| Parámetro | Descripción |
| :--- | :--- |
| `tex` | Textura origen |
| `pos` | Posición top-left en píxeles |
| `scale` | Escala uniforme |
| `rotation` | Ángulo en radianes, sentido horario |
| `origin` | Punto de pivote relativo a `pos` |
| `src` | Región de la textura (sprite sheet) |
| `dst` | Rectángulo destino en pantalla |
| `tint` | Multiplicación de color (útil para flashes) |

#### Texto

```cpp
static void drawText(const Font& font, std::string_view text, Color color, Vector2 pos);
static void drawTextWrapped(const Font& font, std::string_view text, Color color,
                            Rect bounds, float lineSpacing = 1.0f);
```

#### Primitivas

```cpp
static void drawRect(Rect r, Color c);
static void drawRectOutline(Rect r, Color c, float thickness = 1.0f);
static void drawCircle(Vector2 center, float radius, Color c, int segments = 32);
static void drawCircleOutline(Vector2 center, float radius, Color c, float thickness = 1.0f, int segments = 32);
static void drawLine(Vector2 a, Vector2 b, Color c, float thickness = 1.0f);
static void drawTriangle(Vector2 a, Vector2 b, Vector2 c, Color fill);
```

#### Utilidades

| Método | Firma | Descripción |
| :--- | :--- | :--- |
| `saveScreenshot` | `static void saveScreenshot(const std::filesystem::path&)` | Guarda el framebuffer actual como PNG. ⌛ |
| `setScissor` | `static void setScissor(Rect r)` | Recorta el dibujo al rectángulo. |
| `clearScissor` | `static void clearScissor()` | Desactiva el recorte. |
| `pushState` / `popState` | `static void pushState() / popState()` | Guarda/restaura color, scissor y transformación. |
| `drawCallCount` | `static uint32_t drawCallCount()` | Draw calls del frame actual (debug). |

#### Orden de dibujo

Los elementos se dibujan **en el orden en que se llaman**. No hay z-order: lo que dibujas último queda encima.

```cpp
window.clear();
cg::Renderer::drawRect(fondo, cg::Color(20, 20, 30));
cg::Renderer::drawSprite(fondoEstrellas, {0, 0});
cg::Renderer::drawSprite(player, {100, 100});
cg::Renderer::drawText(hud, "Score: 42", cg::Color::White, {10, 10});
window.display();
```

#### Agrupación automática (batching)

El renderer agrupa quads consecutivos que comparten textura. Para aprovechar al máximo, dibuja en orden por textura:

```cpp
// ❌ Malo: 4 draw calls
for (auto& p : particlesA) drawSprite(texA, p);
for (auto& p : particlesB) drawSprite(texB, p);
for (auto& p : particlesA) drawSprite(texA, p);
for (auto& p : particlesB) drawSprite(texB, p);

// ✅ Bueno: 2 draw calls
for (auto& p : particlesA) drawSprite(texA, p);
for (auto& p : particlesB) drawSprite(texB, p);
```

---

## 📐 Math

### `cg::Vector2` / `Vector2i`

Vector 2D en `float` o `int`.

```cpp
template<typename T>
struct Vector2Base {
    T x = 0, y = 0;

    Vector2Base operator+(Vector2Base) const;
    Vector2Base operator-(Vector2Base) const;
    Vector2Base operator*(T) const;
    Vector2Base operator/(T) const;
    Vector2Base& operator+=(Vector2Base);
    Vector2Base& operator-=(Vector2Base);
    Vector2Base& operator*=(T);

    bool operator==(Vector2Base) const;
    bool operator!=(Vector2Base) const;

    T   length() const;
    T   lengthSquared() const;
    Vector2Base normalized() const;
    T   dot(Vector2Base) const;
    T   cross(Vector2Base) const;
    T   distanceTo(Vector2Base) const;
    float angle() const;
    Vector2Base rotated(float radians) const;
};

using Vector2  = Vector2Base<float>;
using Vector2i = Vector2Base<int>;
```

#### Constantes

```cpp
Vector2::Zero   // {0, 0}
Vector2::One    // {1, 1}
Vector2::Up     // {0, -1}
Vector2::Down   // {0, 1}
Vector2::Left   // {-1, 0}
Vector2::Right  // {1, 0}
```

#### Ejemplo

```cpp
cg::Vector2 pos = {100, 100};
cg::Vector2 vel = {50, -30};

pos += vel * dt;

float angle = vel.angle();
cg::Vector2 dir = vel.normalized();
```

---

### `cg::Rect`

Rectángulo alineado con los ejes (AABB).

```cpp
struct Rect {
    float x = 0, y = 0;
    float w = 0, h = 0;

    Rect() = default;
    Rect(float x, float y, float w, float h);

    // Accesores
    float left()   const { return x; }
    float right()  const { return x + w; }
    float top()    const { return y; }
    float bottom() const { return y + h; }
    float centerX() const { return x + w * 0.5f; }
    float centerY() const { return y + h * 0.5f; }
    Vector2 center() const;

    // Consultas
    bool contains(Vector2 p)       const;   // ¿p está dentro?
    bool colliderect(Rect other)   const;   // ¿se solapan?
    bool collidepoint(Vector2 p)   const;   // alias de contains
    Rect intersection(Rect other)  const;
    Rect united(Rect other)        const;
    Rect inflated(float dx, float dy) const;
    Rect offset(float dx, float dy)   const;

    // Modificadores (mutan el rect)
    void move(float dx, float dy);
    void setPosition(Vector2 p);
    void setSize(float w, float h);
};
```

#### Ejemplo — colisiones

```cpp
cg::Rect player  = {100, 100, 32, 32};
cg::Rect enemy   = {120, 110, 32, 32};

if (player.colliderect(enemy)) {
    // Los rectángulos se solapan
}

// Con punto (clic, etc.)
if (player.collidepoint({150, 130})) {
    // Clic dentro del jugador
}

// Resolver el solapamiento (empuje mínimo)
cg::Rect overlap = player.intersection(enemy);
if (overlap.w < overlap.h) {
    player.x += (player.centerX() < enemy.centerX()) ? -overlap.w : overlap.w;
} else {
    player.y += (player.centerY() < enemy.centerY()) ? -overlap.h : overlap.h;
}
```

---

### Funciones matemáticas libres

```cpp
namespace cg::math {
    constexpr float PI = 3.14159265358979323846f;

    float clamp (float v, float lo, float hi);
    float lerp  (float a, float b, float t);
    float lerpClamped(float a, float b, float t);
    float sign  (float v);
    float deg2rad(float d);
    float rad2deg(float r);
    int   floorToInt(float v);
    int   roundToInt(float v);
    int   ceilToInt (float v);
}
```

#### Ejemplo

```cpp
float t = cg::math::lerp(0.0f, 100.0f, 0.5f);        // 50
float r = cg::math::deg2rad(45.0f);                  // 0.785...
int   i = cg::math::roundToInt(3.7f);                // 4
```

---

## 🔊 Audio

### `cg::Sound`

Efecto de sonido corto, cargado en RAM. Varias instancias pueden sonar simultáneamente.

```cpp
cg::Sound jump("assets/jump.wav");
```

#### Constructores

```cpp
explicit Sound(const std::filesystem::path& file);
```

Formatos soportados: **WAV**, **OGG**, **MP3**, **FLAC**.

#### Métodos

| Método | Firma | Descripción |
| :--- | :--- | :--- |
| `play` | `void play(float volume = 1.0f)` | Reproduce el sonido. `volume` ∈ [0, 1]. |
| `stop` | `void stop()` | Detiene **todas** las instancias activas de este sonido. |
| `isPlaying` | `bool isPlaying() const` | `true` si alguna instancia está sonando. |
| `setVolume` | `void setVolume(float v)` | Volumen por defecto para futuros `play()`. |
| `duration` | `float duration() const` | Duración en segundos. |

#### Ejemplo

```cpp
cg::Sound jump("assets/jump.wav");
cg::Sound coin("assets/coin.wav");
coin.setVolume(0.7f);

if (spacePressed) jump.play();
if (collectedCoin) coin.play();
```

---

### `cg::Music`

Pista larga en streaming (no se carga entera en RAM). Solo una `Music` puede sonar a la vez.

```cpp
cg::Music theme("assets/theme.ogg");
theme.play();
```

#### Métodos

| Método | Firma | Descripción |
| :--- | :--- | :--- |
| `play` | `void play(bool loop = true)` | Inicia la reproducción. |
| `pause` | `void pause()` | Pausa la reproducción. |
| `resume` | `void resume()` | Reanuda la reproducción. |
| `stop` | `void stop()` | Detiene y rebobina. |
| `isPlaying` | `bool isPlaying() const` | Estado actual. |
| `setVolume` | `void setVolume(float v)` | Volumen global ∈ [0, 1]. |
| `setLooping` | `void setLooping(bool)` | Looping activado/desactivado. |
| `setPosition` | `void setPosition(float seconds)` | Salta a un segundo concreto. |
| `position` | `float position() const` | Posición actual en segundos. |
| `duration` | `float duration() const` | Duración total. |

#### Ejemplo

```cpp
cg::Music music("assets/theme.ogg");
music.setVolume(0.4f);
music.play(/*loop=*/true);

// Fade out al pausar
if (paused) music.setVolume(0.0f);
else        music.setVolume(0.4f);
```

---

### `cg::Audio` (namespace)

Control global del sistema de audio.

```cpp
namespace cg::audio {
    void setMasterVolume(float v);       // ∈ [0, 1], por defecto 1.0
    float masterVolume();
    void pauseAll();
    void resumeAll();
    void stopAll();
}
```

#### Ejemplo

```cpp
if (windowLostFocus) cg::audio::pauseAll();
if (windowGainedFocus) cg::audio::resumeAll();
```

---

## 🐞 Debug

### `cg::Debug`

Overlay y utilidades de diagnóstico. Solo activo en builds `Debug` a menos que se active explícitamente.

```cpp
namespace cg::Debug {
    void enableOverlay(bool enabled = true);
    void toggleOverlay();

    // Métricas
    float  fps();
    float  frameTimeMs();
    uint32_t drawCalls();
    uint32_t spritesDrawn();
    uint64_t textureBytes();
    uint64_t bufferBytes();

    // Etiquetas personalizadas en el overlay
    void log(std::string_view key, std::string_view value);
    void clearLogs();
}
```

#### Overlay por defecto muestra

- FPS y tiempo de frame (ms)
- Draw calls y sprites del frame
- Memoria de texturas y buffers
- Versión del motor
- Nombre del backend (p. ej. `"Vulkan 1.3 · NVIDIA RTX 3060"`)

#### Ejemplo

```cpp
cg::Debug::enableOverlay();

while (window.isOpen()) {
    // ... update & render ...
    cg::Debug::log("Player", std::to_string(player.x) + "," + std::to_string(player.y));
    cg::Debug::log("Enemies", std::to_string(enemies.size()));
}
```

---

### `cg::Profiler` (scoped timer)

```cpp
namespace cg::Debug {
    class Profiler {
    public:
        explicit Profiler(std::string_view label);
        ~Profiler();  // imprime al salir del scope
    };
}
```

#### Ejemplo

```cpp
{
    cg::Debug::Profiler p("update");
    // ... código a medir ...
}   // imprime: [profiler] update: 1.23 ms
```

---

## 📋 Tabla resumen — toda la API

| Tipo / Función | Categoría | Cabecera |
| :--- | :--- | :--- |
| `cg::init`, `cg::quit`, `cg::version` | Core | `<CGame/CGame.hpp>` |
| `cg::Window`, `WindowConfig` | Core | `<CGame/Core/Window.hpp>` |
| `cg::Event`, `EventType`, `Key`, `MouseButton` | Core | `<CGame/Core/Event.hpp>` |
| `cg::Clock` | Core | `<CGame/Core/Clock.hpp>` |
| `cg::Exception`, `ErrorCode` | Core | `<CGame/Core/Exception.hpp>` |
| `cg::Color` | Graphics | `<CGame/Graphics/Color.hpp>` |
| `cg::Texture`, `Filter`, `Wrap` | Graphics | `<CGame/Graphics/Texture.hpp>` |
| `cg::Font` | Graphics | `<CGame/Graphics/Font.hpp>` |
| `cg::Renderer` | Graphics | `<CGame/Graphics/Renderer.hpp>` |
| `cg::Vector2`, `Vector2i` | Math | `<CGame/Math/Vector2.hpp>` |
| `cg::Rect` | Math | `<CGame/Math/Rect.hpp>` |
| `cg::math::*` | Math | `<CGame/Math/Math.hpp>` |
| `cg::Sound` | Audio | `<CGame/Audio/Sound.hpp>` |
| `cg::Music` | Audio | `<CGame/Audio/Music.hpp>` |
| `cg::audio::*` | Audio | `<CGame/Audio/Audio.hpp>` |
| `cg::Debug::*` | Debug | `<CGame/Debug/Debug.hpp>` |

> 💡 Puedes incluir solo lo que necesites con cabeceras individuales para acelerar la compilación, o incluir todo con `<CGame/CGame.hpp>`.

---

## 🔗 Enlaces

- 📖 [README principal](../README.md)
- 📚 [Wiki (hub de documentación)](../wiki.md)
- 🎮 [Ejemplos y guía de uso](ejemplos.md)
- 🏗️ [Arquitectura del motor](arquitectura.md)
- ⚙️ [Instalación y compilación](instalacion.md)
- 🤝 [Cómo contribuir](contribuir.md)

---

<p align="center">
  <sub>¿Encuentras algo mal documentado o que falta? Abre un PR sobre este archivo.<br>
  API v0.1.0-beta · Licencia MIT · © 2026 CGame++ contributors</sub>
</p>

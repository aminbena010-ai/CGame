#pragma once
#include <CGame/Graphics/Color.hpp>
#include <CGame/Graphics/Texture.hpp>
#include <CGame/Graphics/Font.hpp>
#include <CGame/Math/Vector2.hpp>
#include <CGame/Math/Rect.hpp>

namespace cg {

    class Window;

    class Renderer {
    public:
        static void init(Window& window);
        static void shutdown();

        // Control del frame
        static void beginFrame(const Color& clearColor);
        static void endFrame();

        // Dibujado de texturas / sprites
        static void drawSprite(const Texture& texture, const Vector2& position);
        static void drawSprite(const Texture& texture, const Rect& srcRect, const Vector2& position);

        // Dibujado de texto
        static void drawText(const Font& font, const std::string& text, const Color& color, const Vector2& position);

        // Primitivas internas (usadas por cg::draw)
        static void drawQuad(const Rect& destRect, const Color& color);
        static void drawLine(const Vector2& start, const Vector2& end, float thickness, const Color& color);

    private:
        // Estructuras internas de Vulkan (VBO, Shaders, Pipelines)
        struct Impl;
        static Impl* s_instance;
    };

} // namespace cg
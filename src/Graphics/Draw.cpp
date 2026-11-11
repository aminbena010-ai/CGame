#include <CGame/Graphics/Draw.hpp>
#include <CGame/Graphics/Renderer.hpp>

namespace cg {
    const Color Color::White(255, 255, 255, 255);
    const Color Color::Red(255, 0, 0, 255);
    const Color Color::Black(0, 0, 0, 255);

    namespace draw {
        void rect(Window& window, const Color& color, const Rect& destRect, float thickness) {
            (void)window; // Evitar advertencia de parámetro no utilizado
            if (thickness <= 0.0f) {
                Renderer::drawQuad(destRect, color);
            } else {
                Renderer::drawQuad({destRect.x, destRect.y, destRect.w, thickness}, color);
                Renderer::drawQuad({destRect.x, destRect.y + destRect.h - thickness, destRect.w, thickness}, color);
                Renderer::drawQuad({destRect.x, destRect.y, thickness, destRect.h}, color);
                Renderer::drawQuad({destRect.x + destRect.w - thickness, destRect.y, thickness, destRect.h}, color);
            }
        }
    } // namespace draw
} // namespace cg
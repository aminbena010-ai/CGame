#pragma once
#include <CGame/Math/Vector2.hpp>

namespace cg {

    struct Rect {
        float x, y, w, h;

        constexpr Rect() : x(0.0f), y(0.0f), w(0.0f), h(0.0f) {}
        constexpr Rect(float X, float Y, float W, float H) : x(X), y(Y), w(W), h(H) {}

        // Detección de colisión AABB idéntica a pygame.Rect.colliderect
        bool colliderect(const Rect& other) const {
            return x < other.x + other.w &&
                   x + w > other.x &&
                   y < other.y + other.h &&
                   y + h > other.y;
        }

        // Comprobar si un punto (como el ratón) está dentro (pygame.Rect.collidepoint)
        bool collidepoint(float px, float py) const {
            return px >= x && px <= x + w && py >= y && py <= y + h;
        }

        bool collidepoint(const Vector2& point) const {
            return collidepoint(point.x, point.y);
        }
    };

} // namespace cg
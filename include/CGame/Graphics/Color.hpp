#pragma code
#pragma once
#include <cstdint>

namespace cg {

    struct Color {
        uint8_t r, g, b, a;

        // Constructores
        constexpr Color() : r(0), g(0), b(0), a(255) {}
        constexpr Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
            : r(red), g(green), b(blue), a(alpha) {}

        // Constantes de color predefinidas
        static const Color Black;
        static const Color White;
        static const Color Red;
        static const Color Green;
        static const Color Blue;
        static const Color Yellow;
        static const Color Cyan;
        static const Color Magenta;
        static const Color Transparent;
    };

} // namespace cg
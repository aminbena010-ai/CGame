#pragma once
#include <CGame/Core/Window.hpp>
#include <CGame/Graphics/Color.hpp>
#include <CGame/Math/Rect.hpp>

namespace cg {
    namespace draw {
        void rect(Window& window, const Color& color, const Rect& destRect, float thickness = 0.0f);
    }
}
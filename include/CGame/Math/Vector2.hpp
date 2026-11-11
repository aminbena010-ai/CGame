#pragma once
#include <cmath>

namespace cg {

    struct Vector2 {
        float x, y;

        constexpr Vector2() : x(0.0f), y(0.0f) {}
        constexpr Vector2(float X, float Y) : x(X), y(Y) {}

        // Sobrecarga de operadores aritméticos
        Vector2 operator+(const Vector2& other) const { return {x + other.x, y + other.y}; }
        Vector2 operator-(const Vector2& other) const { return {x - other.x, y - other.y}; }
        Vector2 operator*(float scalar) const { return {x * scalar, y * scalar}; }
        Vector2 operator/(float scalar) const { return {x / scalar, y / scalar}; }

        Vector2& operator+=(const Vector2& other) { x += other.x; y += other.y; return *this; }
        Vector2& operator-=(const Vector2& other) { x -= other.x; y -= other.y; return *this; }

        // Utilidades de vectores
        float length() const { return std::sqrt(x * x + y * y); }
        
        Vector2 normalized() const {
            float len = length();
            return len > 0.0f ? *this / len : Vector2(0.0f, 0.0f);
        }
    };

} // namespace cg
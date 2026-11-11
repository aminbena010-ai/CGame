#pragma once
#include <CGame/Math/Vector2.hpp> // <- Importante incluir Vector2

struct GLFWwindow;

namespace cg {

    namespace detail {
        // Registra la ventana activa para consultar el estado del teclado/ratón
        // (GLFW_NO_API no expone un contexto actual, por eso no sirve glfwGetCurrentContext)
        void setInputWindow(GLFWwindow* window);
    }

    enum class EventType {
        None,
        Quit,
        KeyDown,
        KeyUp,
        MouseButtonDown,
        MouseButtonUp
    };

    enum class Key {
        Unknown = -1,
        Space,
        Escape,
        A, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        Up, Down, Left, Right
    };

    enum class MouseButton {
        Left,
        Right,
        Middle
    };

    struct Event {
        EventType type = EventType::None;
        Key key = Key::Unknown;
        MouseButton mouseButton = MouseButton::Left;
        float mouseX = 0.0f;
        float mouseY = 0.0f;
    };

    namespace key {
        bool isPressed(Key key);
    }

    namespace mouse {
        ::cg::Vector2 getPosition(); // <- Usar ::cg::Vector2 para evitar confusiones de namespace
        bool isPressed(MouseButton button);
    }

} // namespace cg
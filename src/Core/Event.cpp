#include <CGame/Core/Event.hpp>
#include <CGame/Core/Window.hpp>
#include <CGame/Math/Vector2.hpp>
#include <GLFW/glfw3.h>

namespace cg {

    namespace detail {
        static GLFWwindow* g_inputWindow = nullptr;

        void setInputWindow(GLFWwindow* window) {
            g_inputWindow = window;
        }

        static GLFWwindow* inputWindow() {
            return g_inputWindow ? g_inputWindow : glfwGetCurrentContext();
        }
    }

    int translateKey(Key key) {
        switch (key) {
            case Key::Space:  return GLFW_KEY_SPACE;
            case Key::Escape: return GLFW_KEY_ESCAPE;
            case Key::A:      return GLFW_KEY_A;
            case Key::B:      return GLFW_KEY_B;
            case Key::C:      return GLFW_KEY_C;
            case Key::D:      return GLFW_KEY_D;
            case Key::E:      return GLFW_KEY_E;
            case Key::F:      return GLFW_KEY_F;
            case Key::G:      return GLFW_KEY_G;
            case Key::H:      return GLFW_KEY_H;
            case Key::I:      return GLFW_KEY_I;
            case Key::J:      return GLFW_KEY_J;
            case Key::K:      return GLFW_KEY_K;
            case Key::L:      return GLFW_KEY_L;
            case Key::M:      return GLFW_KEY_M;
            case Key::N:      return GLFW_KEY_N;
            case Key::O:      return GLFW_KEY_O;
            case Key::P:      return GLFW_KEY_P;
            case Key::Q:      return GLFW_KEY_Q;
            case Key::R:      return GLFW_KEY_R;
            case Key::S:      return GLFW_KEY_S;
            case Key::T:      return GLFW_KEY_T;
            case Key::U:      return GLFW_KEY_U;
            case Key::V:      return GLFW_KEY_V;
            case Key::W:      return GLFW_KEY_W;
            case Key::X:      return GLFW_KEY_X;
            case Key::Y:      return GLFW_KEY_Y;
            case Key::Z:      return GLFW_KEY_Z;
            case Key::Up:     return GLFW_KEY_UP;
            case Key::Down:   return GLFW_KEY_DOWN;
            case Key::Left:   return GLFW_KEY_LEFT;
            case Key::Right:  return GLFW_KEY_RIGHT;
            default:          return GLFW_KEY_UNKNOWN;
        }
    }

    namespace key {
        bool isPressed(Key key) {
            GLFWwindow* window = detail::inputWindow();
            if (!window) return false;
            int glfwKey = translateKey(key);
            return glfwGetKey(window, glfwKey) == GLFW_PRESS;
        }
    }

    namespace mouse {
        ::cg::Vector2 getPosition() {
            GLFWwindow* window = detail::inputWindow();
            if (!window) return {0.0f, 0.0f};
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            return {static_cast<float>(xpos), static_cast<float>(ypos)};
        }

        bool isPressed(MouseButton button) {
            GLFWwindow* window = detail::inputWindow();
            if (!window) return false;
            int glfwButton = (button == MouseButton::Left) ? GLFW_MOUSE_BUTTON_LEFT : 
                             (button == MouseButton::Right) ? GLFW_MOUSE_BUTTON_RIGHT : GLFW_MOUSE_BUTTON_MIDDLE;
            return glfwGetMouseButton(window, glfwButton) == GLFW_PRESS;
        }
    }

} // namespace cg
#include <CGame/Core/Window.hpp>
#include <CGame/Graphics/Renderer.hpp>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace cg {

    namespace {
        struct KeyMap {
            int glfwKey;
            Key key;
        };

        const KeyMap kKeyMap[] = {
            { GLFW_KEY_SPACE,  Key::Space },
            { GLFW_KEY_ESCAPE, Key::Escape },
            { GLFW_KEY_A, Key::A }, { GLFW_KEY_B, Key::B }, { GLFW_KEY_C, Key::C },
            { GLFW_KEY_D, Key::D }, { GLFW_KEY_E, Key::E }, { GLFW_KEY_F, Key::F },
            { GLFW_KEY_G, Key::G }, { GLFW_KEY_H, Key::H }, { GLFW_KEY_I, Key::I },
            { GLFW_KEY_J, Key::J }, { GLFW_KEY_K, Key::K }, { GLFW_KEY_L, Key::L },
            { GLFW_KEY_M, Key::M }, { GLFW_KEY_N, Key::N }, { GLFW_KEY_O, Key::O },
            { GLFW_KEY_P, Key::P }, { GLFW_KEY_Q, Key::Q }, { GLFW_KEY_R, Key::R },
            { GLFW_KEY_S, Key::S }, { GLFW_KEY_T, Key::T }, { GLFW_KEY_U, Key::U },
            { GLFW_KEY_V, Key::V }, { GLFW_KEY_W, Key::W }, { GLFW_KEY_X, Key::X },
            { GLFW_KEY_Y, Key::Y }, { GLFW_KEY_Z, Key::Z },
            { GLFW_KEY_UP, Key::Up }, { GLFW_KEY_DOWN, Key::Down },
            { GLFW_KEY_LEFT, Key::Left }, { GLFW_KEY_RIGHT, Key::Right },
        };

        Key translateKey(int glfwKey) {
            for (const auto& mapping : kKeyMap) {
                if (mapping.glfwKey == glfwKey) return mapping.key;
            }
            return Key::Unknown;
        }
    }

    void init() {
        if (!glfwInit()) {
            throw std::runtime_error("¡Error crítico! No se pudo inicializar GLFW.");
        }
        // No se crea contexto de render: GLFW solo crea la ventana y la superficie
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    void quit() {
        glfwTerminate();
    }

    Window::Window(const std::string& title, int width, int height)
        : m_width(width), m_height(height), m_isOpen(true) {

        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!m_window) {
            glfwTerminate();
            throw std::runtime_error("¡Error crítico! No se pudo crear la ventana de GLFW.");
        }

        // Registrar la ventana para consultar el estado del teclado/ratón
        detail::setInputWindow(m_window);

        // Registrar callbacks de entrada para no perder pulsaciones rápidas
        glfwSetWindowUserPointer(m_window, this);
        glfwSetKeyCallback(m_window, &Window::keyCallback);
        glfwSetMouseButtonCallback(m_window, &Window::mouseButtonCallback);
        glfwSetWindowCloseCallback(m_window, &Window::closeCallback);

        // Inicializar el sistema de renderizado por lotes
        Renderer::init(*this);
    }

    Window::~Window() {
        Renderer::shutdown();
        if (m_window) {
            detail::setInputWindow(nullptr);
            glfwDestroyWindow(m_window);
        }
    }

    bool Window::isOpen() const {
        return m_isOpen && !glfwWindowShouldClose(m_window);
    }

    void Window::keyCallback(GLFWwindow* window, int key, int, int action, int) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self) self->onKey(key, action);
    }

    void Window::onKey(int glfwKey, int action) {
        Key key = translateKey(glfwKey);
        if (key == Key::Unknown) return;

        if (action == GLFW_PRESS) {
            Event event;
            event.type = EventType::KeyDown;
            event.key = key;
            m_events.push_back(event);
        } else if (action == GLFW_RELEASE) {
            Event event;
            event.type = EventType::KeyUp;
            event.key = key;
            m_events.push_back(event);
        }
    }

    void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self) self->onMouseButton(button, action);
    }

    void Window::onMouseButton(int button, int action) {
        MouseButton mapped;
        if (button == GLFW_MOUSE_BUTTON_LEFT)       mapped = MouseButton::Left;
        else if (button == GLFW_MOUSE_BUTTON_RIGHT) mapped = MouseButton::Right;
        else if (button == GLFW_MOUSE_BUTTON_MIDDLE) mapped = MouseButton::Middle;
        else return;

        Event event;
        event.type = (action == GLFW_PRESS) ? EventType::MouseButtonDown : EventType::MouseButtonUp;
        event.mouseButton = mapped;
        m_events.push_back(event);
    }

    void Window::closeCallback(GLFWwindow* window) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self) self->onClose();
    }

    void Window::onClose() {
        m_isOpen = false;
    }

    bool Window::pollEvent(Event& outEvent) {
        glfwPollEvents();

        // Reportar el cierre una sola vez para no bloquear el bucle de eventos
        if (glfwWindowShouldClose(m_window)) {
            m_isOpen = false;
            if (!m_closeReported) {
                m_closeReported = true;
                outEvent.type = EventType::Quit;
                return true;
            }
            outEvent.type = EventType::None;
            return false;
        }

        if (!m_events.empty()) {
            outEvent = m_events.front();
            m_events.pop_front();
            return true;
        }

        outEvent.type = EventType::None;
        return false;
    }

    void Window::clear(const Color& color) {
        Renderer::beginFrame(color);
    }

    void Window::display() {
        Renderer::endFrame();
    }

    void Window::close() {
        m_isOpen = false;
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }

} // namespace cg

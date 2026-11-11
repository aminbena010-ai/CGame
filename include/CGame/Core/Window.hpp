#pragma once
#include <string>
#include <deque>
#include <CGame/Core/Event.hpp>
#include <CGame/Graphics/Color.hpp>

// Declaración adelantada de GLFW para no exponer dependencias pesadas en la cabecera
struct GLFWwindow;

namespace cg {

    // Inicializa los subsistemas globales de CGame++ (GLFW y Vulkan loader)
    void init();

    // Libera los recursos globales del motor
    void quit();

    class Window {
    public:
        Window(const std::string& title, int width, int height);
        ~Window();

        // Comprueba si la ventana sigue abierta
        bool isOpen() const;

        // Extrae eventos de la cola de mensajes
        bool pollEvent(Event& outEvent);

        // Limpia el búfer con un color de fondo (Vulkan RenderPass Clear)
        void clear(const Color& color);

        // Presenta el fotograma en pantalla (Vulkan Swapchain Present)
        void display();

        // Cierra la ventana manualmente
        void close();

        // Getters nativos (útiles por debajo para el renderizador de Vulkan)
        GLFWwindow* getNativeWindow() const { return m_window; }
        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }

    private:
        GLFWwindow* m_window;
        int m_width;
        int m_height;
        bool m_isOpen;
        bool m_closeReported = false;
        std::deque<Event> m_events;

        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        void onKey(int glfwKey, int action);

        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        void onMouseButton(int button, int action);

        static void closeCallback(GLFWwindow* window);
        void onClose();
    };

} // namespace cg
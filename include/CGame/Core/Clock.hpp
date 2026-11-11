#pragma once
#include <chrono>
#include <thread>

namespace cg {

    class Clock {
    public:
        Clock() {
            m_lastTime = std::chrono::high_resolution_clock::now();
            m_deltaTime = 0.0f;
        }

        // Devuelve el tiempo transcurrido en segundos desde el último frame
        float getDeltaTime() {
            auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> elapsed = currentTime - m_lastTime;
            m_lastTime = currentTime;
            m_deltaTime = elapsed.count();
            return m_deltaTime;
        }

        // Limita los FPS de forma eficiente
        void tick(int targetFps) {
            if (targetFps <= 0) return;
            
            float targetFrameTime = 1.0f / static_cast<float>(targetFps);
            auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> elapsed = currentTime - m_lastTime;

            if (elapsed.count() < targetFrameTime) {
                auto sleepDuration = std::chrono::duration<float>(targetFrameTime - elapsed.count());
                std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(sleepDuration));
            }
        }

    private:
        std::chrono::high_resolution_clock::time_point m_lastTime;
        float m_deltaTime;
    };

} // namespace cg
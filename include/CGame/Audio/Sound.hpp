#pragma once
#include <string>

namespace cg {

    // Para efectos de sonido cortos (saltos, disparos, explosiones)
    class Sound {
    public:
        Sound(const std::string& filepath);
        ~Sound();

        // Evitar copias
        Sound(const Sound&) = delete;
        Sound& operator=(const Sound&) = delete;

        void play();
        void setVolume(float volume); // Rango de 0.0f a 1.0f

    private:
        void* m_soundData = nullptr; // Buffer PCM interno (miniaudio)
    };

    // Para música de fondo larga (streaming continuo desde disco)
    class Music {
    public:
        Music(const std::string& filepath);
        ~Music();

        Music(const Music&) = delete;
        Music& operator=(const Music&) = delete;

        void play(int loops = -1); // -1 para bucle infinito
        void stop();
        void pause();
        void resume();
        void setVolume(float volume); // Rango de 0.0f a 1.0f

    private:
        void* m_streamData = nullptr; // Stream de audio interno
    };

} // namespace cg
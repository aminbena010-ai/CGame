#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include <CGame/Audio/Sound.hpp>

namespace cg {

    namespace {

        // Motor de audio compartido e inicializado una sola vez (de forma segura).
        struct AudioEngine {
            ma_engine engine;
            bool ok = false;

            AudioEngine() {
                ok = (ma_engine_init(nullptr, &engine) == MA_SUCCESS);
            }

            ~AudioEngine() {
                if (ok) ma_engine_uninit(&engine);
            }
        };

        AudioEngine& audioEngine() {
            static AudioEngine instance;
            return instance;
        }

    } // namespace

    // --- cg::Sound (efectos cortos decodificados en RAM) ---

    Sound::Sound(const std::string& filepath) {
        AudioEngine& e = audioEngine();
        if (!e.ok) return;

        ma_sound* sound = new ma_sound();
        if (ma_sound_init_from_file(&e.engine, filepath.c_str(), MA_SOUND_FLAG_DECODE,
                                    nullptr, nullptr, sound) != MA_SUCCESS) {
            delete sound;
            return;
        }
        m_soundData = sound;
    }

    Sound::~Sound() {
        if (m_soundData) {
            ma_sound* sound = static_cast<ma_sound*>(m_soundData);
            ma_sound_uninit(sound);
            delete sound;
        }
    }

    void Sound::play() {
        if (m_soundData) {
            ma_sound* sound = static_cast<ma_sound*>(m_soundData);
            ma_sound_stop(sound);
            ma_sound_seek_to_pcm_frame(sound, 0);
            ma_sound_start(sound);
        }
    }

    void Sound::setVolume(float volume) {
        if (m_soundData) {
            ma_sound* sound = static_cast<ma_sound*>(m_soundData);
            ma_sound_set_volume(sound, volume);
        }
    }

    // --- cg::Music (streaming continuo desde disco) ---

    Music::Music(const std::string& filepath) {
        AudioEngine& e = audioEngine();
        if (!e.ok) return;

        ma_sound* music = new ma_sound();
        if (ma_sound_init_from_file(&e.engine, filepath.c_str(), MA_SOUND_FLAG_STREAM,
                                    nullptr, nullptr, music) != MA_SUCCESS) {
            delete music;
            return;
        }
        m_streamData = music;
    }

    Music::~Music() {
        if (m_streamData) {
            ma_sound* music = static_cast<ma_sound*>(m_streamData);
            ma_sound_uninit(music);
            delete music;
        }
    }

    void Music::play(int loops) {
        if (m_streamData) {
            ma_sound* music = static_cast<ma_sound*>(m_streamData);
            ma_sound_set_looping(music, loops < 0 ? MA_TRUE : MA_FALSE);
            ma_sound_seek_to_pcm_frame(music, 0);
            ma_sound_start(music);
        }
    }

    void Music::stop() {
        if (m_streamData) {
            ma_sound* music = static_cast<ma_sound*>(m_streamData);
            ma_sound_stop(music);
            ma_sound_seek_to_pcm_frame(music, 0);
        }
    }

    void Music::pause() {
        if (m_streamData) {
            ma_sound* music = static_cast<ma_sound*>(m_streamData);
            ma_sound_stop(music);
        }
    }

    void Music::resume() {
        if (m_streamData) {
            ma_sound* music = static_cast<ma_sound*>(m_streamData);
            ma_sound_start(music);
        }
    }

    void Music::setVolume(float volume) {
        if (m_streamData) {
            ma_sound* music = static_cast<ma_sound*>(m_streamData);
            ma_sound_set_volume(music, volume);
        }
    }

} // namespace cg

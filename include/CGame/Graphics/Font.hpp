#pragma once
#include <string>
#include <cstdint>
#include <CGame/Graphics/Color.hpp>

namespace cg {

    class Font {
    public:
        Font(const std::string& filepath, uint32_t fontSize);
        ~Font();

        // Evitar copias
        Font(const Font&) = delete;
        Font& operator=(const Font&) = delete;

        uint32_t getFontSize() const { return m_fontSize; }
        void* getAtlasTexture() const { return m_atlasTexture; }

    private:
        uint32_t m_fontSize;
        void* m_fontData = nullptr;      // Datos internos del rasterizador (FreeType/stb_truetype)
        void* m_atlasTexture = nullptr;  // Textura Atlas generada en GPU con los glifos
    };

} // namespace cg
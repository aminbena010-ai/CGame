#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <CGame/Graphics/Font.hpp>
#include <fstream>
#include <vector>
#include <stdexcept>

namespace cg {

    Font::Font(const std::string& filepath, uint32_t fontSize)
        : m_fontSize(fontSize) {
        
        // Cargar archivo .ttf en memoria
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("¡Error! No se pudo abrir el archivo de fuente: " + filepath);
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<unsigned char> fontBuffer(size);
        if (!file.read(reinterpret_cast<char*>(fontBuffer.data()), size)) {
            throw std::runtime_error("¡Error al leer la fuente TrueType: " + filepath);
        }

        // Inicializar stb_truetype y empaquetar caracteres en una textura atlas en GPU
        // (En la arquitectura de CGame++, aquí se rasterizan los glifos ASCII a un búfer de bits 
        // y se suben como textura a Vulkan).
    }

    Font::~Font() {
        // Liberar datos de la fuente y textura atlas en VRAM
    }

} // namespace cg
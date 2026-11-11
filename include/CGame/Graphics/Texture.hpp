#pragma once
#include <string>
#include <cstdint>

namespace cg {

    class Texture {
    public:
        Texture(const std::string& filepath);
        ~Texture();

        // Evitar copia duplicada de texturas en VRAM
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        // Mover
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;

        uint32_t getWidth() const { return m_width; }
        uint32_t getHeight() const { return m_height; }

        // Identificador interno para el Renderer de Vulkan
        void* getVulkanDescriptorSet() const { return m_descriptorSet; }

    private:
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        void* m_textureImage = nullptr;       // VkImage interno
        void* m_textureImageView = nullptr;   // VkImageView interno
        void* m_textureMemory = nullptr;      // VkDeviceMemory interno
        void* m_descriptorSet = nullptr;      // Descriptor set para los shaders
    };

} // namespace cg
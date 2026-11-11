#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <CGame/Graphics/Texture.hpp>
#include <CGame/Core/Window.hpp>
#include "VkCommon.hpp"
#include <stdexcept>
#include <cstdint>

namespace cg {

    Texture::Texture(const std::string& filepath) {
        int channels = 0;
        stbi_set_flip_vertically_on_load(false);
        unsigned char* data = stbi_load(filepath.c_str(),
            reinterpret_cast<int*>(&m_width),
            reinterpret_cast<int*>(&m_height),
            &channels, STBI_rgb_alpha);

        if (!data) {
            throw std::runtime_error("¡Error! No se pudo cargar la textura: " + filepath);
        }

        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView view = vk::createTextureFromPixels(data, m_width, m_height, image, memory);
        VkDescriptorSet descriptor = vk::createTextureDescriptor(view);

        m_textureImage = reinterpret_cast<void*>(image);
        m_textureImageView = reinterpret_cast<void*>(view);
        m_textureMemory = nullptr;
        m_descriptorSet = reinterpret_cast<void*>(descriptor);

        // Guardamos la VkDeviceMemory para liberarla en el destructor.
        m_textureMemory = reinterpret_cast<void*>(memory);

        stbi_image_free(data);
    }

    Texture::~Texture() {
        if (vk::ctx().device == VK_NULL_HANDLE) return;

        // Esperar a que la GPU deje de usar la textura antes de liberarla
        vkDeviceWaitIdle(vk::ctx().device);

        VkImageView view = reinterpret_cast<VkImageView>(m_textureImageView);
        VkImage image = reinterpret_cast<VkImage>(m_textureImage);
        VkDeviceMemory memory = reinterpret_cast<VkDeviceMemory>(m_textureMemory);

        if (view != VK_NULL_HANDLE) vkDestroyImageView(vk::ctx().device, view, nullptr);
        if (image != VK_NULL_HANDLE) vkDestroyImage(vk::ctx().device, image, nullptr);
        if (memory != VK_NULL_HANDLE) vkFreeMemory(vk::ctx().device, memory, nullptr);
    }

    Texture::Texture(Texture&& other) noexcept
        : m_width(other.m_width), m_height(other.m_height),
          m_textureImage(other.m_textureImage), m_textureImageView(other.m_textureImageView),
          m_textureMemory(other.m_textureMemory), m_descriptorSet(other.m_descriptorSet) {
        other.m_textureImage = nullptr;
        other.m_textureImageView = nullptr;
        other.m_textureMemory = nullptr;
        other.m_descriptorSet = nullptr;
    }

    Texture& Texture::operator=(Texture&& other) noexcept {
        if (this != &other) {
            if (vk::ctx().device != VK_NULL_HANDLE) {
                vkDeviceWaitIdle(vk::ctx().device);
                VkImageView view = reinterpret_cast<VkImageView>(m_textureImageView);
                VkImage image = reinterpret_cast<VkImage>(m_textureImage);
                VkDeviceMemory memory = reinterpret_cast<VkDeviceMemory>(m_textureMemory);

                if (view != VK_NULL_HANDLE) vkDestroyImageView(vk::ctx().device, view, nullptr);
                if (image != VK_NULL_HANDLE) vkDestroyImage(vk::ctx().device, image, nullptr);
                if (memory != VK_NULL_HANDLE) vkFreeMemory(vk::ctx().device, memory, nullptr);
            }

            m_width = other.m_width;
            m_height = other.m_height;
            m_textureImage = other.m_textureImage;
            m_textureImageView = other.m_textureImageView;
            m_textureMemory = other.m_textureMemory;
            m_descriptorSet = other.m_descriptorSet;

            other.m_textureImage = nullptr;
            other.m_textureImageView = nullptr;
            other.m_textureMemory = nullptr;
            other.m_descriptorSet = nullptr;
        }
        return *this;
    }

} // namespace cg

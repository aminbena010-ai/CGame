#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>

namespace cg {
namespace vk {

    // Estado global de Vulkan compartido entre Renderer y Texture.
    struct Context {
        VkInstance instance = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        uint32_t graphicsFamily = 0;

        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout textureSetLayout = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;

        VkImageView whiteView = VK_NULL_HANDLE;
        VkDescriptorSet whiteDescriptor = VK_NULL_HANDLE;
    };

    Context& ctx();

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties,
                      VkBuffer& buffer, VkDeviceMemory& memory);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void transitionImageLayout(VkImage image, VkFormat format,
                               VkImageLayout oldLayout, VkImageLayout newLayout);

    void copyBufferToImage(VkBuffer buffer, VkImage image,
                           uint32_t width, uint32_t height);

    VkImageView createTextureFromPixels(const void* pixels, uint32_t width, uint32_t height,
                                        VkImage& outImage, VkDeviceMemory& outMemory);

    VkDescriptorSet createTextureDescriptor(VkImageView imageView);

} // namespace vk
} // namespace cg

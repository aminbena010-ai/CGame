#include <CGame/Graphics/Renderer.hpp>
#include <CGame/Core/Window.hpp>
#include "VkCommon.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace cg {

    namespace {

        struct Glyph {
            char c;
            unsigned char rows[7];
        };

        // Fuente bitmap 5x7 (bit 4 = columna izquierda)
        const Glyph kFont[] = {
            { ' ', { 0,0,0,0,0,0,0 } },
            { '-', { 0,0,0,0x1F,0,0,0 } },
            { ':', { 0,0x04,0x04,0,0x04,0x04,0 } },
            { '.', { 0,0,0,0,0,0,0x04 } },
            { '!', { 0x04,0x04,0x04,0x04,0x04,0,0x04 } },
            { '0', { 0x0E,0x11,0x13,0x15,0x19,0x11,0x0E } },
            { '1', { 0x04,0x0C,0x04,0x04,0x04,0x04,0x0E } },
            { '2', { 0x0E,0x11,0x01,0x02,0x04,0x08,0x1F } },
            { '3', { 0x1F,0x02,0x04,0x02,0x01,0x11,0x0E } },
            { '4', { 0x02,0x06,0x0A,0x12,0x1F,0x02,0x02 } },
            { '5', { 0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E } },
            { '6', { 0x06,0x08,0x10,0x1E,0x11,0x11,0x0E } },
            { '7', { 0x1F,0x01,0x02,0x04,0x08,0x08,0x08 } },
            { '8', { 0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E } },
            { '9', { 0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C } },
            { 'A', { 0x0E,0x11,0x11,0x1F,0x11,0x11,0x11 } },
            { 'B', { 0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E } },
            { 'C', { 0x0E,0x11,0x10,0x10,0x10,0x11,0x0E } },
            { 'D', { 0x1E,0x11,0x11,0x11,0x11,0x11,0x1E } },
            { 'E', { 0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F } },
            { 'F', { 0x1F,0x10,0x10,0x1E,0x10,0x10,0x10 } },
            { 'G', { 0x0E,0x11,0x10,0x17,0x11,0x11,0x0F } },
            { 'H', { 0x11,0x11,0x11,0x1F,0x11,0x11,0x11 } },
            { 'I', { 0x0E,0x04,0x04,0x04,0x04,0x04,0x0E } },
            { 'J', { 0x07,0x02,0x02,0x02,0x02,0x12,0x0C } },
            { 'K', { 0x11,0x12,0x14,0x18,0x14,0x12,0x11 } },
            { 'L', { 0x10,0x10,0x10,0x10,0x10,0x10,0x1F } },
            { 'M', { 0x11,0x1B,0x15,0x15,0x11,0x11,0x11 } },
            { 'N', { 0x11,0x19,0x15,0x13,0x11,0x11,0x11 } },
            { 'O', { 0x0E,0x11,0x11,0x11,0x11,0x11,0x0E } },
            { 'P', { 0x1E,0x11,0x11,0x1E,0x10,0x10,0x10 } },
            { 'Q', { 0x0E,0x11,0x11,0x11,0x15,0x12,0x0D } },
            { 'R', { 0x1E,0x11,0x11,0x1E,0x14,0x12,0x11 } },
            { 'S', { 0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E } },
            { 'T', { 0x1F,0x04,0x04,0x04,0x04,0x04,0x04 } },
            { 'U', { 0x11,0x11,0x11,0x11,0x11,0x11,0x0E } },
            { 'V', { 0x11,0x11,0x11,0x11,0x11,0x0A,0x04 } },
            { 'W', { 0x11,0x11,0x11,0x15,0x15,0x1B,0x11 } },
            { 'X', { 0x11,0x11,0x0A,0x04,0x0A,0x11,0x11 } },
            { 'Y', { 0x11,0x11,0x0A,0x04,0x04,0x04,0x04 } },
            { 'Z', { 0x1F,0x01,0x02,0x04,0x08,0x10,0x1F } },
        };

        const Glyph* findGlyph(char c) {
            if (c >= 'a' && c <= 'z') c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            for (const auto& g : kFont) {
                if (g.c == c) return &g;
            }
            return nullptr;
        }

        struct Vertex {
            float x, y;
            float u, v;
            float r, g, b, a;
        };

        std::vector<char> readFile(const std::string& path) {
            std::ifstream file(path, std::ios::ate | std::ios::binary);
            if (!file.is_open()) return {};
            size_t size = static_cast<size_t>(file.tellg());
            std::vector<char> data(size);
            file.seekg(0);
            file.read(data.data(), static_cast<std::streamsize>(size));
            return data;
        }

        std::vector<char> readShader(const std::string& name) {
            const char* prefixes[] = { "shaders/", "../shaders/", "assets/shaders/", "../../shaders/" };
            for (const char* p : prefixes) {
                auto data = readFile(std::string(p) + name);
                if (!data.empty()) return data;
            }
            throw std::runtime_error("No se pudo cargar el shader Vulkan: " + name);
        }

    } // namespace

    namespace vk {

        static Context g_context;

        Context& ctx() { return g_context; }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
            VkPhysicalDeviceMemoryProperties memProps{};
            vkGetPhysicalDeviceMemoryProperties(ctx().physicalDevice, &memProps);
            for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
                if ((typeFilter & (1u << i)) &&
                    (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }
            throw std::runtime_error("No hay un tipo de memoria Vulkan adecuado.");
        }

        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties,
                          VkBuffer& buffer, VkDeviceMemory& memory) {
            VkBufferCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            info.size = size;
            info.usage = usage;
            info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            if (vkCreateBuffer(ctx().device, &info, nullptr, &buffer) != VK_SUCCESS) {
                throw std::runtime_error("No se pudo crear el buffer Vulkan.");
            }

            VkMemoryRequirements req{};
            vkGetBufferMemoryRequirements(ctx().device, buffer, &req);

            VkMemoryAllocateInfo alloc{};
            alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc.allocationSize = req.size;
            alloc.memoryTypeIndex = findMemoryType(req.memoryTypeBits, properties);
            if (vkAllocateMemory(ctx().device, &alloc, nullptr, &memory) != VK_SUCCESS) {
                throw std::runtime_error("No se pudo asignar memoria para el buffer Vulkan.");
            }
            vkBindBufferMemory(ctx().device, buffer, memory, 0);
        }

        VkCommandBuffer beginSingleTimeCommands() {
            VkCommandBufferAllocateInfo alloc{};
            alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc.commandPool = ctx().commandPool;
            alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc.commandBufferCount = 1;

            VkCommandBuffer cmd;
            vkAllocateCommandBuffers(ctx().device, &alloc, &cmd);

            VkCommandBufferBeginInfo begin{};
            begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(cmd, &begin);
            return cmd;
        }

        void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submit{};
            submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit.commandBufferCount = 1;
            submit.pCommandBuffers = &commandBuffer;

            vkQueueSubmit(ctx().graphicsQueue, 1, &submit, VK_NULL_HANDLE);
            vkQueueWaitIdle(ctx().graphicsQueue);
            vkFreeCommandBuffers(ctx().device, ctx().commandPool, 1, &commandBuffer);
        }

        static void transitionImageLayoutCmd(VkCommandBuffer cmd, VkImage image,
                                             VkImageLayout oldLayout, VkImageLayout newLayout) {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            VkPipelineStageFlags srcStage;
            VkPipelineStageFlags dstStage;

            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                       newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = 0;
                srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            }

            vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        }

        void transitionImageLayout(VkImage image, VkFormat, VkImageLayout oldLayout,
                                   VkImageLayout newLayout) {
            VkCommandBuffer cmd = beginSingleTimeCommands();
            transitionImageLayoutCmd(cmd, image, oldLayout, newLayout);
            endSingleTimeCommands(cmd);
        }

        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
            VkCommandBuffer cmd = beginSingleTimeCommands();

            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = { width, height, 1 };

            vkCmdCopyBufferToImage(cmd, buffer, image,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
            endSingleTimeCommands(cmd);
        }

        static VkImageView createImageView(VkImage image, VkFormat format) {
            VkImageViewCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.image = image;
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = format;
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            info.subresourceRange.baseMipLevel = 0;
            info.subresourceRange.levelCount = 1;
            info.subresourceRange.baseArrayLayer = 0;
            info.subresourceRange.layerCount = 1;

            VkImageView view;
            if (vkCreateImageView(ctx().device, &info, nullptr, &view) != VK_SUCCESS) {
                throw std::runtime_error("No se pudo crear la VkImageView.");
            }
            return view;
        }

        VkImageView createTextureFromPixels(const void* pixels, uint32_t width, uint32_t height,
                                            VkImage& outImage, VkDeviceMemory& outMemory) {
            const VkDevice device = ctx().device;
            const VkDeviceSize imageSize = static_cast<VkDeviceSize>(width) * height * 4;

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingMemory;
            createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer, stagingMemory);

            void* data = nullptr;
            vkMapMemory(device, stagingMemory, 0, imageSize, 0, &data);
            std::memcpy(data, pixels, static_cast<size_t>(imageSize));
            vkUnmapMemory(device, stagingMemory);

            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = width;
            imageInfo.extent.height = height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateImage(device, &imageInfo, nullptr, &outImage) != VK_SUCCESS) {
                throw std::runtime_error("No se pudo crear la imagen Vulkan.");
            }

            VkMemoryRequirements req{};
            vkGetImageMemoryRequirements(device, outImage, &req);

            VkMemoryAllocateInfo alloc{};
            alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc.allocationSize = req.size;
            alloc.memoryTypeIndex = findMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            if (vkAllocateMemory(device, &alloc, nullptr, &outMemory) != VK_SUCCESS) {
                throw std::runtime_error("No se pudo asignar memoria para la imagen Vulkan.");
            }
            vkBindImageMemory(device, outImage, outMemory, 0);

            transitionImageLayout(outImage, imageInfo.format,
                                  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            copyBufferToImage(stagingBuffer, outImage, width, height);
            transitionImageLayout(outImage, imageInfo.format,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingMemory, nullptr);

            return createImageView(outImage, imageInfo.format);
        }

        VkDescriptorSet createTextureDescriptor(VkImageView imageView) {
            VkDescriptorSetAllocateInfo alloc{};
            alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            alloc.descriptorPool = ctx().descriptorPool;
            alloc.descriptorSetCount = 1;
            alloc.pSetLayouts = &ctx().textureSetLayout;

            VkDescriptorSet set;
            if (vkAllocateDescriptorSets(ctx().device, &alloc, &set) != VK_SUCCESS) {
                throw std::runtime_error("No se pudo asignar el descriptor de textura.");
            }

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = imageView;
            imageInfo.sampler = ctx().sampler;

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = set;
            write.dstBinding = 0;
            write.dstArrayElement = 0;
            write.descriptorCount = 1;
            write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(ctx().device, 1, &write, 0, nullptr);
            return set;
        }

    } // namespace vk

    struct Renderer::Impl {
        Window* window = nullptr;
        GLFWwindow* glfw = nullptr;
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        VkFormat swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;
        VkExtent2D swapchainExtent{};
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImageViews;
        std::vector<VkFramebuffer> framebuffers;
        std::vector<VkSemaphore> renderFinishedSemaphores;

        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;

        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
        VkFence inFlightFence = VK_NULL_HANDLE;

        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
        void* vertexBufferMapped = nullptr;
        VkDeviceSize vertexBufferSize = 8 * 1024 * 1024;
        uint32_t vertexCount = 0;
        uint32_t maxVertices = 0;

        VkImage whiteImage = VK_NULL_HANDLE;
        VkDeviceMemory whiteMemory = VK_NULL_HANDLE;

        bool frameStarted = false;
        bool swapchainValid = true;
        uint32_t currentImageIndex = 0;

        VkShaderModule createShaderModule(const std::vector<char>& code);
        void createInstance();
        void createSurface();
        void pickPhysicalDevice();
        void createDevice();
        void createSwapchain();
        void createImageViews();
        void createRenderPass();
        void createDescriptorSetLayout();
        void createSampler();
        void createPipeline();
        void createFramebuffers();
        void createCommandPool();
        void createCommandBuffer();
        void createSyncObjects();
        void createVertexBuffer();
        void createWhiteTexture();
        void destroySwapchain();
        void recreateSwapchain();
        bool recreateSwapchainSafe();
        void cleanup();
        void pushQuad(VkDescriptorSet descriptor, const Vertex vertices[6]);
    };

    Renderer::Impl* Renderer::s_instance = nullptr;

    VkShaderModule Renderer::Impl::createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = code.size();
        info.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule module;
        if (vkCreateShaderModule(vk::ctx().device, &info, nullptr, &module) != VK_SUCCESS) {
            throw std::runtime_error("No se pudo crear el shader module Vulkan.");
        }
        return module;
    }

    void Renderer::Impl::createInstance() {
        VkApplicationInfo app{};
        app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app.pApplicationName = "CGame++";
        app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app.pEngineName = "CGame++";
        app.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app.apiVersion = VK_API_VERSION_1_0;

        uint32_t glfwExtCount = 0;
        const char** glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);

        VkInstanceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pApplicationInfo = &app;
        info.enabledExtensionCount = glfwExtCount;
        info.ppEnabledExtensionNames = glfwExt;

        if (vkCreateInstance(&info, nullptr, &vk::ctx().instance) != VK_SUCCESS) {
            throw std::runtime_error("No se pudo crear la instancia de Vulkan.");
        }
    }

    void Renderer::Impl::createSurface() {
        if (glfwCreateWindowSurface(vk::ctx().instance, glfw, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("No se pudo crear la superficie Vulkan (GLFW).");
        }
    }

    void Renderer::Impl::pickPhysicalDevice() {
        uint32_t count = 0;
        vkEnumeratePhysicalDevices(vk::ctx().instance, &count, nullptr);
        if (count == 0) {
            throw std::runtime_error("No se encontró ninguna GPU compatible con Vulkan.");
        }

        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(vk::ctx().instance, &count, devices.data());

        for (VkPhysicalDevice device : devices) {
            uint32_t familyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);
            std::vector<VkQueueFamilyProperties> families(familyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());

            for (uint32_t i = 0; i < familyCount; ++i) {
                VkBool32 presentSupport = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

                if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport) {
                    vk::ctx().physicalDevice = device;
                    vk::ctx().graphicsFamily = i;

                    uint32_t extCount = 0;
                    vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);
                    std::vector<VkExtensionProperties> extensions(extCount);
                    vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, extensions.data());

                    bool swapchainSupported = false;
                    for (const auto& e : extensions) {
                        if (std::string(e.extensionName) == VK_KHR_SWAPCHAIN_EXTENSION_NAME) {
                            swapchainSupported = true;
                            break;
                        }
                    }
                    if (swapchainSupported) return;
                }
            }
        }
        throw std::runtime_error("No se encontró una GPU con cola gráfica y soporte de presentación.");
    }

    void Renderer::Impl::createDevice() {
        float priority = 1.0f;
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = vk::ctx().graphicsFamily;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &priority;

        const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        VkPhysicalDeviceFeatures features{};

        VkDeviceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.queueCreateInfoCount = 1;
        info.pQueueCreateInfos = &queueInfo;
        info.enabledExtensionCount = 1;
        info.ppEnabledExtensionNames = extensions;
        info.pEnabledFeatures = &features;

        if (vkCreateDevice(vk::ctx().physicalDevice, &info, nullptr, &vk::ctx().device) != VK_SUCCESS) {
            throw std::runtime_error("No se pudo crear el dispositivo lógico Vulkan.");
        }

        vkGetDeviceQueue(vk::ctx().device, vk::ctx().graphicsFamily, 0, &vk::ctx().graphicsQueue);
    }

    void Renderer::Impl::createSwapchain() {
        VkSurfaceCapabilitiesKHR caps{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk::ctx().physicalDevice, surface, &caps);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(vk::ctx().physicalDevice, surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(vk::ctx().physicalDevice, surface, &formatCount, formats.data());

        VkSurfaceFormatKHR chosen = formats.empty() ? VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR } : formats[0];
        for (const auto& f : formats) {
            if ((f.format == VK_FORMAT_B8G8R8A8_UNORM || f.format == VK_FORMAT_R8G8B8A8_UNORM) &&
                f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                chosen = f;
                break;
            }
        }
        swapchainFormat = chosen.format;

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(vk::ctx().physicalDevice, surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(vk::ctx().physicalDevice, surface, &presentModeCount, presentModes.data());

        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        bool hasMailbox = false;
        bool hasImmediate = false;
        for (VkPresentModeKHR mode : presentModes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) hasMailbox = true;
            if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) hasImmediate = true;
        }
        if (hasMailbox) presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        else if (hasImmediate) presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

        if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            swapchainExtent = caps.currentExtent;
        } else {
            swapchainExtent.width = static_cast<uint32_t>(window->getWidth());
            swapchainExtent.height = static_cast<uint32_t>(window->getHeight());
            swapchainExtent.width = std::clamp(swapchainExtent.width, caps.minImageExtent.width, caps.maxImageExtent.width);
            swapchainExtent.height = std::clamp(swapchainExtent.height, caps.minImageExtent.height, caps.maxImageExtent.height);
        }

        uint32_t imageCount = caps.minImageCount + 1;
        if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
            imageCount = caps.maxImageCount;
        }

        VkSwapchainCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = surface;
        info.minImageCount = imageCount;
        info.imageFormat = chosen.format;
        info.imageColorSpace = chosen.colorSpace;
        info.imageExtent = swapchainExtent;
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.preTransform = caps.currentTransform;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = presentMode;
        info.clipped = VK_TRUE;
        info.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(vk::ctx().device, &info, nullptr, &swapchain) != VK_SUCCESS) {
            throw std::runtime_error("No se pudo crear el swapchain de Vulkan.");
        }

        vkGetSwapchainImagesKHR(vk::ctx().device, swapchain, &imageCount, nullptr);
        swapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(vk::ctx().device, swapchain, &imageCount, swapchainImages.data());
    }

    void Renderer::Impl::createImageViews() {
        swapchainImageViews.resize(swapchainImages.size());
        for (size_t i = 0; i < swapchainImages.size(); ++i) {
            VkImageViewCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.image = swapchainImages[i];
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = swapchainFormat;
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            info.subresourceRange.baseMipLevel = 0;
            info.subresourceRange.levelCount = 1;
            info.subresourceRange.baseArrayLayer = 0;
            info.subresourceRange.layerCount = 1;

            if (vkCreateImageView(vk::ctx().device, &info, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("No se pudo crear una VkImageView del swapchain.");
            }
        }
    }

    void Renderer::Impl::createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapchainFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount = 1;
        info.pAttachments = &colorAttachment;
        info.subpassCount = 1;
        info.pSubpasses = &subpass;
        info.dependencyCount = 1;
        info.pDependencies = &dependency;

        if (vkCreateRenderPass(vk::ctx().device, &info, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("No se pudo crear el render pass de Vulkan.");
        }
    }

    void Renderer::Impl::createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding samplerBinding{};
        samplerBinding.binding = 0;
        samplerBinding.descriptorCount = 1;
        samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerBinding.pImmutableSamplers = nullptr;
        samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = 1;
        info.pBindings = &samplerBinding;

        if (vkCreateDescriptorSetLayout(vk::ctx().device, &info, nullptr, &vk::ctx().textureSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("No se pudo crear el layout de descriptores.");
        }

        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = 1024;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = 1024;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;

        if (vkCreateDescriptorPool(vk::ctx().device, &poolInfo, nullptr, &vk::ctx().descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("No se pudo crear el descriptor pool.");
        }
    }

    void Renderer::Impl::createSampler() {
        VkSamplerCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_LINEAR;
        info.minFilter = VK_FILTER_LINEAR;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.anisotropyEnable = VK_FALSE;
        info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        info.unnormalizedCoordinates = VK_FALSE;
        info.compareEnable = VK_FALSE;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        if (vkCreateSampler(vk::ctx().device, &info, nullptr, &vk::ctx().sampler) != VK_SUCCESS) {
            throw std::runtime_error("No se pudo crear el sampler de Vulkan.");
        }
    }

    void Renderer::Impl::createPipeline() {
        auto vertCode = readShader("sprite.vert.spv");
        auto fragCode = readShader("sprite.frag.spv");

        VkShaderModule vertModule = createShaderModule(vertCode);
        VkShaderModule fragModule = createShaderModule(fragCode);

        VkPipelineShaderStageCreateInfo stages[2]{};
        stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = vertModule;
        stages[0].pName = "main";
        stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = fragModule;
        stages[1].pName = "main";

        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = sizeof(Vertex);
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription attributes[3]{};
        attributes[0].location = 0;
        attributes[0].binding = 0;
        attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributes[0].offset = offsetof(Vertex, x);
        attributes[1].location = 1;
        attributes[1].binding = 0;
        attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributes[1].offset = offsetof(Vertex, u);
        attributes[2].location = 2;
        attributes[2].binding = 0;
        attributes[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributes[2].offset = offsetof(Vertex, r);

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &binding;
        vertexInput.vertexAttributeDescriptionCount = 3;
        vertexInput.pVertexAttributeDescriptions = attributes;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo raster{};
        raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        raster.polygonMode = VK_POLYGON_MODE_FILL;
        raster.cullMode = VK_CULL_MODE_NONE;
        raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        raster.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisample{};
        multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.blendEnable = VK_TRUE;
        blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo colorBlend{};
        colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlend.attachmentCount = 1;
        colorBlend.pAttachments = &blendAttachment;

        VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamic{};
        dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic.dynamicStateCount = 2;
        dynamic.pDynamicStates = dynamicStates;

        VkPushConstantRange pushRange{};
        pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushRange.offset = 0;
        pushRange.size = sizeof(float) * 2;

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &vk::ctx().textureSetLayout;
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &pushRange;

        if (vkCreatePipelineLayout(vk::ctx().device, &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("No se pudo crear el pipeline layout.");
        }

        VkGraphicsPipelineCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        info.stageCount = 2;
        info.pStages = stages;
        info.pVertexInputState = &vertexInput;
        info.pInputAssemblyState = &inputAssembly;
        info.pViewportState = &viewportState;
        info.pRasterizationState = &raster;
        info.pMultisampleState = &multisample;
        info.pColorBlendState = &colorBlend;
        info.pDynamicState = &dynamic;
        info.layout = pipelineLayout;
        info.renderPass = renderPass;
        info.subpass = 0;

        if (vkCreateGraphicsPipelines(vk::ctx().device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline) != VK_SUCCESS) {
            throw std::runtime_error("No se pudo crear el pipeline gráfico de Vulkan.");
        }

        vkDestroyShaderModule(vk::ctx().device, vertModule, nullptr);
        vkDestroyShaderModule(vk::ctx().device, fragModule, nullptr);
    }

    void Renderer::Impl::createFramebuffers() {
        framebuffers.resize(swapchainImageViews.size());
        for (size_t i = 0; i < swapchainImageViews.size(); ++i) {
            VkImageView attachments[] = { swapchainImageViews[i] };

            VkFramebufferCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            info.renderPass = renderPass;
            info.attachmentCount = 1;
            info.pAttachments = attachments;
            info.width = swapchainExtent.width;
            info.height = swapchainExtent.height;
            info.layers = 1;

            if (vkCreateFramebuffer(vk::ctx().device, &info, nullptr, &framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("No se pudo crear un framebuffer de Vulkan.");
            }
        }
    }

    void Renderer::Impl::createCommandPool() {
        VkCommandPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        info.queueFamilyIndex = vk::ctx().graphicsFamily;

        if (vkCreateCommandPool(vk::ctx().device, &info, nullptr, &vk::ctx().commandPool) != VK_SUCCESS) {
            throw std::runtime_error("No se pudo crear el command pool de Vulkan.");
        }
    }

    void Renderer::Impl::createCommandBuffer() {
        VkCommandBufferAllocateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.commandPool = vk::ctx().commandPool;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(vk::ctx().device, &info, &commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("No se pudo crear el command buffer de Vulkan.");
        }
    }

    void Renderer::Impl::createSyncObjects() {
        VkSemaphoreCreateInfo semInfo{};
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(vk::ctx().device, &semInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateFence(vk::ctx().device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("No se pudieron crear los objetos de sincronización de Vulkan.");
        }

        renderFinishedSemaphores.resize(swapchainImages.size());
        for (auto& sem : renderFinishedSemaphores) {
            if (vkCreateSemaphore(vk::ctx().device, &semInfo, nullptr, &sem) != VK_SUCCESS) {
                throw std::runtime_error("No se pudo crear un semáforo de Vulkan.");
            }
        }
    }

    void Renderer::Impl::createVertexBuffer() {
        vk::createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         vertexBuffer, vertexBufferMemory);
        vkMapMemory(vk::ctx().device, vertexBufferMemory, 0, vertexBufferSize, 0, &vertexBufferMapped);
        maxVertices = static_cast<uint32_t>(vertexBufferSize / sizeof(Vertex));
    }

    void Renderer::Impl::createWhiteTexture() {
        const unsigned char white[4] = { 255, 255, 255, 255 };
        VkDeviceMemory whiteMemoryTmp = VK_NULL_HANDLE;
        vk::ctx().whiteView = vk::createTextureFromPixels(white, 1, 1, whiteImage, whiteMemoryTmp);
        // La memoria de la imagen blanca se libera en cleanup (guardamos la image).
        whiteMemory = whiteMemoryTmp;
        vk::ctx().whiteDescriptor = vk::createTextureDescriptor(vk::ctx().whiteView);
    }

    void Renderer::Impl::destroySwapchain() {
        for (VkFramebuffer fb : framebuffers) {
            vkDestroyFramebuffer(vk::ctx().device, fb, nullptr);
        }
        framebuffers.clear();

        for (VkImageView view : swapchainImageViews) {
            vkDestroyImageView(vk::ctx().device, view, nullptr);
        }
        swapchainImageViews.clear();

        for (VkSemaphore sem : renderFinishedSemaphores) {
            vkDestroySemaphore(vk::ctx().device, sem, nullptr);
        }
        renderFinishedSemaphores.clear();

        if (swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(vk::ctx().device, swapchain, nullptr);
            swapchain = VK_NULL_HANDLE;
        }
        swapchainImages.clear();
    }

    void Renderer::Impl::recreateSwapchain() {
        vkDeviceWaitIdle(vk::ctx().device);

        destroySwapchain();

        if (pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(vk::ctx().device, pipeline, nullptr);
            pipeline = VK_NULL_HANDLE;
        }
        if (pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(vk::ctx().device, pipelineLayout, nullptr);
            pipelineLayout = VK_NULL_HANDLE;
        }
        if (renderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(vk::ctx().device, renderPass, nullptr);
            renderPass = VK_NULL_HANDLE;
        }

        createSwapchain();
        createImageViews();
        createRenderPass();
        createPipeline();
        createFramebuffers();

        VkSemaphoreCreateInfo semInfo{};
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        renderFinishedSemaphores.resize(swapchainImages.size());
        for (auto& sem : renderFinishedSemaphores) {
            vkCreateSemaphore(vk::ctx().device, &semInfo, nullptr, &sem);
        }
    }

    bool Renderer::Impl::recreateSwapchainSafe() {
        try {
            recreateSwapchain();
            swapchainValid = true;
            return true;
        } catch (...) {
            // Si la ventana se está cerrando, dejamos de renderizar para salir
            // limpio. Si no, se reintentará en el siguiente frame (p. ej. al
            // desminimizar), sin bloquear el bucle de eventos.
            if (glfwWindowShouldClose(glfw)) {
                swapchainValid = false;
            }
            return false;
        }
    }

    void Renderer::Impl::pushQuad(VkDescriptorSet descriptor, const Vertex vertices[6]) {
        if (!frameStarted) return;
        if (vertexCount + 6 > maxVertices) return;

        Vertex* base = reinterpret_cast<Vertex*>(vertexBufferMapped);
        std::memcpy(base + vertexCount, vertices, sizeof(Vertex) * 6);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout, 0, 1, &descriptor, 0, nullptr);
        vkCmdDraw(commandBuffer, 6, 1, vertexCount, 0);
        vertexCount += 6;
    }

    void Renderer::Impl::cleanup() {
        if (vk::ctx().device == VK_NULL_HANDLE) return;
        vkDeviceWaitIdle(vk::ctx().device);

        if (vertexBufferMapped) {
            vkUnmapMemory(vk::ctx().device, vertexBufferMemory);
            vertexBufferMapped = nullptr;
        }
        if (vertexBuffer != VK_NULL_HANDLE) vkDestroyBuffer(vk::ctx().device, vertexBuffer, nullptr);
        if (vertexBufferMemory != VK_NULL_HANDLE) vkFreeMemory(vk::ctx().device, vertexBufferMemory, nullptr);

        if (whiteImage != VK_NULL_HANDLE) vkDestroyImage(vk::ctx().device, whiteImage, nullptr);
        if (whiteMemory != VK_NULL_HANDLE) vkFreeMemory(vk::ctx().device, whiteMemory, nullptr);
        if (vk::ctx().whiteView != VK_NULL_HANDLE) vkDestroyImageView(vk::ctx().device, vk::ctx().whiteView, nullptr);

        destroySwapchain();

        if (imageAvailableSemaphore != VK_NULL_HANDLE)
            vkDestroySemaphore(vk::ctx().device, imageAvailableSemaphore, nullptr);
        if (inFlightFence != VK_NULL_HANDLE)
            vkDestroyFence(vk::ctx().device, inFlightFence, nullptr);

        if (pipeline != VK_NULL_HANDLE) vkDestroyPipeline(vk::ctx().device, pipeline, nullptr);
        if (pipelineLayout != VK_NULL_HANDLE) vkDestroyPipelineLayout(vk::ctx().device, pipelineLayout, nullptr);
        if (renderPass != VK_NULL_HANDLE) vkDestroyRenderPass(vk::ctx().device, renderPass, nullptr);

        if (vk::ctx().commandPool != VK_NULL_HANDLE)
            vkDestroyCommandPool(vk::ctx().device, vk::ctx().commandPool, nullptr);
        if (vk::ctx().descriptorPool != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(vk::ctx().device, vk::ctx().descriptorPool, nullptr);
        if (vk::ctx().sampler != VK_NULL_HANDLE)
            vkDestroySampler(vk::ctx().device, vk::ctx().sampler, nullptr);
        if (vk::ctx().textureSetLayout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(vk::ctx().device, vk::ctx().textureSetLayout, nullptr);

        vkDestroyDevice(vk::ctx().device, nullptr);

        if (surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(vk::ctx().instance, surface, nullptr);
        if (vk::ctx().instance != VK_NULL_HANDLE)
            vkDestroyInstance(vk::ctx().instance, nullptr);

        vk::ctx() = vk::Context{};
    }

    void Renderer::init(Window& window) {
        s_instance = new Impl();
        s_instance->window = &window;
        s_instance->glfw = window.getNativeWindow();

        s_instance->createInstance();
        s_instance->createSurface();
        s_instance->pickPhysicalDevice();
        s_instance->createDevice();
        s_instance->createCommandPool();
        s_instance->createDescriptorSetLayout();
        s_instance->createSampler();
        s_instance->createSwapchain();
        s_instance->createImageViews();
        s_instance->createRenderPass();
        s_instance->createPipeline();
        s_instance->createFramebuffers();
        s_instance->createCommandBuffer();
        s_instance->createSyncObjects();
        s_instance->createVertexBuffer();
        s_instance->createWhiteTexture();
    }

    void Renderer::shutdown() {
        if (!s_instance) return;
        s_instance->cleanup();
        delete s_instance;
        s_instance = nullptr;
    }

    void Renderer::beginFrame(const Color& clearColor) {
        auto* r = s_instance;
        if (!r) return;
        if (!r->swapchainValid) return;
        if (glfwWindowShouldClose(r->glfw)) return;

        vkWaitForFences(vk::ctx().device, 1, &r->inFlightFence, VK_TRUE, UINT64_MAX);

        uint32_t imageIndex = 0;
        VkResult acquire = vkAcquireNextImageKHR(vk::ctx().device, r->swapchain, UINT64_MAX,
                                                 r->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
        if (acquire == VK_ERROR_SURFACE_LOST_KHR) {
            r->swapchainValid = false;
            return;
        }
        if (acquire == VK_ERROR_OUT_OF_DATE_KHR) {
            if (glfwWindowShouldClose(r->glfw)) return;
            r->recreateSwapchainSafe();
            return;
        }
        if (acquire != VK_SUCCESS && acquire != VK_SUBOPTIMAL_KHR) {
            if (glfwWindowShouldClose(r->glfw)) return;
            throw std::runtime_error("Fallo al adquirir la imagen del swapchain.");
        }

        r->currentImageIndex = imageIndex;

        vkResetFences(vk::ctx().device, 1, &r->inFlightFence);
        vkResetCommandBuffer(r->commandBuffer, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(r->commandBuffer, &beginInfo);

        VkClearValue clear{};
        clear.color.float32[0] = clearColor.r / 255.0f;
        clear.color.float32[1] = clearColor.g / 255.0f;
        clear.color.float32[2] = clearColor.b / 255.0f;
        clear.color.float32[3] = clearColor.a / 255.0f;

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = r->renderPass;
        renderPassInfo.framebuffer = r->framebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = r->swapchainExtent;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clear;

        vkCmdBeginRenderPass(r->commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(r->swapchainExtent.width);
        viewport.height = static_cast<float>(r->swapchainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(r->commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = r->swapchainExtent;
        vkCmdSetScissor(r->commandBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(r->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r->pipeline);

        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(r->commandBuffer, 0, 1, &r->vertexBuffer, offsets);

        float screenSize[2] = { static_cast<float>(r->window->getWidth()),
                                static_cast<float>(r->window->getHeight()) };
        vkCmdPushConstants(r->commandBuffer, r->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
                           0, sizeof(screenSize), screenSize);

        r->vertexCount = 0;
        r->frameStarted = true;
    }

    void Renderer::endFrame() {
        auto* r = s_instance;
        if (!r || !r->frameStarted) return;

        vkCmdEndRenderPass(r->commandBuffer);

        if (vkEndCommandBuffer(r->commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("No se pudo finalizar el command buffer.");
        }

        // Si la ventana se está cerrando, no presentar: evita bloqueos del driver
        if (glfwWindowShouldClose(r->glfw)) {
            r->frameStarted = false;
            return;
        }

        VkSemaphore waitSem = r->imageAvailableSemaphore;
        VkSemaphore signalSem = r->renderFinishedSemaphores[r->currentImageIndex];
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &waitSem;
        submit.pWaitDstStageMask = &waitStage;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &r->commandBuffer;
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &signalSem;

        if (vkQueueSubmit(vk::ctx().graphicsQueue, 1, &submit, r->inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("No se pudo enviar el frame a la GPU.");
        }

        VkPresentInfoKHR present{};
        present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &signalSem;
        present.swapchainCount = 1;
        present.pSwapchains = &r->swapchain;
        present.pImageIndices = &r->currentImageIndex;

        VkResult result = vkQueuePresentKHR(vk::ctx().graphicsQueue, &present);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            r->recreateSwapchainSafe();
        } else if (result == VK_ERROR_SURFACE_LOST_KHR) {
            r->swapchainValid = false;
        }

        r->frameStarted = false;
    }

    void Renderer::drawSprite(const Texture& texture, const Vector2& position) {
        drawSprite(texture, Rect(0, 0, static_cast<float>(texture.getWidth()),
                                 static_cast<float>(texture.getHeight())), position);
    }

    void Renderer::drawSprite(const Texture& texture, const Rect& srcRect, const Vector2& position) {
        if (!s_instance) return;

        VkDescriptorSet descriptor = reinterpret_cast<VkDescriptorSet>(texture.getVulkanDescriptorSet());
        if (descriptor == VK_NULL_HANDLE) {
            descriptor = vk::ctx().whiteDescriptor;
        }

        const float x = position.x;
        const float y = position.y;
        const float w = srcRect.w;
        const float h = srcRect.h;

        Vertex v[6] = {
            { x,     y,     0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
            { x + w, y,     1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
            { x + w, y + h, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
            { x,     y,     0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
            { x + w, y + h, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
            { x,     y + h, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
        };
        s_instance->pushQuad(descriptor, v);
    }

    void Renderer::drawText(const Font& font, const std::string& text, const Color& color, const Vector2& position) {
        if (!s_instance) return;

        const float scale = static_cast<float>(font.getFontSize()) / 7.0f;
        const float pixel = scale < 1.0f ? 1.0f : scale;
        const float advance = 6.0f * pixel;

        float cursorX = position.x;
        float cursorY = position.y;

        for (char c : text) {
            const Glyph* glyph = findGlyph(c);
            if (glyph && c != ' ') {
                for (int row = 0; row < 7; ++row) {
                    unsigned char bits = glyph->rows[row];
                    for (int col = 0; col < 5; ++col) {
                        if (bits & (0x10 >> col)) {
                            drawQuad(Rect(cursorX + col * pixel, cursorY + row * pixel, pixel, pixel), color);
                        }
                    }
                }
            }
            cursorX += advance;
        }
    }

    void Renderer::drawQuad(const Rect& destRect, const Color& color) {
        if (!s_instance) return;

        const float r = color.r / 255.0f;
        const float g = color.g / 255.0f;
        const float b = color.b / 255.0f;
        const float a = color.a / 255.0f;

        const float x = destRect.x;
        const float y = destRect.y;
        const float w = destRect.w;
        const float h = destRect.h;

        Vertex v[6] = {
            { x,     y,     0.0f, 0.0f, r, g, b, a },
            { x + w, y,     0.0f, 0.0f, r, g, b, a },
            { x + w, y + h, 0.0f, 0.0f, r, g, b, a },
            { x,     y,     0.0f, 0.0f, r, g, b, a },
            { x + w, y + h, 0.0f, 0.0f, r, g, b, a },
            { x,     y + h, 0.0f, 0.0f, r, g, b, a },
        };
        s_instance->pushQuad(vk::ctx().whiteDescriptor, v);
    }

    void Renderer::drawLine(const Vector2& start, const Vector2& end, float thickness, const Color& color) {
        drawQuad(Rect(start.x, start.y, thickness, (end - start).length()), color);
    }

} // namespace cg

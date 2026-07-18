#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <optional>
#include <vector>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <stb_image.h>

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

class Application{
public:
    void run();
    
    struct Vertex{
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 tex_coord;

        static VkVertexInputBindingDescription getBindingDescription();
        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
    
        bool operator==(const Vertex& other) const;
    };
private:

    struct QueueFamilyIndices{
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> present;
        std::optional<uint32_t> transfer;

        bool isComplete() const;
    };

    struct SwapChainSupportDetails{
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkPresentModeKHR> present_modes;
        std::vector<VkSurfaceFormatKHR> formats;
    };


    struct BufferCreateInfo{
        VkDeviceSize size;
        VkBufferUsageFlags usage;
        VkMemoryPropertyFlags properties;
        VkBuffer* buffer;
        VkDeviceMemory* buffer_memory;
        VkSharingMode sharing_mode;
        uint32_t* indices;
        uint32_t family_count;
    };

    struct ImageCreateInfo{
        int tex_width, tex_height, tex_depth, tex_channels, mip_levels, array_layers, family_count;
        VkSampleCountFlagBits sample_count;
        uint32_t* indices;
        VkSharingMode sharing_mode;
        VkImageTiling tiling;
        VkFormat format;
        VkImageLayout initial_layout;
        VkImageType image_type = VK_IMAGE_TYPE_2D;
        VkImageUsageFlags image_usage;
        VkImage* image;
        VkDeviceMemory* memory;
        VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    };

    struct UniformBufferObject{
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    static void framebufferResizeCallback(GLFWwindow* window, int new_width, int new_height);
    
    static void check_vk_result(VkResult result);
    static std::vector<char> readFile(const std::string& file_name);
    void createBuffer(BufferCreateInfo *create_info);
    void copyBuffer(VkBuffer srcb, VkBuffer dstb, VkDeviceSize size);
    void createImage(ImageCreateInfo *create_info);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer buffer);
    void copyBufferImage(VkBuffer srcbuffer, VkImage fromimage, uint32_t width, uint32_t height);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspects);

    void initWindow();
    void initVulkan();
    bool checkValidationLayerSupport() const;
    std::vector<const char*> getRequiredExtensions() const;
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice target, VkPhysicalDeviceFeatures& features, VkPhysicalDeviceProperties& properties) const;
    bool checkDeviceExtensionsSupported(VkPhysicalDevice target) const;
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice target) const;
    void createLogicalDevice();
    void createSwapChain();
    SwapChainSupportDetails querySwapchainSupport(VkPhysicalDevice target) const;
    static VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& available_modes);
    static VkSurfaceFormatKHR chooseFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void createImageViews();
    void recreateSwapChain();
    void cleanupSwapChain();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    VkShaderModule createShaderModule(const std::string& path);
    void createFrameBuffers();
    void createDepthResources();
    void createTextureImage();
    void createTextureImageView();
    void createTextureSampler();
    void loadModel();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createCommandPoolBuffer();
    void recordCommandBuffer(VkCommandBuffer buffer, uint32_t image_index);
    void createSyncObjects();
    void initImGUI();
    void setupImGuiStyle(bool dark, float alpha);
    void mainLoop();
    void imGuiLoop();
    void drawFrame();
    void updateUniformBuffer(uint32_t cur_image);
    void cleanUp();

    VkInstance instance = nullptr;

    VkPhysicalDevice p_device = VK_NULL_HANDLE;
    VkDevice device = nullptr;
    VkQueue graphics_queue = nullptr;
    VkQueue present_queue = nullptr;
    VkQueue transfer_queue = nullptr;

    VkBuffer vertex_buffer = nullptr;
    VkDeviceMemory vertex_mem = nullptr;
    VkBuffer index_buffer = nullptr;
    VkDeviceMemory index_mem = nullptr;

    VkImage depth_tex = nullptr;
    VkDeviceMemory depth_memory;
    VkImageView depth_view;

    VkSampler tex_sampler = nullptr;
    VkImage tex_image = nullptr;
    VkDeviceMemory tex_mem = nullptr;
    VkBuffer staging_buffer = nullptr;
    VkDeviceMemory staging_mem = nullptr;
    VkImageView tex_view = nullptr;

    std::vector<VkBuffer> uniform_buffers;
    std::vector<VkDeviceMemory> uniform_buffer_mems;
    std::vector<void*> muniform_buffers;

    VkSwapchainKHR swapchain = nullptr;
    std::vector<VkImage> sc_images;
    std::vector<VkImageView> sc_views;
    VkFormat sc_format;
    VkExtent2D sc_extent;
    std::vector<VkFramebuffer> sc_fb;

    VkRenderPass render_pass = nullptr;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool dpool;
    std::vector<VkDescriptorSet> dsets;
    VkPipelineLayout pl_layout = nullptr;
    VkPipeline pipeline = nullptr;

    std::vector<VkSemaphore> sps_image_available;
    std::vector<VkSemaphore> sps_render_finished;
    std::vector<VkFence> fs_flight;
    bool framebuffer_resized = false;

    //command pool graphics family
    VkCommandPool cmdp = nullptr;
    std::vector<VkCommandBuffer> cmdb;
    
    //command pool transfer family
    VkCommandPool cmdp_t = nullptr;

    VkDebugUtilsMessengerEXT debug_messenger = nullptr;

    VkSurfaceKHR surface = nullptr;

    GLFWwindow* window = nullptr;

    uint32_t cur_frame = 0;

    VkDescriptorPool imm_dpool;

    const uint16_t START_WIDTH = 640;
    const uint16_t START_HEIGHT = 360;

    const char* tex_path = "textures/tex.png";
    const char* model_path = "models/suzanne.obj";

    const std::vector<const char*> VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    const uint32_t MAX_FLIGHT_FRAMES = 2;

    std::vector<Vertex> vertexi;
    
    std::vector<uint32_t> indices;

    const char* WINDOW_TITLE = "Demonstration of my knowledge.";
};
#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <optional>
#include <vector>
#include <glm/glm.hpp>
#include <array>

class Application{
public:
    void run();

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

    struct Vertex{
        glm::vec2 pos;
        glm::vec3 color;

        static VkVertexInputBindingDescription getBindingDescription();
        static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
    };

    struct BufferCreateInfo{
        VkDeviceSize size;
        VkBufferUsageFlags usage;
        VkMemoryPropertyFlags properties;
        VkBuffer* buffer;
        VkDeviceMemory* buffer_memory;
        VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
        uint32_t* indices;
        uint32_t family_count;
    };

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    static void framebufferResizeCallback(GLFWwindow* window, int new_width, int new_height);

    static std::vector<char> readFile(const std::string& file_name);
    static void populateDebugMessengerCI(VkDebugUtilsMessengerCreateInfoEXT& create_info);
    void createBuffer(BufferCreateInfo *create_info);
    void copyBuffer(VkBuffer srcb, VkBuffer dstb, VkDeviceSize size);

    void initWindow();
    void initVulkan();
    bool checkValidationLayerSupport() const;
    std::vector<const char*> getRequiredExtensions() const;
    void createInstance();
    void setupDebugMessenger();
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
    void createGraphicsPipeline();
    VkShaderModule createShaderModule(const std::string& path);
    void createFrameBuffers();
    void createVertexBuffer();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createCommandPoolBuffer();
    void recordCommandBuffer(VkCommandBuffer buffer, uint32_t image_index);
    void createSyncObjects();
    void mainLoop();
    void drawFrame();
    void cleanUp();

    VkInstance instance = nullptr;

    VkPhysicalDevice p_device = VK_NULL_HANDLE;
    VkDevice device = nullptr;
    VkQueue graphics_queue = nullptr;
    VkQueue present_queue = nullptr;
    VkQueue transfer_queue = nullptr;

    VkBuffer vertex_buffer = nullptr;
    VkDeviceMemory vertex_mem = nullptr;

    VkSwapchainKHR swapchain = nullptr;
    std::vector<VkImage> sc_images;
    std::vector<VkImageView> sc_views;
    VkFormat sc_format;
    VkExtent2D sc_extent;
    std::vector<VkFramebuffer> sc_fb;

    VkRenderPass render_pass = nullptr;
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


    const uint16_t START_WIDTH = 640;
    const uint16_t START_HEIGHT = 360;

    const std::vector<const char*> VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
    };
    #ifdef NDEBUG
    const bool ENABLE_VALIDATION_LAYERS = false;
    #else
    const bool ENABLE_VALIDATION_LAYERS = true;
    #endif

    const std::vector<const char*> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    const uint32_t MAX_FLIGHT_FRAMES = 2;

    const std::vector<Vertex>vertexi = {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    const char* WINDOW_TITLE = "Demonstration of my knowledge.";
};
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <optional>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>

class Application{
public:
    void run();

private:

    struct QueueFamilyIndices{
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> present;

        bool isComplete();
    };

    struct SwapChainSupportDetails{
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkPresentModeKHR> present_modes;
        std::vector<VkSurfaceFormatKHR> formats;
    };

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    static void framebufferResizeCallback(GLFWwindow* window, int new_width, int new_height);

    static std::vector<char> readFile(const std::string& file_name);

    void initWindow();
    void initVulkan();
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    void createInstance();
    void setupDebugMessenger();
    void populateDebugMessengerCI(VkDebugUtilsMessengerCreateInfoEXT& create_info);
    void createSurface();
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice target, VkPhysicalDeviceFeatures& features, VkPhysicalDeviceProperties& properties);
    bool checkDeviceExtensionsSupported(VkPhysicalDevice target);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice target);
    void createLogicalDevice();
    void createSwapChain();
    SwapChainSupportDetails querySwapchainSupport(VkPhysicalDevice target);
    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& available_modes);
    VkSurfaceFormatKHR chooseFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void createImageViews();
    void recreateSwapChain();
    void cleanupSwapChain();
    void createRenderPass();
    void createGraphicsPipeline();
    VkShaderModule createShaderModule(const std::string& path);
    void createFrameBuffers();
    void createCommandPoolBuffer();
    void recordCommandBuffer(VkCommandBuffer buffer, uint32_t image_index);
    void createSyncObjects();
    void mainLoop();
    void drawFrame();
    void cleanUp();

    VkInstance instance;

    VkPhysicalDevice p_device = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;

    VkSwapchainKHR swapchain;
    std::vector<VkImage> sc_images;
    std::vector<VkImageView> sc_views;
    VkFormat sc_format;
    VkExtent2D sc_extent;
    std::vector<VkFramebuffer> sc_fb;

    VkRenderPass render_pass;
    VkPipelineLayout pl_layout;
    VkPipeline pipeline;

    std::vector<VkSemaphore> sps_image_available;
    std::vector<VkSemaphore> sps_render_finished;
    std::vector<VkFence> fs_flight;
    bool framebuffer_resized = false;

    VkCommandPool cmdp;
    std::vector<VkCommandBuffer> cmdb;

    VkDebugUtilsMessengerEXT debug_messenger;

    VkSurfaceKHR surface;

    GLFWwindow* window;

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


    const char* WINDOW_TITLE = "Demonstration of my knowledge.";
};
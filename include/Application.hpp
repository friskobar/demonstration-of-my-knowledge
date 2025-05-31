#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <string>

class Application{
public:
    void run();
private:
    struct QueueFamilyIndices;
    struct SwapChainSupportDetails;

    const std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const std::vector<const char*> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    void initWindow();

    void initVulkan();
    void createInstance();
    void createSurface();
    void createMessenger();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();

    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    int rateSuitability(VkPhysicalDevice device);
    Application::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    void createLogicalDevice();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR &capabilities);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    void createSwapChain();
    void createImageViews();

    void createRenderPass();
    void createGraphicsPipeline();

    void mainLoop();

    void cleanUp();

    static std::vector<char> readFile(const std::string& path);
    VkShaderModule createShaderModule(const std::vector<char>& code);

    GLFWwindow* window;

    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_views;
    VkFormat swapchain_format;
    VkExtent2D swapchain_extent;

    VkRenderPass render_pass;
    VkPipelineLayout pl_layout;

    VkQueue graphics_queue;
    VkQueue present_queue;

    VkSurfaceKHR surface;
    VkDevice device;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;

    const uint16_t WIDTH = 800;
    const uint16_t HEIGHT = 600;
};
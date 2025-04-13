#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
class Application{
public:
    void run();
private:
    struct QueueFamilyIndices;

    void initWindow();

    void initVulkan();
    void createInstance();
    void createMessenger();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();

    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    int rateSuitability(VkPhysicalDevice device);
    Application::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    void mainLoop();

    void cleanUp();

    GLFWwindow* window;

    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;

    const uint16_t WIDTH = 800;
    const uint16_t HEIGHT = 600;
};
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Application.hpp"

#include <stdexcept>
#include <vector>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <optional>
#include <set>

const std::vector<const char*> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef DEBUG
    const bool enable_validation_layers = true;
#else
    const bool enable_validation_layers = false;
#endif

#ifdef VERBOSE
    const bool verbose = true;
#else
    const bool verbose = false;
#endif

struct Application::QueueFamilyIndices{
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> present;

    bool is_complete(){
        return graphics.has_value() && present.has_value();
    }
};


void Application::run() {
    if(verbose)
        std::cout<< "run\n";
    initWindow();
    initVulkan();
    mainLoop();
    cleanUp();
}

void Application::initWindow(){
    if(verbose)
        std::cout<< "initWindow\n";
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "DOMK", nullptr, nullptr);
};

void Application::initVulkan(){
    if(verbose)
        std::cout<< "initVulkan\n";
    createInstance();
    createSurface();
    createMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
}

void Application::createInstance(){
    if(verbose)
        std::cout<< "createInstance\n";
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.pEngineName = "DOMK";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

    if(enable_validation_layers){
        createInfo.ppEnabledLayerNames = validation_layers.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    std::vector<const char*> extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if(result != VK_SUCCESS)
        throw std::runtime_error("Could not create VkInstance.");
    else if(enable_validation_layers && !checkValidationLayerSupport())
        throw std::runtime_error("Validation layer requested but not supported.");

};

bool Application::checkValidationLayerSupport(){
    if(verbose)
        std::cout<< "checkValidationLayerSupport\n";
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> available_layers(layerCount);

    vkEnumerateInstanceLayerProperties(&layerCount, available_layers.data());

    for (const char* layerName : validation_layers) {
        bool layerFound = false;

        for (const auto& layerProperties : available_layers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }
    return true;

}

std::vector<const char*> Application::getRequiredExtensions() {
    if(verbose)
        std::cout<< "getRequiredExtensions\n";
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enable_validation_layers) {
        std::cout<< "ADDED VK_EXT_debug_utils" << std::endl;
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void Application::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    if(verbose)
        std::cout<< "populateDebugMessengerCreateInfo\n";
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    if(verbose){
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    } else {
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    }
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
}

void Application::createSurface(){
    if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS){
        throw std::runtime_error("Could not create surface for Windows.");
    }
}

void Application::createMessenger(){
    if(!enable_validation_layers) return;
    if(verbose)
        std::cout<< "createMessenger\n";
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if(func != nullptr){
        func(instance, &createInfo, nullptr , &debug_messenger);
    } else {
        throw std::runtime_error("Could not create debug messenger.");
    }

}

VKAPI_ATTR VkBool32 VKAPI_CALL Application::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::map<VkDebugUtilsMessageSeverityFlagBitsEXT, std::string> severity_translation_dict = {
        {VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT, "Verbose"},
        {VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, "Info"},
        {VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT, "Warning"},
        {VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "Error"},
        {VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT, "MAXFLAGS"}
    };
    std::cout << "[VL, " << severity_translation_dict[messageSeverity] << "] :" << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void Application::pickPhysicalDevice(){
    if(verbose)
        std::cout<< "pickPhysicalDevice\n";
    uint32_t device_count;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    if(device_count == 0){
        std::runtime_error("Could not find any GPU's with vulkan support.");
    }

    std::vector<VkPhysicalDevice> devices(device_count);

    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    
    std::map<uint32_t, VkPhysicalDevice, std::greater<int>> points;
    
    for (const VkPhysicalDevice device : devices) {
        if(isDeviceSuitable(device)){
            uint32_t device_points = rateSuitability(device);
            points.insert(std::make_pair(device_points, device));
        }
    }

    if(points.empty()){
        throw std::runtime_error("Failed to find a suitable GPU.");
    } else {
        physical_device = points.begin()->second;
        std::cout << physical_device << std::endl;
    }
        
}

Application::QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice device){
    if(verbose)
        std::cout<< "findQueueFamilies\n";
    QueueFamilyIndices indices;
    
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> family_properties(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, family_properties.data());

    int i = 0;
    for(const VkQueueFamilyProperties& property : family_properties){
        if(property.queueFlags & VK_QUEUE_GRAPHICS_BIT){
            indices.graphics = i;
        }
        VkBool32 is_present = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &is_present);
        if(is_present){
            indices.present = i;
        }




        if(indices.is_complete()){
            break;
        }
        i++;
    }
    return indices;
}

bool Application::isDeviceSuitable(VkPhysicalDevice device){
    if(verbose)
        std::cout<< "isDeviceSuitable\n";
    QueueFamilyIndices indices = findQueueFamilies(device);
    std::cout << indices.is_complete();
    return indices.is_complete();
}

int Application::rateSuitability(VkPhysicalDevice physical_device) {
    if(verbose)
        std::cout<< "rateSuitability\n";
    uint32_t score = 0;

    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(physical_device, &device_features);
    if (!device_features.geometryShader) {
        return 0;
    }

    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &device_properties);


    if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1;
    }

    score += device_properties.limits.maxGeometryShaderInvocations;

    return score;
}

void Application::createLogicalDevice(){
    QueueFamilyIndices indices = findQueueFamilies(physical_device);

    std::set<uint32_t> nqueue_indices = {indices.graphics.value(), indices.present.value()};
    std::vector<VkDeviceQueueCreateInfo> create_infos;

    const float priority = 1.0f;

    for(uint32_t indice : nqueue_indices){
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = indice;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &priority;
        create_infos.push_back(queue_create_info);

    }
    VkPhysicalDeviceFeatures device_features{};

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(create_infos.size());
    device_create_info.pQueueCreateInfos = create_infos.data();
    device_create_info.pEnabledFeatures = &device_features;

    if(vkCreateDevice(physical_device, &device_create_info, nullptr, &device) != VK_SUCCESS){
        throw std::runtime_error("Could not create VkDevice.");
    }

    vkGetDeviceQueue(device, indices.graphics.value(), 0, &graphics_queue);
    vkGetDeviceQueue(device, indices.present.value(), 0, &present_queue);

}

void Application::mainLoop(){
    if(verbose)
        std::cout<< "mainLoop\n";
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
    };
};

void Application::cleanUp(){
    if(verbose)
        std::cout<< "cleanUp\n";
    if(enable_validation_layers){
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debug_messenger, nullptr);
        }
    }
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
};

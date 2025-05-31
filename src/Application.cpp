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
#include <limits>
#include <algorithm>
#include <fstream>

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

struct Application::SwapChainSupportDetails{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
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
    createSwapChain();
    createImageViews();
    createGraphicsPipeline();
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
        std::cout << "Chosen GPU: " << physical_device << std::endl;
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

    bool extensions_supported = checkDeviceExtensionSupport(device);
    
    bool swap_chain_adequate = false;
    if (extensions_supported){
        SwapChainSupportDetails details = querySwapChainSupport(device);
        swap_chain_adequate = !details.formats.empty() && !details.present_modes.empty();
    }


    return indices.is_complete() && extensions_supported && swap_chain_adequate;
}

VkSurfaceFormatKHR Application::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) {
    for (const auto& availableFormat : available_formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR Application::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_pms) {
    for(const VkPresentModeKHR available_pm : available_pms){
        if(available_pm == VK_PRESENT_MODE_MAILBOX_KHR){
            return available_pm;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR; 
}

VkExtent2D Application::chooseSwapExtent(VkSurfaceCapabilitiesKHR &capabilities) {
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()){
        return capabilities.currentExtent;
    }
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    
    return actualExtent;
}

Application::SwapChainSupportDetails Application::querySwapChainSupport(VkPhysicalDevice device){
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
    if(format_count != 0){
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
    if(present_mode_count != 0){
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
    }

    return details;
}

bool Application::checkDeviceExtensionSupport(VkPhysicalDevice device){
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

    std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());
    for(const VkExtensionProperties extension : available_extensions){
        required_extensions.erase(extension.extensionName);
    }
    return required_extensions.empty();
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
    
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    device_create_info.ppEnabledExtensionNames = device_extensions.data();


    if(vkCreateDevice(physical_device, &device_create_info, nullptr, &device) != VK_SUCCESS){
        throw std::runtime_error("Could not create VkDevice.");
    }

    vkGetDeviceQueue(device, indices.graphics.value(), 0, &graphics_queue);
    vkGetDeviceQueue(device, indices.present.value(), 0, &present_queue);

}

void Application::createSwapChain(){
    SwapChainSupportDetails swap_chain_support = querySwapChainSupport(physical_device);

    VkSurfaceFormatKHR format = chooseSwapSurfaceFormat(swap_chain_support.formats);
    VkPresentModeKHR present_mode = chooseSwapPresentMode(swap_chain_support.present_modes);
    VkExtent2D extent = chooseSwapExtent(swap_chain_support.capabilities);

    uint32_t image_count = 0;

    if(swap_chain_support.capabilities.maxImageCount > 0 && swap_chain_support.capabilities.minImageCount + 1 <= swap_chain_support.capabilities.maxImageCount){
        image_count = swap_chain_support.capabilities.minImageCount + 1;
    } else {
        image_count = swap_chain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = image_count;
    create_info.imageExtent = extent;
    create_info.imageFormat = format.format;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageColorSpace = format.colorSpace;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physical_device);
    uint32_t indices_array[] = {indices.graphics.value(), indices.present.value()};

    if(indices.graphics != indices.present){
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = indices_array;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }

    create_info.preTransform = swap_chain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;

    create_info.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain) != VK_SUCCESS){
        throw std::runtime_error("Could not create swap chain.");
    };

    vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
    swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images.data());
    swapchain_extent = extent;
    swapchain_format = format.format;
}

void Application::createImageViews(){
    swapchain_views.resize(swapchain_images.size());

    for(size_t i = 0; i < swapchain_images.size(); i++){
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = swapchain_images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = swapchain_format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;
        if(vkCreateImageView(device, &create_info, nullptr, &swapchain_views[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to create image view");
        }
    }
}

void Application::createGraphicsPipeline(){
    auto vert_code = readFile("shaders/compiled/default_vert.spv");
    auto frag_code = readFile("shaders/compiled/default_frag.spv");
    
    VkShaderModule vert_shader_module = createShaderModule(vert_code);
    VkShaderModule frag_shader_module = createShaderModule(frag_code);

    VkPipelineShaderStageCreateInfo vcreate_info{};
    vcreate_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vcreate_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vcreate_info.module = vert_shader_module;
    vcreate_info.pName = "main";
    
    VkPipelineShaderStageCreateInfo fcreate_info{};
    fcreate_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fcreate_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fcreate_info.module = frag_shader_module;
    fcreate_info.pName = "main";
    
    VkPipelineShaderStageCreateInfo shaderStages[] = {vcreate_info, fcreate_info};

    //DYNAMIC STATES

    std::vector<VkDynamicState> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    
    VkPipelineDynamicStateCreateInfo dynamic_state_info{};
    dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state_info.pDynamicStates = dynamic_states.data();

    //VERTEX INPUT - WHAT THE FUCK????????!?!!??!!!???

    VkPipelineVertexInputStateCreateInfo vinput_info{};
    vinput_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vinput_info.vertexBindingDescriptionCount = 0;
    vinput_info.pVertexBindingDescriptions = nullptr;
    vinput_info.vertexAttributeDescriptionCount = 0;
    vinput_info.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo assembly_info{};
    assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    assembly_info.primitiveRestartEnable = VK_FALSE;

    //VIEWPORT

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapchain_extent.width;
    viewport.height = (float) swapchain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 0.0f;

    VkRect2D scissors{};

    scissors.offset = {0,0};
    scissors.extent = swapchain_extent;

    VkPipelineViewportStateCreateInfo vpstate_info{};
    vpstate_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vpstate_info.scissorCount = 1;
    vpstate_info.viewportCount = 1;
    //viewports and scissors will be defined later

    //RASTERIZER

    VkPipelineRasterizationStateCreateInfo rasterizer_info{};
    rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_info.depthClampEnable = VK_FALSE; // clamps instead of discarding geometry that exceeds near/far planes
    rasterizer_info.rasterizerDiscardEnable = VK_FALSE; //discards everything that goes into this state
    rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer_info.lineWidth = 1.0f;
    rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer_info.depthBiasEnable = VK_FALSE;
    
    //MULTISAMPLING
    VkPipelineMultisampleStateCreateInfo multisample_info{};
    multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_info.sampleShadingEnable = VK_FALSE;
    multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    //COLOR BLENDING - honestly i get the general idea but dont know exactly how it works
    VkPipelineColorBlendAttachmentState colorbatt_state{};
    colorbatt_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
    colorbatt_state.blendEnable = VK_FALSE;
    colorbatt_state.blendEnable = VK_TRUE;

    VkPipelineColorBlendStateCreateInfo colorb_info{};
    colorb_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorb_info.logicOpEnable = VK_FALSE;
    colorb_info.attachmentCount = 1;
    colorb_info.pAttachments = &colorbatt_state;
    
    //PIPELINE LAYOUT
    //omfg fucking finally

    VkPipelineLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.pSetLayouts = nullptr;
    layout_info.setLayoutCount = 0;
    layout_info.pushConstantRangeCount = 0;
    layout_info.pPushConstantRanges = nullptr;

    if(vkCreatePipelineLayout(device, &layout_info, nullptr, &pl_layout) != VK_SUCCESS){
        throw std::runtime_error("Could not create pipeline layout.");
    }


    vkDestroyShaderModule(device, frag_shader_module, nullptr);
    vkDestroyShaderModule(device, vert_shader_module, nullptr);
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
    for(VkImageView image_view : swapchain_views) {
        vkDestroyImageView(device, image_view, nullptr);
    }

    vkDestroyPipelineLayout(device, pl_layout, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
};

std::vector<char> Application::readFile(const std::string& path){
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if(!file.is_open()){
        throw std::runtime_error("Could not open file.");
    }

    size_t file_size = (size_t) file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();

    return buffer;
}

VkShaderModule Application::createShaderModule(const std::vector<char>& code){
    std::vector<char> frag = readFile("shader/compiled/default_frag.spv");
    std::vector<char> vert = readFile("shader/compiled/default_vert.spv");

    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule module;
    if(vkCreateShaderModule(device, &create_info, nullptr, &module) != VK_SUCCESS){
        throw std::runtime_error("Failed to create shader module.");
    }

    return module;
}
#include "application.hpp"

//honestly i have no idea
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

/*
    Starts the Application.
    First initializes the window, then Vulkan.
    After that, starts the main loop of the application.
    After the main loop is over, cleans up and closes.
*/
void Application::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanUp();
}


bool Application::QueueFamilyIndices::isComplete(){
    return graphics.has_value() && present.has_value();
}

//Validation Layers messages
VKAPI_ATTR VkBool32 VKAPI_CALL Application::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

//Reads a file. byte. per. byte.
std::vector<char> Application::readFile(const std::string& file_name){
    std::ifstream file(file_name, std::ios::ate | std::ios::binary);

    if(!file.is_open()){
        throw std::runtime_error("Couldn't open file " + file_name + "!");
    }

    size_t file_size = (size_t) file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();

    return buffer;
}

/*
    Initializes GLFW(makes opening cross-platform windows easier).
    Creates the window and sets it to the "window" pointer.
*/
void Application::initWindow() {
    glfwInit(); //Initialize GLFW

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Turn off "OpenGL mode"
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Turn off resizing windows, will be turned on later

    window = glfwCreateWindow(START_WIDTH, START_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
}

//Creates the Vulkan Instance.
void Application::initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFrameBuffers();
    createCommandPoolBuffer();
    createSyncObjects();
}

/*
    Checks if every validation layer is supported by the computer.
    A validation layer is a method for handling errors in Vulkan, since those are generally not handled.
*/
bool Application::checkValidationLayerSupport() {
    uint32_t layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> layers;
    vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

    for(const VkLayerProperties layer : layers){
        bool layer_found = false;
        
        for(const char* wanted_layer : VALIDATION_LAYERS){
            if(strcmp(layer.layerName, wanted_layer)){
                layer_found = true;
                break;
            }
        }

        if(!layer_found){
            return false;
        }
    }
    return true;
}

//Gets all extensions required.
std::vector<const char*> Application::getRequiredExtensions(){
    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char*> extensions(glfw_extensions, glfw_extension_count + glfw_extensions);

    if(ENABLE_VALIDATION_LAYERS){
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;

}

/*
    Sets the application name, version, etc.
    Sets required glfw extensions.
    Sets required vk extensions.
    Checks validation layer support.
    Creates vk instance.
*/
void Application::createInstance() {
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.apiVersion = VK_API_VERSION_1_4;
    app_info.pApplicationName = "DOMK Engine";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    app_info.pEngineName = "DOMK";
    app_info.engineVersion = VK_MAKE_VERSION(0, 0, 0);

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    
    
        VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
        if (ENABLE_VALIDATION_LAYERS) {
            create_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
            create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();

            populateDebugMessengerCI(debug_create_info);
            create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_create_info;
        } else {
            create_info.enabledLayerCount = 0;

            create_info.pNext = nullptr;
        }

    //apply glfw/vk extensions

    std::vector<const char*> extensions = getRequiredExtensions();

    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();
    
    if(vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS){
        throw std::runtime_error("Could'nt create Vulkan Instance!");
    }

}

//Setups the Debug Messenger
void Application::setupDebugMessenger(){
    if(!ENABLE_VALIDATION_LAYERS) return;
    VkDebugUtilsMessengerCreateInfoEXT create_info{};
    populateDebugMessengerCI(create_info);

    if(CreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS){
        throw std::runtime_error("Couldn't create debug utils messenger.");
    }

}

//Populates the debug messenger Create Info
void Application::populateDebugMessengerCI(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

//Creates the Vulkan surface to render images upon.
void Application::createSurface(){
    if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Couldn't create window surface.");
    

}

//Picks the best GPU available.
void Application::pickPhysicalDevice(){
    uint32_t device_count;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    if(device_count == 0){
        throw std::runtime_error("No GPUs with Vulkan support found.");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    std::multimap<int, VkPhysicalDevice> map;

    for(VkPhysicalDevice device : devices){
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);

        if(!features.geometryShader) continue;

        uint32_t device_score = 0;
        
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
            device_score += 1;
        }

        device_score += properties.limits.maxImageDimension2D;

        map.insert(std::make_pair(device_score, device));
    }

    if(map.empty()){
        throw std::runtime_error("No GPUS with Vulkan support found and i dont know how we got here.");
    }
    p_device = map.rbegin()->second;
}

//Checks if a Physical Device(GPU) is suitable for usage
bool Application::isDeviceSuitable(VkPhysicalDevice target, VkPhysicalDeviceFeatures& features, VkPhysicalDeviceProperties& properties){
    vkGetPhysicalDeviceFeatures(target, &features);
    vkGetPhysicalDeviceProperties(target, &properties);

    QueueFamilyIndices indices = findQueueFamilies(target);

    bool extensions_supported = checkDeviceExtensionsSupported(target);

    bool swapchain_adequate = false;
    if(extensions_supported){
        SwapChainSupportDetails details = querySwapchainSupport(target);
        swapchain_adequate = !details.formats.empty() && !details.formats.empty();
    }

    return indices.isComplete() && extensions_supported && swapchain_adequate;

}

//Checks if a Physical Device(GPU) supports all required extensions
bool Application::checkDeviceExtensionsSupported(VkPhysicalDevice target){
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(target, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(target, nullptr, &extension_count, extensions.data());

    std::set<std::string> required_extensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

    for(VkExtensionProperties extension:extensions){
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

//Finds all queueFamilies supported by the Physical Device
Application::QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice target){
    QueueFamilyIndices indices;

    uint32_t family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(target, &family_count, nullptr);

    std::vector<VkQueueFamilyProperties> families(family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(target, &family_count, families.data());

    uint32_t i = 0;
    for(VkQueueFamilyProperties family : families){
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics = i;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(target, i, surface, &present_support);

        if (present_support) {
            indices.present = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

//Creates the swap chain
void Application::createSwapChain(){
    SwapChainSupportDetails details = querySwapchainSupport(p_device);

    VkPresentModeKHR present = choosePresentMode(details.present_modes);
    VkSurfaceFormatKHR format = chooseFormat(details.formats);
    VkExtent2D extent = chooseSwapExtent(details.capabilities);

    uint32_t image_count = std::min(details.capabilities.minImageCount + 1, details.capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR ci{};
    ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    ci.surface = surface;
    ci.minImageCount = image_count;
    ci.imageFormat = format.format;
    ci.imageColorSpace = format.colorSpace;
    ci.imageExtent = extent;
    ci.imageArrayLayers = 1;
    ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(p_device);
    uint32_t queueFamilyIndices[] = {indices.graphics.value(), indices.present.value()};

    if (indices.graphics != indices.present) {
        ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        ci.queueFamilyIndexCount = 2;
        ci.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ci.queueFamilyIndexCount = 0; // Optional
        ci.pQueueFamilyIndices = nullptr; // Optional
    }

    ci.preTransform = details.capabilities.currentTransform;
    ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ci.presentMode = present;
    ci.clipped = VK_TRUE;
    ci.oldSwapchain = nullptr;

    if(vkCreateSwapchainKHR(device, &ci, nullptr, &swapchain) != VK_SUCCESS){
        throw std::runtime_error("Couldn't create swapchain.");
    }

    uint32_t real_image_count;
    vkGetSwapchainImagesKHR(device, swapchain, &real_image_count, nullptr);

    sc_images.resize(real_image_count);
    vkGetSwapchainImagesKHR(device, swapchain, &real_image_count, sc_images.data());

    sc_format = format.format;
    sc_extent = extent;
}

//Populates SwapChainSupportDetails based on the VkPhysicalDevice.
Application::SwapChainSupportDetails Application::querySwapchainSupport(VkPhysicalDevice target){
    Application::SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(target, surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(target, surface, &format_count, nullptr);

    if(format_count != 0){
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(target, surface, &format_count, details.formats.data());
    }

    uint32_t present_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(target, surface, &present_count, nullptr);

    if(present_count != 0){
        details.present_modes.resize(present_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(target, surface, &present_count, details.present_modes.data());
    }

    return details;
}

//Chooses preferred VkPresentModeKHR from a list of available present modes.
VkPresentModeKHR Application::choosePresentMode(const std::vector<VkPresentModeKHR>& available_modes){
    for(VkPresentModeKHR mode : available_modes){
        if(mode == VK_PRESENT_MODE_MAILBOX_KHR){
            return mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

//Chooses preferred VkSurfaceFormatKHR from a list of available formats.
VkSurfaceFormatKHR Application::chooseFormat(const std::vector<VkSurfaceFormatKHR>& available_formats){
    for(VkSurfaceFormatKHR format : available_formats){
        if(format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            return format;
        }
    }

    return available_formats[0];
}

/*
    Chooses swap extent
    TODO - study swap extents and screen coordinates.
*/
VkExtent2D Application::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actual_extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actual_extent;
    }
}

//Creates the logical device to interface with the p_device
void Application::createLogicalDevice(){
    Application::QueueFamilyIndices indices = findQueueFamilies(p_device);

    std::vector<VkDeviceQueueCreateInfo> cis;
    std::set<uint32_t> families = {
        indices.graphics.value(),
        indices.present.value()
    };

    float priority = 1.0f;
    for (uint32_t family : families) {
        VkDeviceQueueCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        ci.queueFamilyIndex = family;
        ci.queueCount = 1;
        ci.pQueuePriorities = &priority;
        cis.push_back(ci);
    }

    VkPhysicalDeviceFeatures features{};

    VkDeviceCreateInfo deviceci{};

    deviceci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceci.queueCreateInfoCount = static_cast<uint32_t>(cis.size());
    deviceci.pQueueCreateInfos = cis.data();

    deviceci.pEnabledFeatures = &features;

    deviceci.enabledExtensionCount = 0;

    if(ENABLE_VALIDATION_LAYERS){
        deviceci.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        deviceci.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        deviceci.enabledLayerCount = 0;
    }

    deviceci.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
    deviceci.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data(); 

    if (vkCreateDevice(p_device, &deviceci, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("Couldn't create logical device.");
    }

    vkGetDeviceQueue(device, indices.graphics.value(), 0, &graphics_queue);
    vkGetDeviceQueue(device, indices.present.value(), 0, &present_queue);

}

//Creates the an image view for each VkImage in sc_images.
void Application::createImageViews(){
    sc_views.resize(sc_images.size());

    for(size_t i = 0; i<sc_images.size(); i++){
        VkImageViewCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ci.image = sc_images[i];
        ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ci.format = sc_format;
        ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ci.subresourceRange.baseMipLevel = 0;
        ci.subresourceRange.levelCount = 1;
        ci.subresourceRange.baseArrayLayer = 0;
        ci.subresourceRange.layerCount = 1;

        if(vkCreateImageView(device, &ci, nullptr, &sc_views[i]) != VK_SUCCESS){
            throw std::runtime_error("Couldn't create image view.");
        }
    }
}

void Application::recreateSwapChain(){
    vkDeviceWaitIdle(device);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createFrameBuffers();
}

void Application::cleanupSwapChain(){
    for(VkFramebuffer buffer : sc_fb){
        vkDestroyFramebuffer(device, buffer, nullptr);
    }
    for(VkImageView view : sc_views){
        vkDestroyImageView(device, view, nullptr);
    }

    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void Application::createRenderPass(){
    VkAttachmentDescription color_att{};
    color_att.format = sc_format;
    color_att.samples = VK_SAMPLE_COUNT_1_BIT;
    color_att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    color_att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    color_att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_att.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference att_ref{};
    att_ref.attachment = 0;
    att_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &att_ref;

    VkRenderPassCreateInfo rp_ci{};
    rp_ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &color_att;
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass;
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    rp_ci.dependencyCount = 1;
    rp_ci.pDependencies = &dependency;

    if(vkCreateRenderPass(device, &rp_ci, nullptr, &render_pass) != VK_SUCCESS){
        throw std::runtime_error("Couldn't create render pass.");
    }
}

/*
    Creates the Graphics Pipeline
    okay this is it yall
*/
void Application::createGraphicsPipeline(){
    VkShaderModule vert = createShaderModule("shaders/vert.spv");
    VkShaderModule frag = createShaderModule("shaders/frag.spv");

    VkPipelineShaderStageCreateInfo vert_ci{};
    vert_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_ci.module = vert;
    vert_ci.pName = "main";

    VkPipelineShaderStageCreateInfo frag_ci{};
    frag_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_ci.module = frag;
    frag_ci.pName = "main";

    VkPipelineShaderStageCreateInfo cis[] = {frag_ci, vert_ci};

    std::vector<VkDynamicState> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dyn_ci{};
    dyn_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn_ci.pDynamicStates = dynamic_states.data();
    dyn_ci.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());

    VkPipelineVertexInputStateCreateInfo vertinput_ci{};
    vertinput_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertinput_ci.vertexBindingDescriptionCount = 0;
    vertinput_ci.pVertexBindingDescriptions = nullptr;
    vertinput_ci.vertexAttributeDescriptionCount = 0;
    vertinput_ci.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo input_ci{};
    input_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_ci.primitiveRestartEnable = VK_FALSE;
    input_ci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float) sc_extent.width;
    viewport.height = (float) sc_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 0.0f;

    VkRect2D scissor{};
    scissor.offset = {0,0};
    scissor.extent = sc_extent;

    VkPipelineViewportStateCreateInfo viewport_ci{};
    viewport_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_ci.pScissors = &scissor;
    viewport_ci.pViewports = &viewport;
    viewport_ci.scissorCount = 1;
    viewport_ci.viewportCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer_ci{};
    rasterizer_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_ci.depthClampEnable = VK_FALSE;
    rasterizer_ci.rasterizerDiscardEnable = VK_FALSE;
    rasterizer_ci.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer_ci.lineWidth = 1.0f;
    rasterizer_ci.cullMode = VK_CULL_MODE_NONE;
    rasterizer_ci.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer_ci.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling_ci{};
    multisampling_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling_ci.sampleShadingEnable = VK_FALSE;
    multisampling_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState blend_att{};
    blend_att.blendEnable = VK_TRUE;
    blend_att.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blend_att.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blend_att.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blend_att.colorBlendOp = VK_BLEND_OP_ADD;
    blend_att.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blend_att.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blend_att.alphaBlendOp = VK_BLEND_OP_ADD;
    
    VkPipelineColorBlendStateCreateInfo blend_ci{};
    blend_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blend_ci.logicOpEnable = VK_FALSE;
    blend_ci.logicOp = VK_LOGIC_OP_COPY;
    blend_ci.attachmentCount = 1;
    blend_ci.pAttachments = &blend_att;
    
    VkPipelineLayoutCreateInfo layout_ci{};
    layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if(vkCreatePipelineLayout(device, &layout_ci, nullptr, &pl_layout) != VK_SUCCESS){
        throw std::runtime_error("Couldn't create Pipeline Layout.");
    }

    VkGraphicsPipelineCreateInfo pl_ci{};
    pl_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pl_ci.stageCount = 2;
    pl_ci.pStages = cis;
    pl_ci.layout = pl_layout;
    pl_ci.pVertexInputState = &vertinput_ci;
    pl_ci.pColorBlendState = &blend_ci;
    pl_ci.pInputAssemblyState = &input_ci;
    pl_ci.pViewportState = &viewport_ci;
    pl_ci.pMultisampleState = &multisampling_ci;
    pl_ci.pDynamicState = &dyn_ci;
    pl_ci.pRasterizationState = &rasterizer_ci;

    pl_ci.layout = pl_layout;
    pl_ci.renderPass = render_pass;
    pl_ci.subpass = 0;

    if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pl_ci, nullptr, &pipeline) != VK_SUCCESS){
        throw std::runtime_error("Couldn't create graphics pipeline.");
    }

    vkDestroyShaderModule(device, vert, nullptr);
    vkDestroyShaderModule(device, frag, nullptr);
}

VkShaderModule Application::createShaderModule(const std::string& path){
    const std::vector<char>& file = readFile(path);
    
    VkShaderModuleCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = file.size();
    ci.pCode = reinterpret_cast<const uint32_t*>(file.data());

    VkShaderModule module;
    if(vkCreateShaderModule(device, &ci, nullptr, &module) != VK_SUCCESS){
        throw std::runtime_error("Couldn't create Shader Module.");
    }

    return module;

}

void Application::createFrameBuffers(){
    sc_fb.resize(sc_images.size());

    for(size_t i = 0; i < sc_fb.size(); i++){
        VkImageView atts[] = {
            sc_views[i]
        };

        VkFramebufferCreateInfo fb_ci{};
        fb_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb_ci.attachmentCount = 1;
        fb_ci.pAttachments = atts;
        fb_ci.renderPass = render_pass;
        fb_ci.width = sc_extent.width;
        fb_ci.height = sc_extent.height;
        fb_ci.layers = 1;

        if(vkCreateFramebuffer(device, &fb_ci, nullptr, &sc_fb[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to create framebuffer" + std::to_string(i) + ".");
        }
    }
}

void Application::createCommandPoolBuffer(){

    QueueFamilyIndices indices = findQueueFamilies(p_device);

    VkCommandPoolCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ci.queueFamilyIndex = indices.graphics.value();

    if(vkCreateCommandPool(device, &ci, nullptr, &cmdp) != VK_SUCCESS){
        throw std::runtime_error("Couldn't create graphics command pool.");
    }

    cmdb.resize(MAX_FLIGHT_FRAMES);

    VkCommandBufferAllocateInfo buffer_i{};
    buffer_i.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    buffer_i.commandBufferCount = MAX_FLIGHT_FRAMES;
    buffer_i.commandPool = cmdp;
    buffer_i.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if(vkAllocateCommandBuffers(device, &buffer_i, &cmdb[cur_frame]) != VK_SUCCESS) {
        throw std::runtime_error("Couldn't allocate command buffers.");
    }
}

void Application::recordCommandBuffer(VkCommandBuffer target, uint32_t image_index){
    VkCommandBufferBeginInfo begin_i{};
    begin_i.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    if(vkBeginCommandBuffer(target, &begin_i) != VK_SUCCESS){
        throw std::runtime_error("Couldn't begin recording command buffer.");
    }

    VkRenderPassBeginInfo rp_bi{};
    rp_bi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_bi.renderPass = render_pass;
    rp_bi.framebuffer = sc_fb[image_index];
    rp_bi.renderArea.offset = {0,0};
    rp_bi.renderArea.extent = sc_extent;

    VkClearValue clear = {{{0.2f, 0.2f, 0.2f, 1.0f}}};
    rp_bi.clearValueCount = 1;
    rp_bi.pClearValues = &clear;

    vkCmdBeginRenderPass(cmdb[cur_frame], &rp_bi, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmdb[cur_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(sc_extent.width);
    viewport.height = static_cast<float>(sc_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdb[cur_frame], 0, 1, &viewport);
    
    VkRect2D scissor{};
    scissor.offset = {0,0};
    scissor.extent = sc_extent;
    vkCmdSetScissor(cmdb[cur_frame], 0, 1, &scissor);

    vkCmdDraw(cmdb[cur_frame], 3, 1, 0, 0);

    vkCmdEndRenderPass(cmdb[cur_frame]);

    if(vkEndCommandBuffer(cmdb[cur_frame]) != VK_SUCCESS){
        throw std::runtime_error("Failed to record command buffer.");
    }
}

void Application::createSyncObjects(){
    sps_image_available.resize(MAX_FLIGHT_FRAMES);
    sps_render_finished.resize(MAX_FLIGHT_FRAMES);
    fs_flight.resize(MAX_FLIGHT_FRAMES);

    VkSemaphoreCreateInfo sp_ci{};
    sp_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    sp_ci.pNext = nullptr;

    VkFenceCreateInfo f_ci{};
    f_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    f_ci.flags =  VK_FENCE_CREATE_SIGNALED_BIT;
    f_ci.pNext = nullptr;

    for(size_t i = 0; i < MAX_FLIGHT_FRAMES; i++){
        VkBool32 ia = vkCreateSemaphore(device, &sp_ci, nullptr, &sps_image_available[i]);
        VkBool32 rf = vkCreateSemaphore(device, &sp_ci, nullptr, &sps_render_finished[i]);
        VkBool32 fl = vkCreateFence(device, &f_ci, nullptr, &fs_flight[i]);
        if(ia || rf || fl != VK_SUCCESS){
            throw std::runtime_error("Couldn't create sync objects for a frame.");
        } 
    }
}

//Main loop of the application.
void Application::mainLoop() {
    while(!glfwWindowShouldClose(window)){ // while the window should'nt close:
        glfwPollEvents(); // poll glfw events
        if(vkWaitForFences(device, 1, &fs_flight[cur_frame], VK_TRUE, UINT64_MAX) != VK_SUCCESS){
            throw std::runtime_error("Couldnt wait for flight fences.");
        }
        if(vkResetFences(device, 1, &fs_flight[cur_frame]) != VK_SUCCESS){
            throw std::runtime_error("Couldn't reset flight fences.");
        }
        drawFrame();
    }

    vkDeviceWaitIdle(device);
}

void Application::drawFrame(){
    uint32_t image_index;
    if (vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, sps_image_available[cur_frame], VK_NULL_HANDLE, &image_index) != VK_SUCCESS){
        throw std::runtime_error("Couldn't aquire next image in the swapchain.");
    }
    
    if(vkResetCommandBuffer(cmdb[cur_frame], 0) != VK_SUCCESS){
        throw std::runtime_error("Couldn't reset command buffer.");
    }
    recordCommandBuffer(cmdb[cur_frame], image_index);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {sps_image_available[cur_frame]};
    VkPipelineStageFlags stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmdb[cur_frame];

    VkSemaphore signal_semaphores[] = {sps_render_finished[cur_frame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    if(vkQueueSubmit(graphics_queue, 1, &submit_info, fs_flight[cur_frame]) != VK_SUCCESS){
        throw std::runtime_error("Couldn't submit draw queue commands.");
    }


    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    
    VkSwapchainKHR swapchains[] = {swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;

    if(vkQueuePresentKHR(present_queue, &present_info) != VK_SUCCESS){
        throw std::runtime_error("Couldn't submit queue present commands.");
    }

    cur_frame = (cur_frame + 1) % MAX_FLIGHT_FRAMES;
}

//Cleans up and closes everything.
void Application::cleanUp() {
    if(ENABLE_VALIDATION_LAYERS){
        DestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr); // DESTROY THE THING
    }

    for(size_t i = 0; i < MAX_FLIGHT_FRAMES; i++){ // DESTROY SYNC OBJECTS
        vkDestroySemaphore(device, sps_image_available[i], nullptr);
        vkDestroySemaphore(device, sps_render_finished[i], nullptr);
        vkDestroyFence(device, fs_flight[i], nullptr);
    }    

   
    cleanupSwapChain(); // DESTROY SWAPCHAIN

    vkDestroyCommandPool(device, cmdp, nullptr); // DESTROY COMMAND POOL

    vkDestroyPipeline(device, pipeline, nullptr); // DESTROY PIPELINE
    vkDestroyPipelineLayout(device, pl_layout, nullptr); // DESTROY PIPELINE LAYOUT
    vkDestroyRenderPass(device, render_pass, nullptr); // DESTROY RENDER PASS

    vkDestroyDevice(device, nullptr); // DESTROY LOGICAL DEVICE

    vkDestroySurfaceKHR(instance, surface, nullptr); // DESTROY WINDOW SURFACE
    vkDestroyInstance(instance, nullptr); // DESTROY VULKAN INSTANCE

    glfwDestroyWindow(window); // DESTROY WINDOW
    glfwTerminate(); // TERMINATE GLFW
}

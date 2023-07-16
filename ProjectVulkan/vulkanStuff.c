#include "vulkanStuff.h"
#pragma warning(disable:4996)

/// INSTANCE AND DEVICE CREATION ///
/// ============================ ///
///--------------------------------------------------------------///

void createInstanceRelatedObjects(struct Application* pApp) {
    createInstance(pApp);
    createSurface(pApp);
    pickPhysicalDevice(pApp);
    createLogicalDevice(pApp);
}

// Instance and surface creation
// -----------------------------

void createInstance(struct Application* pApp) {
    if (!checkValidationLayerSupport()) {
        printf("Didn't find the validation layer\n");
    }

    // Create applications informations

    VkApplicationInfo applicationInfo = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        VK_NULL_HANDLE,
        VK_NULL_HANDLE,
        0,
        VK_NULL_HANDLE,
        0,
        VK_API_VERSION_1_0
    };

    // Enable validation layers

    const char layerList[][VK_MAX_EXTENSION_NAME_SIZE] = {
        "VK_LAYER_KHRONOS_validation"
    };
    const char* layers[] = {
        layerList[0]
    };

    // Get the glfw extensions

    uint32_t extensionNumber = 0;
    const char* const* extensions = glfwGetRequiredInstanceExtensions(&extensionNumber);
    
    // Create instance informations

    VkInstanceCreateInfo instanceCreateInfo = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        &applicationInfo,
        1,
        layers,
        extensionNumber,
        extensions
    };

    // Create instance

    VkResult instanceResult = vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &pApp->instance);
    if (instanceResult != VK_SUCCESS) {
        printf("Instance !well created!\n");
    }
}

int checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, VK_NULL_HANDLE);


    VkLayerProperties* availableLayers = (VkLayerProperties*)malloc(layerCount * sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    int layerFound = 0;
    if (availableLayers != NULL){
        for (uint32_t i = 0; i < layerCount; i) {
            if (strcmp(availableLayers[i].layerName, "VK_LAYER_KHRONOS_validation") == 0) {
                layerFound = 1;
                break;
            }
        };
    }
    free(availableLayers);
    return layerFound;
}

void createSurface(struct Application* pApp) {
    if (glfwCreateWindowSurface(pApp->instance, pApp->window, NULL, &pApp->surface) != VK_SUCCESS) printf("Surface !well created!\n");
}

// Physical Device choice
// ----------------------

void pickPhysicalDevice(struct Application* pApp) {
    // Get number of devices on computer

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(pApp->instance, &deviceCount, NULL);

    // Fill my array of devices with the physical devices

    VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(deviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(pApp->instance, &deviceCount, devices);

    // Querying the devices -> must have the good queues

    int32_t queueFamilyCount = 0;
    VkPhysicalDevice* pBestPhysicalDevice = &devices[0];
    if (devices != NULL){
        for (uint32_t i = 0; i < deviceCount; i++) {
            // Discriminate device
            if (isDeviceSuitable(&devices[i], pApp->surface))
            {
                pApp->physicalDevice = devices[i];
                break;
            }
        };
    }
    free(devices);
}

int isDeviceSuitable(VkPhysicalDevice* pDevice, VkSurfaceKHR surface) {
    // Create a struct of indices -> queues indices will be stored there
    struct QueueFamilyIndices indices = findQueueFamilies(pDevice, surface, 1);

    // Try to know if the device support swapChain
    int swapChainAdequate = 0;
    struct SwapChainSupportDetails swapChainSupport = querySwapChainSupport(pDevice, surface);
    swapChainAdequate = !swapChainSupport.formatsEmpty && !swapChainSupport.presentModesEmpty;
    
    // If all the queues has an indice and found swapchain 
    return isQueueComplete(indices) & swapChainAdequate;
}

struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice* pDevice, VkSurfaceKHR surface, int showComponents) {
    // Create the struct to store indices, if value = -1 <=> no value / indice

    struct QueueFamilyIndices indices = { .graphicsFamily = -1 };

    // Get the amount of queue family in specified device

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(*pDevice, &queueFamilyCount, VK_NULL_HANDLE);

    // Store the queue family properties 

    VkQueueFamilyProperties* queueFamilyProperties = (VkQueueFamilyProperties*)malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(*pDevice, &queueFamilyCount, queueFamilyProperties);

    // Testing each queue family
    
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        // If the queue family has the queue flag for graphicsQueue -> store it
        if ((queueFamilyProperties + i) != NULL) {
            if ((queueFamilyProperties + i)->queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
        }
        // If the queue family has the property for present queue -> store it

        VkBool32 presentSupport = 0;
        vkGetPhysicalDeviceSurfaceSupportKHR(*pDevice, i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentsFamily = i;
        }

        // If both conditions are satisfied -> useless to continue 

        if (isQueueComplete(indices)) {
            break;
        }

    };

    free(queueFamilyProperties);
    return indices;
}

struct SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice* pDevice, VkSurfaceKHR surface) {
    // Create the detais struct to store capabilities, format and present mode

    struct SwapChainSupportDetails details = { .formatsEmpty = 1,.presentModesEmpty = 1 };

    // Store capabilities 

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*pDevice, surface, &details.capabilities);

    // Get the amount of formats in specified device

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(*pDevice, surface, &formatCount, NULL);

    // If there is format -> update that there are formats -> store 
    // WARNING HARDCODED -> ALREADY KNOW THERE ARE 25 FORMATS

    if (formatCount) {
        details.formatsEmpty = 0;
        details.formatsCount = formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(*pDevice, surface, &formatCount, &*details.formats);
    }

    // Get the amount of present modes in specified device

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(*pDevice, surface, &presentModeCount, NULL);

    // If there is present modes -> update that there are present modes -> store 
    // WARNING HARDCODED -> ALREADY KNOW THERE ARE 3 PRESENT MODES

    if (presentModeCount) {
        details.presentModesEmpty = 0;
        details.presentModesCount = presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(*pDevice, surface, &presentModeCount, &*details.presentModes);
    }


    return details;
}

int isQueueComplete(struct QueueFamilyIndices indice) {
    if (indice.graphicsFamily >= 0 && indice.presentsFamily >= 0) {
        return 1;
    }
    else {
        return 0;
    }
}

// Logical Device creation
// -----------------------

void createLogicalDevice(struct Application* pApp) {
    // Now we have a physical device, get the queues family indices 

    struct QueueFamilyIndices indices = findQueueFamilies(&pApp->physicalDevice, pApp->surface, 0);

    // Create queues informations
    // WARNING HARDCODED -> ALREADY KNOW THERE ARE 2 QUEUES

    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo* deviceQueueCreateInfo = (VkDeviceQueueCreateInfo*)malloc(QUEUES_COUNT * sizeof(VkDeviceQueueCreateInfo));
    
    if (deviceQueueCreateInfo != NULL){
        for (uint32_t i = 0; i < QUEUES_COUNT; i++) {
            deviceQueueCreateInfo[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            deviceQueueCreateInfo[i].pNext = VK_NULL_HANDLE;
            deviceQueueCreateInfo[i].flags = 0;
            deviceQueueCreateInfo[i].queueFamilyIndex = 0;
            deviceQueueCreateInfo[i].queueCount = 1;
            deviceQueueCreateInfo[i].pQueuePriorities = &queuePriority;
        };
    }
    // Add extension for swapchain

    const char extensionList[][VK_MAX_EXTENSION_NAME_SIZE] = {
        "VK_KHR_swapchain"
    };
    const char* extensions[] = {
        extensionList[0]
    };

    // Add features

    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    vkGetPhysicalDeviceFeatures(pApp->physicalDevice, &physicalDeviceFeatures);

    // Create device informations

    VkDeviceCreateInfo deviceCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        1,
        deviceQueueCreateInfo,
        0,
        VK_NULL_HANDLE,
        1,
        extensions,
        &physicalDeviceFeatures
    };

    // Create the device

    VkResult creationResult = vkCreateDevice(pApp->physicalDevice, &deviceCreateInfo, VK_NULL_HANDLE, &pApp->device);

    // Linking the queues

    vkGetDeviceQueue(pApp->device, indices.graphicsFamily, 0, &pApp->graphicsQueue);
    vkGetDeviceQueue(pApp->device, indices.presentsFamily, 0, &pApp->presentQueue);

    free(deviceQueueCreateInfo);

}

/// SHADER LOADING ///
/// ============== ///
///--------------------------------------------------------------///

char* readShader(const char* fileName, uint32_t* pShaderSize) {
    if (pShaderSize == VK_NULL_HANDLE) {
        return VK_NULL_HANDLE;
    }
    FILE* fp = VK_NULL_HANDLE;
    fp = fopen(fileName, "rb+");
    if (fp == VK_NULL_HANDLE) {
        return VK_NULL_HANDLE;
    }
    fseek(fp, 0l, SEEK_END);
    *pShaderSize = (uint32_t)ftell(fp);
    rewind(fp);

    char* shaderCode = (char*)malloc((*pShaderSize) * sizeof(char));
    if (shaderCode != 0){
        fread(shaderCode, 1, *pShaderSize, fp);
    }
    fclose(fp);
    return shaderCode;
}

VkShaderModule createShaderModule(struct Application* pApp, char* code, uint32_t size) {
    // Create shadermodule informations
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        size,
        (const uint32_t*)code
    };

    // Create shaderodule itself

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(pApp->device, &shaderModuleCreateInfo, NULL, &shaderModule) != VK_SUCCESS) {
        printf("Shader module !well created!\n");
    }
    return shaderModule;
}

/// PIPELINE CREATION ///
/// ================= ///
///--------------------------------------------------------------///

void createPipelineRelatedObjects(struct Application* pApp) {
    createSwapChain(pApp);
    createImageViews(pApp);
    createRenderPass(pApp);
    createGraphicsPipeline(pApp);
}

// SwapChain Creation //
// ------------------ //

void createSwapChain(struct Application* pApp) {
    // Get the details of the swapchain support

    struct SwapChainSupportDetails swapChainSupport = querySwapChainSupport(&pApp->physicalDevice, pApp->surface);

    // Selecting extent, format, and present mode

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formatsCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.presentModesCount);
    VkExtent2D extent = chooseSwapExtent(pApp, swapChainSupport.capabilities);

    // Setting image count as minimum + 1, if > max -> clamp

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // Create swapchain informations

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        VK_NULL_HANDLE,
        0,
        pApp->surface,
        imageCount,
        surfaceFormat.format,
        surfaceFormat.colorSpace,
        extent,
        1,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        NULL,
        swapChainSupport.capabilities.currentTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        presentMode,
        VK_TRUE,
        VK_NULL_HANDLE
    };

    // Create the swapchain itself

    if (vkCreateSwapchainKHR(pApp->device, &swapchainCreateInfo, NULL, &pApp->swapChain) != VK_SUCCESS) {
        printf("Swap chain !well created!\n");
    }

    // Link swapchain to images

    vkGetSwapchainImagesKHR(pApp->device, pApp->swapChain, &imageCount, NULL);
    vkGetSwapchainImagesKHR(pApp->device, pApp->swapChain, &imageCount, pApp->swapChainImages);

    // Store format and extent

    pApp->swapChainImageFormat = surfaceFormat.format;
    pApp->swapChainExtent = extent;

}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR availableFormats[], int formatCount) {
    // Listing all the formats to find the right one

    for (int i = 0; i < formatCount; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR availablePresentModes[], int presentModesCount) {
    // Listing all the present modes to find the right one
    for (int i = 0; i < presentModesCount; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }
    // If didn't find the better one (my case) return the second best one
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(struct Application* pApp, VkSurfaceCapabilitiesKHR capabilities) {
    if (capabilities.currentExtent.width != INT_MAX) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(pApp->window, &width, &height);

        VkExtent2D actualExtent = { (uint32_t)width,(uint32_t)height};
        actualExtent.width = MAX(MIN(actualExtent.width, capabilities.maxImageExtent.width), capabilities.minImageExtent.width);
        actualExtent.height = MAX(MIN(actualExtent.height, capabilities.maxImageExtent.height), capabilities.minImageExtent.height);
        return actualExtent;

    }
}

// Image view creation //
// ------------------- //

void createImageViews(struct Application* pApp) {
    // Set up the image view mapping

    VkComponentMapping componentMapping = {
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY
    };

    VkImageSubresourceRange imageSubresourceRange = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        1,
        0,
        1
    };

    // For each image, create informations

    for (int i = 0; i < IMAGE_COUNT; i++) {
        // Create indexed imageview informations

        VkImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = VK_NULL_HANDLE;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.image = (pApp->swapChainImages[i]);
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = pApp->swapChainImageFormat;
        imageViewCreateInfo.components = componentMapping;
        imageViewCreateInfo.subresourceRange = imageSubresourceRange;

        // Create the imageview itself

        if (vkCreateImageView(pApp->device, &imageViewCreateInfo, VK_NULL_HANDLE, &pApp->swapChainImageViews[i]) != VK_SUCCESS) {
            printf("Images view !well created!\n");
        }
    }

}

// Renderpass creation //
// ------------------- //

void createRenderPass(struct Application* pApp) {
    // Set up the color attachment 

    VkAttachmentDescription colorAttachment = {
        0,
        pApp->swapChainImageFormat,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentRef = {
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    // Set up the subpass

    VkSubpassDescription subpass = {
        0,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,
        VK_NULL_HANDLE,
        1,
        &colorAttachmentRef,
        VK_NULL_HANDLE,
        VK_NULL_HANDLE,
        0,
        VK_NULL_HANDLE
    };

    // Set up dependancy

    VkSubpassDependency dependency = {
        VK_SUBPASS_EXTERNAL,
        0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        0,
    };

    // Create render pass informations

    VkRenderPassCreateInfo renderPassInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        1,
        &colorAttachment,
        1,
        &subpass,
        1,
        &dependency

    };

    // Create the renderpass itself

    if (vkCreateRenderPass(pApp->device, &renderPassInfo, NULL, &pApp->renderPass) != VK_SUCCESS) {
        printf("RenderPass !well created !\n");
    }

}

// Pipeline creation //
// ----------------- //

void createGraphicsPipeline(struct Application* pApp) {
    // Set up the shaders

    uint32_t vertSize = 0;
    uint32_t fragSize = 0;

    // Read and store shaders

    char* vertShaderCode = readShader("Shaders/vert.spV", &vertSize);
    char* fragShaderCode = readShader("Shaders/frag.spv", &fragSize);

    // Create shader modules

    pApp->vertShaderModule = createShaderModule(pApp, vertShaderCode, vertSize);
    pApp->fragShaderModule = createShaderModule(pApp, fragShaderCode, fragSize);

    // Creating vertex shaders infos

    VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_SHADER_STAGE_VERTEX_BIT,
        pApp->vertShaderModule,
        "main",
        VK_NULL_HANDLE
    };

    // Creating fragment shaders infos

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        pApp->fragShaderModule,
        "main",
        VK_NULL_HANDLE
    };

    VkPipelineShaderStageCreateInfo shaderStages[2] = { vertexShaderStageCreateInfo, fragShaderStageCreateInfo };

    // Creating dynamic stats infos

    VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        2,
        dynamicStates
    };

    // Creating vertex input infos

    VkVertexInputBindingDescription bindingDescription = getVertexBindingDescription();
    getAttributeDescriptions(pApp->attributeDescriptions);


    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        1,
        &bindingDescription,
        2,
        pApp->attributeDescriptions
    };

    // Creating input assemblies

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE,
    };

    // Creating Viewports 

    VkViewport viewport = {
        0.0f,
        0.0f,
        (float)pApp->swapChainExtent.width,
        (float)pApp->swapChainExtent.height,
        0.0f,
        1.0f,
    };

    // Creating scissors

    VkRect2D scissor = {
        {0,0},
        pApp->swapChainExtent
    };

    // Creating viewport state

    VkPipelineViewportStateCreateInfo viewportState = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        1,
        &viewport,
        1,
        &scissor
    };

    // Creating rasterization

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_BACK_BIT,
        VK_FRONT_FACE_CLOCKWISE,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };

    // Creating multisampler

    VkPipelineMultisampleStateCreateInfo multisampling = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FALSE,
        1.0f,
        VK_NULL_HANDLE,
        VK_FALSE,
        VK_FALSE
    };

    // Blending colors

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        VK_FALSE,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_FALSE,
        VK_LOGIC_OP_COPY,
        1,
        &colorBlendAttachment,
        {0.0f,0.0f,0.0f,0.0f}
    };

    // Create pipe line layout infos

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        0,
        VK_NULL_HANDLE,
        0,
        VK_NULL_HANDLE
    };

    if (vkCreatePipelineLayout(pApp->device, &pipelineLayoutInfo, NULL, &pApp->pipelineLayout) != VK_SUCCESS) {
        printf("Pipeline !well created!\n");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        2,
        shaderStages,
        &vertexInputCreateInfo,
        &inputAssembly,
        VK_NULL_HANDLE,
        &viewportState,
        &rasterizer,
        &multisampling,
        VK_NULL_HANDLE,
        &colorBlending,
        &dynamicStateCreateInfo,
        pApp->pipelineLayout,
        pApp->renderPass,
        0,
        VK_NULL_HANDLE,
        -1
    };

    if (vkCreateGraphicsPipelines(pApp->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &pApp->graphicsPipeline) != VK_SUCCESS) {
        printf("Graphics Pipeline !well created!\n");
    }


}


/// BUFFERS AND VERTEX AND PRESENT STUFF ///
/// ==================================== ///
///--------------------------------------------------------------///

void createBufferRelatedObjects(struct Application* pApp) {
    createFrameBuffers(pApp);
    createCommandPool(pApp);
    createVertexBuffer(pApp);
    createCommandBuffer(pApp);
    createSyncObjects(pApp);
}

// Vertex description //
// ------------------ //

VkVertexInputBindingDescription getVertexBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {
        0,
        2 * sizeof(float) + 3 * sizeof(float),
        VK_VERTEX_INPUT_RATE_VERTEX
    };
    return bindingDescription;
}

void getAttributeDescriptions(VkVertexInputAttributeDescription* pAttributeDescriptions) {
    pAttributeDescriptions[0].binding = 0;
    pAttributeDescriptions[0].location = 0;
    pAttributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    pAttributeDescriptions[1].offset = 0;

    pAttributeDescriptions[1].binding = 0;
    pAttributeDescriptions[1].location = 1;
    pAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    pAttributeDescriptions[1].offset = 2 * sizeof(float);
}


// FrameBuffer creation //
// -------------------- //

void createFrameBuffers(struct Application* pApp) {
    // For both images create a frame buffer

    for (size_t i = 0; i < IMAGE_COUNT; i++) {

        // Create frame buffer informations 

        VkFramebufferCreateInfo framebufferInfo = {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            VK_NULL_HANDLE,
            0,
            pApp->renderPass,
            1,
            &pApp->swapChainImageViews[i],
            pApp->swapChainExtent.width,
            pApp->swapChainExtent.height,
            1
        };

        // Creata a frame buffer itself

        if (vkCreateFramebuffer(pApp->device, &framebufferInfo, NULL, &pApp->swapChainFramebuffers[i]) != VK_SUCCESS) {
            printf("Framebuffer !well created!\n");
        }

    }
}

// CommandPool creation //
// -------------------- //

void createCommandPool(struct Application* pApp) {
    // Get the queues indices 

    struct QueueFamilyIndices queueFamilyIndices = findQueueFamilies(&pApp->physicalDevice, pApp->surface, 0);

    // Create command pool informations

    VkCommandPoolCreateInfo poolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        VK_NULL_HANDLE,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        (uint32_t) queueFamilyIndices.graphicsFamily
    };

    // Create the command pool itself

    if (vkCreateCommandPool(pApp->device, &poolInfo, NULL, &pApp->commandPool) != VK_SUCCESS) {
        printf("CommandPool !well created!\n");
    }

}

// Vertex buffer creation //
// ---------------------- //

void createVertexBuffer(struct Application* pApp) {
    // Create the buffer informations

    VkBufferCreateInfo bufferInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
       (sizeof(float) * 3 + sizeof(float) * 2) * 3,
       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
       VK_SHARING_MODE_EXCLUSIVE
    };

    // Create the buffer itself

    if (vkCreateBuffer(pApp->device, &bufferInfo, NULL, &pApp->vertexBuffer) != VK_SUCCESS) {
        printf("VertexBuffer !well created!\n");
    }

    // Allocate the memory to the buffer in a new object

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(pApp->device, pApp->vertexBuffer, &memRequirements);

    // Create the memory allocation informations

    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        VK_NULL_HANDLE,
        memRequirements.size,
        findMemoryType(pApp,memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };

    // Create the memory allocation itself 

    if (vkAllocateMemory(pApp->device, &allocInfo, NULL, &pApp->vertexBufferMemory) != VK_SUCCESS) {
        printf("Memory Allocation !well created!\n");
    }

    // Bind the memory

    vkBindBufferMemory(pApp->device, pApp->vertexBuffer, pApp->vertexBufferMemory, 0);
    pApp->bufferInfoSize = bufferInfo.size;

    // Allocate the memmory

    bindVertexBuffers(pApp);
}

void bindVertexBuffers(struct Application* pApp) {
    // create a null pointer to store the vertex buffer data
    void* data;
    vkMapMemory(pApp->device, pApp->vertexBufferMemory, 0, pApp->bufferInfoSize, 0, &data);

    // copy the data

    memcpy(data, pApp->vertices, (size_t)pApp->bufferInfoSize);
    vkUnmapMemory(pApp->device, pApp->vertexBufferMemory);
}

uint32_t findMemoryType(struct Application* pApp, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    // Bullshit i guess
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(pApp->physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
}


// Commands buffers creation //
// ------------------------- //

void createCommandBuffer(struct Application* pApp) {
    // Create the command buffer informations

    VkCommandBufferAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        VK_NULL_HANDLE,
        pApp->commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        MAX_FRAMES_IN_FLIGHT
    };

    // Create the command buffer itself

    if (vkAllocateCommandBuffers(pApp->device, &allocInfo, pApp->commandBuffer) != VK_SUCCESS) {
        printf("CommandBuffer !well created!\n");
    }
}

// Semaphores and fence creation //
// ----------------------------- //

void createSyncObjects(struct Application* pApp) {
    // Create standards semaphore informaions 

    VkSemaphoreCreateInfo semaphoreInfo = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        VK_NULL_HANDLE,
        0
    };

    // Create standards fence informaions 
    VkFenceCreateInfo fenceInfo = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        VK_NULL_HANDLE,
        VK_FENCE_CREATE_SIGNALED_BIT
    };

    // Create 2 semaphores and 2 fences
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(pApp->device, &semaphoreInfo, NULL, &pApp->imageAvailableSemaphore[i]) != VK_SUCCESS ||
            vkCreateSemaphore(pApp->device, &semaphoreInfo, NULL, &pApp->renderFinishedSemaphore[i]) != VK_SUCCESS ||
            vkCreateFence(pApp->device, &fenceInfo, NULL, &pApp->inFlightFence[i]) != VK_SUCCESS) {
            printf("Fence and semaphores !well created!\n");
        }
    }
}

#pragma once
#define GLFW_INCLUDE_VULKAN
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#include "appStruct.c"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>



struct  QueueFamilyIndices {
    int32_t graphicsFamily;
    int32_t presentsFamily;
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR formats[FORMATS_COUNT];
    VkPresentModeKHR presentModes[PRESENT_MODES_COUNT];
    int formatsEmpty;
    int presentModesEmpty;
    int formatsCount;
    int presentModesCount;
};



/// INSTANCE AND DEVICE CREATION ///
/// ============================ ///
///--------------------------------------------------------------///


void createInstanceRelatedObjects(struct Application* pApp);

// Creating Instance and Surface
// -----------------------------

void createInstance(struct Application* pApp);
int checkValidationLayerSupport();
void createSurface(struct Application* pApp);

// Physical Device choice
// ----------------------

void pickPhysicalDevice(struct Application* pApp);
int isDeviceSuitable(VkPhysicalDevice* pDevice, VkSurfaceKHR surface);
int isQueueComplete(struct QueueFamilyIndices indice);
struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice* pDevice, VkSurfaceKHR surface, int showComponents);
struct SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice* pDevice, VkSurfaceKHR surface);

// Logical Device creation
// -----------------------

void createLogicalDevice(struct Application* pApp);

/// SHADER LOADING ///
/// ============== ///


VkShaderModule createShaderModule(struct Application* pApp, char* code, uint32_t size);
char* readShader(const char* fileName, uint32_t* pShaderSize);

/// PIPELINE CREATION ///
/// ================= ///
///--------------------------------------------------------------///


void createPipelineRelatedObjects(struct Application* pApp);

// SwapChain Creation //
// ------------------ //

void createSwapChain(struct Application* pApp);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR availableFormats[], int formatCount);
VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR availablePresentModes[], int presentModesCount);
VkExtent2D chooseSwapExtent(struct Application* pApp, VkSurfaceCapabilitiesKHR capabilities);

// Image view creation //
// ------------------- //

void createImageViews(struct Application* pApp);

// Renderpass creation //
// ------------------- //

void createRenderPass(struct Application* pApp);

// Pipeline creation //
// ----------------- //

void createGraphicsPipeline(struct Application* pApp);

/// BUFFERS AND VERTEX AND PRESENT STUFF ///
/// ==================================== ///
///--------------------------------------------------------------///


void createBufferRelatedObjects(struct Application* pApp);

// Vertex description //
// ------------------ //

VkVertexInputBindingDescription getVertexBindingDescription();
void getAttributeDescriptions(VkVertexInputAttributeDescription* pAttributeDescriptions);

// FrameBuffer creation //
// -------------------- //

void createFrameBuffers(struct Application* pApp);

// CommandPool creation //
// -------------------- //

void createCommandPool(struct Application* pApp);

// Vertex buffer creation //
// ---------------------- //

void createVertexBuffer(struct Application* pApp);

uint32_t findMemoryType(struct Application* pApp, uint32_t typeFilter, VkMemoryPropertyFlags properties);
void bindVertexBuffers(struct Application* pApp);

// Commands buffers creation //
// ------------------------- //

void createCommandBuffer(struct Application* pApp);


// Semaphores and fence creation //
// ----------------------------- //

void createSyncObjects(struct Application* pApp);




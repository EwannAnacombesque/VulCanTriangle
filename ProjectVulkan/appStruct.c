#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <cglm/cglm.h>
#define CIRCLE_DIVISIONS 3
#define IMAGE_COUNT 2
#define MAX_FRAMES_IN_FLIGHT 2
#define QUEUES_COUNT 2
#define PRESENT_MODES_COUNT 3
#define FORMATS_COUNT 25

struct Application {
    // Instance Device Surface Window
    GLFWwindow* window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;


    // Queues
    VkQueue graphicsQueue;
    VkQueue presentQueue;


    // Pipeline
    VkSwapchainKHR swapChain;
    VkImage swapChainImages[IMAGE_COUNT];
    VkImageView swapChainImageViews[IMAGE_COUNT];
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;



    // Shader
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;

    // Buffering

    VkFramebuffer swapChainFramebuffers[IMAGE_COUNT];
    VkCommandPool commandPool;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkVertexInputAttributeDescription attributeDescriptions[2];

    VkCommandBuffer commandBuffer[MAX_FRAMES_IN_FLIGHT];
    VkDeviceSize bufferInfoSize;

    // Sync objects
    VkSemaphore imageAvailableSemaphore[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinishedSemaphore[MAX_FRAMES_IN_FLIGHT];

    // Rendering
    VkFence inFlightFence[MAX_FRAMES_IN_FLIGHT];
    uint32_t currentFrame;

    float* vertices;

    int width;
    int heigth;
    char name[30];
};

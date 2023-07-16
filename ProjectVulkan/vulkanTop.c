#include "vulkanStuff.h"

// Initialisation and destruction //
// ------------------------------ //


void initWindow(struct Application* pApp);
void initVulkan(struct Application* pApp);
void cleanUp(struct Application* pApp);

// Drawing | most important functions //
// ---------------------------------- //

void drawFrame(struct Application* pApp);
void recordCommandBuffer(struct Application* pApp, uint32_t imageIndex);
void recreateSwapChain(struct Application* pApp);


// DEFINITION


// Initialisation and destruction //
// ------------------------------ //

void initVulkan(struct Application* pApp) {
    // Creating instance, surface and device
    createInstanceRelatedObjects(pApp);

    // Creating graphic pipeline
    createPipelineRelatedObjects(pApp);

    // Creating buffers and commands
    createBufferRelatedObjects(pApp);
}

void initWindow(struct Application* pApp) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    pApp->window = glfwCreateWindow(pApp->width, pApp->heigth, "Vulkan", NULL, NULL);
}

void cleanUp(struct Application* pApp) {
    // Destroy the sync objects

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(pApp->device, pApp->imageAvailableSemaphore[i], NULL);
        vkDestroySemaphore(pApp->device, pApp->renderFinishedSemaphore[i], NULL);
        vkDestroyFence(pApp->device, pApp->inFlightFence[i], NULL);
    }

    // Destroy buffer stuff
    vkDestroyBuffer(pApp->device, pApp->vertexBuffer, NULL);
    vkFreeMemory(pApp->device, pApp->vertexBufferMemory, NULL);
    vkDestroyCommandPool(pApp->device, pApp->commandPool, NULL);
    for (int i = 0; i < 2; i++) {
        vkDestroyFramebuffer(pApp->device, pApp->swapChainFramebuffers[i], NULL);
    }

    // Destroy pipeline
    vkDestroyPipeline(pApp->device, pApp->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(pApp->device, pApp->pipelineLayout, NULL);
    vkDestroyRenderPass(pApp->device, pApp->renderPass, NULL);
    for (int i = 0; i < 2; i++) {
        vkDestroyImageView(pApp->device, pApp->swapChainImageViews[i], NULL);
    }
    vkDestroyShaderModule(pApp->device, pApp->fragShaderModule, NULL);
    vkDestroyShaderModule(pApp->device, pApp->vertShaderModule, NULL);
    vkDestroySwapchainKHR(pApp->device, pApp->swapChain, NULL);

    // Destroy device and instance
    vkDestroyDevice(pApp->device, NULL);
    vkDestroySurfaceKHR(pApp->instance, pApp->surface, NULL);
    vkDestroyInstance(pApp->instance, NULL);

    // Destroy glfw stuff

    glfwDestroyWindow(pApp->window);
    glfwTerminate();

}

// Drawing | most important functions //
// ---------------------------------- //

void drawFrame(struct Application* pApp) {

    // Wait for the fence and reset it

    vkWaitForFences(pApp->device, 1, &pApp->inFlightFence[pApp->currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(pApp->device, 1, &pApp->inFlightFence[pApp->currentFrame]);

    uint32_t imageIndex = 0;

    // Get wich image we are at
    VkResult result = vkAcquireNextImageKHR(pApp->device, pApp->swapChain, UINT64_MAX, pApp->imageAvailableSemaphore[pApp->currentFrame], VK_NULL_HANDLE, &imageIndex);

    // If the swapchain is obsolete, recreate it
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain(pApp);
        return;
    }

    vkResetCommandBuffer(pApp->commandBuffer[pApp->currentFrame], 0);

    // Main part of the drawing 

    recordCommandBuffer(pApp, imageIndex);

    // Choose the semaphores

    VkSemaphore waitSemaphores[] = { pApp->imageAvailableSemaphore[pApp->currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { pApp->renderFinishedSemaphore[pApp->currentFrame] };

    // Create submit informations

    VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        VK_NULL_HANDLE,
        1,
        waitSemaphores,
        waitStages,
        1,
        &pApp->commandBuffer[pApp->currentFrame],
        1,
        signalSemaphores,
    };

    // Submit to queues

    if (vkQueueSubmit(pApp->graphicsQueue, 1, &submitInfo, pApp->inFlightFence[pApp->currentFrame]) != VK_SUCCESS) {
        printf("Submitting didn't work\n");
    }


    VkSwapchainKHR swapChains[] = { pApp->swapChain };
    
    VkPresentInfoKHR presentInfo = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        VK_NULL_HANDLE,
        1,
        signalSemaphores,
        1,
        &pApp->swapChain,
        &imageIndex,
        VK_NULL_HANDLE,
    };

    // Send to Present Queue

    vkQueuePresentKHR(pApp->presentQueue, &presentInfo);

    // Update the frame

    pApp->currentFrame = (pApp->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void recordCommandBuffer(struct Application* pApp, uint32_t imageIndex) {

    VkCommandBufferBeginInfo beginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        VK_NULL_HANDLE,
        0,
        VK_NULL_HANDLE
    };
    if (vkBeginCommandBuffer(pApp->commandBuffer[pApp->currentFrame], &beginInfo) != VK_SUCCESS) {
        printf("Begin CommandBuffer !well set\n");
    }

    // Equivalent to fill with a background color

    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };

    VkRect2D renderArea = {
        {0,0},
        pApp->swapChainExtent
    };

    VkRenderPassBeginInfo renderPassInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        VK_NULL_HANDLE,
        pApp->renderPass,
        pApp->swapChainFramebuffers[imageIndex],
        renderArea,
        1,
        &clearColor
    };

    vkCmdBeginRenderPass(pApp->commandBuffer[pApp->currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(pApp->commandBuffer[pApp->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pApp->graphicsPipeline);


    // Recreate viewport
    VkViewport viewport = {
        0.0f,
        0.0f,
        (float)pApp->swapChainExtent.width,
        (float)pApp->swapChainExtent.height,
        0.0f,
        1.0f,
    };

    vkCmdSetViewport(pApp->commandBuffer[pApp->currentFrame], 0, 1, &viewport);

    // Recreate scissors
    VkRect2D scissor = {
        {0,0},
        pApp->swapChainExtent
    };

    vkCmdSetScissor(pApp->commandBuffer[pApp->currentFrame], 0, 1, &scissor);

    // Bind my vertex buffers

    VkBuffer vertexBuffers[] = { pApp->vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(pApp->commandBuffer[pApp->currentFrame], 0, 1, vertexBuffers, offsets);

    // Draw
    // WARNING HARDCODED -> ALREADY KNOW THERE ARE 3 VERTICES
    vkCmdDraw(pApp->commandBuffer[pApp->currentFrame], 3, 1, 0, 0);

    vkCmdEndRenderPass(pApp->commandBuffer[pApp->currentFrame]);

    if (vkEndCommandBuffer(pApp->commandBuffer[pApp->currentFrame]) != VK_SUCCESS) {
        printf("Marche pas :(\n");
    }
}

void recreateSwapChain(struct Application* pApp) {
    vkDeviceWaitIdle(pApp->device);

    createSwapChain(pApp);
    createImageViews(pApp);
    createFrameBuffers(pApp);
}
#include "vulkanStuff.c"
#include "vulkanTop.c"

void run(struct Application* pApp);
void mainLoop(struct Application* pApp);
void rotateVertices(float* vertices, double time, int vertexOffset, int verticesCount);



void main() {
    struct Application VulkanApp = {.width=800,
        .heigth=800,
        .name="Appli",
        .currentFrame = 0};

    float verticeExample[3][5] = {
        {0.0f,-0.5f,    1.0f,0.0f,0.0f},
        {0.5f,0.5f,    0.0f,1.0f,0.0f},
        {-0.5f,0.5f,    0.0f,0.0f,1.0f},
    };

    VulkanApp.vertices = verticeExample;
    run(&VulkanApp);
    glfwTerminate();
}

void run(struct Application *pApp) {
    initWindow(pApp);
    initVulkan(pApp);
    mainLoop(pApp);
    cleanUp(pApp);
}

void mainLoop(struct Application *pApp) {
    while (!glfwWindowShouldClose(pApp->window)) {
        glfwPollEvents();
        drawFrame(pApp);
    }
    vkDeviceWaitIdle(pApp->device);
}

// If you want the vertices to rotate and grow
// Pass your time unit or glfw time
// Don't forget to bindVertexBuffers(pApp) after
// Must be in the main loop

void rotateVertices(float* vertices, double time, int vertexOffset, int verticesCount) {
    double angleDivision = 2 * 3.141592653589 / verticesCount;
    double triangleSize = 0.8 * fabs(sin(time / 4));
    for (int i = 0; i < verticesCount; i++) {
        vertices[i * vertexOffset] = triangleSize*cos(i * angleDivision + time/2 );
        vertices[i * vertexOffset + 1] = triangleSize * sin(i * angleDivision + time/2);
    };
}
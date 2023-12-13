#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "GLTFScene.h"

/*
*   https://github.com/syoyo/tinygltf - glTF loader lib  
* 
    About libraries:
    GLFW    - windows management
    GLAD    - OpenGL function manager
*/

// Globals
int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
std::string WINDOW_CAPTION = "OpenGL gltf scene";

GLTFScene scene;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

int main()
{
    // INIT GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_CAPTION.c_str(), NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // INIT GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Variables
    float deltaTime = 0.f;
    float lastFrame = 0.f;

    scene.init(); // load models, shaders
    scene.render_setup(); // gpu 

    scene.scene_init();

    // Loop
    while (!glfwWindowShouldClose(window))
    {
        // Delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        scene.processInput(window, deltaTime);

        // Rendering
        scene.render(window);

        // Events & Buffer-swap
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    scene.cleanup();

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static float lastX = WINDOW_WIDTH/2, lastY = WINDOW_HEIGHT/2;

    float mouse_xoffset = xpos - lastX;
    float mouse_yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    scene.getCamera().ProcessMouseMovement(mouse_xoffset, mouse_yoffset);
}
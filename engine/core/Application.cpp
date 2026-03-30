#include "Application.h"

#include <iostream>

Application::Application(int width, int height) : _width(width), _height(height)
{
    // Initialize WindowManager
    _windowManager = WindowManager::getInstance();
    _windowManager->init(_width, _height);
	_windowManager->setEventCallbacks(this);
}

Application::~Application()
{
    _windowManager = nullptr;
}

void Application::run()
{
    while (!glfwWindowShouldClose(_windowManager->getHandle()))
    {
        // Set up framebuffer
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(_windowManager->getHandle(), &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render scene
        // ...

        // Swap front and back buffers
        glfwSwapBuffers(_windowManager->getHandle());

        // Poll for and process events
        glfwPollEvents();
    }
}

void Application::setClearColor(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
}

#pragma region EventCallbacks

void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    // Handle mouse button events
}

void Application::resizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    _width = width;
    _height = height;
}

void Application::mouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    // Handle mouse movement events
}

void Application::scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    // Handle scroll events
}

#pragma endregion
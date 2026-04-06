#include "core/Application.h"

#include <iostream>
#include "core/Game.h"
#include "core/Time.h"

namespace engine 
{
    Application::Application(const AppConfig& config) : 
        _config(config),
        _window(config.width, config.height, config.title),
        _assets(config.assetsDir),
        _renderer(config.width, config.height)
    {
        if (config.depthTest) glEnable(GL_DEPTH_TEST);
        if (config.cullFace) glEnable(GL_CULL_FACE);
        _window.setEventCallbacks(this);
    }

    Application::~Application() = default;

    void Application::run(std::unique_ptr<Game> game)
    {
        game->init(_assets, _renderer, _scene, _config);

        while (!_window.shouldClose())
        {
            Time::update();
            game->update(Time::deltaTime());

            _renderer.render(_scene, _assets);
            _window.swapBuffers();
            _window.pollEvents();
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
        _config.width = width;
        _config.height = height;

        _renderer.resize(width, height);
        if (auto* camera = _scene.getCamera())
            camera->setAspect(static_cast<float>(width) / static_cast<float>(height));
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
}
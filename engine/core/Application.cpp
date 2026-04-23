#include "core/Application.h"

#include <iostream>
#include <cmath>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "core/Game.h"
#include "core/Input.h"
#include "core/Time.h"

namespace engine 
{
    Application::Application(const AppConfig& config) : 
        _config(config),
        _window(config.width, config.height, config.title),
        _renderer(config.width, config.height),
        _assets(config.assetsDir)        
    {
        if (config.depthTest) glEnable(GL_DEPTH_TEST);
        if (config.cullFace) glEnable(GL_CULL_FACE);

        _window.setEventCallbacks(this);
        _scene.setPhysicsSystem(&_physics);

        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(_window.getHandle(), &framebufferWidth, &framebufferHeight);
        _renderer.resize(framebufferWidth, framebufferHeight);
        glViewport(0, 0, framebufferWidth, framebufferHeight);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(_window.getHandle(), true);
        ImGui_ImplOpenGL3_Init("#version 150");
    }

    Application::~Application()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void Application::run(std::unique_ptr<Game> game)
    {
        game->init(_assets, _renderer, _scene, _config);
        _scene.start();

        static bool my_tool_active = true;
        static float my_color[4] = { 0.20f, 0.55f, 0.90f, 1.0f };

        while (!_window.shouldClose())
        {
            Time::update();
            Input::update();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Update physics
            _physics.update(Time::deltaTime());

            // Update gameplay logic
            game->update(Time::deltaTime());
            _scene.update(Time::deltaTime());

            // Render
            _renderer.render(_scene, _assets);

            ImGui::Begin("Debug");
            ImGui::Text("FPS: %.1f", Time::deltaTime() > 0.0f ? 1.0f / Time::deltaTime() : 0.0f);
            ImGui::Text("Delta: %.3f ms", Time::deltaTime() * 1000.0f);
            ImGui::End();

            ImGui::Begin("My First Tool", &my_tool_active, ImGuiWindowFlags_MenuBar);
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
                    if (ImGui::MenuItem("Save", "Ctrl+S"))   { /* Do stuff */ }
                    if (ImGui::MenuItem("Close", "Ctrl+W"))  { my_tool_active = false; }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            // Edit a color stored as 4 floats
            ImGui::ColorEdit4("Color", my_color);

            // Generate samples and plot them
            float samples[100];
            for (int n = 0; n < 100; n++)
                samples[n] = std::sinf(n * 0.2f + static_cast<float>(ImGui::GetTime()) * 1.5f);
            ImGui::PlotLines("Samples", samples, 100);

            // Display contents in a scrolling region
            ImGui::TextColored(ImVec4(1,1,0,1), "Important Stuff");
            ImGui::BeginChild("Scrolling");
            for (int n = 0; n < 50; n++)
                ImGui::Text("%04d: Some text", n);
            ImGui::EndChild();
            ImGui::End();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            
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
        Input::onKey(key, action);

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }

    void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        Input::onMouseButton(button, action);
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
        Input::onMouseMove(xPos, yPos);
    }

    void Application::scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
    {
        // wip
        // Input::onScroll(xOffset, yOffset);
    }

    #pragma endregion
}
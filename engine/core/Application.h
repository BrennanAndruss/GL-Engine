#pragma once

#include <glad/glad.h>
#include "WindowManager.h"

class Application : public EventCallbacks
{
public:
    Application(const int width, const int height);
    ~Application();

    void run();

	void setClearColor(float r, float g, float b, float a);

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void resizeCallback(GLFWwindow* window, int width, int height);
	void mouseCallback(GLFWwindow* window, double xPos, double yPos);
	void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);

private:
	WindowManager* _windowManager = nullptr;

	int _width, _height;
};
#include "WindowManager.h"

#include <iostream>

void errorCallback(int error, const char* description)
{
	std::cerr << description << std::endl;
}

WindowManager* WindowManager::getInstance()
{
	static WindowManager instance;
	return &instance;
}

WindowManager::WindowManager() {}

WindowManager::~WindowManager()
{
	if (_windowHandle)
	{
		glfwDestroyWindow(_windowHandle);
		_windowHandle = nullptr;
	}
	glfwTerminate();
}

bool WindowManager::init(const int width, const int height)
{
	glfwSetErrorCallback(errorCallback);

	// Initialize GLFW library
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return false;
	}

	// Request the highest possible version of OpenGL
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	// Create a windowed mode window and its OpenGL context
	_windowHandle = glfwCreateWindow(width, height, "PostFX", nullptr, nullptr);
	if (!_windowHandle)
	{
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(_windowHandle);

	// Initialize GLAD
	if (!gladLoadGL())
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	// Set vsync
	glfwSwapInterval(1);

	// Register callbacks with GLFW
	glfwSetKeyCallback(_windowHandle, keyCallback);
	glfwSetMouseButtonCallback(_windowHandle, mouseButtonCallback);
	glfwSetFramebufferSizeCallback(_windowHandle, resizeCallback);
	glfwSetCursorPosCallback(_windowHandle, mouseCallback);
	glfwSetScrollCallback(_windowHandle, scrollCallback);

	return true;
}

void WindowManager::setEventCallbacks(EventCallbacks* callbacks)
{
	_callbacks = callbacks;
}

GLFWwindow* WindowManager::getHandle()
{
	return _windowHandle;
}

void WindowManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	WindowManager* instance = WindowManager::getInstance();
	if (instance && instance->_callbacks)
	{
		instance->_callbacks->keyCallback(window, key, scancode, action, mods);
	}
}

void WindowManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	WindowManager* instance = WindowManager::getInstance();
	if (instance && instance->_callbacks)
	{
		instance->_callbacks->mouseButtonCallback(window, button, action, mods);
	}
}

void WindowManager::resizeCallback(GLFWwindow* window, int width, int height)
{
	WindowManager* instance = WindowManager::getInstance();
	if (instance && instance->_callbacks)
	{
		instance->_callbacks->resizeCallback(window, width, height);
	}
}

void WindowManager::mouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	WindowManager* instance = WindowManager::getInstance();
	if (instance && instance->_callbacks)
	{
		instance->_callbacks->mouseCallback(window, xPos, yPos);
	}
}

void WindowManager::scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	WindowManager* instance = WindowManager::getInstance();
	if (instance && instance->_callbacks)
	{
		instance->_callbacks->scrollCallback(window, xOffset, yOffset);
	}
}
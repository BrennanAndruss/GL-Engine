#pragma once

// Prevent GLFW from including GL
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace engine
{
	// An interface to write custom event callbacks
	// These callbacks can be retrieved via Window to use with GLFW
	class EventCallbacks
	{
	public:
		virtual void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {};
		virtual void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {};
		virtual void resizeCallback(GLFWwindow* window, int width, int height) {};
		virtual void mouseCallback(GLFWwindow* window, double xPos, double yPos) {};
		virtual void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {};
	};
}

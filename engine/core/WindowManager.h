#pragma once

#include <glad/glad.h>
#include <glfw/glfw3.h>

namespace engine
{
	// An interface to write custom event callbacks
	// These callbacks can be retrieved via WindowManager to use with GLFW
	class EventCallbacks
	{
	public:
		virtual void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) = 0;
		virtual void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) = 0;
		virtual void resizeCallback(GLFWwindow* window, int width, int height) = 0;
		virtual void mouseCallback(GLFWwindow* window, double xPos, double yPos) = 0;
		virtual void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) = 0;
	};

	// A singleton class repsonsible for all window management code
	class WindowManager
	{
	public:
		static WindowManager* getInstance();
		WindowManager(const WindowManager&) = delete;
		WindowManager& operator= (const WindowManager&) = delete;

		bool init(const int width, const int height);

		void setEventCallbacks(EventCallbacks* callbacks);
		GLFWwindow* getHandle();

	protected:
		GLFWwindow* _windowHandle = nullptr;
		EventCallbacks* _callbacks = nullptr;

	private:
		WindowManager();
		~WindowManager();

		// C-style wrapper functions to register as callbacks with GLFW
		// These functions retrieve the associated callbacks from an EventCallbacks object
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void resizeCallback(GLFWwindow* window, int width, int height);
		static void mouseCallback(GLFWwindow* window, double xPos, double yPos);
		static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
	};
}
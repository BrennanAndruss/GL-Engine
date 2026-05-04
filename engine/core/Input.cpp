#include "core/Input.h"

#include <GLFW/glfw3.h>

namespace engine
{
	bool Input::isKeyDown(int key) { return _keys[key]; }
	bool Input::isKeyPressed(int key) { return _keys[key] && !_prevKeys[key]; }
	bool Input::isKeyReleased(int key) { return !_keys[key] && _prevKeys[key]; }
	bool Input::isMouseDown(int button) { return _mouseButtons[button]; }
	glm::vec2 Input::getMousePos() { return _mousePos; }
	glm::vec2 Input::getMouseDelta() { return _mouseDelta; }
	glm::vec2 Input::getScrollDelta() { return _scrollDelta; }

	void Input::update()
	{
		_prevKeys = _keys;
		_mouseDelta = _mousePos - _prevMousePos;
		_prevMousePos = _mousePos;
		// _scrollDelta = glm::vec2(0.0f, 0.0f);
	}

	void Input::onKey(int key, int action)
	{
		if (action == GLFW_PRESS) _keys[key] = true;
		else if (action == GLFW_RELEASE) _keys[key] = false;
	}

	void Input::onMouseMove(double x, double y)
	{
		_mousePos = glm::vec2(x, y);
	}

	void Input::onMouseButton(int button, int action)
	{
		if (action == GLFW_PRESS) _mouseButtons[button] = true;
		else if (action == GLFW_RELEASE) _mouseButtons[button] = false;
	}

	void Input::onScroll(double x, double y)
	{
		// wip
		// _scrollDelta = glm::vec2(x, y);
	}

	void Input::setMouseTrapped(bool trapped)
	{
		_mouseTrapped = trapped;
		double cursorX = 0.0;
		double cursorY = 0.0;
		if (GLFWwindow* window = glfwGetCurrentContext())
		{
			glfwGetCursorPos(window, &cursorX, &cursorY);
		}

		_mousePos = glm::vec2(cursorX, cursorY);
		_prevMousePos = _mousePos;
		_mouseDelta = glm::vec2(0.0f);

		if (trapped)
		{
			glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else
		{
			glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}
#pragma once

namespace engine
{
	class Object;
}

namespace engine
{
	class Component
	{
	public:
		Object* owner = nullptr;

		virtual ~Component() = default;
		virtual void start() {}
		virtual void update(float deltaTime) {}
	};
}
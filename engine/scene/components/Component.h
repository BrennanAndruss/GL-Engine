#pragma once

namespace engine
{
	class Object;
	class AssetManager;
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
		virtual void update(float deltaTime, AssetManager& assets)
		{
			update(deltaTime);
		}
	};
}
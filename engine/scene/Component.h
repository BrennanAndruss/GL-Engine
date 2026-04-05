#pragma once

namespace engine
{
	struct Component
	{
		virtual ~Component() = default;
		virtual void update(float deltaTime) {}
	};

	struct MeshRenderer : public Component
	{
		size_t meshId;
		size_t materialId;
	};
}
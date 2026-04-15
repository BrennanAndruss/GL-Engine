#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "scene/components/Collider.h"

namespace engine
{
	class SphereCollider : public Collider
	{
	public:
		glm::vec3 center = glm::vec3(0.0f);
		float radius = 0.5f;

		void start() override;
		btCollisionShape* getShape() const override { return _shape.get(); }

	private:
		std::unique_ptr<btSphereShape> _shape;
	};
}
#pragma once

#include "scene/components/Component.h"
#include "scene/components/Collider.h"

namespace engine
{
	class RigidBody : public Component
	{
	public:
		float mass = 1.0f;

		void start() override;
		void update(float deltaTime) override;

	private:
		Collider* _collider = nullptr;
		btRigidBody* _body = nullptr;
	};
}
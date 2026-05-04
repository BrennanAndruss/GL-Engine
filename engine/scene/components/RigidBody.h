#pragma once

#include <glm/glm.hpp>
#include "scene/components/Component.h"
#include "scene/components/Collider.h"

// Forward declarations
class btRigidBody;

namespace engine
{
	class RigidBody : public Component
	{
	public:
		enum class BodyType
		{
			Static,
			Kinematic,
			Dynamic,
		};

		float mass = 1.0f;
		BodyType bodyType = BodyType::Static;

		void start() override;
		void update(float deltaTime) override;
		void setBodyType(BodyType type);
		BodyType getBodyType() const { return bodyType; }
		
		void setLinearVelocity(glm::vec3 velocity);
		glm::vec3 getLinearVelocity() const;

	private:
		bool initializeBody();
		void destroyBody();

		Collider* _collider = nullptr;
		btRigidBody* _body = nullptr;
		bool _bodyDirty = true;
	};
}
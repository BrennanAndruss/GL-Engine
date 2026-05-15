#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <functional>
#include "scene/components/Component.h"

// Forward declarations
class btCollisionShape;
class btCollisionObject;
class btBoxShape;
class btCapsuleShape;
class btSphereShape;

namespace engine
{
	enum class Axis
	{
		X,
		Y,
		Z,
	};

	class Collider : public Component
	{
	public:
		bool isTrigger = false;

		virtual btCollisionShape* getShape() const = 0;
		void update(float deltaTime) override;
		btCollisionObject* getCollisionObject() const { return _object; }
		void destroyCollisionObject();

	protected:
		btCollisionObject* _object = nullptr;
	};

	class BoxCollider : public Collider
	{
	public:
		// Constructors declared in header to handle forward declarations
		BoxCollider();
		~BoxCollider() override;

		glm::vec3 center = glm::vec3(0.0f);
		glm::vec3 size = glm::vec3(1.0f);

		void start() override;
		btCollisionShape* getShape() const override;
		void update(float deltaTime) override;
		void rebuild();

	private:
		std::unique_ptr<btBoxShape> _shape;
	};

	class CapsuleCollider : public Collider
	{
	public:
		CapsuleCollider();
		~CapsuleCollider() override;

		glm::vec3 center = glm::vec3(0.0f);
		float radius = 0.5f;
		float height = 2.0f;
		Axis direction = Axis::Y;

		void start() override;
		btCollisionShape* getShape() const override;
		void update(float deltaTime) override;

	private:
		std::unique_ptr<btCapsuleShape> _shape;
	};

	class SphereCollider : public Collider
	{
	public:
		SphereCollider();
		~SphereCollider();

		glm::vec3 center = glm::vec3(0.0f);
		float radius = 0.5f;

		void start() override;
		btCollisionShape* getShape() const override;
		void update(float deltaTime) override;

	private:
		std::unique_ptr<btSphereShape> _shape;
	};
}
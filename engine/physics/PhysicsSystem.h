#pragma once

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <memory>

namespace engine
{
	class PhysicsSystem
	{
	public:
		PhysicsSystem();
		~PhysicsSystem() = default;

		void update(float deltaTime);

		// Type conversion helpers
		static glm::vec3 toGlm(const btVector3& v) { return glm::vec3(v.x(), v.y(), v.z()); }
		static btVector3 toBullet(glm::vec3 v) { return btVector3(v.x, v.y, v.z); }

	private:
		// Bullet world components
		std::unique_ptr<btDefaultCollisionConfiguration> _collisionConfig;
		std::unique_ptr<btCollisionDispatcher> _dispatcher;
		std::unique_ptr<btBroadphaseInterface> _broadphase;
		std::unique_ptr<btSequentialImpulseConstraintSolver> _solver;
		std::unique_ptr<btDiscreteDynamicsWorld> _world;

		// Shape and motion states
		std::vector<std::unique_ptr<btCollisionShape>> _shapes;
		std::vector<std::unique_ptr<btMotionState>> _motionStates;
	};
}
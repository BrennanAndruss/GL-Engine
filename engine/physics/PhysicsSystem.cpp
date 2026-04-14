#include "physics/PhysicsSystem.h"

#include <iostream>

namespace engine
{
	PhysicsSystem::PhysicsSystem()
	{
        _collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
        _dispatcher = std::make_unique<btCollisionDispatcher>(_collisionConfig.get());
        _broadphase = std::make_unique<btDbvtBroadphase>();
        _solver = std::make_unique<btSequentialImpulseConstraintSolver>();
        _world = std::make_unique<btDiscreteDynamicsWorld>(
            _dispatcher.get(), _broadphase.get(), _solver.get(), _collisionConfig.get()
        );
        _world->setGravity(btVector3(0.0f, -20.0f, 0.0f));
	}

    void PhysicsSystem::update(float deltaTime)
    {
        _world->stepSimulation(deltaTime, 10);
    }
}
#include "physics/PhysicsSystem.h"

#include <iostream>

namespace engine
{
    PhysicsSystem::PhysicsSystem()
    {
        // Sets up memory allocators and collision algorithms for different shape pairs
        _collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();

        // Processes collision pairs from the broadphase and runs the narrow phase
        _dispatcher = std::make_unique<btCollisionDispatcher>(_collisionConfig.get());

        // Using two dynamic AABB bounding volume trees for broad phase
        // One tree for static/non-moving objects, another for dynamic objects
        _broadphase = std::make_unique<btDbvtBroadphase>();

        // Resolves collisions and joinst using Sequential Impulse
        _solver = std::make_unique<btSequentialImpulseConstraintSolver>();

        // Main physics world
        _world = std::make_unique<btDiscreteDynamicsWorld>(
            _dispatcher.get(), _broadphase.get(), _solver.get(), _collisionConfig.get()
        );
        _world->setGravity(btVector3(0.0f, -10.0f, 0.0f));

        // Registers a callback that keeps the broadphase pair cache updated for ghost objects (triggers)
        _world->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(
            new btGhostPairCallback()
        );
    }

    void PhysicsSystem::update(float deltaTime)
    {
        _world->stepSimulation(deltaTime, 10);
        checkCollisions();
    }

    btRigidBody* PhysicsSystem::createBody(btCollisionShape* shape, glm::vec3 position,
        float mass, bool isTrigger)
    {
        auto motionState = std::make_unique<btDefaultMotionState>(
            btTransform(btQuaternion::getIdentity(), toBullet(position))
        );

        btVector3 inertia(0.0f, 0.0f, 0.0f);
        if (mass > 0.0f)
        {
            shape->calculateLocalInertia(mass, inertia);
        }

        btRigidBody::btRigidBodyConstructionInfo info(mass, motionState.get(), shape, inertia);

        auto body = new btRigidBody(info);

        if (isTrigger)
        {
            // Trigger body doesn't generate contact forces
            body->setCollisionFlags(
                body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE
            );
        }

        _world->addRigidBody(body);
        _motionStates.push_back(std::move(motionState));

        return body;
    }

    btCollisionObject* PhysicsSystem::createCollisionObject(btCollisionShape* shape,
        glm::vec3 position, bool isTrigger)
    {
        // Create btCollisionObject for static geometry and triggers without physics simulation
        auto* object = new btCollisionObject();
        object->setCollisionShape(shape);

        btTransform t;
        t.setIdentity();
        t.setOrigin(toBullet(position));
        object->setWorldTransform(t);

        int flags = object->getCollisionFlags();
        if (isTrigger)
        {
            // Trigger objects don't generate contact forces
            flags |= btCollisionObject::CF_NO_CONTACT_RESPONSE;
        }
        else
        {
            flags |= btCollisionObject::CF_STATIC_OBJECT;
        }

        object->setCollisionFlags(flags);
        _world->addCollisionObject(object);

        return object;
    }

    void PhysicsSystem::addCollisionObject(btCollisionObject* object, int group, int mask)
    {
        _world->addCollisionObject(object, group, mask);
    }

    void PhysicsSystem::removeBody(btRigidBody* body)
    {
        if (!body) return;
        unregisterCallback(body);
        _world->removeRigidBody(body);
    }

    void PhysicsSystem::removeCollisionObject(btCollisionObject* object)
    {
        if (!object) return;
        unregisterCallback(object);
        _world->removeCollisionObject(object);
    }

    void PhysicsSystem::registerCallback(btCollisionObject* object, CollisionCallback callback)
    {
        _callbacks[object] = callback;
    }

    void PhysicsSystem::unregisterCallback(btCollisionObject* object)
    {
        _callbacks.erase(object);
    }

    void PhysicsSystem::checkCollisions()
    {
        std::set<std::pair<btCollisionObject*, btCollisionObject*>> currentContacts;

        int numManifolds = _dispatcher->getNumManifolds();
        for (int i = 0; i < numManifolds; i++)
        {
            // Get contact points this frame and skip recently separated manifolds
            btPersistentManifold* manifold = _dispatcher->getManifoldByIndexInternal(i);
            if (manifold->getNumContacts() == 0) continue;


            auto* objectA = const_cast<btCollisionObject*>(manifold->getBody0());
            auto* objectB = const_cast<btCollisionObject*>(manifold->getBody1());

            // Normalize pair order so (A, B) and (B, A) are the same key
            auto pair = std::make_pair(
                std::min(objectA, objectB),
                std::max(objectA, objectB)
            );
            currentContacts.insert(pair);

            // Fire callback on first frame of contact
            if (_activeContacts.find(pair) == _activeContacts.end())
            {
                auto itA = _callbacks.find(objectA);
                if (itA != _callbacks.end())
                {
                    itA->second(objectB);
                }

                auto itB = _callbacks.find(objectB);
                if (itB != _callbacks.end())
                {
                    itB->second(objectA);
                }
            }
        }

        _activeContacts = currentContacts;
    }
}
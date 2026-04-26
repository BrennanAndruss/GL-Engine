#pragma once

#include <btBulletDynamicsCommon.h>
#include <functional>
#include "scene/components/Component.h"

namespace engine
{
	class Collider : public Component
	{
	public:
		bool isTrigger = false;

		virtual btCollisionShape* getShape() const = 0;
		btCollisionObject* getCollisionObject() const { return _object; }

	protected:
		btCollisionObject* _object = nullptr;
	};
}
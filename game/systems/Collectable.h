#pragma once

#include "scene/components/Component.h"
#include "resources/Handle.h"

namespace engine
{
	struct Material;
}

class Collectable : public engine::Component
{
public:
	Handle<engine::Material> defaultMat;
	Handle<engine::Material> collectedMat;
	bool isCollected = false;

	void start() override;
	void onCollected();
};
#pragma once

#include "scene/components/Component.h"

class Collectable : public engine::Component
{
public:
	size_t defaultMatId;
	size_t collectedMatId;
	bool isCollected = false;

	void start() override;
	void onCollected();
};
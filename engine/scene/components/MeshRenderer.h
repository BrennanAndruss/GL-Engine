#pragma once

#include "scene/components/Component.h"

namespace engine
{
	struct MeshRenderer : public Component
	{
		size_t meshId;
		size_t materialId;
	};
}
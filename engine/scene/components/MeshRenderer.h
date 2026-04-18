#pragma once

#include "scene/components/Component.h"
#include "resources/Handle.h"

namespace engine
{
	class Mesh;
	class Material;
}

namespace engine
{
	struct MeshRenderer : public Component
	{
		Handle<Mesh> mesh;
		Handle<Material> material;
	};
}
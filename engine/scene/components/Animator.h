#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "scene/components/Component.h"
#include "resources/Handle.h"
#include "resources/SkeletalAnimation.h"

namespace engine
{
	class AssetManager;

	class Animator : public Component
	{
	public:
		Handle<Skeleton> skeleton;
		Handle<AnimationClip> clip;

		float playbackSpeed = 1.0f;
		bool loop = true;
		
        engine::AssetManager* assets = nullptr;
        void update(float deltaTime) override;

		const std::vector<glm::mat4>& getBoneMatrices() const;
		const std::vector<glm::mat4>& getGlobalMatrices() const;

		bool hasPose() const;

		void reset();

	private:
		float _currentTime = 0.0f;

		std::vector<glm::mat4> _boneMatrices;
		std::vector<glm::mat4> _globalMatrices;

		bool _debugPrinted = false;
	};
}
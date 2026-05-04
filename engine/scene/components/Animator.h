#pragma once

#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "scene/components/Component.h"
#include "resources/AssetManager.h"
#include "resources/SkeletalAnimation.h"
#include "resources/Handle.h"

namespace engine
{
	class AssetManager;
	struct Skeleton;
	struct AnimationClip;
}

namespace engine
{
	class Animator : public Component
	{
	public:
		Handle<Skeleton> skeleton;
		Handle<AnimationClip> clip;
		float playbackSpeed = 1.0f;
		bool loop = true;

		void update(float deltaTime, AssetManager& assets) override
		{
			auto* skeletonData = assets.getSkeleton(skeleton);
			auto* clipData = assets.getAnimationClip(clip);
			if (!skeletonData)
			{
				_boneMatrices.clear();
				_currentTime = 0.0f;
				return;
			}

			if (clipData)
			{
				const float ticksPerSecond = (clipData->ticksPerSecond > 0.0f) ? clipData->ticksPerSecond : 25.0f;
				_currentTime += deltaTime * playbackSpeed * ticksPerSecond;

				if (clipData->durationTicks > 0.0f)
				{
					if (loop)
					{
						_currentTime = std::fmod(_currentTime, clipData->durationTicks);
						if (_currentTime < 0.0f)
						{
							_currentTime += clipData->durationTicks;
						}
					}
					else if (_currentTime > clipData->durationTicks)
					{
						_currentTime = clipData->durationTicks;
					}
				}
			}

			_boneMatrices.assign(skeletonData->boneCount(), glm::mat4(1.0f));
			evaluateSkeletonPose(*skeletonData, clipData, _currentTime, _boneMatrices);
		}

		const std::vector<glm::mat4>& getBoneMatrices() const { return _boneMatrices; }
		bool hasPose() const { return !_boneMatrices.empty(); }

	private:
		float _currentTime = 0.0f;
		std::vector<glm::mat4> _boneMatrices;
	};
}

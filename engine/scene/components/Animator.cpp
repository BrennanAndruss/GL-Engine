#include "scene/components/Animator.h"

#include <cmath>
#include <algorithm>
#include <iostream>

#include "resources/AssetManager.h"

namespace engine
{
	void Animator::update(float deltaTime)
    {
	    if (!assets)
	    {
		    return;
	    }

	    auto* skeletonData = assets->getSkeleton(skeleton);
	    auto* clipData = assets->getAnimationClip(clip);

		if (!skeletonData)
		{
			_boneMatrices.clear();
			_globalMatrices.clear();
			_currentTime = 0.0f;
			return;
		}

		if (clipData)
		{
			const float ticksPerSecond =
				clipData->ticksPerSecond > 0.0f
					? clipData->ticksPerSecond
					: 25.0f;

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
		evaluateSkeletonPose(
			*skeletonData,
			clipData,
			_currentTime,
			_boneMatrices,
			!_debugPrinted
		);

		_globalMatrices.assign(skeletonData->boneCount(), glm::mat4(1.0f));
		evaluateSkeletonGlobals(
			*skeletonData,
			clipData,
			_currentTime,
			_globalMatrices
		);

		if (!_debugPrinted && !_boneMatrices.empty())
		{
			_debugPrinted = true;

			std::cout
				<< "[Animator] First pose evaluated. Bones: "
				<< _boneMatrices.size()
				<< ", time: "
				<< _currentTime
				<< " ticks\n";
		}
	}

	const std::vector<glm::mat4>& Animator::getBoneMatrices() const
	{
		return _boneMatrices;
	}

	const std::vector<glm::mat4>& Animator::getGlobalMatrices() const
	{
		return _globalMatrices;
	}

	bool Animator::hasPose() const
	{
		return !_boneMatrices.empty();
	}

	void Animator::reset()
	{
		_currentTime = 0.0f;
		_boneMatrices.clear();
		_globalMatrices.clear();
		_debugPrinted = false;
	}
}
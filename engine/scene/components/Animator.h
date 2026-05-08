#pragma once

#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include <iostream>
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
				evaluateSkeletonPose(*skeletonData, clipData, _currentTime, _boneMatrices, !_debugPrinted);

				// Also compute per-bone global transforms for debug visualization
				_globalMatrices.assign(skeletonData->boneCount(), glm::mat4(1.0f));
				evaluateSkeletonGlobals(*skeletonData, clipData, _currentTime, _globalMatrices);

			// Debug: print first few bone matrices on first evaluation
			if (!_debugPrinted && !_boneMatrices.empty())
			{
				_debugPrinted = true;
				std::cout << "[Animator] First pose evaluation - playback time=" << _currentTime << " ticks\n";
				for (std::size_t i = 0; i < std::min(_boneMatrices.size(), std::size_t(5)); ++i)
				{
					const auto& m = _boneMatrices[i];
					std::cout << "  Bone " << i << ": pos[" 
						<< m[3][0] << ", " << m[3][1] << ", " << m[3][2] << "] "
						<< "scale[" << glm::length(glm::vec3(m[0])) << ", " 
						<< glm::length(glm::vec3(m[1])) << ", " 
						<< glm::length(glm::vec3(m[2])) << "]\n";
				}

				std::vector<int> nodeIndexByBone(_boneMatrices.size(), -1);
				for (std::size_t nodeIdx = 0; nodeIdx < skeletonData->nodes.size(); ++nodeIdx)
				{
					const auto& node = skeletonData->nodes[nodeIdx];
					if (node.isBone && node.boneIndex >= 0 && static_cast<std::size_t>(node.boneIndex) < nodeIndexByBone.size())
					{
						nodeIndexByBone[static_cast<std::size_t>(node.boneIndex)] = static_cast<int>(nodeIdx);
					}
				}

				std::vector<glm::mat4> bindGlobalByNode(skeletonData->nodes.size(), glm::mat4(1.0f));
				if (skeletonData->rootNodeIndex >= 0)
				{
					std::vector<int> stack = { skeletonData->rootNodeIndex };
					while (!stack.empty())
					{
						int idx = stack.back();
						stack.pop_back();
						const auto& node = skeletonData->nodes[static_cast<std::size_t>(idx)];
						glm::mat4 parent = glm::mat4(1.0f);
						if (node.parentIndex >= 0)
						{
							parent = bindGlobalByNode[static_cast<std::size_t>(node.parentIndex)];
						}
						bindGlobalByNode[static_cast<std::size_t>(idx)] = parent * node.bindLocalTransform;
						for (int child : node.children)
						{
							stack.push_back(child);
						}
					}
				}

				std::cout << "[Animator] Per-bone CPU compare (animatedGlobalOrigin vs final*bindGlobalOrigin):\n";
				for (std::size_t i = 0; i < std::min(_boneMatrices.size(), std::size_t(12)); ++i)
				{
					const int nodeIdx = nodeIndexByBone[i];
					const char* name = (nodeIdx >= 0) ? skeletonData->nodes[static_cast<std::size_t>(nodeIdx)].name.c_str() : "<unknown>";
					if (nodeIdx < 0)
					{
						continue;
					}

					const glm::vec3 animatedGlobalOrigin = glm::vec3(_globalMatrices[i] * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
					const glm::vec4 bindGlobalOriginH = bindGlobalByNode[static_cast<std::size_t>(nodeIdx)] * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
					const glm::vec3 finalFromBindOrigin = glm::vec3(_boneMatrices[i] * bindGlobalOriginH);
					const glm::vec3 delta = finalFromBindOrigin - animatedGlobalOrigin;
					std::cout << "  Bone " << i << " (" << name << ")"
						<< " animatedGlobalOrigin=[" << animatedGlobalOrigin.x << ", " << animatedGlobalOrigin.y << ", " << animatedGlobalOrigin.z << "]"
						<< " final*bindGlobalOrigin=[" << finalFromBindOrigin.x << ", " << finalFromBindOrigin.y << ", " << finalFromBindOrigin.z << "]"
						<< " delta=[" << delta.x << ", " << delta.y << ", " << delta.z << "]\n";
				}
			}
		}

		const std::vector<glm::mat4>& getBoneMatrices() const { return _boneMatrices; }
		const std::vector<glm::mat4>& getGlobalMatrices() const { return _globalMatrices; }
		bool hasPose() const { return !_boneMatrices.empty(); }

	private:
		float _currentTime = 0.0f;
		std::vector<glm::mat4> _boneMatrices;
		std::vector<glm::mat4> _globalMatrices;
		bool _debugPrinted = false;
	};
}

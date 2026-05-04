#pragma once

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <algorithm>
#include <cstddef>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine
{
	struct KeyPosition
	{
		float time = 0.0f;
		glm::vec3 value = glm::vec3(0.0f);
	};

	struct KeyRotation
	{
		float time = 0.0f;
		glm::quat value = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	};

	struct KeyScale
	{
		float time = 0.0f;
		glm::vec3 value = glm::vec3(1.0f);
	};

	struct BoneTrack
	{
		std::string nodeName;
		std::vector<KeyPosition> positions;
		std::vector<KeyRotation> rotations;
		std::vector<KeyScale> scales;
	};

	struct SkeletonNode
	{
		std::string name;
		int parentIndex = -1;
		std::vector<int> children;
		glm::mat4 bindLocalTransform = glm::mat4(1.0f);
		glm::mat4 inverseBindMatrix = glm::mat4(1.0f);
		bool isBone = false;
		int boneIndex = -1;
	};

	struct Skeleton
	{
		std::vector<SkeletonNode> nodes;
		std::unordered_map<std::string, int> nodeLookup;
		glm::mat4 globalInverseTransform = glm::mat4(1.0f);
		int rootNodeIndex = -1;

		std::size_t boneCount() const
		{
			std::size_t count = 0;
			for (const auto& node : nodes)
			{
				if (node.isBone && node.boneIndex >= 0)
				{
					count = std::max(count, static_cast<std::size_t>(node.boneIndex + 1));
				}
			}
			return count;
		}

		int findNodeIndex(const std::string& name) const
		{
			auto it = nodeLookup.find(name);
			if (it == nodeLookup.end())
			{
				return -1;
			}
			return it->second;
		}
	};

	struct AnimationClip
	{
		std::string name;
		float durationTicks = 0.0f;
		float ticksPerSecond = 25.0f;
		std::vector<BoneTrack> tracks;
		std::unordered_map<std::string, std::size_t> trackLookup;

		const BoneTrack* findTrack(const std::string& nodeName) const
		{
			auto it = trackLookup.find(nodeName);
			if (it == trackLookup.end())
			{
				return nullptr;
			}
			return &tracks[it->second];
		}
	};

	inline glm::mat4 composeTransform(const glm::vec3& position,
		const glm::quat& rotation,
		const glm::vec3& scale)
	{
		return glm::translate(glm::mat4(1.0f), position) *
			glm::mat4_cast(rotation) *
			glm::scale(glm::mat4(1.0f), scale);
	}

	inline std::size_t findPositionKey(const std::vector<KeyPosition>& keys, float time)
	{
		for (std::size_t i = 0; i + 1 < keys.size(); ++i)
		{
			if (time < keys[i + 1].time)
			{
				return i;
			}
		}
		return keys.size() - 1;
	}

	inline std::size_t findRotationKey(const std::vector<KeyRotation>& keys, float time)
	{
		for (std::size_t i = 0; i + 1 < keys.size(); ++i)
		{
			if (time < keys[i + 1].time)
			{
				return i;
			}
		}
		return keys.size() - 1;
	}

	inline std::size_t findScaleKey(const std::vector<KeyScale>& keys, float time)
	{
		for (std::size_t i = 0; i + 1 < keys.size(); ++i)
		{
			if (time < keys[i + 1].time)
			{
				return i;
			}
		}
		return keys.size() - 1;
	}

	inline glm::vec3 interpolatePosition(const std::vector<KeyPosition>& keys, float time)
	{
		if (keys.empty())
		{
			return glm::vec3(0.0f);
		}

		if (keys.size() == 1)
		{
			return keys.front().value;
		}

		std::size_t idx = findPositionKey(keys, time);
		const KeyPosition& current = keys[idx];
		const KeyPosition& next = keys[std::min(idx + 1, keys.size() - 1)];
		float delta = next.time - current.time;
		float factor = (delta > 0.0f) ? ((time - current.time) / delta) : 0.0f;
		return glm::mix(current.value, next.value, glm::clamp(factor, 0.0f, 1.0f));
	}

	inline glm::quat interpolateRotation(const std::vector<KeyRotation>& keys, float time)
	{
		if (keys.empty())
		{
			return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		}

		if (keys.size() == 1)
		{
			return glm::normalize(keys.front().value);
		}

		std::size_t idx = findRotationKey(keys, time);
		const KeyRotation& current = keys[idx];
		const KeyRotation& next = keys[std::min(idx + 1, keys.size() - 1)];
		float delta = next.time - current.time;
		float factor = (delta > 0.0f) ? ((time - current.time) / delta) : 0.0f;
		return glm::normalize(glm::slerp(current.value, next.value, glm::clamp(factor, 0.0f, 1.0f)));
	}

	inline glm::vec3 interpolateScale(const std::vector<KeyScale>& keys, float time)
	{
		if (keys.empty())
		{
			return glm::vec3(1.0f);
		}

		if (keys.size() == 1)
		{
			return keys.front().value;
		}

		std::size_t idx = findScaleKey(keys, time);
		const KeyScale& current = keys[idx];
		const KeyScale& next = keys[std::min(idx + 1, keys.size() - 1)];
		float delta = next.time - current.time;
		float factor = (delta > 0.0f) ? ((time - current.time) / delta) : 0.0f;
		return glm::mix(current.value, next.value, glm::clamp(factor, 0.0f, 1.0f));
	}

	inline glm::mat4 evaluateTrack(const BoneTrack& track, float time)
	{
		return composeTransform(
			interpolatePosition(track.positions, time),
			interpolateRotation(track.rotations, time),
			interpolateScale(track.scales, time)
		);
	}

	inline void evaluateSkeletonPoseRecursive(
		const Skeleton& skeleton,
		const AnimationClip* clip,
		int nodeIndex,
		const glm::mat4& parentTransform,
		float time,
		std::vector<glm::mat4>& outBoneMatrices)
	{
		if (nodeIndex < 0 || nodeIndex >= static_cast<int>(skeleton.nodes.size()))
		{
			return;
		}

		const SkeletonNode& node = skeleton.nodes[nodeIndex];
		glm::mat4 localTransform = node.bindLocalTransform; // start with bind pose matrix for vertices to go from model space to bone space

		if (clip)
		{
			if (const BoneTrack* track = clip->findTrack(node.name)) // if animation track for this node exists, 
			{
				localTransform = evaluateTrack(*track, time); // then use local transform
			}
		}

		glm::mat4 globalTransform = parentTransform * localTransform; // model space -> bone space -> model space

		if (node.isBone && node.boneIndex >= 0 && node.boneIndex < static_cast<int>(outBoneMatrices.size())) // check if valid bone index and if a bone 
		{
			outBoneMatrices[node.boneIndex] = skeleton.globalInverseTransform * globalTransform * node.inverseBindMatrix;
		}

		for (int childIndex : node.children)
		{
			evaluateSkeletonPoseRecursive(skeleton, clip, childIndex, globalTransform, time, outBoneMatrices);
		}
	}

	inline void evaluateSkeletonPose(
		const Skeleton& skeleton,
		const AnimationClip* clip,
		float time,
		std::vector<glm::mat4>& outBoneMatrices)
	{
		if (skeleton.rootNodeIndex < 0)
		{
			return;
		}

		if (outBoneMatrices.size() < skeleton.boneCount())
		{
			outBoneMatrices.resize(skeleton.boneCount(), glm::mat4(1.0f));
		}

		evaluateSkeletonPoseRecursive(
			skeleton,
			clip,
			skeleton.rootNodeIndex,
			glm::mat4(1.0f),
			time,
			outBoneMatrices
		);
	}
}

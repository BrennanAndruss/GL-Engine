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

	inline glm::quat convertAnimationRotationHandedness(const glm::quat& q)
	{
		return glm::normalize(q);
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

	inline bool shouldUseAnimatedRotationAbsolute(const SkeletonNode& node)
	{
		// Root/hips are usually authored in absolute local orientation.
		if (node.parentIndex < 0)
		{
			return true;
		}
		return node.name.find("Hips") != std::string::npos;
	}

	inline glm::mat4 evaluateTrackWithBindTranslation(const SkeletonNode& node, const BoneTrack& track, float time)
	{
		glm::vec3 position = interpolatePosition(track.positions, time);
		glm::quat rotation = convertAnimationRotationHandedness(interpolateRotation(track.rotations, time));
		glm::vec3 scale = interpolateScale(track.scales, time);

		if (!shouldUseAnimatedRotationAbsolute(node))
		{
			const glm::mat3 bindBasis(node.bindLocalTransform);
		}

		return composeTransform(
			position,
			rotation,
			scale
		);
	}

	inline void evaluateSkeletonPoseRecursive(
		const Skeleton& skeleton,
		const AnimationClip* clip,
		int nodeIndex,
		const glm::mat4& parentTransform,
		float time,
		std::vector<glm::mat4>& outBoneMatrices,
		bool debugFirstFrame = false,
		int debugDepth = 0)
	{
		if (nodeIndex < 0 || nodeIndex >= static_cast<int>(skeleton.nodes.size()))
		{
			return;
		}

		const SkeletonNode& node = skeleton.nodes[nodeIndex];
		glm::mat4 localTransform = node.bindLocalTransform; // start with bind pose matrix for vertices to go from model space to bone space

		bool hasTrack = false;
		if (clip)
		{
			if (const BoneTrack* track = clip->findTrack(node.name)) // if animation track for this node exists, 
			{
				localTransform = evaluateTrackWithBindTranslation(node, *track, time);
				hasTrack = true;
			}
		}

		glm::mat4 globalTransform = parentTransform * localTransform; // model space -> bone space -> model space

		if (node.isBone && node.boneIndex >= 0 && node.boneIndex < static_cast<int>(outBoneMatrices.size())) // check if valid bone index and if a bone 
		{
			// Compute both conventions for debug, but use the global-inverse version for skinning.
			glm::mat4 finalWithGlobalInv = skeleton.globalInverseTransform * globalTransform * node.inverseBindMatrix;
			glm::mat4 finalWithoutGlobalInv = globalTransform * node.inverseBindMatrix;

			// Apply the global inverse in the active skinning path.
			outBoneMatrices[node.boneIndex] = finalWithGlobalInv;

			// Debug output for first frame
			if (debugFirstFrame && debugDepth < 3)
			{
				std::cout << "  [Pose] Bone " << node.boneIndex << " (" << node.name << "):\n"
					<< "    Local[3]: [" << localTransform[3][0] << ", " << localTransform[3][1] << ", " << localTransform[3][2] << "]"
					<< (hasTrack ? " (from track)" : " (bind pose)") << "\n"
					<< "    Global[3]: [" << globalTransform[3][0] << ", " << globalTransform[3][1] << ", " << globalTransform[3][2] << "]\n"
					<< "    FinalWithoutGlobalInv[3]: [" << finalWithoutGlobalInv[3][0] << ", " 
					<< finalWithoutGlobalInv[3][1] << ", " 
					<< finalWithoutGlobalInv[3][2] << "]\n"
					<< "    FinalWithGlobalInv[3]: [" << finalWithGlobalInv[3][0] << ", " 
					<< finalWithGlobalInv[3][1] << ", " 
					<< finalWithGlobalInv[3][2] << "]\n";
			}
		}

		for (int childIndex : node.children)
		{
			evaluateSkeletonPoseRecursive(skeleton, clip, childIndex, globalTransform, time, outBoneMatrices, debugFirstFrame, debugDepth + 1);
		}
	}

	inline void evaluateSkeletonPose(
		const Skeleton& skeleton,
		const AnimationClip* clip,
		float time,
		std::vector<glm::mat4>& outBoneMatrices,
		bool debugFirstFrame = false)
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
			outBoneMatrices,
			debugFirstFrame
		);
	}

	// Evaluate global transforms (animated) for each bone (indexed by boneIndex)
	inline void evaluateSkeletonGlobalsRecursive(
		const Skeleton& skeleton,
		const AnimationClip* clip,
		int nodeIndex,
		const glm::mat4& parentTransform,
		float time,
		std::vector<glm::mat4>& outGlobalTransforms)
	{
		if (nodeIndex < 0 || nodeIndex >= static_cast<int>(skeleton.nodes.size()))
		{
			return;
		}

		const SkeletonNode& node = skeleton.nodes[nodeIndex];
		glm::mat4 localTransform = node.bindLocalTransform;
		if (clip)
		{
			if (const BoneTrack* track = clip->findTrack(node.name))
			{
				localTransform = evaluateTrackWithBindTranslation(node, *track, time);
			}
		}

		glm::mat4 globalTransform = parentTransform * localTransform;

		if (node.isBone && node.boneIndex >= 0 && node.boneIndex < static_cast<int>(outGlobalTransforms.size()))
		{
			outGlobalTransforms[node.boneIndex] = globalTransform;
		}

		for (int childIndex : node.children)
		{
			evaluateSkeletonGlobalsRecursive(skeleton, clip, childIndex, globalTransform, time, outGlobalTransforms);
		}
	}

	inline void evaluateSkeletonGlobals(
		const Skeleton& skeleton,
		const AnimationClip* clip,
		float time,
		std::vector<glm::mat4>& outGlobalTransforms)
	{
		if (skeleton.rootNodeIndex < 0) return;
		if (outGlobalTransforms.size() < skeleton.boneCount())
		{
			outGlobalTransforms.resize(skeleton.boneCount(), glm::mat4(1.0f));
		}

		evaluateSkeletonGlobalsRecursive(
			skeleton,
			clip,
			skeleton.rootNodeIndex,
			glm::mat4(1.0f),
			time,
			outGlobalTransforms
		);
	}
}

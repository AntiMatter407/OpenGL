#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <learnopengl/bone.h>
#include <functional>
#include <learnopengl/animdata.h>
#include <learnopengl/model_animation.h>

struct BoneNodeData
{
	glm::mat4 transformation;
	std::string name;
	std::vector<BoneNodeData> children;
};



class Animation
{
public:
	Animation() = default;

	Animation(const std::string& animationPath, Model* model)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
		assert(scene && scene->mRootNode);
		auto animation = scene->mAnimations[0];
		m_Duration = animation->mDuration;
		m_TicksPerSecond = animation->mTicksPerSecond;
		std::cout << "Animation number: " << scene->mNumAnimations << std::endl;
		std::cout << "Animation Duration: " << m_Duration << std::endl;
		std::cout << "Animation TicksPerSecond  " << m_TicksPerSecond << std::endl;
		aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
		globalTransformation = globalTransformation.Inverse();
		ReadMissingBones(animation, *model);
		ReadHierarchyData(m_RootNode, scene->mRootNode);
	}

	~Animation()
	{
	}

	Bone* FindBone(const std::string& name)
	{
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
			[&](const Bone& Bone)
		{
			return Bone.GetBoneName() == name;
		}
		);
		if (iter == m_Bones.end()) return nullptr;
		else return &(*iter);
	}


	inline float GetTicksPerSecond() { return m_TicksPerSecond; }
	inline float GetDuration() { return m_Duration; }
	inline const BoneNodeData& GetRootNode() { return m_RootNode; }
	inline const std::map<std::string, BoneInfo>& GetBoneIDMap()
	{
		return m_BoneInfoMap;
	}

private:
	void ReadMissingBones(const aiAnimation* animation, Model& model)
	{
		int size = animation->mNumChannels;

		auto& boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
		int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

		//reading channels(bones engaged in an animation and their keyframes)
		for (int i = 0; i < size; i++)
		{
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;

			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				boneInfoMap[boneName].id = boneCount;
				boneCount++;
			}
			m_Bones.push_back(Bone(channel->mNodeName.data,
				boneInfoMap[channel->mNodeName.data].id, channel));
		}

		m_BoneInfoMap = boneInfoMap;
	}

	void ReadHierarchyData(BoneNodeData& dest, const aiNode* src)
	{
		assert(src);

		bool isBone = false;
		std::string NodeName(src->mName.data);
		if (m_BoneInfoMap.find(NodeName) != m_BoneInfoMap.end())    // if the node corresponds to a bone/skeletal joint
		{
			dest.name = src->mName.data;
			dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
			isBone = true;
		}
		for (int i = 0; i < src->mNumChildren; i++)
		{
			if (isBone) {
				BoneNodeData newData;
				ReadHierarchyData(newData, src->mChildren[i]);
				if (newData.name != "") {
					dest.children.push_back(newData);     //adding the bone to the hierarchy, if the bone is valid.
				}
				
			}
			else {
				ReadHierarchyData(dest, src->mChildren[i]);
			}
			
		}
	}
	float m_Duration;
	int m_TicksPerSecond;
	std::vector<Bone> m_Bones;
	BoneNodeData m_RootNode;
	std::map<std::string, BoneInfo> m_BoneInfoMap;
};


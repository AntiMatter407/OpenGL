#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <learnopengl/animation.h>
#include <learnopengl/bone.h>
#include <GLFW/glfw3.h>

GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 



class Animator
{
public:
	Animator(Animation* animation)
	{
		m_CurrentTime = 0.0;
		m_CurrentAnimation = animation;
			
		InitAnim();
	}

	void ScanSkeleton(const BoneNodeData* node, const BoneNodeData* parent)
	{
		std::string nodeName = node->name;
		std::string parentName = parent? parent->name : "";

		auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end() && boneInfoMap.find(parentName) != boneInfoMap.end())
		{
			int index = boneInfoMap[nodeName].id;
			int parentIndex = boneInfoMap[parentName].id;
			m_BoneLink.push_back(parentIndex);
			m_BoneLink.push_back(index);

		}
		for (int i = 0; i < node->children.size(); i++)
			ScanSkeleton(&node->children[i], node);

	}

	void InitAnim()
	{
		m_FinalBoneMatrices.reserve(100);
		m_BonePositions.reserve(100);
		for (int i = 0; i < 100; i++) {
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
			m_BonePositions.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		ScanSkeleton(&m_CurrentAnimation->GetRootNode(), nullptr);
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		
	}

	void UpdateAnimation(float dt)
	{
		m_DeltaTime = dt;
		if (m_CurrentAnimation)
		{
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
		}
	}

	void PlayAnimation(Animation* pAnimation)
	{
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
		InitAnim();
	}

	void CalculateBoneTransform(const BoneNodeData* node, glm::mat4 parentTransform)
	{
		std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

		if (Bone)
		{
			Bone->Update(m_CurrentTime);
			nodeTransform = Bone->GetLocalTransform();
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			int index = boneInfoMap[nodeName].id;
			glm::mat4 offset = boneInfoMap[nodeName].offset;
			m_FinalBoneMatrices[index] = globalTransformation * offset;
			m_BonePositions[index] = globalTransformation * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		}

		for (int i = 0; i < node->children.size(); i++)
			CalculateBoneTransform(&node->children[i], globalTransformation);
	}

	std::vector<glm::mat4> GetFinalBoneMatrices()
	{
		return m_FinalBoneMatrices;
	}

	void DrawBones()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
		glLineWidth(2.0f);
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, m_BonePositions.size()*sizeof(glm::vec4), &m_BonePositions[0], GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,  m_BoneLink.size()*sizeof(int), &m_BoneLink[0], GL_STREAM_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);

		glDrawElements(GL_LINES, m_BoneLink.size(), GL_UNSIGNED_INT, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	std::vector<glm::vec4> GetBonePositions()
	{
		return m_BonePositions;
	}

	Animation* getAnimation()
	{
		return m_CurrentAnimation;
	}

private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
	std::vector<glm::vec4> m_BonePositions;
	std::vector<unsigned int> m_BoneLink;
	Animation* m_CurrentAnimation;
	float m_CurrentTime;
	float m_DeltaTime;
	unsigned int VAO, VBO, EBO;
};

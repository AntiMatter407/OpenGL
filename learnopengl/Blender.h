#pragma once


#include <learnopengl/animator.h>


class Blender
{
public:
	Blender(float r = 0.5) :ratio(r), m_Animator1(nullptr), m_Animator2(nullptr) 
	{
		InitMatirices();
	}
	Blender(Animator* anim1, Animator* anim2, float ratio) : m_Animator1(anim1), m_Animator2(anim2), ratio(ratio) 
	{
		InitMatirices();
	}
	void SetAnimation(Animator* anim1, Animator* anim2, float ratio)
	{
		m_Animator1 = anim1;
		m_Animator2 = anim2;
		ratio = ratio;
	}

	void ScanSkeleton(const BoneNodeData* node, const BoneNodeData* parent)
	{
		std::string nodeName = node->name;
		std::string parentName = parent ? parent->name : "";

		auto boneInfoMap = m_Animator1->getAnimation()->GetBoneIDMap();
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

	void SetRatio(float r) { ratio = r; }

	~Blender() = default;

	void InitMatirices()
	{
		m_BlenderBoneMatrices.reserve(100);
		m_BonePositions.reserve(100);
		for (int i = 0; i < 100; i++) {
			m_BlenderBoneMatrices.push_back(glm::mat4(1.0f));
			m_BonePositions.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		ScanSkeleton(&m_Animator1->getAnimation()->GetRootNode(), nullptr);
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
	}

	void update(float dt)
	{
		if (m_Animator1 && m_Animator2) {
			if (m_Animator1->GetFinalBoneMatrices().size() != m_Animator2->GetFinalBoneMatrices().size()) return;
			m_Animator1->UpdateAnimation(dt);
			m_Animator2->UpdateAnimation(dt);
			const std::vector<glm::mat4>& FinalBoneMatrices1 = m_Animator1->GetFinalBoneMatrices();
			const std::vector<glm::mat4>& FinalBoneMatrices2 = m_Animator2->GetFinalBoneMatrices();
			const std::vector<glm::vec4>& BonePosition1 = m_Animator1->GetBonePositions();
			const std::vector<glm::vec4>& BonePosition2 = m_Animator2->GetBonePositions();
			auto mNumBone = FinalBoneMatrices1.size();
			for (int i = 0; i < mNumBone; i++) {
				m_BlenderBoneMatrices[i] = (1 - ratio) * FinalBoneMatrices1[i] + ratio * FinalBoneMatrices2[i];
				m_BonePositions[i] = (1 - ratio) * BonePosition1[i] + ratio * BonePosition2[i];
			}

		}
	}
	std::vector<glm::mat4> GetBlenderBoneMatrices()
	{
		return m_BlenderBoneMatrices;
	}

	void DrawBones()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
		glLineWidth(2.0f);
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, m_BonePositions.size() * sizeof(glm::vec4), &m_BonePositions[0], GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_BoneLink.size() * sizeof(int), &m_BoneLink[0], GL_STREAM_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);

		glDrawElements(GL_LINES, m_BoneLink.size(), GL_UNSIGNED_INT, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

private:
	// timing
	float Curtime = 0.0f;
	float ratio = 0.0f;

	unsigned int VAO, VBO, EBO;

	// animation
	Animator* m_Animator1, * m_Animator2;
	std::vector<glm::vec4> m_BonePositions;
	std::vector<unsigned int> m_BoneLink;

	// matrix
	std::vector<glm::mat4> m_BlenderBoneMatrices;
};
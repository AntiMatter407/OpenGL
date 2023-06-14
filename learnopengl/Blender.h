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

	void SetRatio(float r) { ratio = r; }

	~Blender() = default;

	void InitMatirices()
	{
		m_BlenderBoneMatrices.reserve(100);
		for (int i = 0; i < 100; i++) {
			m_BlenderBoneMatrices.push_back(glm::mat4(1.0f));
		}
	}

	void update(float dt)
	{
		if (m_Animator1 && m_Animator2) {
			if (m_Animator1->GetFinalBoneMatrices().size() != m_Animator2->GetFinalBoneMatrices().size()) return;
			m_Animator1->UpdateAnimation(dt);
			m_Animator2->UpdateAnimation(dt);
			std::vector<glm::mat4> FinalBoneMatrices1 = m_Animator1->GetFinalBoneMatrices();
			std::vector<glm::mat4> FinalBoneMatrices2 = m_Animator2->GetFinalBoneMatrices();
			auto mNumBone = FinalBoneMatrices1.size();
			for (int i = 0; i < mNumBone; i++) {
				m_BlenderBoneMatrices[i] = (1 - ratio) * FinalBoneMatrices1[i] + ratio * FinalBoneMatrices2[i];
			}

		}
	}
	std::vector<glm::mat4> GetBlenderBoneMatrices()
	{
		return m_BlenderBoneMatrices;
	}

private:
	// timing
	float Curtime = 0.0f;
	float ratio = 0.0f;

	// animation
	Animator* m_Animator1, * m_Animator2;

	// matrix
	std::vector<glm::mat4> m_BlenderBoneMatrices;
};
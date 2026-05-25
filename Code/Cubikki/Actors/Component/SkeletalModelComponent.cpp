#include "Cubikki/Actors/Component/SkeletalModelComponent.hpp"
#include "Cubikki/Actors/Component/ComponentSystem.hpp"

#include "imgui.h"

std::map<std::filesystem::path, PBE::SkeletalModel*> g_LoadedModels;

SkeletalModelSystem::SkeletalModelSystem(PBE::Renderer* renderer)
	: m_pRenderer(renderer)
{
}

SkeletalModelSystem::~SkeletalModelSystem()
{
	for (auto& pair : g_LoadedModels)
	{
		delete pair.second;
	}
	g_LoadedModels.clear();

	for (auto& entry : m_Components)
	{
		if (entry.m_InUse)
		{
			SkeletalModelComponent& component = entry.m_Component;
			if (component.m_Animator)
			{
				component.m_Animator->Free(m_pRenderer->m_Allocator);
				delete component.m_Animator;
				component.m_Animator = nullptr;
			}
		}
	}
}

UniqueComponentReference SkeletalModelSystem::CreateComponentFromFilepath(std::string_view name, std::filesystem::path modelPath)
{
	PBE::SkeletalModel* model;
	auto itr = g_LoadedModels.find(modelPath);
	if (itr != g_LoadedModels.end())
	{
		model = itr->second;
	}
	else
	{
		PBE::SkeletalModelCreateInfo info;
		info.m_Name = name;
		info.m_Renderer = m_pRenderer;
		model = new PBE::SkeletalModel(info);
		model->LoadFromFile(modelPath);
		g_LoadedModels[modelPath] = model;
	}

	return CreateComponent(name, model);
}

void SkeletalModelSystem::CmdDraw(PBE::Camera* camera, VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex)
{
	vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pRenderer->m_SkinnedMeshPipeline);
	camera->CmdBind(buffer, layout, frameIndex);
	for (auto itr = m_Components.begin(); itr != m_Components.end(); ++itr)
	{
		auto& entry = *itr;
		if (entry.m_InUse)
		{
			SkeletalModelComponent& component = entry.m_Component;
			if (component.m_Visible)
			{
				if (TransformComponent* transform = component.m_Transform.GetAs<TransformComponent>())
				{
					entry.m_Component.m_Animator->UpdateDescriptorSets(frameIndex);
					entry.m_Component.m_Animator->CmdBind(buffer, layout, frameIndex);
					entry.m_Component.m_Model->CmdDraw(camera, transform->m_Transform, buffer, layout, frameIndex);
				}
			}
		}
	}
	vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pRenderer->m_StaticMeshPipeline);
}

void SkeletalModelComponent::SetParentTransform(ComponentReference transform)
{
	m_Transform = transform;
}

void SkeletalModelComponent::SetAnimationState(std::string name, float time, float weight)
{
	if (m_Animator)
	{
		m_Animator->SetAnimationState(name, time, weight);
	}
}

void SkeletalModelComponent::SetAnimationWeight(std::string name, float weight)
{
	if (m_Animator)
	{
		m_Animator->SetWeight(name, weight);
	}
}

void SkeletalModelComponent::SetAnimationTime(std::string name, float time)
{
	if (m_Animator)
	{
		m_Animator->SetTime(name, time);
	}
}

void SkeletalModelComponent::GetAnimationWeight(std::string name, float& out_weight) const
{
	if (m_Animator)
	{
		m_Animator->GetWeight(name, out_weight);
	}
}

void SkeletalModelComponent::GetAnimationTime(std::string name, float& out_time) const
{
	if (m_Animator)
	{
		m_Animator->GetTime(name, out_time);
	}
}

void SkeletalModelComponent::OnAttach(Actor* actor)
{
	PBE::AnimatorCreateInfo info = {};
	info.m_Model = m_Model;
	info.m_Renderer = m_Model->m_CreateInfo.m_Renderer;

	m_Animator = new PBE::Animator(info);
}

void SkeletalModelComponent::OnDetach(Actor* actor)
{
	if (m_Animator)
	{
		m_Animator->Free(m_Model->m_CreateInfo.m_Renderer->m_Allocator);
		delete m_Animator;
		m_Animator = nullptr;
	}
}

void SkeletalModelComponent::ImGui()
{
	std::vector<std::string> names;
	m_Animator->GetAnimationNames(names);

	for (auto& name : names)
	{
		if (ImGui::TreeNodeEx(name.c_str()))
		{
			float weight = 0.f;
			m_Animator->GetWeight(name, weight);
			ImGui::SliderFloat("Weight", &weight, 0.f, 1.f);
			m_Animator->SetWeight(name, weight);

			float time = 0.f;
			m_Animator->GetTime(name, time);
			ImGui::SliderFloat("Time", &time, 0.f, m_Model->m_Animations[name]->m_Duration);
			m_Animator->SetTime(name, time);
			ImGui::TreePop();
		}

	}
}


SkeletalModelComponent::SkeletalModelComponent(std::string_view name, PBE::SkeletalModel* model)
	: Component(name)
	, m_Model(model)
	, m_Visible(true)
{
}

SkeletalModelComponent::SkeletalModelComponent(SkeletalModelComponent&& other) noexcept
	: Component(std::move(other))              // <-- move the base!
	, m_Model(other.m_Model)
	, m_Animator(other.m_Animator)
	, m_Transform(std::move(other.m_Transform))
	, m_Visible(other.m_Visible)
{
	other.m_Model = nullptr;
	other.m_Animator = nullptr;
	other.m_Visible = false;
}

SkeletalModelComponent& SkeletalModelComponent::operator=(SkeletalModelComponent&& other) noexcept {
	if (this != &other) {
		Component::operator=(std::move(other)); // <-- move-assign base
		m_Model = std::exchange(other.m_Model, nullptr);
		m_Animator = std::exchange(other.m_Animator, nullptr);
		m_Transform = std::move(other.m_Transform);
		m_Visible = std::exchange(other.m_Visible, false);
	}
	return *this;
}

SkeletalModelComponent::~SkeletalModelComponent() noexcept
{
	if(m_Animator)
	{
		m_Animator->Free(m_Model->m_CreateInfo.m_Renderer->m_Allocator);
		delete m_Animator;
		m_Animator = nullptr;
	}
}

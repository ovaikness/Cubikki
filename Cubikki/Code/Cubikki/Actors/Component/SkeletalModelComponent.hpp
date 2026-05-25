#pragma once

#include "Cubikki/Actors/Actor.hpp"

#include "Cubikki/Actors/Component/ComponentList.hpp"
#include "Cubikki/Actors/Component/ComponentSystem.hpp"
#include "Cubikki/Actors/Component/SkeletalModelComponent.hpp"
#include "Cubikki/Actors/Component/Component.hpp"
#include "Cubikki/Actors/Component/TransformComponent.hpp"

#include "Engine/Scene/SkeletalModel.hpp"
#include "Engine/Graphics/Renderer.hpp"

#include <string>
#include <string_view>
#include <filesystem>
#include <map>

class Actor;

class SkeletalModelComponent : public Component
{
public:
	bool m_Visible{ true };
	PBE::SkeletalModel* m_Model{ nullptr };
	PBE::Animator* m_Animator{ nullptr };
	ComponentReference m_Transform;
	void SetParentTransform(ComponentReference transform);
	void SetAnimationState(std::string name, float time, float weight);
	void SetAnimationWeight(std::string name, float weight);
	void SetAnimationTime(std::string name, float time);
	void GetAnimationWeight(std::string name, float& out_weight) const;
	void GetAnimationTime(std::string name, float& out_time) const;
	virtual void OnAttach(Actor* actor) override;
	virtual void OnDetach(Actor* actor) override;
	virtual void ImGui() override;
public:
	SkeletalModelComponent() = default;
	SkeletalModelComponent(std::string_view name, PBE::SkeletalModel* model);
	SkeletalModelComponent(SkeletalModelComponent&& other) noexcept;
	SkeletalModelComponent& operator=(SkeletalModelComponent&& other) noexcept;
	virtual ~SkeletalModelComponent() noexcept;
};

class SkeletalModelSystem : public ComponentSystem<SkeletalModelComponent>
{
public:
	std::map<std::filesystem::path, PBE::SkeletalModel*> m_LoadedModels;
	PBE::Renderer* m_pRenderer;
public:
	SkeletalModelSystem(PBE::Renderer* renderer);
	~SkeletalModelSystem();
	UniqueComponentReference CreateComponentFromFilepath(std::string_view name, std::filesystem::path modelPath);
public:
	void CmdDraw(PBE::Camera* camera, VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex);
};
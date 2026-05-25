#pragma once

#include "Cubikki/Actors/Component/Component.hpp"
#include "Cubikki/Actors/Component/ComponentSystem.hpp"

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Mat4.hpp"

class TransformComponent : public Component
{
public:
	bool m_Replicates = false;
	ComponentReference m_ParentTransform;
	PBE::Mat4 m_Transform{ PBE::Mat4::IDENTITY };
public:
	TransformComponent() = default;
	TransformComponent(std::string_view name);
	TransformComponent(std::string_view name, PBE::Vec3 position = PBE::Vec3(0.f,0.f,0.f), PBE::EulerAngles rotation = PBE::EulerAngles(0.f, 0.f, 0.f), PBE::Vec3 scale = PBE::Vec3(1.f));
public:
	PBE::Vec3 GetPosition();
	PBE::EulerAngles GetRotation();
	PBE::Vec3 GetScale();

	PBE::Vec3 GetForward() const;
	PBE::Vec3 GetRight() const;
	PBE::Vec3 GetUp() const;

	void SetPosition(PBE::Vec3 position);
	void SetRotation(PBE::EulerAngles rotation);
	void SetScale(PBE::Vec3 scale);

	PBE::Mat4 GetTransform();
	void SetMatrix(PBE::Mat4 transform);

	virtual void ImGui() override;
protected:
	PBE::Vec3 m_Position { 0.f };
	PBE::EulerAngles m_Rotation { 0.f,0.f,0.f };
	PBE::Vec3 m_Scale { 1.f };
};

class TransformSystem : public ComponentSystem<TransformComponent>
{
public:
	TransformSystem() = default;
	~TransformSystem() = default;

	void Update();
};
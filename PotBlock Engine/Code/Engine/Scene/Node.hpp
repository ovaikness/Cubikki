#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat4.hpp"
#include "Engine/Math/Rotor3D.hpp"

#include "Engine/Graphics/Mesh.hpp"
#include "Engine/Graphics/Material.hpp"
#include "Engine/Graphics/Camera.hpp"

#undef min
#undef max

#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <set>

namespace PBE
{
	class Node
	{
		public:
		std::string m_Name;
		uint32_t m_BoneIndex;
		float m_BoundingRadius = -1.f;

		std::vector<GPUMesh*>  m_Meshes;
		std::vector<Material*> m_Materials;
		std::set<std::string> m_GameplayTags;

		Mat4 m_LocalTransform;
		Mat4 m_WorldTransform;
		Mat4 m_InverseBindMatrix;
		public:
		Node();
		~Node();

		void AddDebugVerts(CPUMesh* mesh);
		void CmdDraw(Camera* camera, Mat4 const& transform, VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex);

		void SetWorldTransform(Mat4 const& transform);
		void SetLocalTransform(Mat4 const& transform);

		void SetLocalPosition(Vec3 const& position);
		void SetScale(Vec3 const& scale);
		void SetLocalRotation(EulerAngles const& rotation);

		Vec3 GetPosition() const;
		Vec3 GetScale() const;

		EulerAngles GetRotationEulerAngles() const;
		Rotor3D GetRotationQuaternion() const;

		Mat4 GetLocalTransform() const;
		Mat4 GetWorldTransform() const;

		Node* GetParent() const;
		std::vector<Node*> const& GetChildren() const;

		Node* FindNode(std::string_view name);
		void AddChild(Node* child);
		void RemoveChild(Node* child);

		void UpdateLocalTransform();
		void UpdateWorldTransform();
		protected:
		Node* m_Parent;
		std::vector<Node*> m_Children;

		Vec3 m_Position;
		Vec3 m_Scale;
		EulerAngles m_RotationEuler;
		Rotor3D m_RotationRotor;
	};
}
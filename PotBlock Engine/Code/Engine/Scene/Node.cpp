#include "Engine/Scene/Node.hpp"
#include "Engine/Math/Sphere.hpp"
#include "Engine/Graphics/Camera.hpp"

#include <map>


PBE::Node::Node()
	: m_Parent(nullptr)
	, m_Position(Vec3::ZERO)
	, m_Scale(Vec3::ONE)
	, m_RotationEuler(EulerAngles::ZERO)
	, m_LocalTransform(Mat4::IDENTITY)
	, m_WorldTransform(Mat4::IDENTITY)
	, m_BoneIndex(0)
{
}

PBE::Node::~Node()
{
	if (m_Parent)
	{
		m_Parent->RemoveChild(this);
	}
	
	for (Node* child : m_Children)
	{
		delete child;
	}

	m_Children.clear();
}

void PBE::Node::AddDebugVerts(CPUMesh* mesh)
{
	Vec3 position = m_WorldTransform.GetTranslation3D();
	mesh->AddVertsForSphere(position, 2.f, 16, 16, Rgba8::CYAN);

	for (Node* child : m_Children)
	{
		child->AddDebugVerts(mesh);
	}
}

void PBE::Node::CmdDraw(Camera* camera, Mat4 const& transform, VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex)
{
	Sphere boundingSphere;
	boundingSphere.m_Center = m_WorldTransform.GetTranslation3D() + transform.GetTranslation3D();
	boundingSphere.m_radius = m_BoundingRadius;

	Frustum frustum = camera->GetFrustum();

	for (uint32_t i = 0; i < m_Meshes.size(); ++i)
	{
		Material* material = m_Materials[i];
		GPUMesh*  mesh     = m_Meshes[i];
	
		if (frustum.IsSphereInside(boundingSphere) || m_BoundingRadius < 0.f)
		{
			CameraModelPushConstantData push;
			push.m_Model = transform;

			Mat4 normalMatrix = transform.GetTransposed().GetInverse();

			push.m_Normal = normalMatrix;

			vkCmdPushConstants(
				buffer,
				layout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(CameraModelPushConstantData),
				&push
			);

			material->CmdBind(buffer, layout, frameIndex);
			mesh->CmdBind(buffer);
			mesh->CmdDraw(buffer);
		}
	}


	for (size_t i = 0; i < m_Children.size(); i++)
	{
		m_Children[i]->CmdDraw(camera, transform, buffer, layout, frameIndex);
	}
}

void PBE::Node::SetWorldTransform(Mat4 const& transform)
{
	if (m_Parent)
	{
		Mat4 parentWorldTransform = m_Parent->GetWorldTransform();
		Mat4 parentWorldInverse = parentWorldTransform.GetInverse();
		Mat4 localTransform = parentWorldInverse.GetAppended(transform);

		m_Position = localTransform.GetTranslation3D();
		m_Scale = localTransform.GetScale3D();
		m_RotationRotor = localTransform.GetRotationAsQuaternion();
		m_RotationEuler = localTransform.GetRotationAsEulerAngles();
	}
	else
	{
		m_Position = transform.GetTranslation3D();
		m_Scale = transform.GetScale3D();
		m_RotationRotor = transform.GetRotationAsQuaternion();
		m_RotationEuler = transform.GetRotationAsEulerAngles();
	}
}

void PBE::Node::SetLocalTransform(Mat4 const& transform)
{
	m_Position = transform.GetTranslation3D();
	m_Scale = transform.GetScale3D();
	m_RotationRotor = transform.GetRotationAsQuaternion();
	m_RotationEuler = transform.GetRotationAsEulerAngles();
}

void PBE::Node::SetLocalPosition(Vec3 const& position)
{
	m_Position = position;
}

void PBE::Node::SetScale(Vec3 const& scale)
{
	m_Scale = scale;
}

void PBE::Node::SetLocalRotation(EulerAngles const& rotation)
{
	m_RotationEuler = rotation;
	m_RotationRotor = Rotor3D::CreateFromEulerAngles(rotation);
}

PBE::Vec3 PBE::Node::GetPosition() const
{
	return m_Position;
}

PBE::Vec3 PBE::Node::GetScale() const
{
	return m_Scale;
}

PBE::EulerAngles PBE::Node::GetRotationEulerAngles() const
{
	return m_RotationEuler;
}

PBE::Rotor3D PBE::Node::GetRotationQuaternion() const
{
	return m_RotationRotor;
}

PBE::Mat4 PBE::Node::GetLocalTransform() const
{
	return m_LocalTransform;
}

PBE::Mat4 PBE::Node::GetWorldTransform() const
{
	return m_WorldTransform;
}

PBE::Node* PBE::Node::GetParent() const
{
	return m_Parent;
}

std::vector<PBE::Node*> const& PBE::Node::GetChildren() const
{
	return m_Children;
}

PBE::Node* PBE::Node::FindNode(std::string_view name)
{
	if (m_Name == name)
	{
		return this;
	}
	for (Node* child : m_Children)
	{
		Node* found = child->FindNode(name);
		if (found)
		{
			return found;
		}
	}
	return nullptr;
}

void PBE::Node::AddChild(Node* child)
{
	if (child->m_Parent)
	{
		child->m_Parent->RemoveChild(child);
	}

	child->m_Parent = this;

	m_Children.push_back(child);
}

void PBE::Node::RemoveChild(Node* child)
{
	auto iter = std::find(m_Children.begin(), m_Children.end(), child);
	if (iter != m_Children.end())
	{
		m_Children.erase(iter);
	}
}

void PBE::Node::UpdateLocalTransform()
{
	m_LocalTransform = Mat4::CreateTRS(m_Position, m_RotationRotor, m_Scale);
}

void PBE::Node::UpdateWorldTransform()
{
	if (m_Parent)
	{
		m_WorldTransform = m_LocalTransform.GetAppended(m_Parent->GetWorldTransform());
	}
	else
	{
		m_WorldTransform = m_LocalTransform;
	}

	for (Node* child : m_Children)
	{
		child->UpdateWorldTransform();
	}
}

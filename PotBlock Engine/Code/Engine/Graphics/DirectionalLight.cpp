#include "Engine/Graphics/DirectionalLight.hpp"
#include "Engine/Graphics/Camera.hpp"
#include "Engine/Math/Vec3.hpp"

VkDescriptorSetLayout PBE::DirectionalLight::s_DescriptorSetLayout;

VkDescriptorPool PBE::DirectionalLight::s_DescriptorPool;

void PBE::DirectionalLight::CreateDescriptorSetLayout(VkDevice /* device */)
{


}

void PBE::DirectionalLight::CreateDescriptorPool(VkDevice /*device*/, uint32_t /*count*/)
{

}

PBE::DirectionalLight::DirectionalLight()
{

}

void PBE::DirectionalLight::UpdateCSM(Camera* camera, uint32_t /*frameIndex*/)
{
	std::vector<PBE::Vec4> corners = camera->GetFrustumCorners();
	PBE::Vec3 center(0.f);
	for (PBE::Vec4 const& corner : corners)
	{
		center.x += corner.x;
		center.y += corner.y;
		center.z += corner.z;
	}

	center /= (float)corners.size();

	//PBE::Vec3 lightDir = m_CreateInfo.m_Direction;
	//
	//PBE::Mat4 lightView = PBE::Mat4::CreateLookAt(center - lightDir, center, PBE::Vec3::UP);
	//
	//float zDistance = camera->GetFar() - camera->GetNear();
}

#include "Engine/Scene/SkeletalModel.hpp"
#include "Engine/Graphics/Renderer.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb/stb_image.h>

using namespace PBE;

static PBE::Mat4 ProcessMatrix(aiMatrix4x4 const& matrix)
{
	return Mat4(
		matrix.a1, matrix.b1, matrix.c1, matrix.d1, // Column 1
		matrix.a2, matrix.b2, matrix.c2, matrix.d2, // Column 2
		matrix.a3, matrix.b3, matrix.c3, matrix.d3, // Column 3
		matrix.a4, matrix.b4, matrix.c4, matrix.d4  // Column 4
	);
}

static void ProcessMesh(SkeletalModel* scene, aiMesh const* aiMesh)
{
	CPUMesh mesh;

	if (aiMesh->HasBones())
	{
		for (uint32_t i = 0; i < aiMesh->mNumBones; ++i)
		{
			aiBone* bone = aiMesh->mBones[i];
			Mat4 offsetMatrix = ProcessMatrix(bone->mOffsetMatrix);
			scene->m_Bones[bone->mName.C_Str()] =
				Bone
			{
				.m_Name = bone->mName.C_Str(),
				.m_Index = i,
				.m_InverseBindMatrix = offsetMatrix
			};
		}
	}

	if (aiMesh->HasPositions())
	{
		mesh.m_Positions = std::vector<Vec3>();
		mesh.m_Positions->reserve(aiMesh->mNumVertices);
		for (uint32_t j = 0; j < aiMesh->mNumVertices; ++j)
		{
			Vec3 position = Vec3(
				aiMesh->mVertices[j].x,
				aiMesh->mVertices[j].y,
				aiMesh->mVertices[j].z
			);

			mesh.m_Positions->emplace_back(
				position
			);
		}
	}
	else
	{
		throw std::runtime_error("Mesh does not have positions. Cannot create valid mesh!");
	}

	if (aiMesh->HasTextureCoords(0))
	{
		mesh.m_UVs = std::vector<Vec2>();
		mesh.m_UVs->reserve(aiMesh->mNumVertices);
		for (uint32_t i = 0; i < aiMesh->mNumVertices; ++i)
		{
			mesh.m_UVs->emplace_back(
				aiMesh->mTextureCoords[0][i].x,
				aiMesh->mTextureCoords[0][i].y
			);
		}
	}

	if (aiMesh->HasNormals())
	{
		mesh.m_Normals = std::vector<Vec3>();
		mesh.m_Normals->reserve(aiMesh->mNumVertices);
		for (uint32_t i = 0; i < aiMesh->mNumVertices; ++i)
		{
			Vec3 normal(
				aiMesh->mNormals[i].x,
				aiMesh->mNormals[i].y,
				aiMesh->mNormals[i].z
			);

			mesh.m_Normals->emplace_back(
				normal
			);
		}
	}

	if (aiMesh->HasTangentsAndBitangents())
	{
		mesh.m_Bitangents = std::vector<Vec3>();
		mesh.m_Tangents = std::vector<Vec3>();
		mesh.m_Bitangents->reserve(aiMesh->mNumVertices);
		mesh.m_Tangents->reserve(aiMesh->mNumVertices);

		for (uint32_t i = 0; i < aiMesh->mNumVertices; ++i)
		{
			Vec3 tangent(
				aiMesh->mTangents[i].x,
				aiMesh->mTangents[i].y,
				aiMesh->mTangents[i].z
			);

			Vec3 bitangent(
				aiMesh->mBitangents[i].x,
				aiMesh->mBitangents[i].y,
				aiMesh->mBitangents[i].z
			);

			mesh.m_Tangents->emplace_back(tangent);
			mesh.m_Bitangents->emplace_back(bitangent);
		}
	}

	if (aiMesh->HasVertexColors(0))
	{
		mesh.m_Colors = std::vector<Rgba8>();
		mesh.m_Colors->reserve(aiMesh->mNumVertices);

		for (uint32_t i = 0; i < aiMesh->mNumVertices; ++i)
		{

			Rgba8 linearColor = Rgba8((unsigned char)aiMesh->mColors[0][i].r, (unsigned char)aiMesh->mColors[0][i].g, (unsigned char)aiMesh->mColors[0][i].b, (unsigned char)aiMesh->mColors[0][i].a);
			Rgba8 srgbColor = linearColor.LinearToSRGB();
			mesh.m_Colors->emplace_back(
				srgbColor
			);
		}
	}
	else
	{
		mesh.m_Colors = std::vector<Rgba8>(aiMesh->mNumVertices, Rgba8::WHITE);
	}

	if (aiMesh->HasFaces())
	{
		mesh.m_Indices.reserve(aiMesh->mNumFaces * 3);
		for (uint32_t i = 0; i < aiMesh->mNumFaces; ++i)
		{
			aiFace face = aiMesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; ++j)
			{
				mesh.m_Indices.push_back(face.mIndices[j]);
			}
		}
	}

	if (aiMesh->HasBones())
	{
		mesh.m_BoneWeights = std::vector<BoneWeights>(aiMesh->mNumVertices, { 0.f, 0.f, 0.f, 0.f });
		mesh.m_BoneIndices = std::vector<BoneIndices>(aiMesh->mNumVertices, { -1, -1, -1, -1 });

		for (uint32_t i = 0; i < aiMesh->mNumBones; ++i)
		{
			aiBone* bone = aiMesh->mBones[i];
			std::string boneName = bone->mName.C_Str();
			for (uint32_t j = 0; j < bone->mNumWeights; ++j)
			{
				aiVertexWeight weight = bone->mWeights[j];
				int vertexId = weight.mVertexId;
				// Find the first available slot (up to 4) for the bone weight.
				for (int k = 0; k < 4; ++k)
				{
					if (vertexId >= mesh.m_BoneIndices->size() || weight.mWeight == 0.f)
					{
						continue;
					}
					if (mesh.m_BoneIndices->at(vertexId).m_Indices[k] == -1)
					{
						// Assign the bone index and weight to the first empty slot.
						mesh.m_BoneIndices->at(vertexId).m_Indices[k] = scene->m_Bones[boneName].m_Index;
						mesh.m_BoneWeights->at(vertexId).m_Weights[k] = weight.mWeight;
						goto l_ExitWeighting; // Exit after assigning to avoid exceeding 4 bones per vertex.
					}
				}
				l_ExitWeighting:;
			}
		}
	}

	GPUMesh* gpuMesh = scene->m_CreateInfo.m_Renderer->CreateGPUMesh(mesh, scene->m_CreateInfo.m_Name);
	scene->m_Meshes.push_back(gpuMesh);
}

static void ProcessAnimation(SkeletalModel* scene, aiAnimation const* anim)
{
	Animation* animation = new Animation();
	animation->m_Name = anim->mName.C_Str();
	animation->m_Duration = (float)anim->mDuration;
	animation->m_TicksPerSecond = (float)anim->mTicksPerSecond;
	
	for (uint32_t i = 0; i < anim->mNumMorphMeshChannels; ++i)
	{
		aiMeshMorphAnim* morphAnim = anim->mMorphMeshChannels[i];
		MeshMorphTarget morphTarget;
		morphTarget.m_Name = morphAnim->mName.C_Str();
		for (uint32_t j = 0; j < morphAnim->mNumKeys; ++j)
		{
			aiMeshMorphKey key = morphAnim->mKeys[j];
			MeshMorphKey morphKey;
			
			for (uint32_t k = 0; k < key.mNumValuesAndWeights; ++k)
			{
				morphKey.m_Weights.push_back((float)key.mWeights[k]);
				morphKey.m_Values.push_back((int)key.mValues[k]);
			}
			morphTarget.m_Keys.push_back(morphKey);
		}

		animation->m_MorphTargets.push_back(morphTarget);
	}
	for (uint32_t i = 0; i < anim->mNumChannels; ++i)
	{
		aiNodeAnim* nodeAnim = anim->mChannels[i];
		AnimationChannel channel;
		channel.m_NodeName = nodeAnim->mNodeName.C_Str();
		
		for (uint32_t j = 0; j < nodeAnim->mNumPositionKeys; ++j)
		{
			aiVectorKey key = nodeAnim->mPositionKeys[j];
			Vec3 position = Vec3(
				key.mValue.x,
				key.mValue.y,
				key.mValue.z  
			);

			std::pair<float, Vec3> pair((float)key.mTime, position);

			channel.m_Translations.m_Keys.insert(pair);
		}

		for (uint32_t j = 0; j < nodeAnim->mNumRotationKeys; ++j)
		{
			aiQuatKey key = nodeAnim->mRotationKeys[j];

			// Invert the rotation to match the handedness of the engine. Assimp uses a different handedness.
			Rotor3D rotation(-key.mValue.x, -key.mValue.y, -key.mValue.z, key.mValue.w);

			std::pair<float, Rotor3D> pair((float)key.mTime, rotation);
			channel.m_Rotations.m_Keys.insert(pair);
		}

		for (uint32_t j = 0; j < nodeAnim->mNumScalingKeys; ++j)
		{
			aiVectorKey key = nodeAnim->mScalingKeys[j];

			Vec3 scale = Vec3(
				key.mValue.x,
				key.mValue.y,
				key.mValue.z
			);

			std::pair<float, Vec3> pair((float)key.mTime, scale);
			channel.m_Scales.m_Keys.insert(pair);
		}

		animation->m_Channels[nodeAnim->mNodeName.C_Str()] = channel;
	}
	scene->m_Animations[animation->m_Name] = animation;
}

static void ProcessMaterial(SkeletalModel* targetScene, std::filesystem::path path, aiScene const* scene, aiMaterial const* material)
{
	//#TODO Make materials unique. Right now each mesh recreates a new material.
	aiString texturePath;

	aiColor3D color(0.f, 0.f, 0.f);
	material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
	Rgba8 diffuseColor(
		(uint8_t)(color.r * 255),
		(uint8_t)(color.g * 255),
		(uint8_t)(color.b * 255),
		255
	);
	diffuseColor = diffuseColor.LinearToSRGB();

	color = aiColor3D(0.f, 0.f, 0.f);
	material->Get(AI_MATKEY_COLOR_SPECULAR, color);
	Rgba8 specularColor(
		(uint8_t)(color.r * 255),
		(uint8_t)(color.g * 255),
		(uint8_t)(color.b * 255),
		255
	);
	specularColor = specularColor.LinearToSRGB();

	color = aiColor3D(0.f, 0.f, 0.f);
	material->Get(AI_MATKEY_COLOR_AMBIENT, color);
	Rgba8 ambientColor(
		(uint8_t)(color.r * 255),
		(uint8_t)(color.g * 255),
		(uint8_t)(color.b * 255),
		255
	);
	ambientColor = ambientColor.LinearToSRGB();

	float shininess = 0.f;
	material->Get(AI_MATKEY_SHININESS, shininess);

	Texture diffuseTexture = targetScene->m_CreateInfo.m_Renderer->GetWhiteTexture();
	Texture specularTexture = targetScene->m_CreateInfo.m_Renderer->GetWhiteTexture();
	Texture normalTexture = targetScene->m_CreateInfo.m_Renderer->GetWhiteTexture();
	Texture parallaxTexture = targetScene->m_CreateInfo.m_Renderer->GetWhiteTexture();
	auto loadTexture = [&](aiTextureType type, Texture& texture)
		{
			if (material->GetTexture(type, 0, &texturePath) == AI_SUCCESS)
			{
				std::filesystem::path textureFilePath = path;
				std::filesystem::path texturePathFileOnly = texturePath.C_Str();
				texturePathFileOnly = texturePathFileOnly.filename();
				textureFilePath.remove_filename();
				textureFilePath /= "textures/";
				textureFilePath /= texturePathFileOnly;

				if (texturePath.C_Str()[0] == '*')
				{
					aiString embeddedTexturePath;
					material->GetTexture(type, 0, &embeddedTexturePath);
					int textureIndex = atoi(embeddedTexturePath.C_Str());
					aiTexture* embeddedTexture = scene->mTextures[textureIndex];
					aiTexel* texels = embeddedTexture->pcData;
					int extentWidth, extentHeight;

					stbi_uc* image = stbi_load_from_memory((stbi_uc const*)texels, embeddedTexture->mWidth, &extentWidth, &extentHeight, nullptr, STBI_rgb_alpha);
					if (!image)
					{
						throw std::runtime_error("Failed to load embedded texture from memory");
					}
					VkExtent2D extent = { (uint32_t)extentWidth, (uint32_t)extentHeight };

					std::string name = path.string() + std::string(" Diffuse Texture");
					texture = targetScene->m_CreateInfo.m_Renderer->LoadFromMemoryTexture2D(name, extent, (char const*)image, SamplerType::NEAREST_CLAMP);
				}
				else
				{
					texture = targetScene->m_CreateInfo.m_Renderer->LoadOrGetTexture2D(textureFilePath, SamplerType::NEAREST_CLAMP);
				}
			}
		};

	loadTexture(aiTextureType_DIFFUSE, diffuseTexture);
	loadTexture(aiTextureType_SPECULAR, specularTexture);
	loadTexture(aiTextureType_NORMALS, normalTexture);
	loadTexture(aiTextureType_HEIGHT, parallaxTexture);

	MaterialCreateInfo materialCreateInfo{
		targetScene->m_CreateInfo.m_Renderer->m_Device,
		targetScene->m_CreateInfo.m_Renderer->m_Allocator,
		diffuseTexture,
		specularTexture,
		normalTexture,
		parallaxTexture,
		MaterialConstants{
			diffuseColor.ToVec4(),
			specularColor.ToVec4(),
			ambientColor.ToVec4(),
			shininess
		}
	};

	targetScene->m_Materials.push_back(targetScene->m_CreateInfo.m_Renderer->CreateMaterial(materialCreateInfo));
}

static PBE::Node* ProcessNode(SkeletalModel* scene, aiNode const* node)
{
	Node* newNode = new Node();
	newNode->m_Name = node->mName.C_Str();

	// Process local transformation
	Mat4 localTransform = ProcessMatrix(node->mTransformation);
	newNode->m_LocalTransform = localTransform;

	// Initialize Bind and Inverse Bind Matrices (these might be updated for bone nodes)
	newNode->m_InverseBindMatrix = Mat4::IDENTITY;

	// Add meshes and materials
	for (uint32_t i = 0; i < node->mNumMeshes; ++i)
	{
		newNode->m_Meshes.push_back(scene->m_Meshes[node->mMeshes[i]]);
		newNode->m_Materials.push_back(scene->m_Materials[node->mMeshes[i]]);
	}

	// Recursive call for child nodes
	for (uint32_t i = 0; i < node->mNumChildren; ++i)
	{
		Node* child = ProcessNode(scene, node->mChildren[i]);
		newNode->AddChild(child);
	}

	// Set the inverse bind matrices if this node is a bone
	auto boneIter = scene->m_Bones.find(newNode->m_Name);
	if (boneIter != scene->m_Bones.end())
	{
		// If the node represents a bone, set its bind and inverse bind matrices
		newNode->m_InverseBindMatrix = boneIter->second.m_InverseBindMatrix;
	}

	return newNode;
}


PBE::SkeletalModel::SkeletalModel(SkeletalModelCreateInfo const& createInfo)
	: m_CreateInfo( createInfo )
	, m_RootNode( nullptr )
{
}

PBE::SkeletalModel::~SkeletalModel()
{
	for (auto& mesh : m_Meshes)
	{
		m_CreateInfo.m_Renderer->FreeGPUMesh(mesh);
	}
	for (auto& material : m_Materials)
	{
		delete material;
	}
	for (auto& animation : m_Animations)
	{
		delete animation.second;
	}

	m_Meshes.clear();
	m_Materials.clear();
	m_Animations.clear();
}

void PBE::SkeletalModel::LoadFromFile(std::filesystem::path path)
{
	Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	aiScene const* scene = importer.ReadFile(
		path.string(),
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace
	);

	if (scene == nullptr)
	{
		return;
	}

	// Load Animations
	for (uint32_t i = 0; i < scene->mNumAnimations; ++i)
	{
		aiAnimation* aiAnimation = scene->mAnimations[i];
		ProcessAnimation(this, aiAnimation);
	}

	// Load Meshes
	for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh const* aiMesh = scene->mMeshes[i];
		ProcessMesh(this,aiMesh);
	}

	// Load Materials
	for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
	{
		size_t materialIndex = (size_t)scene->mMeshes[i]->mMaterialIndex;
		aiMaterial* aiMaterial = scene->mMaterials[materialIndex];
		ProcessMaterial(this,path, scene, aiMaterial);
	}

	// Load Nodes
	m_RootNode = ProcessNode(this,scene->mRootNode);
	m_RootNode->SetLocalTransform(m_RootNode->GetLocalTransform());
	m_RootNode->UpdateLocalTransform();
	m_RootNode->UpdateWorldTransform();
}

void PBE::SkeletalModel::CmdDraw(Camera* camera, Mat4 const& transform, VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex)
{
	if (m_RootNode)
	{
		m_RootNode->CmdDraw(camera, transform, buffer, layout, frameIndex);
	}
}

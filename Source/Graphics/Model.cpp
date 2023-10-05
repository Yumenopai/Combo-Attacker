#include <filesystem>
#include <fstream>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include "Misc.h"
#include "AssimpImporter.h"
#include "Model.h"
#include "GpuResourceUtils.h"

//�R���X�g���N�^
Model::Model(ID3D11Device* device, const char* filename, float scaling) : scaling(scaling)
{
	std::filesystem::path filepath(filename);
	std::filesystem::path dirpath(filepath.parent_path());

	//�Ǝ��`���̃��f���t�@�C���̑��݊m�F
	filepath.replace_extension(".cereal");
	if (std::filesystem::exists(filepath))
	{
		//�Ǝ��`���̃��f���t�@�C���̓ǂݍ���
		Deserialize(filepath.string().c_str());
	}
	else
	{
		//�ėp���f���t�@�C���ǂݍ���
		AssimpImporter importer(filename);

		//�}�e���A���f�[�^�ǂݎ��
		importer.LoadMaterials(materials);

		//�m�[�h�f�[�^�ǂݎ��
		importer.LoadNodes(nodes);

		//���b�V���f�[�^�ǂݎ��
		importer.LoadMeshes(meshes, nodes);

		//�A�j���[�V�����f�[�^�ǂݎ��
		importer.LoadAnimations(animations, nodes);

		//�Ǝ��`���̃��f���t�@�C����ۑ�
		Serialize(filepath.string().c_str());
	}

	//�m�[�h�\�z
	for (size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
	{
		Node& node = nodes.at(nodeIndex);

		//�e�q�֌W���\�z
		node.parent = node.parentIndex >= 0 ? &nodes.at(node.parentIndex) : nullptr;
		if (node.parent != nullptr)
		{
			node.parent->children.emplace_back(&node);
		}
	}

	//�}�e���A���\�z
	for (Material& material : materials)
	{
		if (material.diffuseTextureFileName.empty())
		{
			//�_�~�[�e�N�X�`���쐬
			HRESULT hr = GpuResourceUtils::CreateDummyTexture(device, 0xFFFFFFFF, material.diffuseMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		else
		{
			//�f�B�t���[�Y�e�N�X�`���ǂݍ���
			std::filesystem::path diffuseTexturePath(dirpath / material.diffuseTextureFileName);
			HRESULT hr = GpuResourceUtils::LoadTexture(device, diffuseTexturePath.string().c_str(), material.diffuseMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}

		if (material.normalTextureFileName.empty())
		{
			//�@���_�~�[�e�N�X�`���쐬
			HRESULT hr = GpuResourceUtils::CreateDummyTexture(device, 0xFFFF7F7F, material.normalMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		else
		{
			//�@���e�N�X�`���ǂݍ���
			std::filesystem::path texturePath(dirpath / material.normalTextureFileName);
			HRESULT hr = GpuResourceUtils::LoadTexture(device, texturePath.string().c_str(), material.normalMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
	}

	//���b�V���\�z
	for (Mesh& mesh : meshes)
	{
		//�Q�ƃm�[�h�ݒ�
		mesh.node = &nodes.at(mesh.nodeIndex);

		//�Q�ƃ}�e���A���ݒ�
		mesh.material = &materials.at(mesh.materialIndex);

		//���_�o�b�t�@
		{
			D3D11_BUFFER_DESC bufferDesc = {};
			D3D11_SUBRESOURCE_DATA subresourceData = {};

			bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * mesh.vertices.size());
			bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;
			subresourceData.pSysMem = mesh.vertices.data();
			subresourceData.SysMemPitch = 0;
			subresourceData.SysMemSlicePitch = 0;

			HRESULT hr = device->CreateBuffer(&bufferDesc, &subresourceData, mesh.vertexBuffer.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}

		//�C���f�b�N�X�o�b�t�@
		{
			D3D11_BUFFER_DESC bufferDesc = {};
			D3D11_SUBRESOURCE_DATA subresourceData = {};

			bufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * mesh.indices.size());
			bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;
			subresourceData.pSysMem = mesh.indices.data();
			subresourceData.SysMemPitch = 0;
			subresourceData.SysMemSlicePitch = 0;

			HRESULT hr = device->CreateBuffer(&bufferDesc, &subresourceData, mesh.indexBuffer.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}

		//�{�[���\�z
		for (Bone& bone : mesh.bones)
		{
			//�Q�ƃm�[�h�ݒ�
			bone.node = &nodes.at(bone.nodeIndex);
		}
	}
	//�m�[�h�L���b�V��
	nodeCaches.resize(nodes.size());
}

//�g�����X�t�H�[���X�V����
void Model::UpdateTransform(const XMFLOAT4X4& worldTransform)
{
	XMMATRIX ParentWorldTransform = XMLoadFloat4x4(&worldTransform);

	//�E����W�n���獶����W�n�֕ϊ�����s��
	XMMATRIX CoordinateSystemTransform = XMMatrixScaling(-scaling, scaling, scaling);

	for (Node& node : nodes)
	{
		//���[�J���s��Z�o
		XMMATRIX S = XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z);
		XMMATRIX R = XMMatrixRotationQuaternion(XMLoadFloat4(&node.rotation));
		XMMATRIX T = XMMatrixTranslation(node.position.x, node.position.y, node.position.z);
		XMMATRIX LocalTransform = S * R * T;
	
		//�O���[�o���s��Z�o
		XMMATRIX ParentGlobalTransform;
		if (node.parent != nullptr)
		{
			ParentGlobalTransform = XMLoadFloat4x4(&node.parent->globalTransform);
		}
		else
		{
			ParentGlobalTransform = XMMatrixIdentity();
		}
		XMMATRIX GlobalTransform = LocalTransform * ParentGlobalTransform;

		//���[���h�s��Z�o
		XMMATRIX WorldTransform = GlobalTransform * CoordinateSystemTransform * ParentWorldTransform;

		//�v�Z���ʂ��i�[
		XMStoreFloat4x4(&node.localTransform, LocalTransform);
		XMStoreFloat4x4(&node.globalTransform, GlobalTransform);
		XMStoreFloat4x4(&node.worldTransform, WorldTransform);
	}
}

//�A�j���[�V�����Đ�
void Model::PlayAnimation(int index, bool loop, float blendSeconds)
{
	currentAnimationIndex = index;
	currentAnimationSeconds = 0;
	animationLoop = loop;
	animationPlaying = true;

	//�u�����h�p�����[�^
	animationBlending = blendSeconds > 0.0f;
	currentAnimationBlendSeconds = 0.0f;
	animationBlendSecondslength = blendSeconds;

	//���݂̎p�����L���b�V������
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		const Node& src = nodes.at(i);
		NodeCache& dst = nodeCaches.at(i);

		dst.position = src.position;
		dst.rotation = src.rotation;
		dst.scale = src.scale;
	}
}

//�A�j���[�V�����Đ�����
bool Model::IsPlayAnimation() const
{
	if (currentAnimationIndex < 0) return false;
	if (currentAnimationIndex >= animations.size()) return false;
	return animationPlaying;
}

//�A�j���[�V�����X�V����
void Model::UpdateAnimation(float elapsedTime)
{
	ComputeAnimation(elapsedTime);

	ComputeBlending(elapsedTime);
}

//�A�j���[�V�����v�Z����
void Model::ComputeAnimation(float elapsedTime)
{
	if (!IsPlayAnimation()) return;

	//�w��̃A�j���[�V�����f�[�^���擾
	const Animation& animation = animations.at(currentAnimationIndex);
	
	//�m�[�h���̃A�j���[�V��������
	for (size_t nodeIndex = 0; nodeIndex < animation.nodeAnims.size(); ++nodeIndex)
	{
		Node& node = nodes.at(nodeIndex);
		const NodeAnim& nodeAnim = animation.nodeAnims.at(nodeIndex);

		//�ʒu
		for (size_t index = 0; index < nodeAnim.positionKeyframes.size() - 1; ++index)
		{
			//���݂̎��Ԃ��ǂ̃L�[�t���[���̊Ԃɂ��邩���肷��
			const VectorKeyframe& keyframe0 = nodeAnim.positionKeyframes.at(index);
			const VectorKeyframe& keyframe1 = nodeAnim.positionKeyframes.at(index + 1);
			if (currentAnimationSeconds >= keyframe0.seconds && currentAnimationSeconds < keyframe1.seconds)
			{
				//�Đ����ԂƃL�[�t���[���̎��Ԃ���⊮�����Z�o����
				float rate = (currentAnimationSeconds - keyframe0.seconds) / (keyframe1.seconds - keyframe0.seconds);
				//�O�̃L�[�t���[���Ǝ��̃L�[�t���[���̎p����⊮
				XMVECTOR V0 = XMLoadFloat3(&keyframe0.value);
				XMVECTOR V1 = XMLoadFloat3(&keyframe1.value);
				XMVECTOR V = XMVectorLerp(V0, V1, rate);
				//�v�Z���ʂ��m�[�h�Ɋi�[
				DirectX::XMStoreFloat3(&node.position, V);
			}
		}
		//��]
		for (size_t index = 0; index < nodeAnim.rotationKeyframes.size() - 1; ++index)
		{
			//���݂̎��Ԃ��ǂ̃L�[�t���[���̊Ԃɂ��邩���肷��
			const QuaternionKeyframe& keyframe0 = nodeAnim.rotationKeyframes.at(index);
			const QuaternionKeyframe& keyframe1 = nodeAnim.rotationKeyframes.at(index + 1);
			if (currentAnimationSeconds >= keyframe0.seconds && currentAnimationSeconds < keyframe1.seconds)
			{
				//�Đ����ԂƃL�[�t���[���̎��Ԃ���⊮�����Z�o����
				float rate = (currentAnimationSeconds - keyframe0.seconds) / (keyframe1.seconds - keyframe0.seconds);
				//�O�̃L�[�t���[���Ǝ��̃L�[�t���[���̎p����⊮
				XMVECTOR Q0 = XMLoadFloat4(&keyframe0.value);
				XMVECTOR Q1 = XMLoadFloat4(&keyframe1.value);
				XMVECTOR Q = XMQuaternionSlerp(Q0, Q1, rate);
				//�v�Z���ʂ��m�[�h�Ɋi�[
				DirectX::XMStoreFloat4(&node.rotation, Q);
			}
		}
		//�X�P�[��
		for (size_t index = 0; index < nodeAnim.scaleKeyframes.size() - 1; ++index)
		{
			//���݂̎��Ԃ��ǂ̃L�[�t���[���̊Ԃɂ��邩���肷��
			const VectorKeyframe& keyframe0 = nodeAnim.scaleKeyframes.at(index);
			const VectorKeyframe& keyframe1 = nodeAnim.scaleKeyframes.at(index + 1);
			if (currentAnimationSeconds >= keyframe0.seconds && currentAnimationSeconds < keyframe1.seconds)
			{
				//�Đ����ԂƃL�[�t���[���̎��Ԃ���⊮�����Z�o����
				float rate = (currentAnimationSeconds - keyframe0.seconds) / (keyframe1.seconds - keyframe0.seconds);
				//�O�̃L�[�t���[���Ǝ��̃L�[�t���[���̎p����⊮
				XMVECTOR V0 = XMLoadFloat3(&keyframe0.value);
				XMVECTOR V1 = XMLoadFloat3(&keyframe1.value);
				XMVECTOR V = XMVectorLerp(V0, V1, rate);
				//�v�Z���ʂ��m�[�h�Ɋi�[
				DirectX::XMStoreFloat3(&node.scale, V);
			}
		}
	}

	//���Ԍo��
	currentAnimationSeconds += elapsedTime;

	//�Đ����Ԃ��I�[���Ԃ𒴂�����
	if (currentAnimationSeconds >= animation.secondsLength)
	{
		if (animationLoop)
		{
			//�Đ����Ԃ������߂�
			currentAnimationSeconds -= animation.secondsLength;
		}
		else
		{
			//�Đ��I�����Ԃɂ���
			currentAnimationSeconds = animation.secondsLength;
			animationPlaying = false;
		}
	}
}

//�u�����f�B���O�v�Z����
void Model::ComputeBlending(float elapsedTime)
{
	if (!animationBlending) return;

	//�u�����h���̌v�Z
	float rate = currentAnimationBlendSeconds / animationBlendSecondslength;

	//�u�����h�v�Z
	int count = static_cast<int>(nodes.size());
	for (int i = 0; i < count; ++i)
	{
		const NodeCache& cache = nodeCaches.at(i);
		Node& node = nodes.at(i);

		XMVECTOR S0 = XMLoadFloat3(&cache.scale);
		XMVECTOR S1 = XMLoadFloat3(&node.scale);
		XMVECTOR R0 = XMLoadFloat4(&cache.rotation);
		XMVECTOR R1 = XMLoadFloat4(&node.rotation);
		XMVECTOR T0 = XMLoadFloat3(&cache.position);
		XMVECTOR T1 = XMLoadFloat3(&node.position);

		XMVECTOR S = XMVectorLerp(S0, S1, rate);
		XMVECTOR R = XMQuaternionSlerp(R0, R1, rate);
		XMVECTOR T = XMVectorLerp(T0, T1, rate);

		DirectX::XMStoreFloat3(&node.scale, S);
		DirectX::XMStoreFloat4(&node.rotation, R);
		DirectX::XMStoreFloat3(&node.position, T);
	}

	//���Ԍo��
	currentAnimationBlendSeconds += elapsedTime;
	if (currentAnimationBlendSeconds >= animationBlendSecondslength)
	{
		currentAnimationBlendSeconds = animationBlendSecondslength;
		animationBlending = false;
	}
}

// �m�[�h����
Model::Node* Model::FindNode(const char* name)
{
	//�S�Ẵm�[�h�𑍓�����Ŗ��O��r����
	for (Node& node : nodes)
	{
		if (node.name == name) return &node;
	}

	//������Ȃ�����
	return nullptr;
}

namespace DirectX
{
	template<class Archive>
	void serialize(Archive& archive, XMUINT4& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z),
			cereal::make_nvp("w", v.w)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMFLOAT2& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMFLOAT3& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z)
			);
	}

	template<class Archive>
	void serialize(Archive& archive, XMFLOAT4& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z),
			cereal::make_nvp("w", v.w)
		);
	}
	
	template<class Archive>
	void serialize(Archive& archive, XMFLOAT4X4& m)
	{
		archive(
			cereal::make_nvp("_11", m._11),
			cereal::make_nvp("_12", m._12),
			cereal::make_nvp("_13", m._13),
			cereal::make_nvp("_14", m._14),
			cereal::make_nvp("_21", m._21),
			cereal::make_nvp("_22", m._22),
			cereal::make_nvp("_23", m._23),
			cereal::make_nvp("_24", m._24),
			cereal::make_nvp("_31", m._31),
			cereal::make_nvp("_32", m._32),
			cereal::make_nvp("_33", m._33),
			cereal::make_nvp("_34", m._34),
			cereal::make_nvp("_41", m._41),
			cereal::make_nvp("_42", m._42),
			cereal::make_nvp("_43", m._43),
			cereal::make_nvp("_44", m._44)
		);
	}
}

template<class Archive>
void Model::Node::serialize(Archive& archive)
{
	archive(
		CEREAL_NVP(name),
		CEREAL_NVP(path),
		CEREAL_NVP(parentIndex),
		CEREAL_NVP(position),
		CEREAL_NVP(rotation),
		CEREAL_NVP(scale)
	);
}

template<class Archive>
void Model::Material::serialize(Archive& archive)
{
	archive(
		CEREAL_NVP(name),
		CEREAL_NVP(diffuseTextureFileName),
		CEREAL_NVP(normalTextureFileName),
		CEREAL_NVP(color)
	);
}

template<class Archive>
void Model::Vertex::serialize(Archive& archive)
{
	archive(
		CEREAL_NVP(position),
		CEREAL_NVP(boneWeight),
		CEREAL_NVP(boneIndex),
		CEREAL_NVP(texcoord),
		CEREAL_NVP(normal),
		CEREAL_NVP(tangent)
	);
}

template<class Archive>
void Model::Bone::serialize(Archive& archive)
{
	archive(
		CEREAL_NVP(nodeIndex),
		CEREAL_NVP(offsetTransform)
	);
}

template<class Archive>
void Model::Mesh::serialize(Archive& archive)
{
	archive(
		CEREAL_NVP(vertices),
		CEREAL_NVP(indices),
		CEREAL_NVP(bones),
		CEREAL_NVP(nodeIndex),
		CEREAL_NVP(materialIndex)
	);
}

template<class Archive>
void Model::VectorKeyframe::serialize(Archive& archive)
{
	archive(
		CEREAL_NVP(seconds),
		CEREAL_NVP(value)
	);
}

template<class Archive>
void Model::QuaternionKeyframe::serialize(Archive& archive)
{
	archive(
		CEREAL_NVP(seconds),
		CEREAL_NVP(value)
	);
}

template<class Archive>
void Model::NodeAnim::serialize(Archive& archive)
{
	archive(
		CEREAL_NVP(positionKeyframes),
		CEREAL_NVP(rotationKeyframes),
		CEREAL_NVP(scaleKeyframes)
	);
}

template<class Archive>
void Model::Animation::serialize(Archive& archive)
{
	archive(
		CEREAL_NVP(name),
		CEREAL_NVP(secondsLength),
		CEREAL_NVP(nodeAnims)
	);
}

//�V���A���C�Y
void Model::Serialize(const char* filename)
{
	std::ofstream ostream(filename, std::ios::binary);
	if (ostream.is_open())
	{
		cereal::BinaryOutputArchive archive(ostream);
		try
		{
			archive(
				CEREAL_NVP(nodes),
				CEREAL_NVP(materials),
				CEREAL_NVP(meshes),
				CEREAL_NVP(animations)
			);
		}
		catch (...)
		{
			_ASSERT_EXPR_A(false, "Model serialize failed.");
		}
	}
}

//�f�V���A���C�Y
void Model::Deserialize(const char* filename)
{
	std::ifstream istream(filename, std::ios::binary);
	if (istream.is_open())
	{
		cereal::BinaryInputArchive archive(istream);
		try
		{
			archive(
				CEREAL_NVP(nodes),
				CEREAL_NVP(materials),
				CEREAL_NVP(meshes),
				CEREAL_NVP(animations)
			);
		}
		catch (...)
		{
			_ASSERT_EXPR_A(false, "Model deserialize failed.");
		}
	}
	else
	{
		_ASSERT_EXPR_A(false, "Model File not found.");
	}
}
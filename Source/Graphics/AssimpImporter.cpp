#include <fstream>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include "Misc.h"
#include "AssimpImporter.h"

// �R���X�g���N�^
AssimpImporter::AssimpImporter(const char* filename)
	: filepath(filename) 
{
	// �g���q�擾
	std::string extension = filepath.extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(), tolower);	// ��������

	// FBX�t�@�C���̏ꍇ�͓���ȃC���|�[�g�I�v�V�����ݒ������
	if (extension == ".fbx")
	{
		// $AssimpFBX$���t�����ꂽ�]�v�ȃm�[�h���쐬���Ă��܂��̂�}������
		aImporter.SetPropertyInteger(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	}

	// �C���|�[�g���̃I�v�V�����t���O
	uint32_t aFlags = aiProcess_Triangulate				// ���p�`���O�p�`������
					| aiProcess_JoinIdenticalVertices	// �d�����_���}�[�W����
					| aiProcess_PopulateArmatureData	// �{�[���̎Q�ƃf�[�^���擾�ł���悤�ɂ���
					| aiProcess_CalcTangentSpace;		// �ڐ����v�Z����

	// �t�@�C���ǂݍ���
	aScene = aImporter.ReadFile(filename, aFlags);
	_ASSERT_EXPR_A(aScene, aImporter.GetErrorString());
}

// �m�[�h�f�[�^��ǂݍ���
void AssimpImporter::LoadNodes(NodeList& nodes)
{
	LoadNodes(nodes, aScene->mRootNode, -1);
}

// ���b�V���f�[�^��ǂݍ���
void AssimpImporter::LoadMeshes(MeshList& meshes, const NodeList& nodes)
{
	LoadMeshes(meshes, nodes, aScene->mRootNode);
}

// �}�e���A���f�[�^��ǂݍ���
void AssimpImporter::LoadMaterials(MaterialList& materials)
{
	// �f�B���N�g���p�X�擾
	std::filesystem::path dirpath(filepath.parent_path());

	materials.resize(aScene->mNumMaterials);
	for (uint32_t aMaterialIndex = 0; aMaterialIndex < aScene->mNumMaterials; ++aMaterialIndex)
	{
		const aiMaterial* aMaterial = aScene->mMaterials[aMaterialIndex];
		Model::Material& material = materials.at(aMaterialIndex);

		// �}�e���A����
		aiString aMaterialName;
		aMaterial->Get(AI_MATKEY_NAME, aMaterialName);
		material.name = aMaterialName.C_Str();

		// �f�B�t���[�Y�F
		aiColor3D aDiffuseColor;
		if (AI_SUCCESS == aMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aDiffuseColor))
		{
			material.color = aiColor3DToXMFLOAT4(aDiffuseColor);
		}

		// �e�N�X�`���ǂݍ��݊֐�
		auto loadTexture = [&](aiTextureType aTextureType, std::string& textureFilename)
		{
			// �e�N�X�`���t�@�C���p�X�擾
			aiString aTextureFilePath;
			if (AI_SUCCESS == aMaterial->GetTexture(aTextureType, 0, &aTextureFilePath))
			{
				// ���ߍ��݃e�N�X�`�����m�F
				const aiTexture* aTexture = aScene->GetEmbeddedTexture(aTextureFilePath.C_Str());
				if (aTexture != nullptr)
				{
					// �e�N�X�`���t�@�C���p�X�쐬
					std::filesystem::path textureFilePath(aTextureFilePath.C_Str());
					if (textureFilePath == "*0")
					{
						// �e�N�X�`���t�@�C�������Ȃ������ꍇ�̓}�e���A�����ƃe�N�X�`���^�C�v����쐬
						textureFilePath = material.name + "_" + aiTextureTypeToString(aTextureType) + "." + aTexture->achFormatHint;
					}
					textureFilePath = "Textures" / textureFilePath.filename();

					// ���ߍ��݃e�N�X�`�����o�͂���f�B���N�g�����m�F
					std::filesystem::path outputDirPath(dirpath / textureFilePath.parent_path());
					if (!std::filesystem::exists(outputDirPath))
					{
						// �Ȃ�������f�B���N�g���쐬
						std::filesystem::create_directories(outputDirPath);
					}
					// �o�̓f�B���N�g���ɉ摜�t�@�C����ۑ�
					std::filesystem::path outputFilePath(dirpath / textureFilePath);
					if (!std::filesystem::exists(outputFilePath))
					{
						// mHeight��0�̏ꍇ�͉摜�̐��f�[�^�Ȃ̂ł��̂܂܃o�C�i���o��
						if (aTexture->mHeight == 0)
						{
							std::ofstream os(outputFilePath.string().c_str(), std::ios::binary);
							os.write(reinterpret_cast<char*>(aTexture->pcData), aTexture->mWidth);
						}
						else
						{
							// ���j�A�ȉ摜�f�[�^��.png�ŏo��
							outputFilePath.replace_extension(".png");
							stbi_write_png(
								outputFilePath.string().c_str(),
								static_cast<int>(aTexture->mWidth),
								static_cast<int>(aTexture->mHeight),
								static_cast<int>(sizeof(uint32_t)),
								aTexture->pcData, 0);
						}
					}
					// �e�N�X�`���t�@�C���p�X���i�[
					textureFilename = textureFilePath.string();
				}
				else
				{
					// �e�N�X�`���t�@�C���p�X�����̂܂܊i�[
					textureFilename = aTextureFilePath.C_Str();
				}
			}
		};

		// �f�B�t���[�Y�}�b�v
		loadTexture(aiTextureType_DIFFUSE, material.diffuseTextureFileName);

		// �m�[�}���}�b�v
		loadTexture(aiTextureType_NORMALS, material.normalTextureFileName);
	}
}

// �m�[�h�f�[�^���ċA�ǂݍ���
void AssimpImporter::LoadNodes(NodeList& nodes, const aiNode* aNode, int parentIndex)
{
	// aiNode*����Model::Node�̃C���f�b�N�X���擾�ł���悤�ɂ���
	std::map<const aiNode*, int>::iterator it = nodeIndexMap.find(aNode);
	if (it == nodeIndexMap.end())
	{
		nodeIndexMap[aNode] = static_cast<int>(nodes.size());
	}

	// �g�����X�t�H�[���f�[�^���o��
	aiVector3D aScale, aPosition;
	aiQuaternion aRotation;
	aNode->mTransformation.Decompose(aScale, aRotation, aPosition);

	// �m�[�h�f�[�^�i�[
	Model::Node& node = nodes.emplace_back();
	node.name = aNode->mName.C_Str();
	node.parentIndex = parentIndex;
	node.scale = aiVector3DToXMFLOAT3(aScale);
	node.rotation = aiQuaternionToXMFLOAT4(aRotation);
	node.position = aiVector3DToXMFLOAT3(aPosition);

	parentIndex = static_cast<int>(nodes.size() - 1);

	// �ċA�I�Ɏq�m�[�h����������
	for (uint32_t aNodeIndex = 0; aNodeIndex < aNode->mNumChildren; ++aNodeIndex)
	{
		LoadNodes(nodes, aNode->mChildren[aNodeIndex], parentIndex);
	}
}

// ���b�V���f�[�^��ǂݍ���
void AssimpImporter::LoadMeshes(MeshList& meshes, const NodeList& nodes, const aiNode* aNode)
{
	// ���b�V���f�[�^�ǂݎ��
	for (uint32_t aMeshIndex = 0; aMeshIndex < aNode->mNumMeshes; ++aMeshIndex)
	{
		const aiMesh* aMesh = aScene->mMeshes[aNode->mMeshes[aMeshIndex]];

		// ���b�V���f�[�^�i�[
		Model::Mesh& mesh = meshes.emplace_back();
		mesh.vertices.resize(aMesh->mNumVertices);
		mesh.indices.resize(aMesh->mNumFaces * 3);
		mesh.materialIndex = static_cast<int>(aMesh->mMaterialIndex);
		mesh.nodeIndex = nodeIndexMap[aNode];

		// ���_�f�[�^
		for (uint32_t aVertexIndex = 0; aVertexIndex < aMesh->mNumVertices; ++aVertexIndex)
		{
			Model::Vertex& vertex = mesh.vertices.at(aVertexIndex);
			// �ʒu
			if (aMesh->HasPositions())
			{
				vertex.position = aiVector3DToXMFLOAT3(aMesh->mVertices[aVertexIndex]);
			}
			// �e�N�X�`�����W
			if (aMesh->HasTextureCoords(0))
			{
				vertex.texcoord = aiVector3DToXMFLOAT2(aMesh->mTextureCoords[0][aVertexIndex]);
				vertex.texcoord.y = 1.0f - vertex.texcoord.y;
			}
			// �@��
			if (aMesh->HasNormals())
			{
				vertex.normal = aiVector3DToXMFLOAT3(aMesh->mNormals[aVertexIndex]);
			}
			// �ڐ�
			if (aMesh->HasTangentsAndBitangents())
			{
				vertex.tangent = aiVector3DToXMFLOAT3(aMesh->mTangents[aVertexIndex]);
			}
		}

		// �C���f�b�N�X�f�[�^
		for (uint32_t aFaceIndex = 0; aFaceIndex < aMesh->mNumFaces; ++aFaceIndex)
		{
			const aiFace& aFace = aMesh->mFaces[aFaceIndex];
			uint32_t index = aFaceIndex * 3;
			mesh.indices[index + 0] = aFace.mIndices[2];
			mesh.indices[index + 1] = aFace.mIndices[1];
			mesh.indices[index + 2] = aFace.mIndices[0];
		}

		// �X�L�j���O�f�[�^
		if (aMesh->mNumBones > 0)
		{
			// �{�[���e���̓f�[�^
			struct BoneInfluence
			{
				uint32_t	indices[4] = { 0, 0, 0, 0 };
				float		weights[4] = { 1, 0, 0, 0 };
				int			useCount = 0;

				void Add(uint32_t index, float weight)
				{
					if (useCount >= 4) return;
					for (int i = 0; i < useCount; ++i)
					{
						if (indices[i] == index)
						{
							return;
						}
					}
					indices[useCount] = index;
					weights[useCount] = weight;
					useCount++;
				}
			};
			std::vector<BoneInfluence> boneInfluences;
			boneInfluences.resize(aMesh->mNumVertices);

			// ���b�V���ɉe������{�[���f�[�^�����W����
			for (uint32_t aBoneIndex = 0; aBoneIndex < aMesh->mNumBones; ++aBoneIndex)
			{
				const aiBone* aBone = aMesh->mBones[aBoneIndex];

				// ���_�e���̓f�[�^�𒊏o
				for (uint32_t aWightIndex = 0; aWightIndex < aBone->mNumWeights; ++aWightIndex)
				{
					const aiVertexWeight& aWeight = aBone->mWeights[aWightIndex];
					BoneInfluence& boneInfluence = boneInfluences.at(aWeight.mVertexId);
					boneInfluence.Add(aBoneIndex, aWeight.mWeight);
				}

				// �{�[���f�[�^�擾
				Model::Bone& bone = mesh.bones.emplace_back();
				bone.nodeIndex = nodeIndexMap[aBone->mNode];
				bone.offsetTransform = aiMatrix4x4ToXMFLOAT4X4(aBone->mOffsetMatrix);

			}
			// ���_�e���̓f�[�^���i�[
			for (size_t vertexIndex = 0; vertexIndex < mesh.vertices.size(); ++vertexIndex)
			{
				Model::Vertex& vertex = mesh.vertices.at(vertexIndex);
				BoneInfluence& boneInfluence = boneInfluences.at(vertexIndex);
				vertex.boneWeight.x = boneInfluence.weights[0];
				vertex.boneWeight.y = boneInfluence.weights[1];
				vertex.boneWeight.z = boneInfluence.weights[2];
				vertex.boneWeight.w = boneInfluence.weights[3];
				vertex.boneIndex.x = boneInfluence.indices[0];
				vertex.boneIndex.y = boneInfluence.indices[1];
				vertex.boneIndex.z = boneInfluence.indices[2];
				vertex.boneIndex.w = boneInfluence.indices[3];
			}
		}

	}

	// �ċA�I�Ɏq�m�[�h����������
	for (uint32_t aNodeIndex = 0; aNodeIndex < aNode->mNumChildren; ++aNodeIndex)
	{
		LoadMeshes(meshes, nodes, aNode->mChildren[aNodeIndex]);
	}
}

// �m�[�h�C���f�b�N�X�擾
int AssimpImporter::GetNodeIndex(const NodeList& nodes, const char* name)
{
	int index = 0;
	for (const Model::Node& node : nodes)
	{
		if (node.name == name)
		{
			return index;
		}
		index++;
	}
	return -1;
}

// �A�j���[�V�����f�[�^��ǂݍ���
void AssimpImporter::LoadAnimations(AnimationList& animations, const NodeList& nodes)
{
	for (uint32_t aAnimationIndex = 0; aAnimationIndex < aScene->mNumAnimations; ++aAnimationIndex)
	{
		const aiAnimation* aAnimation = aScene->mAnimations[aAnimationIndex];
		Model::Animation& animation = animations.emplace_back();

		// �A�j���[�V�������
		animation.name = aAnimation->mName.C_Str();
		animation.secondsLength = static_cast<float>(aAnimation->mDuration / aAnimation->mTicksPerSecond);

		// �m�[�h���̃A�j���[�V����
		animation.nodeAnims.resize(nodes.size());
		for (uint32_t aChannelIndex = 0; aChannelIndex < aAnimation->mNumChannels; ++aChannelIndex)
		{
			const aiNodeAnim* aNodeAnim = aAnimation->mChannels[aChannelIndex];
			int nodeIndex = GetNodeIndex(nodes, aNodeAnim->mNodeName.C_Str());
			if (nodeIndex < 0) continue;

			const Model::Node& node = nodes.at(nodeIndex);
			Model::NodeAnim& nodeAnim = animation.nodeAnims.at(nodeIndex);

			// �ʒu
			for (uint32_t aPositionIndex = 0; aPositionIndex < aNodeAnim->mNumPositionKeys; ++aPositionIndex)
			{
				const aiVectorKey& aKey = aNodeAnim->mPositionKeys[aPositionIndex];
				// �Ȃ���Unity�ŏo�͂����A�j���[�V�����f�[�^�ɂ̓S�~�Ǝv����L�[�t���[�������݂��Ă���ꍇ������B
				// �����_�����݂���t���[���i���ԁj���S�~�f�[�^���ۂ��̂ŏ��O����B
				if (fabs(std::round(aKey.mTime) - aKey.mTime) > 0.001) continue;
				Model::VectorKeyframe& keyframe = nodeAnim.positionKeyframes.emplace_back();
				keyframe.seconds = static_cast<float>(aKey.mTime / aAnimation->mTicksPerSecond);
				keyframe.value = aiVector3DToXMFLOAT3(aKey.mValue);
			}
			// ��]
			for (uint32_t aRotationIndex = 0; aRotationIndex < aNodeAnim->mNumRotationKeys; ++aRotationIndex)
			{
				const aiQuatKey& aKey = aNodeAnim->mRotationKeys[aRotationIndex];
				// �Ȃ���Unity�ŏo�͂����A�j���[�V�����f�[�^�ɂ̓S�~�Ǝv����L�[�t���[�������݂��Ă���ꍇ������B
				// �����_�����݂���t���[���i���ԁj���S�~�f�[�^���ۂ��̂ŏ��O����B
				if (fabs(std::round(aKey.mTime) - aKey.mTime) > 0.001) continue;
				Model::QuaternionKeyframe& keyframe = nodeAnim.rotationKeyframes.emplace_back();
				keyframe.seconds = static_cast<float>(aKey.mTime / aAnimation->mTicksPerSecond);
				keyframe.value = aiQuaternionToXMFLOAT4(aKey.mValue);
			}
			// �X�P�[��
			for (uint32_t aScalingIndex = 0; aScalingIndex < aNodeAnim->mNumScalingKeys; ++aScalingIndex)
			{
				const aiVectorKey& aKey = aNodeAnim->mScalingKeys[aScalingIndex];
				// �Ȃ���Unity�ŏo�͂����A�j���[�V�����f�[�^�ɂ̓S�~�Ǝv����L�[�t���[�������݂��Ă���ꍇ������B
				// �����_�����݂���t���[���i���ԁj���S�~�f�[�^���ۂ��̂ŏ��O����B
				if (fabs(std::round(aKey.mTime) - aKey.mTime) > 0.001) continue;
				Model::VectorKeyframe& keyframe = nodeAnim.scaleKeyframes.emplace_back();
				keyframe.seconds = static_cast<float>(aKey.mTime / aAnimation->mTicksPerSecond);
				keyframe.value = aiVector3DToXMFLOAT3(aKey.mValue);
			}

			// �S�ẴL�[�̒l���قړ������e�������ꍇ�͍��
			DirectX::XMVECTOR Epsilon = DirectX::XMVectorReplicate(0.00001f);
			// �ʒu
			{
				bool result = true;
				DirectX::XMVECTOR A = DirectX::XMLoadFloat3(&nodeAnim.positionKeyframes.at(0).value);
				for (size_t i = 1; i < nodeAnim.positionKeyframes.size(); ++i)
				{
					DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&nodeAnim.positionKeyframes.at(i).value);
					if (!DirectX::XMVector3NearEqual(A, B, Epsilon))
					{
						result = false;
						break;
					}
				}
				if (result)
				{
					nodeAnim.positionKeyframes.resize(1);
				}
			}
			// ��]
			{
				bool result = true;
				DirectX::XMVECTOR A = DirectX::XMLoadFloat4(&nodeAnim.rotationKeyframes.at(0).value);
				for (size_t i = 1; i < nodeAnim.rotationKeyframes.size(); ++i)
				{
					DirectX::XMVECTOR B = DirectX::XMLoadFloat4(&nodeAnim.rotationKeyframes.at(i).value);
					if (!DirectX::XMVector4NearEqual(A, B, Epsilon))
					{
						result = false;
						break;
					}
				}
				if (result)
				{
					nodeAnim.rotationKeyframes.resize(1);
				}
			}
			// �X�P�[��
			{
				bool result = true;
				DirectX::XMVECTOR A = DirectX::XMLoadFloat3(&nodeAnim.scaleKeyframes.at(0).value);
				for (size_t i = 1; i < nodeAnim.scaleKeyframes.size(); ++i)
				{
					DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&nodeAnim.scaleKeyframes.at(i).value);
					if (!DirectX::XMVector3NearEqual(A, B, Epsilon))
					{
						result = false;
						break;
					}
				}
				if (result)
				{
					nodeAnim.scaleKeyframes.resize(1);
				}
			}
		}
		// �A�j���[�V�������Ȃ������m�[�h�ɑ΂��ď����p���̃L�[�t���[����ǉ�����
		for (size_t nodeIndex = 0; nodeIndex < animation.nodeAnims.size(); ++nodeIndex)
		{
			const Model::Node& node = nodes.at(nodeIndex);
			Model::NodeAnim& nodeAnim = animation.nodeAnims.at(nodeIndex);
			// �ړ�
			if (nodeAnim.positionKeyframes.size() == 0)
			{
				Model::VectorKeyframe& keyframe = nodeAnim.positionKeyframes.emplace_back();
				keyframe.seconds = 0.0f;
				keyframe.value = node.position;
			}
			if (nodeAnim.positionKeyframes.size() == 1)
			{
				Model::VectorKeyframe& keyframe = nodeAnim.positionKeyframes.emplace_back();
				keyframe.seconds = animation.secondsLength;
				keyframe.value = nodeAnim.positionKeyframes.at(0).value;
			}
			// ��]
			if (nodeAnim.rotationKeyframes.size() == 0)
			{
				Model::QuaternionKeyframe& keyframe = nodeAnim.rotationKeyframes.emplace_back();
				keyframe.seconds = 0.0f;
				keyframe.value = node.rotation;
			}
			if (nodeAnim.rotationKeyframes.size() == 1)
			{
				Model::QuaternionKeyframe& keyframe = nodeAnim.rotationKeyframes.emplace_back();
				keyframe.seconds = animation.secondsLength;
				keyframe.value = nodeAnim.rotationKeyframes.at(0).value;
			}
			// �X�P�[��
			if (nodeAnim.scaleKeyframes.size() == 0)
			{
				Model::VectorKeyframe& keyframe = nodeAnim.scaleKeyframes.emplace_back();
				keyframe.seconds = 0.0f;
				keyframe.value = node.scale;
			}
			if (nodeAnim.scaleKeyframes.size() == 1)
			{
				Model::VectorKeyframe& keyframe = nodeAnim.scaleKeyframes.emplace_back();
				keyframe.seconds = animation.secondsLength;
				keyframe.value = nodeAnim.scaleKeyframes.at(0).value;
			}
		}
	}
}

// aiVector3D �� XMFLOAT2
DirectX::XMFLOAT2 AssimpImporter::AssimpImporter::aiVector3DToXMFLOAT2(const aiVector3D& aValue)
{
	return DirectX::XMFLOAT2(
		static_cast<float>(aValue.x),
		static_cast<float>(aValue.y)
	);
}

// aiVector3D �� XMFLOAT3
DirectX::XMFLOAT3 AssimpImporter::aiVector3DToXMFLOAT3(const aiVector3D& aValue)
{
	return DirectX::XMFLOAT3(
		static_cast<float>(aValue.x),
		static_cast<float>(aValue.y),
		static_cast<float>(aValue.z)
	);
}

// aiColor3D �� XMFLOAT4
DirectX::XMFLOAT4 AssimpImporter::AssimpImporter::aiColor3DToXMFLOAT4(const aiColor3D& aValue)
{
	return DirectX::XMFLOAT4(
		static_cast<float>(aValue.r),
		static_cast<float>(aValue.g),
		static_cast<float>(aValue.b),
		1.0f
	);
}

// aiQuaternion �� XMFLOAT4
DirectX::XMFLOAT4 AssimpImporter::aiQuaternionToXMFLOAT4(const aiQuaternion& aValue)
{
	return DirectX::XMFLOAT4(
		static_cast<float>(aValue.x),
		static_cast<float>(aValue.y),
		static_cast<float>(aValue.z),
		static_cast<float>(aValue.w)
	);
}

// aiMatrix4x4 �� XMFLOAT4X4
DirectX::XMFLOAT4X4 AssimpImporter::aiMatrix4x4ToXMFLOAT4X4(const aiMatrix4x4& aValue)
{
	return DirectX::XMFLOAT4X4(
		static_cast<float>(aValue.a1),
		static_cast<float>(aValue.b1),
		static_cast<float>(aValue.c1),
		static_cast<float>(aValue.d1),
		static_cast<float>(aValue.a2),
		static_cast<float>(aValue.b2),
		static_cast<float>(aValue.c2),
		static_cast<float>(aValue.d2),
		static_cast<float>(aValue.a3),
		static_cast<float>(aValue.b3),
		static_cast<float>(aValue.c3),
		static_cast<float>(aValue.d3),
		static_cast<float>(aValue.a4),
		static_cast<float>(aValue.b4),
		static_cast<float>(aValue.c4),
		static_cast<float>(aValue.d4)
	);
}

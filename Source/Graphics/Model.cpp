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

//コンストラクタ
Model::Model(ID3D11Device* device, const char* filename, float scaling) : scaling(scaling)
{
	std::filesystem::path filepath(filename);
	std::filesystem::path dirpath(filepath.parent_path());

	//独自形式のモデルファイルの存在確認
	filepath.replace_extension(".cereal");
	if (std::filesystem::exists(filepath))
	{
		//独自形式のモデルファイルの読み込み
		Deserialize(filepath.string().c_str());
	}
	else
	{
		//汎用モデルファイル読み込み
		AssimpImporter importer(filename);

		//マテリアルデータ読み取り
		importer.LoadMaterials(materials);

		//ノードデータ読み取り
		importer.LoadNodes(nodes);

		//メッシュデータ読み取り
		importer.LoadMeshes(meshes, nodes);

		//アニメーションデータ読み取り
		importer.LoadAnimations(animations, nodes);

		//独自形式のモデルファイルを保存
		Serialize(filepath.string().c_str());
	}

	//ノード構築
	for (size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
	{
		Node& node = nodes.at(nodeIndex);

		//親子関係を構築
		node.parent = node.parentIndex >= 0 ? &nodes.at(node.parentIndex) : nullptr;
		if (node.parent != nullptr)
		{
			node.parent->children.emplace_back(&node);
		}
	}

	//マテリアル構築
	for (Material& material : materials)
	{
		if (material.diffuseTextureFileName.empty())
		{
			//ダミーテクスチャ作成
			HRESULT hr = GpuResourceUtils::CreateDummyTexture(device, 0xFFFFFFFF, material.diffuseMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		else
		{
			//ディフューズテクスチャ読み込み
			std::filesystem::path diffuseTexturePath(dirpath / material.diffuseTextureFileName);
			HRESULT hr = GpuResourceUtils::LoadTexture(device, diffuseTexturePath.string().c_str(), material.diffuseMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}

		if (material.normalTextureFileName.empty())
		{
			//法線ダミーテクスチャ作成
			HRESULT hr = GpuResourceUtils::CreateDummyTexture(device, 0xFFFF7F7F, material.normalMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		else
		{
			//法線テクスチャ読み込み
			std::filesystem::path texturePath(dirpath / material.normalTextureFileName);
			HRESULT hr = GpuResourceUtils::LoadTexture(device, texturePath.string().c_str(), material.normalMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
	}

	//メッシュ構築
	for (Mesh& mesh : meshes)
	{
		//参照ノード設定
		mesh.node = &nodes.at(mesh.nodeIndex);

		//参照マテリアル設定
		mesh.material = &materials.at(mesh.materialIndex);

		//頂点バッファ
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

		//インデックスバッファ
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

		//ボーン構築
		for (Bone& bone : mesh.bones)
		{
			//参照ノード設定
			bone.node = &nodes.at(bone.nodeIndex);
		}
	}
	//ノードキャッシュ
	nodeCaches.resize(nodes.size());
}

//トランスフォーム更新処理
void Model::UpdateTransform(const XMFLOAT4X4& worldTransform)
{
	XMMATRIX ParentWorldTransform = XMLoadFloat4x4(&worldTransform);

	//右手座標系から左手座標系へ変換する行列
	XMMATRIX CoordinateSystemTransform = XMMatrixScaling(-scaling, scaling, scaling);

	for (Node& node : nodes)
	{
		//ローカル行列算出
		XMMATRIX S = XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z);
		XMMATRIX R = XMMatrixRotationQuaternion(XMLoadFloat4(&node.rotation));
		XMMATRIX T = XMMatrixTranslation(node.position.x, node.position.y, node.position.z);
		XMMATRIX LocalTransform = S * R * T;
	
		//グローバル行列算出
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

		//ワールド行列算出
		XMMATRIX WorldTransform = GlobalTransform * CoordinateSystemTransform * ParentWorldTransform;

		//計算結果を格納
		XMStoreFloat4x4(&node.localTransform, LocalTransform);
		XMStoreFloat4x4(&node.globalTransform, GlobalTransform);
		XMStoreFloat4x4(&node.worldTransform, WorldTransform);
	}
}

//アニメーション再生
void Model::PlayAnimation(int index, bool loop, float blendSeconds)
{
	currentAnimationIndex = index;
	currentAnimationSeconds = 0;
	animationLoop = loop;
	animationPlaying = true;

	//ブレンドパラメータ
	animationBlending = blendSeconds > 0.0f;
	currentAnimationBlendSeconds = 0.0f;
	animationBlendSecondslength = blendSeconds;

	//現在の姿勢をキャッシュする
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		const Node& src = nodes.at(i);
		NodeCache& dst = nodeCaches.at(i);

		dst.position = src.position;
		dst.rotation = src.rotation;
		dst.scale = src.scale;
	}
}

//アニメーション再生中か
bool Model::IsPlayAnimation() const
{
	if (currentAnimationIndex < 0) return false;
	if (currentAnimationIndex >= animations.size()) return false;
	return animationPlaying;
}

//アニメーション更新処理
void Model::UpdateAnimation(float elapsedTime)
{
	ComputeAnimation(elapsedTime);

	ComputeBlending(elapsedTime);
}

//アニメーション計算処理
void Model::ComputeAnimation(float elapsedTime)
{
	if (!IsPlayAnimation()) return;

	//指定のアニメーションデータを取得
	const Animation& animation = animations.at(currentAnimationIndex);
	
	//ノード毎のアニメーション処理
	for (size_t nodeIndex = 0; nodeIndex < animation.nodeAnims.size(); ++nodeIndex)
	{
		Node& node = nodes.at(nodeIndex);
		const NodeAnim& nodeAnim = animation.nodeAnims.at(nodeIndex);

		//位置
		for (size_t index = 0; index < nodeAnim.positionKeyframes.size() - 1; ++index)
		{
			//現在の時間がどのキーフレームの間にいるか判定する
			const VectorKeyframe& keyframe0 = nodeAnim.positionKeyframes.at(index);
			const VectorKeyframe& keyframe1 = nodeAnim.positionKeyframes.at(index + 1);
			if (currentAnimationSeconds >= keyframe0.seconds && currentAnimationSeconds < keyframe1.seconds)
			{
				//再生時間とキーフレームの時間から補完率を算出する
				float rate = (currentAnimationSeconds - keyframe0.seconds) / (keyframe1.seconds - keyframe0.seconds);
				//前のキーフレームと次のキーフレームの姿勢を補完
				XMVECTOR V0 = XMLoadFloat3(&keyframe0.value);
				XMVECTOR V1 = XMLoadFloat3(&keyframe1.value);
				XMVECTOR V = XMVectorLerp(V0, V1, rate);
				//計算結果をノードに格納
				DirectX::XMStoreFloat3(&node.position, V);
			}
		}
		//回転
		for (size_t index = 0; index < nodeAnim.rotationKeyframes.size() - 1; ++index)
		{
			//現在の時間がどのキーフレームの間にいるか判定する
			const QuaternionKeyframe& keyframe0 = nodeAnim.rotationKeyframes.at(index);
			const QuaternionKeyframe& keyframe1 = nodeAnim.rotationKeyframes.at(index + 1);
			if (currentAnimationSeconds >= keyframe0.seconds && currentAnimationSeconds < keyframe1.seconds)
			{
				//再生時間とキーフレームの時間から補完率を算出する
				float rate = (currentAnimationSeconds - keyframe0.seconds) / (keyframe1.seconds - keyframe0.seconds);
				//前のキーフレームと次のキーフレームの姿勢を補完
				XMVECTOR Q0 = XMLoadFloat4(&keyframe0.value);
				XMVECTOR Q1 = XMLoadFloat4(&keyframe1.value);
				XMVECTOR Q = XMQuaternionSlerp(Q0, Q1, rate);
				//計算結果をノードに格納
				DirectX::XMStoreFloat4(&node.rotation, Q);
			}
		}
		//スケール
		for (size_t index = 0; index < nodeAnim.scaleKeyframes.size() - 1; ++index)
		{
			//現在の時間がどのキーフレームの間にいるか判定する
			const VectorKeyframe& keyframe0 = nodeAnim.scaleKeyframes.at(index);
			const VectorKeyframe& keyframe1 = nodeAnim.scaleKeyframes.at(index + 1);
			if (currentAnimationSeconds >= keyframe0.seconds && currentAnimationSeconds < keyframe1.seconds)
			{
				//再生時間とキーフレームの時間から補完率を算出する
				float rate = (currentAnimationSeconds - keyframe0.seconds) / (keyframe1.seconds - keyframe0.seconds);
				//前のキーフレームと次のキーフレームの姿勢を補完
				XMVECTOR V0 = XMLoadFloat3(&keyframe0.value);
				XMVECTOR V1 = XMLoadFloat3(&keyframe1.value);
				XMVECTOR V = XMVectorLerp(V0, V1, rate);
				//計算結果をノードに格納
				DirectX::XMStoreFloat3(&node.scale, V);
			}
		}
	}

	//時間経過
	currentAnimationSeconds += elapsedTime;

	//再生時間が終端時間を超えたら
	if (currentAnimationSeconds >= animation.secondsLength)
	{
		if (animationLoop)
		{
			//再生時間を巻き戻す
			currentAnimationSeconds -= animation.secondsLength;
		}
		else
		{
			//再生終了時間にする
			currentAnimationSeconds = animation.secondsLength;
			animationPlaying = false;
		}
	}
}

//ブレンディング計算処理
void Model::ComputeBlending(float elapsedTime)
{
	if (!animationBlending) return;

	//ブレンド率の計算
	float rate = currentAnimationBlendSeconds / animationBlendSecondslength;

	//ブレンド計算
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

	//時間経過
	currentAnimationBlendSeconds += elapsedTime;
	if (currentAnimationBlendSeconds >= animationBlendSecondslength)
	{
		currentAnimationBlendSeconds = animationBlendSecondslength;
		animationBlending = false;
	}
}

// ノード検索
Model::Node* Model::FindNode(const char* name)
{
	//全てのノードを総当たりで名前比較する
	for (Node& node : nodes)
	{
		if (node.name == name) return &node;
	}

	//見つからなかった
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

//シリアライズ
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

//デシリアライズ
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
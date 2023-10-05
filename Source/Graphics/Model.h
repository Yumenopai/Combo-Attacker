#pragma once
#include <memory>
#include <vector>
#include <DirectXMath.h>
#include <wrl.h>
#include <d3d11.h>
#include <string>

using namespace DirectX;

class Model
{
public:
	Model(ID3D11Device* device, const char* filename, float scaling = 1.0f);

	struct Node
	{
		std::string			name;
		std::string			path;
		int					parentIndex;
		XMFLOAT3			position;
		XMFLOAT4			rotation;
		XMFLOAT3			scale;

		XMFLOAT4X4			localTransform;
		XMFLOAT4X4			worldTransform;

		Node*				parent = nullptr;
		std::vector<Node*>	children;

		XMFLOAT4X4			globalTransform;

		template<class Archive>
		void serialize(Archive& archive);
	};

	struct Material
	{
		std::string name;
		std::string diffuseTextureFileName;
		XMFLOAT4	color = { 1,1,1,1 };
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseMap;
		std::string normalTextureFileName;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalMap;

		template<class Archive>
		void serialize(Archive& archive);
	};

	struct Vertex
	{
		XMFLOAT3 position = { 0,0,0 };
		XMFLOAT4 boneWeight = { 1,0,0,0 };//合計が1.0になる
		XMUINT4	 boneIndex = { 0,0,0,0 };
		XMFLOAT2 texcoord = { 0,0 };
		XMFLOAT3 normal = { 0,0,0 };
		XMFLOAT3 tangent = { 0,0,0 };

		template<class Archive>
		void serialize(Archive& archive);
	};

	struct Bone
	{
		int			nodeIndex;
		XMFLOAT4X4	offsetTransform;
		Node*		node = nullptr;

		template<class Archive>
		void serialize(Archive& archive);
	};

	struct Mesh
	{
		std::vector<Vertex>		vertices;
		std::vector<uint32_t>	indices;

		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

		int						materialIndex = 0;
		Material*				material = nullptr;
		int						nodeIndex = 0;
		Node*					node = nullptr;
		std::vector<Bone>		bones;

		template<class Archive>
		void serialize(Archive& archive);
	};

	struct VectorKeyframe
	{
		float		seconds;
		XMFLOAT3	value;

		template<class Archive>
		void serialize(Archive& archive);
	};

	struct QuaternionKeyframe
	{
		float		seconds;
		XMFLOAT4	value;

		template<class Archive>
		void serialize(Archive& archive);
	};

	struct NodeAnim
	{
		std::vector<VectorKeyframe>		positionKeyframes;
		std::vector<QuaternionKeyframe>	rotationKeyframes;
		std::vector<VectorKeyframe>		scaleKeyframes;

		template<class Archive>
		void serialize(Archive& archive);
	};

	struct Animation
	{
		std::string				name;
		float					secondsLength;
		std::vector<NodeAnim>	nodeAnims;

		template<class Archive>
		void serialize(Archive& archive);
	};

	//ルートノード取得
	Node* GetRootNode() { return nodes.data(); }
	// ノードリスト取得
	const std::vector<Node>& GetNodes() const { return nodes; }
	std::vector<Node>& GetNodes() { return nodes; }

	//メッシュデータ取得
	const std::vector<Mesh>& GetMeshes() const { return meshes; }

	//マテリアルデータ取得
	const std::vector<Material>& GetMaterials() const { return materials; }

	//トランスフォーム更新処理
	void UpdateTransform(const XMFLOAT4X4& worldTransform);

	//アニメーション再生
	void PlayAnimation(int index, bool loop, float blendSeconds = 0.2f);

	//アニメーション再生中か
	bool IsPlayAnimation() const;

	//アニメーション更新処理
	void UpdateAnimation(float elapsedTime);

	//アニメーションデータ更新
	const std::vector<Animation>& GetAnimations() const { return animations; }

	// ノード検索
	Node* FindNode(const char* name);

	//現在のアニメーション再生時間取得
	float GetCurrentAnimationSeconds() const { return currentAnimationSeconds; }

private:
	//アニメーション計算処理
	void ComputeAnimation(float elapsedTime);

	//ブレンディング計算処理
	void ComputeBlending(float elapsedTime);

	//シリアライズ
	void Serialize(const char* filename);

	//デシリアライズ
	void Deserialize(const char* filename);

private:
	std::vector<Mesh>		meshes;
	std::vector<Material>	materials;
	std::vector<Node>		nodes;
	std::vector<Animation>	animations;

	int		currentAnimationIndex = -1;
	float	currentAnimationSeconds = 0.0f;
	bool	animationPlaying = false;
	bool	animationLoop = false;

	struct NodeCache
	{
		XMFLOAT3 position = { 0,0,0 };
		XMFLOAT4 rotation = { 0,0,0,1 };
		XMFLOAT3 scale = { 1,1,1 };
	};
	std::vector<NodeCache> nodeCaches;

	float	currentAnimationBlendSeconds = 0.0f;
	float	animationBlendSecondslength = -1.0f;
	bool	animationBlending = false;
	float	scaling = 1.0f;
};
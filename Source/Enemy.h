#pragma once

#include "Shader/Shader.h"
#include "Character.h"

class Player;

class Enemy :public Character
{
public:
	Enemy(){}
	~Enemy() override{}

	//更新処理
	virtual void Update(float elapsedTime) = 0;

	//描画処理
	virtual void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) = 0;
	virtual void Render(const RenderContext& rc, ModelShader* shader) = 0;

	//破棄
	void Destroy();

	//デバッグプリミティブ描画
	virtual void DrawDebugPrimitive();

	//ダメージ加算
	bool ApplyDamage(int damage, float invincibleTime, Player* attacker, int playerNo);

	//Setter
	void SetTargetPosition(DirectX::XMFLOAT3& pos) { targetPosition = pos; }
	void SetFirstAttacker(Player* player) { FirstAttacker = player; }
	void SetLastAttacker(Player* player) { LastAttacker = player; }
	void AddAttackedDamage(int playerNo, int addDamage) { attackedDamage[playerNo] += addDamage; }

	//Getter
	virtual const float GetEffectOffset_Y() = 0;
	Player* GetFirstAttacker() const { return FirstAttacker; }
	Player* GetCurrentAttacker() const { return CurrentAttacker; }
	Player* GetLastAttacker() const { return LastAttacker; }

protected:
	DirectX::XMFLOAT3 targetPosition = { 0,0,0 };

	int damage;

	// パラメータ
	Player* FirstAttacker = nullptr;
	Player* CurrentAttacker = nullptr;
	Player* LastAttacker = nullptr;
	int attackedDamage[2] = { 0,0 };
};

#pragma once

#include "Shader/Shader.h"
#include "Character.h"
#include "PlayerManager.h"

#define PL1P 0
#define PLAI 1

class Player;

class Enemy :public Character
{
protected:
	// 対象位置
	DirectX::XMFLOAT3 targetPosition = { 0,0,0 };

	// 与えるダメージ
	int attackDamage = 0;

	// 一番目に攻撃したプレイヤー
	Player* FirstAttacker = nullptr;
	// 直近に攻撃したプレイヤー
	Player* CurrentAttacker = nullptr;
	// とどめをさすプレイヤー
	Player* LastAttacker = nullptr;
	// 自身がダメージを受けた量(プレイヤー毎)
	int attackedDamage[PLAYER_count] = { 0,0 };

public:
	Enemy(){}
	~Enemy() override{}

	// 更新処理
	virtual void Update(float elapsedTime) = 0;

	// シャドウマップ描画
	virtual void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) = 0;
	// 描画
	virtual void Render(const RenderContext& rc, ModelShader* shader) = 0;

	// 破棄
	void Destroy();

	// デバッグプリミティブ描画
	virtual void DrawDebugPrimitive();

	// 攻撃されていない場合はnullptrで返す
	Player* GetMostAttackPlayer() const;

	// ダメージ加算
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
	int GetAttackedDamage(int playerNo) const {	return attackedDamage[playerNo]; }
	int GetAttackedDamagePersent(int playerNo) const {
		return 100 * attackedDamage[playerNo] / (attackedDamage[PL1P] + attackedDamage[PLAI]);
	}
};

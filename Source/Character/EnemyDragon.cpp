#include "EnemyDragon.h"
#include "Graphics/Graphics.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "SceneClear.h"

//�R���X�g���N�^
EnemyDragon::EnemyDragon()
{
	radius = dragon_radius;
	height = dragon_height;
	health = maxHealth = dragon_max_health;
	attackDamage = dragon_attack_damage;

	// ���ʏ�������
	EnemyBoss::Init();
}

//�f�X�g���N�^
EnemyDragon::~EnemyDragon()
{
}

//�J�ڎ��A�j���[�V�����Đ�
void EnemyDragon::TransitionPlayAnimation(State nowState)
{
	//�A�j���[�V�����ݒ�
	Animation anime = Animation::Idle1;
	bool animeLoop = true;

	switch (nowState)
	{
	case State::Idle:
		anime = Animation::Idle1;
		break;
	case State::Wander:
		anime = Animation::Walk;
		break;
	case State::Pursuit:
		anime = Animation::Run;
		break;
	case State::Attack:
		anime = Animation::Attack01;
		animeLoop = false;
		break;
	case State::IdleBattle:
		anime = Animation::Idle02;
		break;
	case State::Scream:
		anime = Animation::Scream;
		animeLoop = false;
		break;
	case State::AttackClaw:
		anime = Animation::AttackClaw;
		animeLoop = false;
		break;
	case State::GetHit:
		anime = Animation::GetDamage;
		animeLoop = false;
		break;
	case State::Die:
		anime = Animation::Die;
		animeLoop = false;
		break;
	}

	//�A�j���[�V�����Đ�
	model->PlayAnimation(static_cast<int>(anime), animeLoop);
}

//���S�X�e�[�g�X�V����
void EnemyDragon::UpdateDieState(float elapsedTime)
{
	// ��ꏈ��
	EnemyBoss::UpdateDieState(elapsedTime);

	// �A�j���[�V�����I����
	if (!model->IsPlayAnimation())
	{
		// �N���A��ʂ֑J��
		SceneManager::Instance().ChangeScene(new SceneLoading(new SceneClear(), -255));
	}
}

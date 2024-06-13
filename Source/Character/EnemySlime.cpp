#include "EnemySlime.h"
#include "Constant/EnemyConst.h"
#include "Graphics/Graphics.h"
#include "MathScript.h"

//�R���X�g���N�^
EnemySlime::EnemySlime()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, slime_model_file, slime_render_size);
	eye_ball_node_name = slime_eye_ball_node_name;

	angle.y = Math::RandomRange(-360, 360);
	radius = slime_radius;
	height = slime_height;
	health = maxHealth = slime_max_health;

	attackDamage = slime_attack_damage;
	// ���ʏ���������
	EnemyNormal::Init();
}

//�f�X�g���N�^
EnemySlime::~EnemySlime()
{
}

//�J�ڎ��A�j���[�V�����Đ�
void EnemySlime::TransitionPlayAnimation(State nowState)
{
	//�A�j���[�V�����ݒ�
	Animation anime = Animation::IdleNormal;
	bool animeLoop = true;

	switch (nowState)
	{
	case State::Idle:
		anime = Animation::IdleNormal;
		break;
	case State::Wander:
		anime = Animation::WalkFWD;
		break;
	case State::Pursuit:
		anime = Animation::RunFWD;
		break;
	case State::Attack:
		anime = Animation::Attack1;
		animeLoop = false;
		break;
	case State::IdleBattle:
		anime = Animation::IdleBattle;
		break;
	case State::HitDamage:
		anime = Animation::GetHit;
		animeLoop = false;
		break;
	}

	//�A�j���[�V�����Đ�
	model->PlayAnimation(static_cast<int>(anime), animeLoop);
}

#pragma once

#include<DirectXMath.h>
using namespace DirectX;

//�L�����N�^�[
class Character
{
public:
	Character() {}
	virtual ~Character() {}

	//�s��X�V
	void UpdateTransform();

	//�ʒu�擾
	const XMFLOAT3& GetPosition() const { return position; }
	//�ʒu�ݒ�
	void SetPosition(const XMFLOAT3& position) { this->position = position; }
	//��]�擾
	const XMFLOAT3& GetAngle() const { return angle; }
	//��]�ݒ�
	void SetAngle(const XMFLOAT3& angle) { this->angle = angle; }
	//�X�P�[���擾
	const XMFLOAT3& GetScale() const { return scale; }
	//�X�P�[���ݒ�
	void SetScale(const XMFLOAT3& scale) { this->scale = scale; }
	//���a�擾
	float GetRadius() const { return radius; }

	//�n�ʂɐڐG���Ă��邩
	bool IsGround() const { return isGround; }
	//�����擾
	float GetHeight() const { return height; }
	//�_���[�W��^����
	bool ApplyDamage(int damage, float invincibleTime);
	//�Ռ���^����
	void AddImpulse(const XMFLOAT3& impulse);

	//���N��Ԃ��擾
	int GetHealth() const { return health; }
	//�ő匒�N��Ԃ��擾
	int GetMaxHealth() const { return maxHealth; }

protected:
	//�ړ�����
	void Move(float vx, float vz, float speed);
	//���񏈗�
	void Turn(float elapsedTime, float vx, float vz, float speed);
	//�W�����v����
	void Jump(float speed);
	//���͏����X�V
	void UpdateVelocity(float elapsedTime);
	//���G���ԍX�V
	void UpdateInvincibleTimer(float elapsedTime);

	//���n�������ɌĂ΂��
	virtual void OnLanding(float elapsedTime) {}
	//�_���[�W�󂯂����ɌĂ΂��
	virtual void OnDamaged() {}
	//���S�������ɌĂ΂��
	virtual void OnDead() {}

	//�������͍X�V
	virtual void UpdateVerticalVelocity(float elapsedFrame);
private:
	//�����ړ��X�V
	void UpdateVerticalMove(float elapsedTime);
	//�������͍X�V
	void UpdateHorizontalVelocity(float elapsedFrame);
	//�����ړ��X�V
	void UpdateHorizontalMove(float elapsedTime);

protected:
	XMFLOAT3	position = { 0,0,0 };
	XMFLOAT3	angle = { 0,0,0 };
	XMFLOAT3	scale = { 1,1,1 };
	XMFLOAT4X4 transform = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};

	float radius = 0.5f;

	float	gravity = -1.0f;
	XMFLOAT3 velocity = { 0,0,0 };
	bool	isGround = false;
	float	height = 2.0f;
	int		health = 40;
	int		maxHealth = 40;
	float	invincibleTimer = 1.0f;

	float	friction = 0.5f;

	float acceleration = 1.0f;
	float maxMoveSpeed = 5.0f;
	float moveVecX = 0.0f;
	float moveVecZ = 0.0f;
	float stepOffset = 1.0f;
	float slopeRate = 1.0f;
};
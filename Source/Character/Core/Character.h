#pragma once

#include<DirectXMath.h>
using namespace DirectX;

//�L�����N�^�[
class Character
{
protected:
	// �ʒu
	XMFLOAT3 position = { 0,0,0 };
	// �p�x
	XMFLOAT3 angle = { 0,0,0 };
	// �X�P�[��
	XMFLOAT3 scale = { 1,1,1 };
	// �g�����X�t�H�[��
	XMFLOAT4X4 transform = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};
	// �_���[�W���a
	float radius = 0.5f;

	// �d��
	const float gravity = -1.0f;
	// �����x
	XMFLOAT3 velocity = { 0,0,0 };
	// �n�ʂɒ��n���Ă��邩
	bool isGround = false;
	// ����
	float height = 2.0f;
	// HP
	int health = 40;
	// �ő�HP
	int maxHealth = 40;
	// �_���[�W���G���ԃ^�C�}�[
	float invincibleTimer = 0.0f;
	// ���C��
	float	friction = 0.5f;

	// ������
	float acceleration = 1.0f;
	// �ő�ړ��X�s�[�h
	float maxMoveSpeed = 5.0f;
	// �ړ��x�N�g��_X
	float moveVecX = 0.0f;
	// �ړ��x�N�g��_Y
	float moveVecZ = 0.0f;
	// ���C�L���X�g�p�����I�t�Z�b�g
	float stepOffset = 1.0f;
	// �X�Η�
	float slopeRate = 1.0f;

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
	bool ApplyDamage(int damage, float invincibleTime = 0);
	//�Ռ���^����
	void AddImpulse(const XMFLOAT3& impulse);

	//���N%��Ԃ��擾
	int GetHealthRate() const { return 100 * health / maxHealth; }
	//���N��Ԑݒ�
	void SetHealth(int h) { health = h; }
	//���N��Ԃ��擾
	int GetHealth() const { return health; }
	//�ő匒�N��Ԃ��擾
	int GetMaxHealth() const { return maxHealth; }
	// ���N��Ԃ�ݒ�
	void AddHealth(const int h) { health += h; }

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
};
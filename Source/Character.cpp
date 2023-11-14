#include "Character.h"
#include "Player.h"
#include "Stage.h"
#include "Input/Input.h"

//�s��X�V����
void Character::UpdateTransform()
{
	//�X�P�[���s��쐬
	XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	//��]�s��쐬
	XMMATRIX R = XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
	//�ʒu�s��쐬
	XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	//3�̍s��g�ݍ��킹�A���[���h�s��쐬
	XMMATRIX W = S * R * T;
	//�v�Z�������[���h�s����o��
	XMStoreFloat4x4(&transform, W);
}

bool Character::ApplyDamage(int damage, float invincibleTime)
{
	if (damage == 0)		return false;
	if (invincibleTime <= 0) return false;

	//�_���[�W����
	invincibleTimer = invincibleTime;
	health -= damage;

	//���S�ʒm
	if (health <= 0)
	{
		OnDead();
	}
	//�_���[�W�ʒm
	else
	{
		OnDamaged();
	}

	//�ύX���̂�
	return true;
}

void Character::AddImpulse(const DirectX::XMFLOAT3& impulse)
{
	//���͂ɗ͂�������
	velocity.x += impulse.x;
	velocity.y += impulse.y;
	velocity.z += impulse.z;
}

//�ړ�����
void Character::Move(float vx, float vz, float speed)
{
	//�ړ������x�N�g����ݒ�
	moveVecX = vx;
	moveVecZ = vz;

	//�ő呬�x�ݒ�
	maxMoveSpeed = speed;
}

//���񏈗�
void Character::Turn(float elapsedTime, float vx, float vz, float speed)
{
	speed *= elapsedTime;

	//�i�s�x�N�g�����[���x�N�g���̏ꍇ�͏�������K�v�Ȃ�
	float vecLength = vx * vx + vz * vz;
	if (vecLength < 0.00001f) return;

	//�i�s�x�N�g����P�ʃx�N�g����
	vecLength = sqrtf(vecLength);
	vx /= vecLength;
	vz /= vecLength;

	//���g�̉�]�l����O���������߂�
	float frontX = sinf(angle.y);
	float frontZ = cosf(angle.y);

	//��]�p�����߂邽�߁A2�̒P�ʃx�N�g���̓��ς��v�Z����
	float dot = frontX * vx + frontZ * vz;

	//���ϒl��-1.0�`1.0�ŕ\������Ă���A2�̒P�ʃx�N�g���̊p�x��
	//�������ق�1.0�ɋ߂Â��Ƃ��������𗘗p���ĉ�]���x�𒲐�����
	float rot = 1.0f - dot;

	//���E������s�����߂�2�̒P�ʃx�N�g���̊O�ς��v�Z����
	float cross = frontZ * vx - frontX * vz;

	//2D�̊O�ϒl�����̏ꍇ�����̏ꍇ�ɂ���č��E���肪�s����
	//���E������s�����Ƃɂ���č��E��]��I������
	if (rot > speed) rot = speed;

	if (cross < 0.0f)
	{
		angle.y -= rot;
	}
	else
	{
		angle.y += rot;
	}
}


//�W�����v����
void Character::Jump(float speed)
{
	//������̗͂�ݒ�
	velocity.y = speed;
}

//���͏���
void Character::UpdateVelocity(float elapsedTime)
{
	//�o�߃t���[��
	float elapsedFrame = 60.0f * elapsedTime;

	//�������͍X�V
	UpdateVerticalVelocity(elapsedFrame);
	UpdateVerticalMove(elapsedTime);
	//�����ړ��X�V
	UpdateHorizontalVelocity(elapsedFrame);
	UpdateHorizontalMove(elapsedTime);
}

void Character::UpdateInvincibleTimer(float elapsedTime)
{
	if (invincibleTimer > 0.0f)
	{
		invincibleTimer -= elapsedTime;
	}
}

//�������͍X�V
void Character::UpdateVerticalVelocity(float elapsedFrame)
{
	//�d�͏���
	velocity.y += gravity * elapsedFrame;
}

float Lerp(float a, float b, float t)
{
	return a * (1.0f - t) + (b * t);
}
//�����ړ��X�V
void Character::UpdateVerticalMove(float elapsedTime)
{
	//���������̈ړ���
	float my = velocity.y * elapsedTime;
	slopeRate = 0.0f;

	//�L�����N�^�[��Y�������ƂȂ�@���x�N�g��
	XMFLOAT3 normal = { 0,1,0 };
	
	//������
	if (my < 0.0f)
	{
		//���C�̊J�n�ʒu�͑�����菭����
		XMFLOAT3 start = { position.x, position.y + stepOffset, position.z };
		//���C�̏I�_�ʒu�͈ړ���̈ʒu
		XMFLOAT3 end = { position.x, position.y + my, position.z };

		//���C�L���X�g�ɂ��n�ʔ���
		HitResult hit;
		if (Stage::Instance().RayCast(start, end, hit))
		{
			//�@���x�N�g���擾
			normal = hit.normal;

			//�n�ʂɐڒn���Ă���
			position = hit.position;

			//��]
			angle.y += hit.rotation.y;

			//�X�Η��̌v�Z
			{
				float nx = hit.normal.x;
				float ny = hit.normal.y;
				float nz = hit.normal.z;
				float lengthXZ = sqrtf(nx * nx + nz * nz);
				slopeRate = 1.0f - (ny / (lengthXZ + ny));
			}

			//���n����
			if (!isGround)
			{
				OnLanding(elapsedTime);
			}
			isGround = true;
			velocity.y = 0.0f;
		}
		else
		{
			//�󒆂ɕ����Ă���
			position.y += my;
			isGround = false;
		}
	}
	//�㏸��
	else if (my > 0.0f)
	{
		position.y += my;
		isGround = false;
	}

	//�n�ʂ̌����ɉ����悤��XZ����]
	{
		//Y�����@���x�N�g�������Ɍ����I�C���[�p��]���Z�o
		//���`�⊮�Ŋ��炩�ɉ�]����
		//float ax = atan2f(normal.z, normal.y);
		//angle.x = Lerp(angle.x, ax, 0.2f);

		//float az = -atan2f(normal.x, normal.y);
		//angle.z = Lerp(angle.z, az, 0.2f);
	}
}

//�������͍X�V
void Character::UpdateHorizontalVelocity(float elapsedFrame)
{
	//XZ���ʂ̑��͂�����
	float length = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
	if (length > 0.0f)
	{
		//���C��
		float friction = this->friction * elapsedFrame;

		//���C�ɂ�鉡�����̌�������
		//���������͂̑傫���̕������C�͂��傫����
		if (length > friction)
		{
			//�P�ʃx�N�g����
			float vx = velocity.x / length;
			float vz = velocity.z / length;

			//velocity -= |velocity| * friction;
			velocity.x -= vx * friction;
			velocity.z -= vz * friction;
		}
		//�������̑��͂����C�͈ȉ��ɂȂ����̂ő��͂𖳌���
		else
		{
			velocity.x = 0.0f;
			velocity.z = 0.0f;
		}
	}

	//XZ���ʂ̑��͂�����
	if (length <= maxMoveSpeed)
	{
		//�ړ��x�N�g�����[���łȂ��Ȃ����
		float moveVecLength = moveVecX * moveVecX + moveVecZ * moveVecZ;
		if (moveVecLength > 0.0f)
		{
			//������
			float acceleration = this->acceleration * elapsedFrame;
			//�ړ��x�N�g���ɂ���������
			velocity.x += moveVecX * acceleration;
			velocity.z += moveVecZ * acceleration;

			//�ő呬�x����
			float nowLength = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
			if (nowLength > maxMoveSpeed)
			{
				float vx = velocity.x / nowLength;
				float vz = velocity.z / nowLength;

				velocity.x = vx * maxMoveSpeed;
				velocity.z = vz * maxMoveSpeed;
			}

			//�����ŃK�^�K�^���Ȃ��悤�ɂ���
			if (isGround && slopeRate > 0.0f)
			{
				velocity.y -= length * elapsedFrame * slopeRate;
			}
		}
	}
	//�ړ��x�N�g�������Z�b�g
	moveVecX = 0.0f;
	moveVecZ = 0.0f;
}

//�����ړ��X�V
void Character::UpdateHorizontalMove(float elapsedTime)
{
	//�������͗ʌv�Z
	float velocityLengthXZ = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
	if (velocityLengthXZ > 0.0f)
	{
		//�����ړ��n
		float mx = velocity.x * elapsedTime;
		float mz = velocity.z * elapsedTime;

		//���C�̊J�n�ʒu�ƏI�_�ʒu
		XMFLOAT3 start = { position.x,		position.y + stepOffset / 2,position.z };
		XMFLOAT3 end = { position.x + mx ,position.y + stepOffset / 2,position.z + mz };

		//���C�L���X�g�ɂ��ǔ���
		HitResult hit;
		if (Stage::Instance().RayCast(start, end, hit))
		{
			//�ǊJ�n����{���i�ވʒu�܂ł̃x�N�g��
			XMVECTOR Start = XMLoadFloat3(&hit.position);
			XMVECTOR End = XMLoadFloat3(&end);
			XMVECTOR Vec = XMVectorSubtract(End, Start);

			//�ǂ̖@��
			XMVECTOR Normal = XMLoadFloat3(&hit.normal);

			//���˃x�N�g����@���Ɏˉe
			XMVECTOR Dot = XMVector3Dot(XMVectorNegate(Vec), Normal);
			Dot = XMVectorScale(Dot, 1.1f); //�ǂ̂߂荞�ݑ΍�

			//�␳�ʒu�̌v�Z
			XMVECTOR CorrectPosition = XMVectorMultiplyAdd(Normal, Dot, End);
			XMFLOAT3 correctPosition;
			XMStoreFloat3(&correctPosition, CorrectPosition);

			//�ǎC��␳��̕����փ��C�L���X�g
			HitResult hit2;
			if (!Stage::Instance().RayCast(start, correctPosition, hit2))
			{
				//�ǎC������ŕǂɓ�����Ȃ�������␳�ʒu�Ɉړ�
				position.x = correctPosition.x;
				position.z = correctPosition.z;
			}
			else
			{
				//�ǂɓ��������_�Œ�~
				position.x = hit2.position.x;
				position.z = hit2.position.z;
			}
		}
		else
		{
			//�ړ�
			position.x += mx;
			position.z += mz;
		}
	}
}
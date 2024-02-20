#include "Player1P.h"
#include "Input/Input.h"

static Player1P* instance = nullptr;

//�C���X�^���X�擾
Player1P& Player1P::Instance()
{
	return *instance;
}

//�R���X�g���N�^
Player1P::Player1P()
{
	//�C���X�^���X�|�C���^�ݒ�
	instance = this;

	// ������
	Player::Init();

	position = { -7,5,-66 };
}

Player1P::~Player1P()
{
}

//�X�V
void Player1P::Update(float elapsedTime)
{
	UpdateEnemyDistance(elapsedTime);
	UpdateUtils(elapsedTime);
}

// HP�`��
void Player1P::HPBarRender(const RenderContext& rc, Sprite* gauge)
{
	Player::HPBarRender(rc, gauge, true);
}

// �{�^������
bool Player1P::InputButtonDown(InputState button)
{
	return Input::Instance().GetGamePad().GetButtonDown() & static_cast<unsigned int>(button);
}
bool Player1P::InputButton(InputState button)
{
	return Input::Instance().GetGamePad().GetButton() & static_cast<unsigned int>(button);
}
bool Player1P::InputButtonUp(InputState button)
{
	return Input::Instance().GetGamePad().GetButtonUp() & static_cast<unsigned int>(button);
}

//�X�e�B�b�N���͒l����ړ��x�N�g�����擾
XMFLOAT3 Player1P::GetMoveVec() const
{
	//���͏����擾
	GamePad& gamePad = Input::Instance().GetGamePad();
	float ax = gamePad.GetAxisLX();
	float ay = gamePad.GetAxisLY();

	//�J���������ƃX�e�B�b�N�̓��͒l�ɂ���Đi�s�������v�Z����
	Camera& camera = Camera::Instance();
	const DirectX::XMFLOAT3& cameraRight = camera.GetRight();
	const DirectX::XMFLOAT3& cameraFront = camera.GetFront();

	//�ړ��x�N�g����XZ���ʂɐ����ȃx�N�g���ɂȂ�悤�ɂ���

	//�J�����E�����x�N�g����XZ�P�ʃx�N�g���ɕϊ�
	float cameraRightX = cameraRight.x;
	float cameraRightZ = cameraRight.z;
	float cameraRightLength = sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
	if (cameraRightLength > 0.0f)
	{
		//�P�ʃx�N�g����
		cameraRightX /= cameraRightLength;
		cameraRightZ /= cameraRightLength;
	}

	//�J�����O�����x�N�g����XZ�P�ʃx�N�g���ɕϊ�
	float cameraFrontX = cameraFront.x;
	float cameraFrontZ = cameraFront.z;
	float cameraFrontLength = sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
	if (cameraFrontLength > 0.0f)
	{
		//�P�ʃx�N�g����
		cameraFrontX /= cameraFrontLength;
		cameraFrontZ /= cameraFrontLength;
	}

	//�X�e�B�b�N�̐������͒l���J�����E�����ɔ��f���A
	//�X�e�B�b�N�̐������͒l���J�����O�����ɔ��f���A�i�s�x�N�g�����v�Z����
	XMFLOAT3 vec;
	vec.x = cameraFrontX * ay + cameraRightX * ax;
	vec.z = cameraFrontZ * ay + cameraRightZ * ax;

	//Y�������ɂ͈ړ����Ȃ�
	vec.y = 0.0f;

	/*************************/
	XMVECTOR MoveVec = XMLoadFloat3(&vec);

	// �ړ����͒����ŋߋ������o�^����Ă���
	if (XMVectorGetX(XMVector3LengthSq(MoveVec)) != 0 && nearestDist < FLT_MAX)
	{
		// �������Ă���̂��G�l�~�[�x�N�g���Ɖs�p�֌W�Ȃ�
		float dot = XMVectorGetX(XMVector3Dot(MoveVec, XMLoadFloat3(&nearestVec)));

		// �ŋ߃G�l�~�[�Ɍ�����
		if (dot > 0) vec = nearestVec;
	}

	return vec;
}
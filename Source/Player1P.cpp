#include "Player1P.h"
#include "Input/Input.h"
#include "PlayerAI.h"

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

	//��������
	InitialWeapon = AttackType::Sword;

	// ������
	Player::Init();

	position = p1_initial_position;
	turn_speed = p1_turn_speed;

	playerName = p1_name;
	serialNumber = p1_serial_number;
	nameColor = p1_name_color;
	//UI
	hpGaugePosition_Y = p1_hp_gauge_position_y;
}

Player1P::~Player1P()
{
}

//�X�V
void Player1P::Update(float elapsedTime)
{
	if (targetPlayer == nullptr) {
		targetPlayer = &PlayerAI::Instance();
	}

	UpdateEnemyDistance(elapsedTime);
	UpdateUtils(elapsedTime);
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
	XMFLOAT3 vec = {};
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

// ===========���͏���===========
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

// ����ύX����
void Player1P::InputChangeWeapon(AttackType weapon/* = AttackType::None*/)
{
	// �񕜏������o����ꍇ�A����͕ύX�ł��Ȃ�
	if (enableRecoverTransition) return;
	// ������Ă��Ȃ�����return
	if (!InputButtonDown(Player::InputState::Player)) return;

	// �w�肳��Ă����炻���ݒ肷��
	if (weapon != AttackType::None) {
		CurrentUseWeapon = weapon;
		return;
	}

	// ���ɏ������Ă�����̂�I������
	CurrentUseWeapon = GetNextWeapon();
}

// �^�[�Q�b�g�񕜏���
void Player1P::InputRecover()
{
	if (!enableRecoverTransition) return;
	if (!InputButtonDown(Player::InputState::Buddy)) return;

	// �񕜃X�e�[�g�ֈڍs
	ChangeState(State::Recover);
}
#include <map>
#include <iomanip>
#include <sstream>

#include "Player.h"
#include "Constant/UtilsDefineConst.h"
#include "Constant/UIConst.h"
#include "Character/Manager/PlayerManager.h"
#include "Character/State/Player/PlayerEachState.h"
#include "Character/Manager/EnemyManager.h"
#include "Graphics/Graphics.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/Input.h"
#include "Enemy.h"
#include "Stage.h"
#include "imgui.h"

//�R���X�g���N�^
Player::Player()
{
}

Player::~Player()
{
}

// ������
void Player::Init()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	//�v���C���[���f���ǂݍ���
	model = std::make_unique<Model>(device, "Data/Model/SD-UnityChan/UnityChan.fbx", model_size);

	// ����o�^
	HaveWeapons.clear();
	for (AttackType type = AttackType::Sword; type < AttackType::MaxCount; type = SC_AT(SC_INT(type) + 1))
	{
		HaveWeapons.insert(std::pair<Player::AttackType, bool>(type, false));
	}
	// ��������
	CurrentUseWeapon = InitialWeapon;
	HaveWeapons[InitialWeapon] = true;

	enemySearch.clear();
	enemyDist.clear();
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();//�S�Ă̓G�Ƒ�������ŏՓˏ���
	for (int i = 0; i < enemyCount; i++)
	{
		enemySearch[enemyManager.GetEnemy(i)] = EnemySearch::None;
		enemyDist[enemyManager.GetEnemy(i)] = FLT_MAX;
	}

	// HP�ݒ�
	health = maxHealth = max_health;

	// StateMachine�𐶐� 
	stateMachine = new PlayerStateMachine();
	// �X�e�[�g�}�V���ɃX�e�[�g�o�^ 
	stateMachine->RegisterState(new StateIdle(this));
	stateMachine->RegisterState(new StateIdleToRun(this));
	stateMachine->RegisterState(new StateRun(this));
	stateMachine->RegisterState(new StateRunToIdle(this));
	stateMachine->RegisterState(new StateJumpStart(this));
	stateMachine->RegisterState(new StateJumpLoop(this));
	stateMachine->RegisterState(new StateJumpAir(this));
	stateMachine->RegisterState(new StateJumpEnd(this));
	stateMachine->RegisterState(new StateDamage(this));
	stateMachine->RegisterState(new StateDead(this));
	stateMachine->RegisterState(new StateRecover(this));
	stateMachine->RegisterState(new StateAttackHammer1(this));
	stateMachine->RegisterState(new StateAttackHammer2(this));
	stateMachine->RegisterState(new StateAttackHammerJump(this));
	stateMachine->RegisterState(new StateAttackSpear1(this));
	stateMachine->RegisterState(new StateAttackSpear2(this));
	stateMachine->RegisterState(new StateAttackSpear3(this));
	stateMachine->RegisterState(new StateAttackSpearJump(this));
	stateMachine->RegisterState(new StateAttackSword1(this));
	stateMachine->RegisterState(new StateAttackSword2(this));
	stateMachine->RegisterState(new StateAttackSword3(this));
	stateMachine->RegisterState(new StateAttackSwordJump(this));

	// �X�e�[�g���Z�b�g 
	stateMachine->SetState(SC_INT(State::Idle));
}

// �X�V����
void Player::UpdateUtils(float elapsedTime)
{
	// �񕜑J�ډ\��
	enableRecoverTransition = EnableRecoverTransition();

	// �X�e�[�g���ɒ��ŏ�������
	stateMachine->Update(elapsedTime);

	// �W�����v����
	UpdateJumpState(elapsedTime);

	// �v���C���[�ƃG�l�~�[�Ƃ̏Փˏ���
	CollisionPlayerVsEnemies();

	// ���͏����X�V
	UpdateVelocity(elapsedTime);

	// �X�e�[�W�̒[�̉��Ǐ���
	StageSideWall();

	// �I�u�W�F�N�g�s��X�V
	UpdateTransform();

	// ���f���A�j���[�V�����X�V����
	model->UpdateAnimation(elapsedTime);

	// ���f���s��X�V
	model->UpdateTransform(transform);
}

// �G�Ƃ̋����X�V
void Player::UpdateEnemyDistance(float elapsedTime)
{
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();

	nearestEnemy = nullptr;
	nearestDist = FLT_MAX;
	nearestVec = {};
	int noneEnemy = 0;
	for (int i = 0; i < enemyCount; i++)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);
		// ���ꂼ��̃G�l�~�[�̋�������
		XMVECTOR PosPlayer = XMLoadFloat3(&GetPosition());
		XMVECTOR PosEnemy = XMLoadFloat3(&enemy->GetPosition());

		XMVECTOR DistVec = XMVectorSubtract(PosEnemy, PosPlayer);
		float dist = XMVectorGetX(XMVector3Length(DistVec));

		enemyDist[enemy] = dist; //�e�G�l�~�[�Ƃ̋��������X�V���ɋL�^����
		if (dist < judge_dist_attack)
		{
			enemySearch[enemy] = EnemySearch::Attack;
		}
		else if (dist < judge_dist_find)
		{
			enemySearch[enemy] = EnemySearch::Find;
		}
		else
		{
			enemySearch[enemy] = EnemySearch::None;
			noneEnemy++;
			// �G�l�~�[�����Ȃ��Ȃ������A���݂̃G�l�~�[�T���X�e�[�g��None�ɂ���
			if (i == (enemyCount - 1) && enemyCount == noneEnemy)
			{
				currentEnemySearch = EnemySearch::None;
			}
			continue;
		}

		/***********************/
		if (dist < nearestDist) 
		{
			// secondEnemy�̓o�^
			secondDist = nearestDist;
			secondDistEnemyVec = nearestVec;

			// �ŋ߃G�l�~�[�̓o�^
			nearestEnemy = enemy;
			nearestDist = dist;
			currentEnemySearch = enemySearch[enemy];
			XMStoreFloat3(&nearestVec, DistVec);
		}
	}
}

// �W�����v����
void Player::UpdateJumpState(float elapsedTime)
{
	// �U�����̏ꍇ�̓W�����v�����Ȃ�
	if (isAttacking) return;

	switch (jumpTrg)
	{
	case JumpState::CanJump:
		// �����Ă���Ԃ̏���
		if (InputButton(Player::InputState::Jump))
		{
			velocity.y += first_jump_speed * elapsedTime;
			// �w������x�܂ł���������
			if (velocity.y > max_jump_speed)
			{
				jumpTrg = JumpState::CanDoubleJump;
			}
		}
		// ��񗣂�����
		else if (InputButtonUp(Player::InputState::Jump))
		{
			jumpTrg = JumpState::CanDoubleJump;
		}
		break;

	case JumpState::CanDoubleJump:
		// 2�i�ڃW�����v�͍������ߕs��
		if (InputButtonDown(Player::InputState::Jump))
		{
			if (velocity.y > 0) velocity.y += second_jump_speed;
			else				velocity.y = second_jump_speed;

			jumpTrg = JumpState::CannotJump;
		}
		// ��i�ڃW�����v���̍U���{�^��
		else if (InputAttackFromJump())
		{
			jumpTrg = JumpState::CannotJump;
		}

		//break;
		[[fallthrough]];
	case JumpState::CannotJump:

		// �W�����v�\��Ԃ̎��̂ݒʂ�Ȃ�
		// ���n��(�n�ʂɗ����Ă��鎞�͏펞����)
		if (isGround)
		{
			// ���n���ɉ������ς̏ꍇ�͏�������Ȃ��悤�ɂ���
			if (InputButton(Player::InputState::Jump))
			{
				jumpTrg = JumpState::CannotJump;
			}
			// ������Ă��Ȃ����͒n�ʂɂ���̂ŃW�����v�\��Ԃɂ���
			else
			{
				jumpTrg = JumpState::CanJump;
			}
		}
		break;
	}
}

// �V���h�E�}�b�v�p�`��
void Player::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	shadowMap->Draw(rc, model.get());
}
// �`��
void Player::Render(const RenderContext& rc, ModelShader* shader)
{
	shader->Draw(rc, model.get());
}

// HP�`��
void Player::RenderHPBar(ID3D11DeviceContext* dc, Sprite* gauge, FontSprite* font)
{
	Graphics& graphics = Graphics::Instance();
	static const float screenWidth = static_cast<float>(graphics.GetScreenWidth());

	// ���O�\��
	StringRender(dc, font, playerName,
		{ hp_gauge_name_position.x, hpGaugePosition_Y + hp_gauge_name_position.y },
		nameColor);

	// Lv�\��
	std::string levelStr = "Lv:" + std::to_string(currentLevel);
	StringRender(dc, font, levelStr,
		{ lv_display_position_x, hpGaugePosition_Y + display_under_text_offset_y },
		nameColor);

	// HP�\��;
	std::ostringstream ss;
	ss << std::setw(hp_display_digit) << std::setfill('0') << GetHealth(); // �����w��
	std::string hp(ss.str());
	std::string hpStr = hp + '/' + std::to_string(GetMaxHealth());
	StringRender(dc, font, hpStr,
		{ hp_display_position_x, hpGaugePosition_Y + display_under_text_offset_y },
		nameColor);

	//�Q�[�W�`��(���n)
	gauge->Render(dc,
		(screenWidth / 2) - (hp_gauge_size.x / 2), // X_�����ɔz�u���邽�ߕ��̔����ƃQ�[�W�����̔����ŋ��߂�
		hpGaugePosition_Y, // Y
		SPRITE_position_default_z, // Z
		hp_gauge_size.x + hp_gauge_frame_expansion,
		hp_gauge_size.y + hp_gauge_frame_expansion,
		SPRITE_angle_default,
		hp_gauge_frame_color // �w�i�J���[
	);
	//�Q�[�W�`��
	gauge->Render(dc,
		(screenWidth / 2) - (hp_gauge_size.x / 2) + hp_gauge_frame_expansion / 2, // X_�����ɔz�u���邽�ߕ��̔����ƃQ�[�W�����̔����ŋ��߂�
		hpGaugePosition_Y + hp_gauge_frame_expansion / 2, // Y_�㉺�̊g�������킹���T�C�Y���ő����Ă��邽�ߔ�������
		SPRITE_position_default_z, // Z
		hp_gauge_size.x * (GetHealthRate() / 100.0f), //�S�����������ɕϊ�
		hp_gauge_size.y,
		SPRITE_angle_default,
		GetHpWorning() ? hp_gauge_color_wornimg : hp_gauge_color_normal // �Q�[�W�J���[/HP����ƐF��ς���
	);
}

// �L�����N�^�[���O�`��
void Player::RenderCharacterOverHead(const RenderContext& rc, FontSprite* font, Sprite* message)
{
	const DirectX::XMFLOAT4X4& view = rc.view;
	const DirectX::XMFLOAT4X4& projection = rc.projection;

	//�r���[�|�[�g
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	rc.deviceContext->RSGetViewports(&numViewports, &viewport);

	//�ϊ��s��
	XMMATRIX View = XMLoadFloat4x4(&view);
	XMMATRIX Projection = XMLoadFloat4x4(&projection);
	XMMATRIX World = XMMatrixIdentity();

	//Player����̃��[���h���W
	XMFLOAT3 worldPosition = GetPosition();
	worldPosition.y += GetHeight() + 0.4f;
	XMVECTOR WorldPosition = XMLoadFloat3(&worldPosition);

	//���[���h����X�N���[���ւ̕ϊ�
	XMVECTOR ScreenPosition = XMVector3Project(
		WorldPosition,
		viewport.TopLeftX, viewport.TopLeftY,
		viewport.Width, viewport.Height,
		viewport.MinDepth, viewport.MaxDepth,
		Projection, View, World);

	XMFLOAT3 screenPosition;
	XMStoreFloat3(&screenPosition, ScreenPosition);
	//�J�����̔w��ɂ��邩�A���炩�ɗ���Ă���Ȃ�`�悵�Ȃ�
	if (screenPosition.z > 0.0f && screenPosition.z < 1.0f) //0.0f�`1.0f�̊�
	{
		// ���O
		StringRender(rc.deviceContext, font, playerName,
			{ screenPosition.x + name_offset.x, screenPosition.y + name_offset.y },
			nameColor);
		// ���b�Z�[�W�`��
		RenderCharacterMessage(rc.deviceContext, message, { screenPosition.x, screenPosition.y });
	}
}
// �L�����N�^�[���b�Z�[�W�`��
void Player::RenderCharacterMessage(ID3D11DeviceContext* dc, Sprite* message, DirectX::XMFLOAT2 position)
{
	// ���b�Z�[�W
	if (messageNumber == PlayerMessage::WeaponGet || messageNumber == PlayerMessage::LevelUp)
	{
		// �^�C�}�[����
		messageYTimer += message_timer_increase;
		// �^�C�}�[�Ŏ����I��false�ɐ؂�ւ��郁�b�Z�[�W�`��
		message->Render(dc,
			{ position.x + message_offset.x, position.y + message_offset.y - messageYTimer, SPRITE_position_default_z },
			message_size,
			{ message_sprite_size.x, message_sprite_size.y * SC_INT(messageNumber) },
			message_sprite_size,
			SPRITE_angle_default,
			SPRITE_color_default);

		// �^�C�}�[����莞�Ԃ܂ł�����
		if (messageYTimer > message_timer_max)
		{
			enableShowMessage[SC_INT(messageNumber)] = false;
			messageNumber = PlayerMessage::None;
		}
	}
	else if (enableShowMessage[SC_INT(PlayerMessage::WeaponGet)])
	{
		// true�ɂȂ������̏�������
		messageNumber = PlayerMessage::WeaponGet;
		messageYTimer = 0.0f;
	}
	else if (enableShowMessage[SC_INT(PlayerMessage::LevelUp)])
	{
		// true�ɂȂ������̏�������
		messageNumber = PlayerMessage::LevelUp;
		messageYTimer = 0.0f;
	}
	else
	{
		for (PlayerMessage mes = PlayerMessage::Attack; mes < PlayerMessage::MaxCount; mes = SC_PM(SC_INT(mes) + 1))
		{
			if (!enableShowMessage[SC_INT(mes)]) continue;

			// true�̎��͏펞�\�����郁�b�Z�[�W�̕`��
			message->Render(dc,
				{ position.x + message_offset.x, position.y + message_offset.y - messageYTimer, SPRITE_position_default_z },
				message_size,
				{ message_sprite_size.x, message_sprite_size.y * SC_INT(mes) },
				message_sprite_size,
				SPRITE_angle_default,
				SPRITE_color_default);
			break; // ���ނ����\�������Ȃ��̂ň�`�悵�����_��break
		}
	}
}

// ��������`��
void Player::RenderHaveWeapons(ID3D11DeviceContext* dc, Sprite* frame, Sprite* weapon)
{
	//HaveWeaponFrame
	for (int i = 0; i < HaveWeapons.size(); i++)
	{
		float textureCutPosition_x = 0;
		// ���ݎg�p���̕���
		if (i == SC_INT(CurrentUseWeapon)) {
			textureCutPosition_x = WeaponIcon_sprite_size.x;
		}
		else {
			// �������Ă��镐�킩�ǂ����ŃZ�b�g����
			textureCutPosition_x = HaveWeapons[SC_AT(i)] ? WeaponIcon_sprite_size.x * 2 : 0; // 2�{���W�����E0�{���W����
		}
		frame->Render(dc,
			{ WeaponFrame_position_x + WeaponFrame_offset.x * i, hpGaugePosition_Y + WeaponFrame_offset.y, SPRITE_position_default_z },
			WeaponFrame_render_size,
			{ textureCutPosition_x, WeaponIcon_sprite_size.y }, 
			WeaponIcon_sprite_size,
			SPRITE_angle_default,
			SPRITE_color_default);
	}
	//HaveWeapon
	for (int i = 0; i < HaveWeapons.size(); i++)
	{
		float textureCutPosition_x = WeaponIcon_sprite_size.x * (i + 1); // i+1�{���W����
		float textureCutPosition_y = 0;
		float color_a = HaveWeapons[SC_AT(i)] ? SPRITE_color_default.z : WeaponIcon_color_translucent_a; // ����������𔼓����ɂ���
		float color_rb = (SC_AT(i) == CurrentUseWeapon) ? WeaponIcon_color_decrease_rb : SPRITE_color_default.y; // �g�p���̕���ɗΐF��t����
		weapon->Render(dc,
			{ WeaponIcon_position_x + WeaponFrame_offset.x * i, hpGaugePosition_Y + WeaponIcon_offset_y, SPRITE_position_default_z },
			WeaponIcon_size,
			{ textureCutPosition_x, textureCutPosition_y },
			WeaponIcon_sprite_size,
			SPRITE_angle_default,
			{ color_rb, SPRITE_color_default.y, color_rb, color_a });
	}
}
// �����`��/�v���C���[UI�p
void Player::StringRender(ID3D11DeviceContext* dc, FontSprite* font,
	std::string str,			// �e�L�X�g
	DirectX::XMFLOAT2 position,	// �ʒu
	DirectX::XMFLOAT4 color)	// �F
{
	font->Textout(dc, str,
		{ position.x, position.y, TEXT_depth_default },
		TEXT_display_size_default,
		TEXT_cut_position_default,
		TEXT_cut_size_default,
		TEXT_angle_default,
		color);
}

//���n�������ɌĂ΂��
void Player::OnLanding(float elapsedTime)
{
	if (isAttacking) //�U����(��ɃW�����v�U����)
	{
		// ���n���Ă����͉��������Ȃ����߂����ŏ����������Ȃ�
		// �eStateUpdate�ɂăA�j���[�V�����I�����IdleState�֑J�ڂ���
	}
	else if (InputMove(elapsedTime))
	{
		ChangeState(State::Run);
	}
	else {
		ChangeState(State::JumpEnd);
	}
}

void Player::OnDamaged()
{
	ChangeState(State::Damage);
}

void Player::OnDead()
{
	ChangeState(State::Dead);
}

void Player::UpdateWeaponPositions(Model* model, Weapon& weapon)
{
	Model::Node* weaponBone = model->FindNode(weapon.nodeName);
	XMMATRIX W = XMLoadFloat4x4(&weaponBone->worldTransform);
	XMVECTOR V = weapon.tipOffset;
	XMVECTOR P = XMVector3Transform(V, W);
	XMStoreFloat3(&weapon.position, P);
}

void Player::HorizontalVelocityByAttack(bool plus, int velo, float elapsedTime)
{
	if (enemySearch[nearestEnemy] == EnemySearch::Attack) return;

	if (plus)
	{
		velocity.x += sinf(angle.y) * velo * elapsedTime;
		velocity.z += cosf(angle.y) * velo * elapsedTime;
	}
	else
	{
		velocity.x = sinf(angle.y) * velo * elapsedTime;
		velocity.z = cosf(angle.y) * velo * elapsedTime;
	}
}

//�������͍X�V
void Player::UpdateVerticalVelocity(float elapsedFrame)
{
	auto state = static_cast<Player::State>(stateMachine->GetStateNumber());

	// �W�����v�X�s�A�[�U���̂ݕʂ̉������������g�p����
	if (state == State::AttackSpearJump)
	{
		if (velocity.y > 0) // ����Y���W����������l�������0�ɂ���
		{
			velocity.y = 0;
		}
		velocity.y += gravity * spear_jamp_attack_velocity_rate * elapsedFrame;
	}
	else
	{
		velocity.y += gravity * elapsedFrame;
	}
}

// ===========���͏���===========
//�ړ����͏���
bool Player::InputMove(float elapsedTime)
{
	XMFLOAT3 moveVec = GetMoveVec();

	//�ړ�����
	Move(moveVec.x, moveVec.z, move_speed);
	//���񏈗�
	Turn(elapsedTime, moveVec.x, moveVec.z, turn_speed);

	//�i�s�x�N�g�����[���x�N�g���łȂ��ꍇ�͓��͂��ꂽ
	return moveVec.x != 0 || moveVec.y != 0 || moveVec.z != 0;
}

// �U�����͏���
bool Player::InputAttackFromNoneAttack()
{
	// ������Ă��Ȃ�����return
	if (!InputButtonDown(Player::InputState::Attack)) return false;
	enableSpecialAttack = true;

	if (targetPlayer->enableSpecialAttack
		&& targetPlayer->CurrentUseWeapon == this->CurrentUseWeapon)
	{
		// �W�����v�U�����X�y�V�����Z�Ƃ��Ďg�p����
		InputAttackFromJump();
		targetPlayer->InputAttackFromJump();
	}
	else
	{
		switch (CurrentUseWeapon)
		{
		case Player::AttackType::Hammer:
			ChangeState(State::AttackHammer1);
			break;
		case Player::AttackType::Spear:
			ChangeState(State::AttackSpear1);
			break;
		case Player::AttackType::Sword:
			ChangeState(State::AttackSword1);
			break;
		}
	}
	return true;
}
bool Player::InputAttackFromJump()
{
	// ������Ă��Ȃ�����return
	if (!InputButtonDown(Player::InputState::Attack)) return false;

	// �X�y�V�����Zfalse
	enableSpecialAttack = false;

	switch (CurrentUseWeapon)
	{
	case Player::AttackType::Hammer:
		ChangeState(State::AttackHammerJump);
		break;
	case Player::AttackType::Spear:
		ChangeState(State::AttackSpearJump);
		break;
	case Player::AttackType::Sword:
		ChangeState(State::AttackSwordJump);
		break;
	}

	return true;
}

// ���̑I�𕐊�擾
Player::AttackType Player::GetNextWeapon()
{
	// ���݂̎��̔ԍ��̕����I������
	AttackType type = CurrentUseWeapon;
	while (true)
	{
		// ���̕����I��
		type = SC_AT(SC_INT(type) + 1);
		// �͈͊O�ɂȂ�Ώ��߂ɖ߂�
		if (type == AttackType::MaxCount) {
			type = AttackType::Sword;
		}

		// �S����������break
		if (type == CurrentUseWeapon) break;
		// �������Ȃ�continue
		if (!HaveWeapons[type]) continue;

		// ���ɏ������Ă�����̂�I������
		return type;
	}
	// ���݂̕�������̂܂ܕԂ�
	return CurrentUseWeapon;
}

// �A�C�e���Q�b�g
void Player::AddHaveWeapon(Player::AttackType weapon/* = AttackType::None*/)
{
	// �w�肳��Ă����炻���ݒ肷��
	if (weapon != AttackType::None) {
		HaveWeapons[weapon] = true;
		return;
	}

	// ���ݎ����Ă��Ȃ����탊�X�g
	std::unordered_map<AttackType,bool> remainWeapon = HaveWeapons;
	for (const auto& weapon : HaveWeapons)
	{
		if (HaveWeapons[weapon.first]) {
			remainWeapon.erase(weapon.first);
		}
	}

	// �S�Ď����Ă���ꍇ��return
	if (remainWeapon.size() == 0) return;

	// ����
	int num = rand() % remainWeapon.size();
	int i = 0;
	// �l�����镐����m�肷��
	for (const auto& weapon : remainWeapon)
	{
		if (i == num) {
			HaveWeapons[weapon.first] = true;
			return;
		}
		i++;
	}
}

//�񕜑J�ڊm�F����
bool Player::EnableRecoverTransition()
{
	// �댯�łȂ����false
	if (!targetPlayer->GetHpWorning()) return false;

	XMVECTOR thisPlayerPosition = XMLoadFloat3(&GetPosition());
	XMVECTOR targetPlayerPosition = XMLoadFloat3(&targetPlayer->GetPosition());
	// �v���C���[���m�̋���
	float distSq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(thisPlayerPosition, targetPlayerPosition)));

	// �񕜉\�ȋ����ɂȂ�ΑJ�ډ\
	return distSq < recover_dist * recover_dist;
}

// �ߋ����U�����̊p�x����
void Player::ForceTurnByAttack(float elapsedTime)
{
	// �G�̔������ɐi�ޕ�������������
	if (enemySearch[nearestEnemy] >= EnemySearch::Find)
	{
		//���񏈗�
		Turn(elapsedTime, nearestVec.x, nearestVec.z, turn_speed);
	}
}

//�v���C���[�ƃG�l�~�[�Ƃ̏Փˏ���
void Player::CollisionPlayerVsEnemies()
{
	EnemyManager& enemyManager = EnemyManager::Instance();

	//�S�Ă̓G�Ƒ�������ŏՓˏ���
	int enemyCount = enemyManager.GetEnemyCount();
	for (int i = 0; i < enemyCount; i++)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);

		//�Փˏ���
		DirectX::XMFLOAT3 outPosition;
		if (Collision::IntersectCylinderVsCylinder(
			enemy->GetPosition(), enemy->GetRadius(), enemy->GetHeight(),
			position, radius, height,
			XMFLOAT3{}, outPosition))
		{
			SetPosition(outPosition);
		}
	}
}

// �X�e�[�W�̒[�̉��Ǐ���
void Player::StageSideWall()
{
	// �����I�ȉ����Ԃ�
	if (position.x > STAGE_side_max) position.x = STAGE_side_max;
	if (position.x < STAGE_side_min) position.x = STAGE_side_min;
	if (position.z > STAGE_side_max) position.z = STAGE_side_max;
	if (position.z < STAGE_side_min) position.z = STAGE_side_min;
}

// ����ƃG�l�~�[�̏Փˏ���
void Player::CollisionWeaponsVsEnemies(Weapon weapon)
{
	//�w��̃m�[�h�ƑS�Ă̓G�𑍓�����ŏՓˏ���
	EnemyManager& enemyManager = EnemyManager::Instance();
	for (int i = 0; i < enemyManager.GetEnemyCount(); i++)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);

		//�Փˏ���
		XMFLOAT3 outPosition;
		if (Collision::IntersectSphereVsCylinder(
			weapon.position, weapon.radius,
			enemy->GetPosition(), enemy->GetRadius(), enemy->GetHeight(),
			outPosition
		))
		{
			// �X�y�V�����Zfalse
			enableSpecialAttack = false;

			// �U�����肵�Ȃ��ꍇ��return
			if (attackingEnemyNumber == i && !isAttackJudge) return;

			// �_���[�W�v�Z
			int damage = weapon.damage + SC_INT(currentLevel * damage_increase);
			// �_���[�W��^���Ȃ��ꍇ��return
			if (!enemy->ApplyDamage(damage, invincible_time, this, serialNumber)) return;

			currentAttackEnemy = enemy;
			attackingEnemyNumber = i;
			isAttackJudge = false;
			allDamage += damage;
			// �q�b�g�G�t�F�N�g�Đ�
			{
				outPosition.y += enemy->GetHeight() * enemy->GetEffectOffset_Y();
				PlayEffect(EffectNumber::Hit, outPosition);
			}

			// ������΂���4��ڈȏ�
			if (attackCount >= impulse_attack_count)
			{
				// ������΂��U��
				const XMFLOAT3& ep = enemy->GetPosition();

				// �m�b�N�o�b�N�����̎Z�o
				float vx = ep.x - enemy->GetCurrentAttacker()->position.x;
				float vz = ep.z - enemy->GetCurrentAttacker()->position.z;
				float lengthXZ = sqrtf(vx * vx + vz * vz);
				vx /= lengthXZ;
				vz /= lengthXZ;
				// Y�����̒���
				float vy = impulse_power_adjust_rate_y;

				// �m�b�N�o�b�N
				enemy->AddImpulse({ impulse_power * vx , impulse_power * vy, impulse_power * vz });
			}
		}
		else if(attackingEnemyNumber == i)//�U�����̃G�l�~�[�ƈ�U�U�����O�ꂽ���A���񓖂��������ɔ�����s��
		{
			isAttackJudge = true;
		}
	}
}

#pragma region DEBUG_DRAW
//�f�o�b�O�v���~�e�B�u�`��
void Player::DrawDebugPrimitive() const
{
	Gizmos* gizmos = Graphics::Instance().GetGizmos();
	//�Փ˔���p�̃f�o�b�O�~����`��
	gizmos->DrawCylinder(
		position,		//�ʒu
		radius,			//���a
		height,			//����
		GIZMOS_cylinder_angle_default,		//�p�x
		GIZMOS_color_red);	//�F

	if (Hammer.flag1 || Hammer.flag2 || Hammer.flagJump)
	{
		gizmos->DrawSphere(
			Hammer.position,
			Hammer.radius,
			GIZMOS_color_blue
		);
	}
	if (Spear.flag1 || Spear.flag2 || Spear.flag3 || Spear.flagJump)
	{
		gizmos->DrawSphere(
			Spear.position,
			Spear.radius,
			GIZMOS_color_blue
		);
	}
	if (Sword.flag1 || Sword.flag2 || Sword.flag3 || Sword.flagJump)
	{
		gizmos->DrawSphere(
			Sword.position,
			Sword.radius,
			GIZMOS_color_blue
		);
	}

	Camera& camera = Camera::Instance();
	//�`��R���e�L�X�g�ݒ�
	RenderContext rc;
	rc.camera = &camera;
	rc.deviceContext = Graphics::Instance().GetDeviceContext();
	rc.renderState = Graphics::Instance().GetRenderState();

	//�`����s
	gizmos->Render(rc);
}

void Player::DebugMenu()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Player", nullptr, ImGuiWindowFlags_None))
	{
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//�ʒu
			ImGui::DragFloat3("Position", &position.x, 0.1f);

			//��]
			XMFLOAT3 a = {};
			a.x = XMConvertToDegrees(angle.x);
			a.y = XMConvertToDegrees(angle.y);
			a.z = XMConvertToDegrees(angle.z);
			ImGui::DragFloat3("Angle", &a.x, 1.0f);
			if (a.y > 360) a.y = 0;
			if (a.y < 0) a.y = 360;
			angle.x = XMConvertToRadians(a.x);
			angle.y = XMConvertToRadians(a.y);
			angle.z = XMConvertToRadians(a.z);

			//�X�P�[��
			ImGui::DragFloat3("Scale", &scale.x, 0.01f);
		}

		ImGui::Checkbox("attacking", &isAttackJudge);

		ImGui::End();
	}
}
#pragma endregion
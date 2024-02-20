#include "Player1P.h"
#include "Input/Input.h"

static Player1P* instance = nullptr;

//インスタンス取得
Player1P& Player1P::Instance()
{
	return *instance;
}

//コンストラクタ
Player1P::Player1P()
{
	//インスタンスポインタ設定
	instance = this;

	// 初期化
	Player::Init();

	position = { -7,5,-66 };
}

Player1P::~Player1P()
{
}

//更新
void Player1P::Update(float elapsedTime)
{
	UpdateEnemyDistance(elapsedTime);
	UpdateUtils(elapsedTime);
}

// HP描画
void Player1P::HPBarRender(const RenderContext& rc, Sprite* gauge)
{
	Player::HPBarRender(rc, gauge, true);
}

// ボタン判定
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

//スティック入力値から移動ベクトルを取得
XMFLOAT3 Player1P::GetMoveVec() const
{
	//入力情報を取得
	GamePad& gamePad = Input::Instance().GetGamePad();
	float ax = gamePad.GetAxisLX();
	float ay = gamePad.GetAxisLY();

	//カメラ方向とスティックの入力値によって進行方向を計算する
	Camera& camera = Camera::Instance();
	const DirectX::XMFLOAT3& cameraRight = camera.GetRight();
	const DirectX::XMFLOAT3& cameraFront = camera.GetFront();

	//移動ベクトルはXZ平面に水平なベクトルになるようにする

	//カメラ右方向ベクトルをXZ単位ベクトルに変換
	float cameraRightX = cameraRight.x;
	float cameraRightZ = cameraRight.z;
	float cameraRightLength = sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
	if (cameraRightLength > 0.0f)
	{
		//単位ベクトル化
		cameraRightX /= cameraRightLength;
		cameraRightZ /= cameraRightLength;
	}

	//カメラ前方向ベクトルをXZ単位ベクトルに変換
	float cameraFrontX = cameraFront.x;
	float cameraFrontZ = cameraFront.z;
	float cameraFrontLength = sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
	if (cameraFrontLength > 0.0f)
	{
		//単位ベクトル化
		cameraFrontX /= cameraFrontLength;
		cameraFrontZ /= cameraFrontLength;
	}

	//スティックの水平入力値をカメラ右方向に反映し、
	//スティックの垂直入力値をカメラ前方向に反映し、進行ベクトルを計算する
	XMFLOAT3 vec;
	vec.x = cameraFrontX * ay + cameraRightX * ax;
	vec.z = cameraFrontZ * ay + cameraRightZ * ax;

	//Y軸方向には移動しない
	vec.y = 0.0f;

	/*************************/
	XMVECTOR MoveVec = XMLoadFloat3(&vec);

	// 移動入力中且つ最近距離が登録されている
	if (XMVectorGetX(XMVector3LengthSq(MoveVec)) != 0 && nearestDist < FLT_MAX)
	{
		// 向かっているのがエネミーベクトルと鋭角関係なら
		float dot = XMVectorGetX(XMVector3Dot(MoveVec, XMLoadFloat3(&nearestVec)));

		// 最近エネミーに向かう
		if (dot > 0) vec = nearestVec;
	}

	return vec;
}
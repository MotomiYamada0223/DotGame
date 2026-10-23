#include "Camera.h"
#include "GameConstants.h"
#include <DxLib.h>

Camera::Camera()
	: mScrollX(0)
	, mScrollY(0)
	, mIsScrolling(false)
	, mShakePower(0.0f)
	, mShakeRatioX(0.0f)
	, mShakeRatioY(0.0f)
	, mShakeDuration(0)
	, mShakeTimer(0)
	, mShakeOffsetX(0)
	, mShakeOffsetY(0)
	, mIsLocked(false)
{
}

Camera::~Camera()
{
}

// プレイヤーの位置や移動方向に応じてカメラのスクロール位置を計算するため
void Camera::Update(
	int playerScreenX,
	int playerScreenY,
	float moveDirection,
	float currentSpeed,
	int backgroundWidth,
	int screenWidth,
	int backgroundHeight,
	int screenHeight
)
{
	// スクロールがロックされている場合は位置を更新せず、画面揺れのみ処理
	if (mIsLocked)
	{
		mIsScrolling = false;
		UpdateShake();
		return;
	}


	int oldScrollPos = mScrollX;
	// プレイヤーが右のスクロール境界を越えたら、越えた分だけスクロールを加算する
	if (playerScreenX > MapScrollConstants::ScrollStartRightX)
	{
		mScrollX += (playerScreenX - MapScrollConstants::ScrollStartRightX);
	}
	// プレイヤーが左のスクロール境界を越えたら、越えた分だけスクロールを減算する
	else if (playerScreenX < MapScrollConstants::ScrollStartLeftX)
	{
		mScrollX += (playerScreenX - MapScrollConstants::ScrollStartLeftX);
	}

	// マップの左端を超えないようにするため
	if (mScrollX < 0)
	{
		mScrollX = 0;
	}
	// マップの右端を超えないようにするため
	int maxScrollX = backgroundWidth - screenWidth;
	if (maxScrollX < 0) { maxScrollX = 0; }
	if (mScrollX > maxScrollX) { mScrollX = maxScrollX; }

	// プレイヤーが画面上の一定のライン
	if (playerScreenY < MapScrollConstants::ScrollStartUpY)
	{
		// プレイヤーが上に上がった分だけ、カメラを上に動かす
		int diffY = static_cast<int>(playerScreenY - MapScrollConstants::ScrollStartUpY * MapScrollConstants::BlockMapSpeedScale);
		mScrollY += diffY;
	}


	// マップの下端を超えないようにするため（背景の高さから画面の高さを引いた値を最大スクロール量とする）
	int maxScrollY = backgroundHeight - screenHeight;
	if (maxScrollY < 0) { maxScrollY = 0; }

	// mScrollYは下方向へ進むにつれてマイナスになるため、下限を -maxScrollY に制限する
	if (mScrollY > maxScrollY)
	{
		mScrollY = maxScrollY;
	}
	mIsScrolling = (oldScrollPos != mScrollX);



	// 画面揺れの更新
	UpdateShake();
}

void Camera::StartShake(float power, float ratioX, float ratioY, int durationFrames)
{
	mShakePower = power;
	mShakeRatioX = ratioX;
	mShakeRatioY = ratioY;
	mShakeDuration = (durationFrames > 0) ? durationFrames : 1;
	mShakeTimer = mShakeDuration;
	UpdateShake();
}

void Camera::UpdateShake()
{
	if (mShakeTimer > 0)
	{
		float progress = 1.0f - static_cast<float>(mShakeTimer) / static_cast<float>(mShakeDuration);
		float decay = 1.0f - progress; // 残り時間に応じた減衰

		// -1.0f から +1.0f の不規則振動
		float randX = static_cast<float>(GetRand(200) - 100) / 100.0f;
		float randY = static_cast<float>(GetRand(200) - 100) / 100.0f;

		mShakeOffsetX = static_cast<int>(randX * mShakePower * mShakeRatioX * decay);
		mShakeOffsetY = static_cast<int>(randY * mShakePower * mShakeRatioY * decay);

		mShakeTimer--;
	}
	else
	{
		mShakeOffsetX = 0;
		mShakeOffsetY = 0;
	}
}
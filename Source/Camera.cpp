#include "Camera.h"
#include "GameConstants.h"
#include <DxLib.h>
#include <algorithm>
#include <string>
#include <cmath>

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
	, mNeedScrollInit(true)
{
}

Camera::~Camera()
{
}

// プレイヤーの位置や移動方向に応じてカメラのスクロール位置を計算するため
void Camera::Update(
    int playerScreenX,
    int playerMapY,
    float moveDirection,
    float currentSpeed,
    int backgroundWidth,
    int screenWidth,
    int backgroundHeight,
    int screenHeight
)
{
    // 最大スクロール量を保存（描画時のクランプで使用）
    mMaxScrollX = (std::max)(0, backgroundWidth - screenWidth);
    mMaxScrollY = (std::max)(0, backgroundHeight - screenHeight);


    if (mIsLocked)
    {
        mIsScrolling = false;
        UpdateShake();
        return;
    }

    int oldScrollPos = mScrollX;

    // 横スクロール（丸めて整数化）
    if (playerScreenX > MapScrollConstants::ScrollStartRightX && moveDirection > 0.0f)
    {
        mScrollX += static_cast<int>(std::round(currentSpeed * moveDirection));
    }
    else if (playerScreenX < MapScrollConstants::ScrollStartLeftX && moveDirection < 0.0f)
    {
        mScrollX += static_cast<int>(std::round(currentSpeed * moveDirection));
    }

    mScrollX = LimitValue(mScrollX, 0, mMaxScrollX);

    // 縦スクロール（補間も丸める）
    int targetScrollY = LimitValue(playerMapY - 720, 0, mMaxScrollY);


    if (mNeedScrollInit)
    {
        mScrollYf = static_cast<float>(targetScrollY);
        mNeedScrollInit = false;
    }
    else
    {
        mScrollYf += (targetScrollY - mScrollYf) * 0.15f;
    }
    mScrollY = LimitValue(static_cast<int>(std::round(mScrollYf)), 0, mMaxScrollY);


    mIsScrolling = (oldScrollPos != mScrollX);

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
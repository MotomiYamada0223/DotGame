#include "Camera.h"
#include "GameConstants.h"
#include <DxLib.h>

Camera::Camera()
	: mScrollX(0)
	, mScrollY(0)
	, mIsScrolling(false)
{
}

Camera::~Camera()
{
}

// プレイヤーの位置や移動方向に応じてカメラのスクロール位置を計算するため
void Camera::Update(int playerScreenX, int playerScreenY, float moveDirection, float currentSpeed, int backgroundWidth, int screenWidth)
{
	if (CheckHitKey(KEY_INPUT_1))
	{
		mScrollX = 0;
		mScrollY = 0;
	}

	int oldScrollPos = mScrollX;

	// 右側の線を超えていて、右に移動中なら右へスクロールするため
	if (playerScreenX > MapScrollConstants::ScrollStartRightX && moveDirection > 0.0f)
	{
		mScrollX += static_cast<int>(currentSpeed);
	}
	// 左側の線を超えていて、左に移動中なら左へスクロールするため
	else if (playerScreenX < MapScrollConstants::ScrollStartLeftX && moveDirection < 0.0f)
	{
		mScrollX -= static_cast<int>(currentSpeed);
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

	// マップの左端を超えないようにするため
	if (mScrollY > 0)
	{
		mScrollY = 0;
	}

	mIsScrolling = (oldScrollPos != mScrollX);
}
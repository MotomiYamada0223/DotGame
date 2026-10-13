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
	// プレイヤーが右のスクロール境界を越えたら、越えた分だけスクロールを加算する
	// (moveDirectionやcurrentSpeedに依存しないため、動く床に乗っている受動的な移動でも正しく追従する)
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

	// マップの左端を超えないようにするため
	if (mScrollY > 0)
	{
		mScrollY = 0;
	}

	mIsScrolling = (oldScrollPos != mScrollX);
}
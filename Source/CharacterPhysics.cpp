#include "CharacterPhysics.h"


CharacterPhysics::CharacterPhysics()
{
}


CharacterPhysics::~CharacterPhysics()
{
}


// ============================================================
// 移動 + 重力 + 当たり判定
// ============================================================

void CharacterPhysics::UpdateMoveAndCollision(
	VECTOR& position,
	float& velocityY,
	bool& isGrounded,
	bool& isJumping,
	BlockMap& blockMap,
	float width,
	float height,
	float gravity,
	float moveSpeed)
{
	// ========================================================
	// 重力を加える
	// ========================================================

	velocityY += gravity;


	// ========================================================
	// 次のY座標
	// ========================================================

	float nextY = position.y + velocityY;


	// ========================================================
	// プレイヤーの左右
	// ========================================================

	float playerLeft =
		position.x - width / 2.0f;

	float playerRight =
		position.x + width / 2.0f;


	// ========================================================
	// 現在のプレイヤー上下
	// ========================================================

	float oldTop =
		position.y - height / 2.0f;

	float oldBottom =
		position.y + height / 2.0f;


	// ========================================================
	// 次のプレイヤー上下
	// ========================================================

	float nextTop =
		nextY - height / 2.0f;

	float nextBottom =
		nextY + height / 2.0f;


	// ========================================================
	// 初期状態
	// ========================================================

	isGrounded = false;


	// ========================================================
	// 落下中
	// ========================================================

	if (velocityY > 0.0f)
	{
		// ----------------------------------------------------
		// 足元だけを調べる
		// ----------------------------------------------------

		int collisionX = -1;
		int collisionY = -1;


		bool hitFloor =
			blockMap.CheckCollisionBlock(
				playerLeft,
				nextBottom,
				width,
				1.0f,
				&collisionX,
				&collisionY
			);


		// ----------------------------------------------------
		// 床に当たった
		// ----------------------------------------------------

		if (hitFloor)
		{
			// 今回見つかった赤ピクセルのY座標
			float floorY =
				static_cast<float>(collisionY);


			// ------------------------------------------------
			// 本当に「上から落ちてきた」場合だけ接地
			// ------------------------------------------------

			if (oldBottom <= floorY ||
				nextBottom >= floorY)
			{
				// プレイヤーの足を赤ピクセルの上に置く
				position.y =
					floorY - height / 2.0f;


				// 落下速度を止める
				velocityY = 0.0f;


				// 接地
				isGrounded = true;
				isJumping = false;
			}
			else
			{
				// それ以外はそのまま移動
				position.y = nextY;

				isGrounded = false;
				isJumping = true;
			}
		}
		else
		{
			// ------------------------------------------------
			// 床に当たっていない
			// ------------------------------------------------

			position.y = nextY;

			isGrounded = false;
			isJumping = true;
		}
	}


	// ========================================================
	// 上昇中
	// ========================================================

	else if (velocityY < 0.0f)
	{
		// ----------------------------------------------------
		// 頭の部分だけを調べる
		// ----------------------------------------------------

		int collisionX = -1;
		int collisionY = -1;


		bool hitCeiling =
			blockMap.CheckCollisionBlock(
				playerLeft,
				nextTop,
				width,
				1.0f,
				&collisionX,
				&collisionY
			);


		// ----------------------------------------------------
		// 天井に当たった
		// ----------------------------------------------------

		if (hitCeiling)
		{
			// 赤ピクセルのY座標
			float ceilingY =
				static_cast<float>(collisionY);


			// ------------------------------------------------
			// プレイヤーの頭を赤ピクセルの下に置く
			// ------------------------------------------------

			position.y =
				ceilingY + 1.0f + height / 2.0f;


			// 上昇を止める
			velocityY = 0.0f;


			// ジャンプ状態
			isGrounded = false;
			isJumping = true;
		}
		else
		{
			// ------------------------------------------------
			// 天井に当たっていない
			// ------------------------------------------------

			position.y = nextY;

			isGrounded = false;
			isJumping = true;
		}
	}


	// ========================================================
	// 垂直速度が0
	// ========================================================

	else
	{
		position.y = nextY;
	}
}
#include "CharacterPhysics.h"
#include "Collision.h"

CharacterPhysics::CharacterPhysics()
{
}

CharacterPhysics::~CharacterPhysics()
{
}

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
	// 縦方向の物理処理
	// 重力
	velocityY += gravity;

	// 次のY座標
	float nextY = position.y + velocityY;

	// 現在のプレイヤー上下
	float oldTop =
		position.y - height / 2.0f;

	float oldBottom =
		position.y + height / 2.0f;

	// 次のプレイヤー矩形
	float playerLeft =
		position.x - width / 2.0f;

	float playerTop =
		nextY - height / 2.0f;

	// 衝突したブロック
	int blockX = -1;
	int blockY = -1;

	bool hitBlock =
		blockMap.CheckCollisionBlock(
			playerLeft,
			playerTop,
			width,
			height,
			&blockX,
			&blockY
		);

	// ブロックに衝突した
	if (hitBlock)
	{
		float blockLeft = (float)blockX * (float)CHIP_SIZE;
		float blockTop = (float)blockY * (float)CHIP_SIZE;
		float blockRight = (float)blockLeft + (float)CHIP_SIZE;
		float blockBottom = (float)blockTop + (float)CHIP_SIZE;

		// 落下中
		if (velocityY > 0.0f)
		{
			// 次のフレームの足元座標
			float nextBottom = nextY + height / 2.0f;

			// 落下してきて、足元がブロックの上部を通過・または接した場合
			if (oldBottom <= blockTop || nextBottom >= blockTop)
			{
				position.y =
					blockTop - height / 2.0f;

				velocityY = 0.0f;

				isGrounded = true;
				isJumping = false;

			}
			else
			{
				// 横から当たった等の場合
				position.y = nextY;

				isGrounded = false;
				isJumping = true;

			}
		}
		// 上昇中
		else if (velocityY < 0.0f)
		{
			// ブロックの下面に頭が当たった
			if (oldTop >= blockBottom)
			{
				position.y =
					blockBottom + height / 2.0f;

				velocityY = 0.0f;

				isGrounded = false;
				isJumping = true;

			}
			else
			{
				// まだブロックに阻まれていないので上昇
				position.y = nextY;

				isGrounded = false;
				isJumping = true;

			}
		}
		// velocityY == 0
		else
		{
			position.y = nextY;
		}
	}
	// ブロックに当たっていない場合
	else
	{
		position.y = nextY;

		isGrounded = false;
		isJumping = true;
	}
}
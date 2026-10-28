#include "CharacterPhysics.h"
#include "GameConstants.h"

// クラスの初期化を行う
CharacterPhysics::CharacterPhysics()
{
}

// クラスの終了処理を行う
CharacterPhysics::~CharacterPhysics()
{
}

BlockMap::CollisionType CharacterPhysics::CheckCollision(
	const BlockMap& blockMap,
	float x,
	float y,
	float width,
	float height,
	int* blockX,
	int* blockY) const
{
	// 引数はすべてマップ座標（mvPosition と同じ座標系）
	const int left = static_cast<int>(x);
	const int right = static_cast<int>(x + width - 1.0f);
	const int top = static_cast<int>(y);
	const int bottom = static_cast<int>(y + height - 1.0f);

	for (int mapY = top; mapY <= bottom; ++mapY)
	{
		for (int mapX = left; mapX <= right; ++mapX)
		{
			const BlockMap::CollisionType type =
				blockMap.GetCollisionType(mapX, mapY);

			if (type == BlockMap::CollisionType::None) { continue; }

			if (blockX != nullptr) { *blockX = mapX; }
			if (blockY != nullptr) { *blockY = mapY; }
			return type;
		}
	}
	return BlockMap::CollisionType::None;
}



// キャラクターの移動、重力処理、マップタイルとの当たり判定を解決する
BlockMap::CollisionType CharacterPhysics::UpdateMoveAndCollision(
	VECTOR& position,
	float& velocityY,
	bool& isGrounded,
	bool& isJumping,
	const BlockMap& blockMap,
	float width,
	float height,
	float gravity,
	float moveSpeed,
	float moveDirection)
{
	BlockMap::CollisionType detectedSpecialType =
		BlockMap::CollisionType::None;

	bool isHittingWall = false;

	// X方向移動
	const float nextX = position.x + (moveSpeed * moveDirection);
	if (moveDirection != 0.0f)
	{
		const float halfWidth = width * HalfMultiply;
		const float halfHeight = height * HalfMultiply;
		const float wallCheckX = (moveDirection < 0.0f) ? (nextX - halfWidth) : (nextX + halfWidth);
		const float wallCheckTop = position.y - halfHeight + PlayerBlockCollision::WallCheckMargin;
		const float wallCheckHeight = height - PlayerBlockCollision::WallCheckMargin * 2.0f;

		int wallX = -1;
		int wallY = -1;

		const BlockMap::CollisionType wallType =
			CheckCollision(
				blockMap,
				wallCheckX,
				wallCheckTop,
				PlayerBlockCollision::CollisionThickness,
				wallCheckHeight,
				&wallX,
				&wallY
			);

		// BlockまたはBossAreaなら壁なので移動しない
		if (wallType == BlockMap::CollisionType::Block || wallType == BlockMap::CollisionType::BossArea)
		{
			isHittingWall = true;
			if (wallType == BlockMap::CollisionType::BossArea &&
				detectedSpecialType != BlockMap::CollisionType::Death &&
				detectedSpecialType != BlockMap::CollisionType::Goal)
			{
				detectedSpecialType = wallType;
			}
		}
		else
		{
			position.x = nextX;
			if (wallType != BlockMap::CollisionType::None)
			{
				detectedSpecialType = wallType;
			}
		}
	}

	// Y方向移動
	velocityY += gravity;
	const float nextY = position.y + velocityY;
	const float halfWidth = width * HalfMultiply;
	const float halfHeight = height * HalfMultiply;
	const float playerLeft = position.x - halfWidth;
	const float oldBottom = position.y + halfHeight;
	const float nextTop = nextY - halfHeight;
	const float nextBottom = nextY + halfHeight;

	// 接地状態を一旦解除
	isGrounded = false;

	// 落下中
	if (velocityY > 0.0f)
	{
		int collisionX = -1;
		int collisionY = -1;

		const BlockMap::CollisionType floorType =
			CheckCollision(
				blockMap,
				playerLeft,
				nextBottom,
				width,
				PlayerBlockCollision::CollisionThickness,
				&collisionX,
				&collisionY
			);

		if (floorType != BlockMap::CollisionType::None)
		{
			if (floorType == BlockMap::CollisionType::Block || floorType == BlockMap::CollisionType::BossArea)
			{
				const float floorY = static_cast<float>(collisionY);

				// Blockの上面に乗ったかどうかの判定
				if (oldBottom <= floorY || nextBottom >= floorY)
				{
					position.y = floorY - halfHeight;
					velocityY = 0.0f;
					isGrounded = true;
					isJumping = false;
				}
				else
				{
					position.y = nextY;
					isGrounded = false;
					isJumping = true;
				}

				if (floorType == BlockMap::CollisionType::BossArea &&
					detectedSpecialType != BlockMap::CollisionType::Death &&
					detectedSpecialType != BlockMap::CollisionType::Goal)
				{
					detectedSpecialType = floorType;
				}
			}
			else
			{
				detectedSpecialType = floorType;
				position.y = nextY;
				isGrounded = false;
				isJumping = true;
			}
		}
		else
		{
			position.y = nextY;
			isGrounded = false;
			isJumping = true;
		}
	}

	// 上昇中
	else if (velocityY < 0.0f)
	{
		int collisionX = -1;
		int collisionY = -1;

		const BlockMap::CollisionType ceilingType =
			CheckCollision(
				blockMap,
				playerLeft,
				nextTop,
				width,
				PlayerBlockCollision::CollisionThickness,
				&collisionX,
				&collisionY
			);

		if (ceilingType != BlockMap::CollisionType::None)
		{
			if (ceilingType == BlockMap::CollisionType::Block || ceilingType == BlockMap::CollisionType::BossArea)
			{
				const float ceilingY =
					static_cast<float>(collisionY);

				position.y =
					ceilingY + PlayerBlockCollision::CollisionThickness + halfHeight;

				velocityY = 0.0f;

				if (ceilingType == BlockMap::CollisionType::BossArea &&
					detectedSpecialType != BlockMap::CollisionType::Death &&
					detectedSpecialType != BlockMap::CollisionType::Goal)
				{
					detectedSpecialType = ceilingType;
				}
			}
			else
			{
				detectedSpecialType = ceilingType;
				position.y = nextY;
			}

			isGrounded = false;
			isJumping = true;
		}
		else
		{
			position.y = nextY;
			isGrounded = false;
			isJumping = true;
		}
	}
	else // 垂直速度が0
	{
		position.y = nextY;
	}

	return detectedSpecialType;
}



// 指定した矩形が何らかの地形に接触しているかどうかを判定する
bool CharacterPhysics::IsBlockCollision(
	const BlockMap& blockMap,
	float x,
	float y,
	float width,
	float height) const
{
	return CheckCollision(
		blockMap,
		x,
		y,
		width,
		height
	) != BlockMap::CollisionType::None;
}
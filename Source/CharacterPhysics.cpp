#include "CharacterPhysics.h"

namespace
{
	// 壁判定時に床や天井のブロックを誤検出しないように上下を縮めるマージン
	constexpr float WallCheckMargin = 20.0f;

	// 壁や床、天井の判定を行う際の厚み（1ピクセル）
	constexpr float CollisionThickness = 1.0f;
}

// クラスの初期化を行う
CharacterPhysics::CharacterPhysics()
{
}

// クラスの終了処理を行う
CharacterPhysics::~CharacterPhysics()
{
}

// 矩形領域とマップタイルが接触しているかを判定し、接触したタイル種別と座標を返す
BlockMap::CollisionType CharacterPhysics::CheckCollision(
	const BlockMap& blockMap,
	float x,
	float y,
	float width,
	float height,
	int* blockX,
	int* blockY) const
{
	// 浮動小数点的な座標を整数ピクセルに変換
	const int left = static_cast<int>(x);
	const int right = static_cast<int>(x + width - 1.0f);
	const int top = static_cast<int>(y);
	const int bottom = static_cast<int>(y + height - 1.0f);

	// 指定矩形内を1ピクセルずつ調べる
	for (int pixelY = top; pixelY <= bottom; ++pixelY)
	{
		for (int pixelX = left; pixelX <= right; ++pixelX)
		{
			const BlockMap::CollisionType type =
				blockMap.GetCollisionType(pixelX, pixelY);

			if (type == BlockMap::CollisionType::None)
			{
				continue;
			}

			if (blockX != nullptr)
			{
				*blockX = pixelX;
			}

			if (blockY != nullptr)
			{
				*blockY = pixelY;
			}

			return type;
		}
	}

	return BlockMap::CollisionType::None;
}

// キャラクターの移動、重力処理、およびマップタイルとの当たり判定を解決する
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
	// 特殊地形の検出結果
	BlockMap::CollisionType detectedSpecialType =
		BlockMap::CollisionType::None;

	// デバッグ用
	bool isHittingWall = false;

	// =========================
	// X方向移動
	// =========================

	const float nextX =
		position.x + (moveSpeed * moveDirection);

	if (moveDirection != 0.0f)
	{
		const float halfWidth = width * 0.5f;
		const float halfHeight = height * 0.5f;

		const float wallCheckX =
			(moveDirection < 0.0f)
			? (nextX - halfWidth)
			: (nextX + halfWidth);

		const float wallCheckTop =
			position.y - halfHeight + WallCheckMargin;

		const float wallCheckHeight =
			height - WallCheckMargin * 2.0f;

		int wallX = -1;
		int wallY = -1;

		const BlockMap::CollisionType wallType =
			CheckCollision(
				blockMap,
				wallCheckX,
				wallCheckTop,
				CollisionThickness,
				wallCheckHeight,
				&wallX,
				&wallY
			);

		// Blockなら壁なので移動しない
		if (wallType == BlockMap::CollisionType::Block)
		{
			isHittingWall = true;
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

	// =========================
	// Y方向移動
	// =========================

	velocityY += gravity;

	const float nextY =
		position.y + velocityY;

	const float halfWidth = width * 0.5f;
	const float halfHeight = height * 0.5f;

	const float playerLeft =
		position.x - halfWidth;

	const float oldBottom =
		position.y + halfHeight;

	const float nextTop =
		nextY - halfHeight;

	const float nextBottom =
		nextY + halfHeight;

	// 接地状態を一旦解除
	isGrounded = false;

	// =========================
	// 落下中
	// =========================

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
				CollisionThickness,
				&collisionX,
				&collisionY
			);

		if (floorType != BlockMap::CollisionType::None)
		{
			if (floorType == BlockMap::CollisionType::Block)
			{
				const float floorY =
					static_cast<float>(collisionY);

				// Blockの上面に乗ったかどうかの判定
				if (oldBottom <= floorY || nextBottom >= floorY)
				{
					position.y =
						floorY - halfHeight;

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

	// =========================
	// 上昇中
	// =========================

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
				CollisionThickness,
				&collisionX,
				&collisionY
			);

		if (ceilingType != BlockMap::CollisionType::None)
		{
			if (ceilingType == BlockMap::CollisionType::Block)
			{
				const float ceilingY =
					static_cast<float>(collisionY);

				position.y =
					ceilingY + CollisionThickness + halfHeight;

				velocityY = 0.0f;
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

	// =========================
	// 垂直速度が0
	// =========================

	else
	{
		position.y = nextY;
	}

	// =========================
	// デバッグ描画
	// =========================

	if (moveDirection != 0.0f)
	{
		const float debugX =
			(moveDirection < 0.0f)
			? (nextX - halfWidth)
			: (nextX + halfWidth);

		const float debugTop =
			position.y - halfHeight;

		DrawBox(
			static_cast<int>(debugX),
			static_cast<int>(debugTop),
			static_cast<int>(debugX + CollisionThickness),
			static_cast<int>(debugTop + height),
			GetColor(255, 255, 0),
			FALSE
		);
	}

	if (velocityY > 0.0f)
	{
		DrawBox(
			static_cast<int>(playerLeft),
			static_cast<int>(nextBottom),
			static_cast<int>(playerLeft + width),
			static_cast<int>(nextBottom + CollisionThickness),
			GetColor(0, 255, 255),
			FALSE
		);
	}

	if (velocityY < 0.0f)
	{
		DrawBox(
			static_cast<int>(playerLeft),
			static_cast<int>(nextTop),
			static_cast<int>(playerLeft + width),
			static_cast<int>(nextTop + CollisionThickness),
			GetColor(255, 0, 255),
			FALSE
		);
	}

	if (isGrounded)
	{
		DrawFormatString(
			0,
			40,
			GetColor(255, 255, 255),
			"Grounded: ON"
		);
	}

	if (isHittingWall)
	{
		DrawFormatString(
			0,
			60,
			GetColor(255, 255, 0),
			"Wall: ON"
		);
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
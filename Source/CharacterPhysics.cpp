#include "CharacterPhysics.h"

namespace
{
	// 壁判定時に床や天井のブロックを誤検出しないように上下を縮めるマージン
	const float WallCheckMargin = 20.0f;

	// 壁や床、天井の判定を行う際の厚み（1ピクセル）
	const float CollisionThickness = 1.0f;
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
	// 浮動小数点的な座標をタイル単位の整数ピクセルに変換する
	int left = static_cast<int>(x);
	int right = static_cast<int>(x + width - 1.0f);
	int top = static_cast<int>(y);
	int bottom = static_cast<int>(y + height - 1.0f);

	// 指定した矩形内を1ピクセルずつ調べる
	for (int pixelY = top; pixelY <= bottom; pixelY++)
	{
		for (int pixelX = left; pixelX <= right; pixelX++)
		{
			// 指定ピクセルの衝突タイプを取得する
			BlockMap::CollisionType type = blockMap.GetCollisionType(pixelX, pixelY);
			if (type == BlockMap::CollisionType::None)
			{
				continue;
			}

			// 衝突したピクセル座標を保存する
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
	BlockMap::CollisionType detectedSpecialType = BlockMap::CollisionType::None;
	// デバッグ用
	bool isHittingWall = false;

	// 入力された移動方向と速度から次のX座標を算出する
	float nextX = position.x + (moveSpeed * moveDirection);

	// 左右の移動判定を共通化するため、チェック対象のX座標を決定する
	if (moveDirection != 0.0f)
	{
		float wallCheckX = (moveDirection < 0.0f) ? (nextX - width / 2.0f) : (nextX + width / 2.0f);
		float wallCheckTop = position.y - height / 2.0f + WallCheckMargin;
		float wallCheckHeight = height - WallCheckMargin * 2.0f;

		int wallX = -1;
		int wallY = -1;

		BlockMap::CollisionType wallType = CheckCollision(
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

	// 垂直速度に重力を加算する
	velocityY += gravity;

	// 重力反映後の次のY座標を算出する
	float nextY = position.y + velocityY;

	// プレイヤーの矩形情報を算出する
	float playerLeft = position.x - width / 2.0f;
	float oldBottom = position.y + height / 2.0f;
	float nextTop = nextY - height / 2.0f;
	float nextBottom = nextY + height / 2.0f;

	// 接地状態を一旦解除する
	isGrounded = false;

	// 落下中の処理
	if (velocityY > 0.0f)
	{
		int collisionX = -1;
		int collisionY = -1;

		BlockMap::CollisionType floorType = CheckCollision(
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
				float floorY = static_cast<float>(collisionY);

				// Blockの上面に乗ったかどうかの判定
				if (oldBottom <= floorY || nextBottom >= floorY)
				{
					// プレイヤーの足をBlockの上面に合わせる
					position.y = floorY - height / 2.0f;
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
	// 上昇中の処理
	else if (velocityY < 0.0f)
	{
		int collisionX = -1;
		int collisionY = -1;

		BlockMap::CollisionType ceilingType = CheckCollision(
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
				float ceilingY = static_cast<float>(collisionY);

				// プレイヤーの頭をBlockの下に配置してすり抜けを防止する
				position.y = ceilingY + CollisionThickness + height / 2.0f;
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
	// 垂直速度が0の場合
	else
	{
		position.y = nextY;
	}

	// 壁判定の描画
	if (moveDirection != 0.0f)
	{
		float debugX = (moveDirection < 0.0f) ? (nextX - width / 2.0f) : (nextX + width / 2.0f);
		float debugTop = position.y - height / 2.0f;

		DrawBox(
			static_cast<int>(debugX),
			static_cast<int>(debugTop),
			static_cast<int>(debugX + CollisionThickness),
			static_cast<int>(debugTop + height),
			GetColor(255, 255, 0),
			FALSE
		);
	}

	// 床判定の描画
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

	// 天井判定の描画
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

	// 接地状態のデバッグ描画
	if (isGrounded)
	{
		DrawFormatString(0, 40, GetColor(255, 255, 255), "Grounded: ON");
	}

	// 壁接触状態のデバッグ描画
	if (isHittingWall)
	{
		DrawFormatString(0, 60, GetColor(255, 255, 0), "Wall: ON");
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
	BlockMap::CollisionType type = CheckCollision(
		blockMap,
		x,
		y,
		width,
		height
	);

	return (type != BlockMap::CollisionType::None);
}
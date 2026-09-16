#include "CharacterPhysics.h"


CharacterPhysics::CharacterPhysics()
{
}


CharacterPhysics::~CharacterPhysics()
{
}


// ============================================================
// 矩形とマップタイルのピクセル単位当たり判定
// ============================================================
//
// 指定した矩形の中にあるCollisionTypeを調べる。
// Noneなら何もない。
// Block / Death / Goal などがあれば、そのタイプを返す。
//
// blockX / blockYには、最初に見つかった衝突ピクセルの座標を入れる。
//
// ============================================================

BlockMap::CollisionType CharacterPhysics::CheckCollision(
	const BlockMap& blockMap,
	float x,
	float y,
	float width,
	float height,
	int* blockX,
	int* blockY) const
{
	int left =
		static_cast<int>(x);

	int right =
		static_cast<int>(
			x + width - 1.0f
			);

	int top =
		static_cast<int>(y);

	int bottom =
		static_cast<int>(
			y + height - 1.0f
			);


	// --------------------------------------------------------
	// 指定した矩形内を1ピクセルずつ調べる
	// --------------------------------------------------------

	for (int pixelY = top;
		pixelY <= bottom;
		pixelY++)
	{
		for (int pixelX = left;
			pixelX <= right;
			pixelX++)
		{
			BlockMap::CollisionType type =
				blockMap.GetCollisionType(
					pixelX,
					pixelY
				);


			// ------------------------------------------------
			// 何もないなら次のピクセルへ
			// ------------------------------------------------

			if (type ==
				BlockMap::CollisionType::None)
			{
				continue;
			}


			// ------------------------------------------------
			// 衝突したピクセル座標を保存
			// ------------------------------------------------

			if (blockX != nullptr)
			{
				*blockX = pixelX;
			}

			if (blockY != nullptr)
			{
				*blockY = pixelY;
			}


			// ------------------------------------------------
			// CollisionTypeを返す
			// ------------------------------------------------

			return type;
		}
	}


	// --------------------------------------------------------
	// 何もなかった
	// --------------------------------------------------------

	return BlockMap::CollisionType::None;
}


// ============================================================
// 移動 + 重力 + マップタイルとの当たり判定
// ============================================================
//
// X方向とY方向を分けて処理する。
//
//
// X方向
// ・左移動 → 左端1pxを壁判定
// ・右移動 → 右端1pxを壁判定
//
//
// Y方向
// ・落下 → 足元1pxを床判定
// ・上昇 → 頭1pxを天井判定
//
//
//
// 戻り値
// ・None  → 特殊地形なし
// ・Death → Deathに触れた
// ・Goal  → Goalに触れた
//
// Blockは物理的に止めるだけなので、戻り値にはしない。
//
// ============================================================

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
	// ========================================================
	// 特殊地形の検出結果
	// ========================================================

	BlockMap::CollisionType detectedSpecialType =
		BlockMap::CollisionType::None;


	// ========================================================
	// デバッグ用
	// ========================================================

	bool isHittingWall = false;


	// ========================================================
	// 1. X方向の移動と壁判定
	// ========================================================

	float nextX =
		position.x +
		(moveSpeed * moveDirection);


	// --------------------------------------------------------
	// 左に移動
	// --------------------------------------------------------

	if (moveDirection < 0.0f)
	{
		// ----------------------------------------------------
		// 次の左端
		// ----------------------------------------------------

		float nextLeft =
			nextX -
			width / 2.0f;


		// ----------------------------------------------------
		// プレイヤーの上端
		// ----------------------------------------------------

		float playerTop =
			position.y -
			height / 2.0f;


		// ----------------------------------------------------
		// 左端1pxだけを調べる
		// ----------------------------------------------------

		int wallX = -1;
		int wallY = -1;


		BlockMap::CollisionType wallType =
			CheckCollision(
				blockMap,
				nextLeft,
				playerTop,
				1.0f,
				height,
				&wallX,
				&wallY
			);


		// ----------------------------------------------------
		// Blockなら壁なので移動しない
		// ----------------------------------------------------

		if (wallType ==
			BlockMap::CollisionType::Block)
		{
			isHittingWall = true;
		}
		else
		{
			// ------------------------------------------------
			// Blockではないので移動
			// ------------------------------------------------

			position.x = nextX;


			// ------------------------------------------------
			// Death / Goalなら記録
			// ------------------------------------------------

			if (wallType !=
				BlockMap::CollisionType::None)
			{
				detectedSpecialType = wallType;
			}
		}
	}


	// --------------------------------------------------------
	// 右に移動
	// --------------------------------------------------------

	else if (moveDirection > 0.0f)
	{
		// ----------------------------------------------------
		// 次の右端
		// ----------------------------------------------------

		float nextRight =
			nextX +
			width / 2.0f;


		// ----------------------------------------------------
		// プレイヤーの上端
		// ----------------------------------------------------

		float playerTop =
			position.y -
			height / 2.0f;


		// ----------------------------------------------------
		// 右端1pxだけを調べる
		// ----------------------------------------------------

		int wallX = -1;
		int wallY = -1;


		BlockMap::CollisionType wallType =
			CheckCollision(
				blockMap,
				nextRight,
				playerTop,
				1.0f,
				height,
				&wallX,
				&wallY
			);


		// ----------------------------------------------------
		// Blockなら壁なので移動しない
		// ----------------------------------------------------

		if (wallType ==
			BlockMap::CollisionType::Block)
		{
			isHittingWall = true;
		}
		else
		{
			// ------------------------------------------------
			// Blockではないので移動
			// ------------------------------------------------

			position.x = nextX;


			// ------------------------------------------------
			// Death / Goalなら記録
			// ------------------------------------------------

			if (wallType !=
				BlockMap::CollisionType::None)
			{
				detectedSpecialType = wallType;
			}
		}
	}


	// ========================================================
	// 2. 重力
	// ========================================================

	velocityY += gravity;


	// ========================================================
	// 3. 次のY座標
	// ========================================================

	float nextY =
		position.y +
		velocityY;


	// ========================================================
	// 4. プレイヤーの左右
	// ========================================================

	float playerLeft =
		position.x -
		width / 2.0f;


	// ========================================================
	// 5. 現在のプレイヤーの上下
	// ========================================================

	float oldTop =
		position.y -
		height / 2.0f;

	float oldBottom =
		position.y +
		height / 2.0f;


	// ========================================================
	// 6. 次のプレイヤーの上下
	// ========================================================

	float nextTop =
		nextY -
		height / 2.0f;

	float nextBottom =
		nextY +
		height / 2.0f;


	// ========================================================
	// 接地状態を一旦解除
	// ========================================================

	isGrounded = false;


	// ========================================================
	// 7. 床判定用の矩形
	// ========================================================
	//
	// プレイヤーの足元1pxだけを見る。
	//
	//       Player
	//      ┌───────┐
	//      │       │
	//      │       │
	//      └───────┘
	//      █████████ ← ここだけ調べる
	//
	// ========================================================

	float floorCheckLeft =
		playerLeft;

	float floorCheckTop =
		nextBottom;

	float floorCheckWidth =
		width;

	float floorCheckHeight =
		1.0f;


	// ========================================================
	// 8. 天井判定用の矩形
	// ========================================================

	float ceilingCheckLeft =
		playerLeft;

	float ceilingCheckTop =
		nextTop;

	float ceilingCheckWidth =
		width;

	float ceilingCheckHeight =
		1.0f;


	// ========================================================
	// 9. 落下中
	// ========================================================

	if (velocityY > 0.0f)
	{
		int collisionX = -1;
		int collisionY = -1;


		BlockMap::CollisionType floorType =
			CheckCollision(
				blockMap,
				floorCheckLeft,
				floorCheckTop,
				floorCheckWidth,
				floorCheckHeight,
				&collisionX,
				&collisionY
			);


		// ----------------------------------------------------
		// 何かに触れた
		// ----------------------------------------------------

		if (floorType !=
			BlockMap::CollisionType::None)
		{
			// =================================================
			// Block
			// =================================================

			if (floorType ==
				BlockMap::CollisionType::Block)
			{
				float floorY =
					static_cast<float>(
						collisionY
						);


				// ------------------------------------------------
				// Blockの上面に乗った
				// ------------------------------------------------

				if (oldBottom <= floorY ||
					nextBottom >= floorY)
				{
					// --------------------------------------------
					// プレイヤーの足を
					// Blockの上面に合わせる
					// --------------------------------------------

					position.y =
						floorY -
						height / 2.0f;


					// --------------------------------------------
					// 落下停止
					// --------------------------------------------

					velocityY = 0.0f;


					// --------------------------------------------
					// 地面に立っている
					// --------------------------------------------

					isGrounded = true;
					isJumping = false;
				}
				else
				{
					// --------------------------------------------
					// まだ床に到達していない
					// --------------------------------------------

					position.y = nextY;

					isGrounded = false;
					isJumping = true;
				}
			}


			// =================================================
			// Death / Goal
			// =================================================

			else
			{
				detectedSpecialType =
					floorType;


				// ------------------------------------------------
				// Death / Goalは物理的には止めない
				// ------------------------------------------------

				position.y = nextY;

				isGrounded = false;
				isJumping = true;
			}
		}


		// ----------------------------------------------------
		// 何もない
		// ----------------------------------------------------

		else
		{
			position.y = nextY;

			isGrounded = false;
			isJumping = true;
		}
	}


	// ========================================================
	// 10. 上昇中
	// ========================================================

	else if (velocityY < 0.0f)
	{
		int collisionX = -1;
		int collisionY = -1;


		BlockMap::CollisionType ceilingType =
			CheckCollision(
				blockMap,
				ceilingCheckLeft,
				ceilingCheckTop,
				ceilingCheckWidth,
				ceilingCheckHeight,
				&collisionX,
				&collisionY
			);


		// ----------------------------------------------------
		// 何かに触れた
		// ----------------------------------------------------

		if (ceilingType !=
			BlockMap::CollisionType::None)
		{
			// =================================================
			// Block
			// =================================================

			if (ceilingType ==
				BlockMap::CollisionType::Block)
			{
				float ceilingY =
					static_cast<float>(
						collisionY
						);


				// ------------------------------------------------
				// プレイヤーの頭を
				// Blockの下に配置
				// ------------------------------------------------

				position.y =
					ceilingY +
					1.0f +
					height / 2.0f;


				// ------------------------------------------------
				// 上昇停止
				// ------------------------------------------------

				velocityY = 0.0f;
			}


			// =================================================
			// Death / Goal
			// =================================================

			else
			{
				detectedSpecialType =
					ceilingType;


				position.y = nextY;
			}


			isGrounded = false;
			isJumping = true;
		}


		// ----------------------------------------------------
		// 何もない
		// ----------------------------------------------------

		else
		{
			position.y = nextY;

			isGrounded = false;
			isJumping = true;
		}
	}


	// ========================================================
	// 11. 垂直速度が0
	// ========================================================

	else
	{
		position.y = nextY;
	}


	// ========================================================
	// デバッグ描画
	// ========================================================
	//
	// 黄色  → 壁判定
	// 水色  → 床判定
	// ピンク → 天井判定
	//
	// ========================================================


	// --------------------------------------------------------
	// 壁判定の描画
	// --------------------------------------------------------

	if (moveDirection < 0.0f)
	{
		// 左壁

		float debugLeft =
			nextX -
			width / 2.0f;


		float debugTop =
			position.y -
			height / 2.0f;


		DrawBox(
			static_cast<int>(debugLeft),
			static_cast<int>(debugTop),
			static_cast<int>(debugLeft + 1.0f),
			static_cast<int>(debugTop + height),
			GetColor(255, 255, 0),
			FALSE
		);
	}
	else if (moveDirection > 0.0f)
	{
		// 右壁

		float debugRight =
			nextX +
			width / 2.0f;


		float debugTop =
			position.y -
			height / 2.0f;


		DrawBox(
			static_cast<int>(debugRight),
			static_cast<int>(debugTop),
			static_cast<int>(debugRight + 1.0f),
			static_cast<int>(debugTop + height),
			GetColor(255, 255, 0),
			FALSE
		);
	}


	// --------------------------------------------------------
	// 床判定の描画
	// --------------------------------------------------------

	if (velocityY > 0.0f)
	{
		DrawBox(
			static_cast<int>(floorCheckLeft),
			static_cast<int>(floorCheckTop),
			static_cast<int>(
				floorCheckLeft +
				floorCheckWidth
				),
			static_cast<int>(
				floorCheckTop +
				floorCheckHeight
				),
			GetColor(0, 255, 255),
			FALSE
		);
	}


	// --------------------------------------------------------
	// 天井判定の描画
	// --------------------------------------------------------

	if (velocityY < 0.0f)
	{
		DrawBox(
			static_cast<int>(ceilingCheckLeft),
			static_cast<int>(ceilingCheckTop),
			static_cast<int>(
				ceilingCheckLeft +
				ceilingCheckWidth
				),
			static_cast<int>(
				ceilingCheckTop +
				ceilingCheckHeight
				),
			GetColor(255, 0, 255),
			FALSE
		);
	}


	// --------------------------------------------------------
	// 接地デバッグ
	// --------------------------------------------------------

	if (isGrounded)
	{
		DrawFormatString(
			0,
			40,
			GetColor(255, 255, 255),
			"Grounded: ON"
		);
	}


	// --------------------------------------------------------
	// 壁デバッグ
	// --------------------------------------------------------

	if (isHittingWall)
	{
		DrawFormatString(
			0,
			60,
			GetColor(255, 255, 0),
			"Wall: ON"
		);
	}


	// ========================================================
	// 特殊地形の結果を返す
	// ========================================================

	return detectedSpecialType;
}


// ============================================================
// 指定した矩形が何らかのCollisionTypeに触れているか
// ============================================================
//
// 注意：
// この関数はBlockだけではなく、Death / Goalにもtrueを返す。
// 「Blockだけ」を調べたい場合は、CheckCollision()の結果を
// CollisionType::Blockと比較する。
//
// ============================================================

bool CharacterPhysics::IsBlockCollision(
	const BlockMap& blockMap,
	float x,
	float y,
	float width,
	float height) const
{
	BlockMap::CollisionType type =
		CheckCollision(
			blockMap,
			x,
			y,
			width,
			height
		);


	return (
		type !=
		BlockMap::CollisionType::None
		);
}
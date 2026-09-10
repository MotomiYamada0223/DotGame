#include "Player.h"
#include "DxLib.h"
#include "Texture.h"
#include "Master.h"
#include "SceneManager.h"
#include "Scene.h"
#include "ObjectManager.h"
#include "Enemy.h"
#include "Collision.h"

Player::Player(VECTOR initPos) 
	// TextureAnimationは使わず一枚絵としてロード
	
	: Object2D("Resource/Image/BattlePlayer111.png", initPos) 
{
	SetTag(Object2D::BattlePlayer2D);
	isJumping = false;
	isGrounded = true;
	velocityY = 0.0f;
	isFacingRight = true;
	isAttacking = false;
	attackTimer = 0;
	isHitDamage = false;

	mCurrentFrame = 0;
	mFrameTimer = 0;

  // プレイヤー当たり判定サイズ
	playerWidth = 128.0f;
	playerHeight = 256.0f;
}

Player::~Player()
{
}

void Player::PlayerMove(BlockMap& blockMap)
{
	bool moved = false;

	// 左移動 A
	if (CheckHitKey(KEY_INPUT_A) == 1)
	{
		float nextX = mvPosition.x - moveSpeed;
		float playerLeft = nextX - playerWidth / 2.0f;
		float playerTop = mvPosition.y - playerHeight / 2.0f;

		if (!blockMap.IsCollision(playerLeft, playerTop, playerWidth, playerHeight))
		{
			mvPosition.x = nextX;
			isFacingRight = false;

			// 実際に移動した時だけtrueにする
			moved = true;
		}
	}

	// 右移動 D
	if (CheckHitKey(KEY_INPUT_D) == 1)
	{
		float nextX = mvPosition.x + moveSpeed;
		float playerLeft = nextX - playerWidth / 2.0f;
		float playerTop = mvPosition.y - playerHeight / 2.0f;

		if (!blockMap.IsCollision(playerLeft, playerTop, playerWidth, playerHeight))
		{
			mvPosition.x = nextX;
			isFacingRight = true;

			// 実際に移動した時だけtrueにする
			moved = true;
		}
	}


	// ジャンプ開始 Space
	if (CheckHitKey(KEY_INPUT_SPACE) == 1 &&
		isGrounded)
	{
		isJumping = true;
		isGrounded = false;
		velocityY = jumpPower;
	}


	// ========================================
	// 縦方向の物理処理
	// ========================================

	// 重力
	velocityY += gravity;

	// 次のY座標
	float nextY = mvPosition.y + velocityY;

	// 現在のプレイヤー上下
	float oldTop =
		mvPosition.y - playerHeight / 2.0f;

	float oldBottom =
		mvPosition.y + playerHeight / 2.0f;

	// 次のプレイヤー矩形
	float playerLeft =
		mvPosition.x - playerWidth / 2.0f;

	float playerTop =
		nextY - playerHeight / 2.0f;

	// 衝突したブロック
	int blockX = -1;
	int blockY = -1;

	bool hitBlock =
		blockMap.CheckCollisionBlock(
			playerLeft,
			playerTop,
			playerWidth,
			playerHeight,
			blockX,
			blockY
		);

	// ========================================
	// ブロックに衝突した
	// ========================================
	if (hitBlock)
	{
		float blockLeft =
			blockX * CHIP_SIZE;

		float blockTop =
			blockY * CHIP_SIZE;

		float blockRight =
			blockLeft + CHIP_SIZE;

		float blockBottom =
			blockTop + CHIP_SIZE;

		// ----------------------------
		// 落下中
		// ----------------------------
		if (velocityY > 0.0f)
		{
			// 次のフレームの足元座標
			float nextBottom = nextY + playerHeight / 2.0f;

			// 落下してきて、足元がブロックの上部を通過・または接した場合
			// （移動前の足元がブロックより上にあった、あるいは十分に近かった場合）
			if (oldBottom <= blockTop || nextBottom >= blockTop)
			{
				mvPosition.y =
					blockTop - playerHeight / 2.0f;

				velocityY = 0.0f;

				isGrounded = true;
				isJumping = false;

			}
			else
			{
				// 横から当たった等の場合
				mvPosition.y = nextY;

				isGrounded = false;
				isJumping = true;

			}
		}
		// ----------------------------
		// 上昇中
		// ----------------------------
		else if (velocityY < 0.0f)
		{
			// ブロックの下面に頭が当たった
			if (oldTop >= blockBottom)
			{
				mvPosition.y =
					blockBottom + playerHeight / 2.0f;

				velocityY = 0.0f;

				isGrounded = false;
				isJumping = true;

			}
			else
			{
				// まだブロックに阻まれていないので上昇
				mvPosition.y = nextY;

				isGrounded = false;
				isJumping = true;

			}
		}
		// ----------------------------
		// velocityY == 0
		// ----------------------------
		else
		{
			mvPosition.y = nextY;
		}
	}
	// ========================================
	// ブロックに当たっていない
	// ========================================
	else
	{
		mvPosition.y = nextY;

		isGrounded = false;
		isJumping = true;
	}


	// 攻撃 F
	if (CheckHitKey(KEY_INPUT_F) == 1 &&
		!isAttacking)
	{
		isAttacking = true;

		attackTimer = attackDuration;
	}


	// 攻撃エフェクト更新
	if (isAttacking)
	{
		attackTimer--;

		if (attackTimer <= 0)
		{
			isAttacking = false;
		}
	}


	// アニメーション更新
	if (moved)
	{
		mFrameTimer++;
		if (mFrameTimer >= FRAME_INTERVAL)
		{
			mFrameTimer = 0;
			mCurrentFrame =
				(mCurrentFrame + 1) % TOTAL_FRAMES;
		}
	}
	else
	{
		mCurrentFrame = 0;

		mFrameTimer = 0;
	}
}

void Player::Update()
{
	// --- 当たり判定処理 ---
	isHitDamage = false;

	ObjectManager* objManager = Master::mpSceneManager->GetCurrentScene()->GetObjectManager();
	std::vector<Object2D*> enemyList = objManager->GetObject2DListByTag(Object2D::Enemy2D);

	// 自身の矩形
	VECTOR myPos = VGet(mvPosition.x - FRAME_WIDTH / 2.0f, mvPosition.y - FRAME_HEIGHT / 2.0f, 0.0f);
	VECTOR mySize = VGet((float)FRAME_WIDTH, (float)FRAME_HEIGHT, 0.0f);

	VECTOR atkPos = VGet(0, 0, 0);
	VECTOR atkSize = VGet(0, 0, 0);
	bool hasAttackRect = false;

	if (isAttacking)
	{
		hasAttackRect = true;
		int attackWidth = 60;
		int attackHeight = 40;
		int halfSizeX = FRAME_WIDTH / 2;
		float atkLeft;

		if (isFacingRight)
		{
			atkLeft = mvPosition.x + halfSizeX;
		}
		else
		{
			atkLeft = mvPosition.x - halfSizeX - attackWidth;
		}
		float atkTop = mvPosition.y - attackHeight / 2.0f;

		atkPos = VGet(atkLeft, atkTop, 0.0f);
		atkSize = VGet((float)attackWidth, (float)attackHeight, 0.0f);
	}

	for (Object2D* obj : enemyList)
	{
		Enemy* enemy = dynamic_cast<Enemy*>(obj);
		if (!enemy) continue;

		// 敵の矩形
		VECTOR enePos = VGet(enemy->GetPosition().x - enemy->GetSizeX() / 2.0f, enemy->GetPosition().y - enemy->GetSizeY() / 2.0f, 0.0f);
		VECTOR eneSize = VGet((float)enemy->GetSizeX(), (float)enemy->GetSizeY(), 0.0f);

		// 1. プレイヤー自身と敵の衝突判定 (ダメージで赤くする)
		if (Collision::CheckRectToRect(myPos, mySize, enePos, eneSize))
		{
			isHitDamage = true; 
		}

		// 2. 攻撃判定と敵の衝突判定 (敵を赤く光らせる)
		if (hasAttackRect)
		{
			if (Collision::CheckRectToRect(atkPos, atkSize, enePos, eneSize))
			{
				enemy->OnDamaged();
			}
		}
	}

	Object2D::Update();
}

void Player::Draw()
{
	if (mpTexture != nullptr)
	{
		const int srcX = mCurrentFrame * FRAME_WIDTH;

		// 向きに応じた基本の Y 座標を設定する
		int srcY = 768; // 右向きの画像座標

		if (!isFacingRight)
		{
			srcY = 512; // 左向きの画像座標
		}

		// ダメージ中なら赤く変色させる
		if (isHitDamage)
		{
			SetDrawBright(255, 100, 100);
		}

		// 指定の場所だけ描画する
		DrawRectGraph(
			static_cast<int>(mvPosition.x - FRAME_WIDTH / 2),
			static_cast<int>(mvPosition.y - FRAME_HEIGHT / 2),
			srcX,
			srcY,
			FRAME_WIDTH,
			FRAME_HEIGHT,
			mpTexture->GetHandle(),
			true
		);

		// 色を元に戻す
		if (isHitDamage)
		{
			SetDrawBright(255, 255, 255);
		}
	}
	else
	{
		Object2D::Draw();
	}

	// 攻撃エフェクトの描画
	if (isAttacking)
	{
		int attackWidth = 60;
		int attackHeight = 40;
		int rectLeft, rectTop, rectRight, rectBottom;
		int halfSizeX = FRAME_WIDTH / 2;

		if (isFacingRight)
		{
			rectLeft = static_cast<int>(mvPosition.x) + halfSizeX;
			rectRight = rectLeft + attackWidth;
		}
		else
		{
			rectLeft = static_cast<int>(mvPosition.x) - halfSizeX - attackWidth;
			rectRight = rectLeft + attackWidth;
		}

		rectTop = static_cast<int>(mvPosition.y) - attackHeight / 2;
		rectBottom = rectTop + attackHeight;

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
		DrawBox(rectLeft, rectTop, rectRight, rectBottom, GetColor(255, 50, 50), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// ブロックマップとの当たり判定デバッグ表示
	int left = static_cast<int>(mvPosition.x - playerWidth / 2.0f);
	int top = static_cast<int>(mvPosition.y - playerHeight / 2.0f);
	int right = static_cast<int>(mvPosition.x + playerWidth / 2.0f);
	int bottom = static_cast<int>(mvPosition.y + playerHeight / 2.0f);
	DrawBox(left, top, right, bottom, GetColor(255, 0, 0), FALSE);
}
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
	velocityY = 0.0f;
	groundY = initPos.y; 
	isFacingRight = true;
	isAttacking = false;
	attackTimer = 0;
	isHitDamage = false;

	mCurrentFrame = 0;
	mFrameTimer = 0;
}

Player::~Player()
{
}

void Player::Update()
{
	bool moved = false;

	// 左右移動 (A, D)
	
	if (CheckHitKey(KEY_INPUT_A) == 1)
	{
		mvPosition.x -= moveSpeed;
		isFacingRight = false;
		moved = true;
	}
	if (CheckHitKey(KEY_INPUT_D) == 1)
	{
		mvPosition.x += moveSpeed;
		isFacingRight = true;
		moved = true;
	}

	// ジャンプ (Space)
	
	if (CheckHitKey(KEY_INPUT_SPACE) == 1 && !isJumping)
	{
		isJumping = true;
		velocityY = jumpPower;
		moved = true;
	}

	// ジャンプ中の処理
	
	if (isJumping)
	{
		velocityY += gravity;
		mvPosition.y += velocityY;

		// 着地判定
		
		if (mvPosition.y >= groundY)
		{
			mvPosition.y = groundY;
			isJumping = false;
			velocityY = 0.0f;
		}
		moved = true;
	}

	// 攻撃 (F)
	
	if (CheckHitKey(KEY_INPUT_F) == 1 && !isAttacking)
	{
		isAttacking = true;
		attackTimer = attackDuration;
	}

	// 攻撃エフェクトの更新
	
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
			mCurrentFrame = (mCurrentFrame + 1) % TOTAL_FRAMES;
		}
	}
	else
	{
		mCurrentFrame = 0;
		mFrameTimer = 0;
	}

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
		
		// 初期画像を右向きのにしている
		
		int srcY = 768; 

		// 向きに応じた Y 座標（上下左右で変える）
		
		if (CheckHitKey(KEY_INPUT_A) && isJumping) srcY = 512;
		else if (isJumping) srcY = 768;
		else if (CheckHitKey(KEY_INPUT_A)) srcY = 512;
		else if (CheckHitKey(KEY_INPUT_D)) srcY = 768;
		else if (!isFacingRight) srcY = 512; 

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
}

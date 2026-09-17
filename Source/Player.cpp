#include "Player.h"
#include "DxLib.h"
#include "Texture.h"
#include "Master.h"
#include "SceneManager.h"
#include "Scene.h"
#include "ObjectManager.h"
#include "Enemy.h"
#include "Collision.h"
#include "GameConstants.h"
#include "BlockAction.h"


Player::Player(VECTOR initPos)
// TextureAnimationは使わず一枚絵としてロード
	: Object2D(CharacterGraphPath::PlayerAnimation, initPos)
{
	SetTag(Object2D::Player2D);
	mbIsJumping = false;
	isGrounded = true;
	velocityY = 0.0f;
	isFacingRight = true;
	isAttacking = false;
	attackTimer = 0;
	isHitDamage = false;
	isDead = false;
	deadTimer = 0;
	alpha = 255.0f;
	mSpawnPos = initPos;
	mDeadState = 0;
	mpBlockMap = nullptr;
	mStatus.hp = PlayerConstants::MaxHp;

	mCurrentFrame = 0;
	mFrameTimer = 0;

	// 画像の読み込み
	mHeartFullGraph = LoadGraph(CharacterGraphPath::HeartFull.c_str());
	if (mHeartFullGraph == -1) { printfDx("体力MAXの画像がない。"); }
	mHeartHalfGraph = LoadGraph(CharacterGraphPath::HeartHalf.c_str());
	if (mHeartHalfGraph == -1) { printfDx("体力半分の画像がない。"); }
	mHeartEmptyGraph = LoadGraph(CharacterGraphPath::HeartEmpty.c_str());
	if (mHeartEmptyGraph == -1) { printfDx("体力0の画像がない。"); }


	// プレイヤー当たり判定サイズ
	// Width...幅
	// Height...足元の位置
	mfPlayerWidth = PlayerConstants::PlayerCollisionWidth;
	mfPlayerHeight = PlayerConstants::PlayerCollisionHeight;
}

Player::~Player()
{
	DeleteGraph(mHeartFullGraph);
	DeleteGraph(mHeartHalfGraph);
	DeleteGraph(mHeartEmptyGraph);
}

void Player::UpdateStatusByProgress(GameProgress progress)
{
	switch (progress)
	{
	case GameProgress::Tutorial1:
		mMaxHp = 15; mHp = 15;
		mAttack = 5; mDefense = 5; mMagic = 5;
		mIsBlinkWallDeathImmune = false;
		break;
	case GameProgress::PostTutorial1:
	case GameProgress::Tutorial2:
		mMaxHp = 30; mHp = 30;
		mAttack = 15; mDefense = 15; mMagic = 15;
		mIsBlinkWallDeathImmune = true;
		break;
	case GameProgress::PostTutorial2:
	case GameProgress::Tutorial3:
		mMaxHp = 90; mHp = 90;
		mAttack = 45; mDefense = 45; mMagic = 45;
		mIsBlinkWallDeathImmune = true;
		mAttackReachLevel = 1;
		mIsAttackFlashy = true;
		mBlinkCooldownLevel = 1;
		mIsPoisonImmune = true;
		mIsPetrificationImmune = true;
		break;
	case GameProgress::PostTutorial3:
	case GameProgress::BossTree:
	case GameProgress::BossSnake:
	case GameProgress::BossDragon:
		mMaxHp = 450; mHp = 450;
		mAttack = 250; mDefense = 250; mMagic = 250;
		mIsBlinkWallDeathImmune = true;
		mAttackReachLevel = 2;
		mIsAttackFlashy = true;
		mBlinkCooldownLevel = 1;
		mIsPoisonImmune = true;
		mIsPetrificationImmune = true;
		mIsFireImmune = true;
		break;
	}
}

void Player::PlayerMove(BlockMap& blockMap)
{
	mpBlockMap = &blockMap;
	// 死亡時は操作を受け付けない
	if (isDead)
	{
		return;
	}

	// 左右移動
	float moveDirection = 0.0f;
	if (CheckHitKey(KEY_INPUT_A) == 1)
	{
		moveDirection = -1.0f;
		isFacingRight = false;
	}
	else if (CheckHitKey(KEY_INPUT_D) == 1)
	{
		moveDirection = 1.0f;
		isFacingRight = true;
	}


	// キャラクターの物理処理
	BlockMap::CollisionType collisionType =
		mCharacterPhysics.UpdateMoveAndCollision(
			mvPosition,
			velocityY,
			isGrounded,
			mbIsJumping,
			blockMap,
			mfPlayerWidth,
			mfPlayerHeight,
			gravity,
			moveSpeed,
			moveDirection
		);
	// 特殊地形の処理
	mBlockAction.SetCollisionType(collisionType);
	mBlockAction.ExecuteDeath(mStatus.hp);
	mBlockAction.ExecuteGoal();



	// ジャンプ開始
	if (CheckHitKey(KEY_INPUT_SPACE) == 1 &&
		isGrounded)
	{
		mbIsJumping = true;
		isGrounded = false;
		velocityY = jumpPower;
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
	if (moveDirection != 0.0f)
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
	if (isDead)
	{
		DeadProcess();
		Object2D::Update();
		return;
	}

	// --- 当たり判定処理 ---
	isHitDamage = false;

	ObjectManager* objManager = Master::mpGameManager->GetSceneManager()->GetCurrentScene()->GetObjectManager();
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
		int attackWidth = PlayerConstants::PlayerAttackWidth;
		int attackHeight = PlayerConstants::PlayerAttackHeight;
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

	// デバッグ用: Kキーで5ダメージ
	static bool kKeyWasDown = false;
	bool kKeyIsDown = (CheckHitKey(KEY_INPUT_K) == 1);
	if (kKeyIsDown && !kKeyWasDown && !isDead)
	{
		TakeDamage(5);
	}

	kKeyWasDown = kKeyIsDown;

	if (mHp <= 0 && !isDead)
	{
		isDead = true;
		mDeadState = 1; // SCATTER
		deadTimer = 0;
		mFragments.clear();

		int fragSize = 16;
		int srcBaseY = isFacingRight ? 384 : 256;

		for (int y = 0; y < FRAME_HEIGHT; y += fragSize)
		{
			for (int x = 0; x < FRAME_WIDTH; x += fragSize)
			{
				PlayerFragment frag;
				frag.pos.x = mvPosition.x - (FRAME_WIDTH / 2.0f) + (float)x;
				frag.pos.y = mvPosition.y - (FRAME_HEIGHT / 2.0f) + (float)y;
				frag.srcX = x;
				frag.srcY = srcBaseY + y;
				frag.width = fragSize;
				frag.height = fragSize;

				frag.vel.x = ((float)GetRand(100) / 100.0f * 10.0f) - 5.0f;
				frag.vel.y = ((float)GetRand(100) / 100.0f * -15.0f) - 5.0f;

				mFragments.push_back(frag);
			}
		}
	}

	Object2D::Update();
}


void Player::Draw()
{
	if (isDead)
	{
		SetDrawBright(255, 255, 255); // 色をリセット
		for (const auto& frag : mFragments)
		{
			if (mpTexture != nullptr)
			{
				DrawRectGraph(
					static_cast<int>(frag.pos.x),
					static_cast<int>(frag.pos.y),
					frag.srcX + (mCurrentFrame * FRAME_WIDTH),
					frag.srcY,
					frag.width,
					frag.height,
					mpTexture->GetHandle(),
					true
				);
			}
			else
			{
				// テクスチャが無い場合の保険
				DrawBox(
					static_cast<int>(frag.pos.x),
					static_cast<int>(frag.pos.y),
					static_cast<int>(frag.pos.x + frag.width),
					static_cast<int>(frag.pos.y + frag.height),
					GetColor(255, 0, 0),
					TRUE
				);
			}
		}
		return;
	}



	if (mpTexture != nullptr)
	{
		const int srcX = mCurrentFrame * FRAME_WIDTH;

		// 向きに応じた基本の Y 座標を設定する
		int srcY = 384; // 右向きの画像座標

		if (!isFacingRight)
		{
			srcY = 256; // 左向きの画像座標
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

	// UI（ハート）の描画（右上）
	if (mMaxHp > 0)
	{
		int hpLevel = (mHp * 6) / mMaxHp;
		if (mHp > 0 && hpLevel == 0) hpLevel = 1;

		int drawX = ScreenSize::ScrrenWidth - 150; // 右上に配置
		int drawY = 20;
		for (int i = 0; i < 3; i++)
		{
			int heartState = hpLevel - (i * 2);
			int graph = mHeartEmptyGraph;
			if (heartState >= 2) graph = mHeartFullGraph;
			else if (heartState == 1) graph = mHeartHalfGraph;

			if (graph != -1)
			{
				DrawGraph(drawX + i * 40, drawY, graph, TRUE);
			}
		}
	}


}



// デバッグ表示をしている関数
// GameSceneで呼び出している
void Player::DebugDraw()
{
	// レイヤー自身の当たり判定のデバッグ表示（青色）
	int playerLeft = static_cast<int>(mvPosition.x - mfPlayerWidth / 2.0f);
	int playerTop = static_cast<int>(mvPosition.y - mfPlayerHeight / 2.0f);
	int playerRight = static_cast<int>(mvPosition.x + mfPlayerWidth / 2.0f);
	int playerBottom = static_cast<int>(mvPosition.y + mfPlayerHeight / 2.0f);
	DrawBox(playerLeft, playerTop, playerRight, playerBottom, GetColor(0, 0, 255), FALSE);

	// HPのデバッグ
	DrawFormatString((int)mvPosition.x, (int)mvPosition.y + 10, ColorOption::White, "HP: %d", mStatus.hp);


	// シーン上のすべての敵の当たり判定をデバッグ表示（黄緑色）
	ObjectManager* objManager = Master::mpGameManager->GetSceneManager()->GetCurrentScene()->GetObjectManager();
	if (objManager != nullptr)
	{
		std::vector<Object2D*> enemyList = objManager->GetObject2DListByTag(Object2D::Enemy2D);
		for (Object2D* obj : enemyList)
		{
			Enemy* enemy = dynamic_cast<Enemy*>(obj);
			if (!enemy) continue;

			int eneLeft = static_cast<int>(enemy->GetPosition().x - enemy->GetSizeX() / 2.0f);
			int eneTop = static_cast<int>(enemy->GetPosition().y - enemy->GetSizeY() / 2.0f);
			int eneRight = static_cast<int>(eneLeft + enemy->GetSizeX());
			int eneBottom = static_cast<int>(eneTop + enemy->GetSizeY());

			DrawBox(eneLeft, eneTop, eneRight, eneBottom, GetColor(0, 255, 0), FALSE);
		}
	}
}


void Player::DeadProcess()
{
	deadTimer++;

	if (mDeadState == 1) // 飛び散り
	{
		for (auto& frag : mFragments)
		{
			frag.pos.x += frag.vel.x;

			if (mpBlockMap &&
				mCharacterPhysics.IsBlockCollision(
					*mpBlockMap,
					frag.pos.x,
					frag.pos.y,
					static_cast<float>(frag.width),
					static_cast<float>(frag.height)))
			{
				frag.pos.x -= frag.vel.x;
				frag.vel.x *= -0.6f;
			}

			frag.pos.y += frag.vel.y;

			if (mpBlockMap &&
				mCharacterPhysics.IsBlockCollision(
					*mpBlockMap,
					frag.pos.x,
					frag.pos.y,
					static_cast<float>(frag.width),
					static_cast<float>(frag.height)))
			{
				frag.pos.y -= frag.vel.y;
				frag.vel.y *= -0.4f;
				frag.vel.x *= 0.9f; // 摩擦
			}
		}

		if (deadTimer > 150) // 2.5秒経過
		{
			mDeadState = 2; // 戻り
			deadTimer = 0;
			for (auto& frag : mFragments)
			{
				frag.vel.y = -5.0f - ((float)GetRand(50) / 10.0f);
				frag.vel.x = ((float)GetRand(100) / 100.0f * 4.0f) - 2.0f;
			}
		}
	}
	else if (mDeadState == 2) // 戻り
	{
		bool allReturned = true;

		for (auto& frag : mFragments)
		{
			float targetX = mSpawnPos.x - (FRAME_WIDTH / 2.0f) + frag.srcX;
			float targetY = mSpawnPos.y - (FRAME_HEIGHT / 2.0f) + (frag.srcY % FRAME_HEIGHT);

			float dx = targetX - frag.pos.x;
			float dy = targetY - frag.pos.y;
			float dist = sqrtf(dx * dx + dy * dy);

			if (dist > 2.0f)
			{
				frag.pos.x += dx * 0.05f;
				frag.pos.y += dy * 0.05f;
				allReturned = false;
			}
			else
			{
				frag.pos.x = targetX;
				frag.pos.y = targetY;
			}
		}

		// 全て集まったか、タイムアウト（5秒）で強制復活
		if ((allReturned && deadTimer > 60) || deadTimer > 300)
		{
			mvPosition = mSpawnPos;
			isDead = false;
			mDeadState = 0;
			deadTimer = 0;
			isFacingRight = true;
			mFragments.clear();
			mHp = mMaxHp; // 復活時にHPをリセット
		}
	}
}

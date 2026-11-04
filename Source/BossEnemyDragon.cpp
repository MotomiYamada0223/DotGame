#include "BossEnemyDragon.h"
#include "GameConstants.h"
#include "FontManager.h"
#include "DxLib.h"
#include "BlockMap.h"
#include "Master.h"
#include "GameManager.h"
#include "SceneManager.h"
#include "GameScene.h"
#include "Player.h"
#include <cmath>
#include <cstdio>

BossEnemyDragon::BossEnemyDragon(VECTOR initPos)
	: Enemy("", initPos)
	, mState(State::Idle)
	, mAnimFrame(0)
	, mAnimTimer(0)
	, mStateTimer(0)
	, mAttackPhase(0)
	, mAttackCooldown(60)
	, mFlightCooldown(180)
	, mEntranceTimer(0)
	, mPostEntranceTimer(0)
	, mHasBrokenCeiling(false)
	, mEntranceLandAnimDone(false)
	, mFacingDirection(-1.0f)
	, mDrawScale(1.6f)
	, mHoverTargetY(1480.0f)
	, mSmoothHp(80.0f)
	, mHpShakeTimer(0)
{
	UpdateStatusByProgress(GameProgress::Tutorial1);

	// 1. 待機アニメーション (4コマ: sprite_idle1.png ~ sprite_idle4.png)
	for (int i = 1; i <= 4; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_idle/sprite_idle%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mIdleAnim.push_back(h);
	}

	// 2. 移動アニメーション (4コマ: sprite_move1.png ~ sprite_move4.png)
	for (int i = 1; i <= 4; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_move/sprite_move%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mMoveAnim.push_back(h);
	}

	// 3. 攻撃アニメーション (9コマ: sprite_attack1.png ~ sprite_attack9.png)
	for (int i = 1; i <= 9; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_attack/sprite_attack%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mAttackAnim.push_back(h);
	}

	// 4. 離陸アニメーション (7コマ: sprite_takeoff1.png ~ sprite_takeoff7.png)
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_takeoff/sprite_takeoff%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mTakeoffAnim.push_back(h);
	}

	// 5. 飛行アニメーション (7コマ: sprite_fly1.png ~ sprite_fly7.png)
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_flying/sprite_fly%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mFlyAnim.push_back(h);
	}

	// 6. 空中攻撃アニメーション (7コマ: sprite_airAttack1.png ~ sprite_airAttack7.png)
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_airborne/sprite_airAttack%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mAirAttackAnim.push_back(h);
	}

	// 7. 着地アニメーション (7コマ: sprite_landing1.png ~ sprite_landing7.png)
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_landing/sprite_landing%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mLandingAnim.push_back(h);
	}

	// 敵の当たり判定サイズの設定
	mfEnemyWidth = 220.0f;
	mfEnemyHeight = 200.0f;
	moveSpeed = 0.0f;
}

BossEnemyDragon::~BossEnemyDragon()
{
	for (int h : mIdleAnim) if (h != -1) DeleteGraph(h);
	for (int h : mMoveAnim) if (h != -1) DeleteGraph(h);
	for (int h : mAttackAnim) if (h != -1) DeleteGraph(h);
	for (int h : mTakeoffAnim) if (h != -1) DeleteGraph(h);
	for (int h : mFlyAnim) if (h != -1) DeleteGraph(h);
	for (int h : mAirAttackAnim) if (h != -1) DeleteGraph(h);
	for (int h : mLandingAnim) if (h != -1) DeleteGraph(h);
}

void BossEnemyDragon::UpdateStatusByProgress(GameProgress progress)
{
	mMaxHp = 80;
	mHp = 80;
	mAttack = 2;
	mDefense = 5;
	mMagic = 10;
	mSmoothHp = 80.0f;
}

void BossEnemyDragon::StartEntrance()
{
	mState = State::EntranceDrop;
	mEntranceTimer = 0;
	mPostEntranceTimer = 0;
	mHasBrokenCeiling = false;
	mEntranceLandAnimDone = false;
	mAnimFrame = 0;
	mAnimTimer = 0;
	isGrounded = false;
	velocityY = 6.0f;
	mFireProjectiles.clear();
	mBurningGrounds.clear();
}

void BossEnemyDragon::TakeDamage(int damage)
{
	if (mState == State::EntranceDrop || mState == State::EntranceWait) return;
	mHp -= damage;
	if (mHp < 0) mHp = 0;
	mHpShakeTimer = 15;
}

bool BossEnemyDragon::IsAttacking() const
{
	if (mState == State::Attack)
	{
		return ((mAttackPhase == 2 || mAttackPhase == 4) && mAnimFrame >= 3 && mAnimFrame <= 6);
	}
	return false;
}

void BossEnemyDragon::GetAttackHitbox(float& left, float& top, float& right, float& bottom) const
{
	float reach = (mState == State::AirAttack) ? 190.0f : 170.0f;
	if (mFacingDirection < 0.0f)
	{
		// 左向き攻撃
		right = mvPosition.x - 10.0f;
		left = mvPosition.x - (mfEnemyWidth * 0.5f + reach);
	}
	else
	{
		// 右向き攻撃
		left = mvPosition.x + 10.0f;
		right = mvPosition.x + (mfEnemyWidth * 0.5f + reach);
	}

	if (mState == State::AirAttack)
	{
		top = mvPosition.y - mfEnemyHeight * 0.3f;
		bottom = mvPosition.y + mfEnemyHeight * 0.65f;
	}
	else
	{
		top = mvPosition.y - mfEnemyHeight * 0.45f;
		bottom = mvPosition.y + mfEnemyHeight * 0.48f;
	}
}

void BossEnemyDragon::Update()
{
	Enemy::Update();

	if (mHpShakeTimer > 0) mHpShakeTimer--;
	if (mSmoothHp < 0.0f) mSmoothHp = static_cast<float>(mHp);
	if (mSmoothHp > mHp)
	{
		mSmoothHp -= 1.0f;
		if (mSmoothHp < mHp) mSmoothHp = static_cast<float>(mHp);
	}
	else if (mSmoothHp < mHp)
	{
		mSmoothHp = static_cast<float>(mHp);
	}

	GameScene* gameScene = nullptr;
	if (Master::mpGameManager && Master::mpGameManager->GetSceneManager())
	{
		gameScene = dynamic_cast<GameScene*>(Master::mpGameManager->GetSceneManager()->GetCurrentScene());
	}

	// 死亡時の点滅処理
	if (mHp <= 0)
	{
		if (!isDeadBlinking)
		{
			isDeadBlinking = true;
			deadBlinkTimer = 120;
			if (gameScene)
			{
			}
		}
		deadBlinkTimer--;
		if (deadBlinkTimer <= 0)
		{
			SetDeleteFlag(true);
		}
		return;
	}

	Player* player = (gameScene != nullptr) ? gameScene->GetPlayer() : nullptr;

	// =========================================================
	// 演出1: 天井を破り落下 (EntranceDrop)
	// =========================================================
	if (mState == State::EntranceDrop)
	{
		if (player)
		{
			float dx = player->GetPosition().x - mvPosition.x;
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;
		}

		// 空中降下フレーム（着地アニメーション前の 0..2）
		mAnimTimer++;
		if (mAnimTimer >= 6)
		{
			mAnimTimer = 0;
			if (mAnimFrame < 2) mAnimFrame++;
		}

		// 地面に着地したら EntranceWait へ移行し、着地アニメーション後半（衝撃エフェクト 3..6）を再生
		if (isGrounded)
		{
			mState = State::EntranceWait;
			mEntranceTimer = 240; // キャラクタークライマックスと同様の4秒間
			mEntranceLandAnimDone = false;
			mAnimFrame = 3;
			mAnimTimer = 0;
			if (gameScene)
			{
			}
		}
		return;
	}

	// =========================================================
	// 演出2: 着地後の咆哮・ライト表示待機 (EntranceWait)
	// =========================================================
	if (mState == State::EntranceWait)
	{
		mAnimTimer++;
		if (!mEntranceLandAnimDone)
		{
			if (mAnimTimer >= 7)
			{
				mAnimTimer = 0;
				mAnimFrame++;
				if (mAnimFrame >= static_cast<int>(mLandingAnim.size()))
				{
					mEntranceLandAnimDone = true;
					mAnimFrame = 0;
				}
			}
		}
		else
		{
			if (mAnimTimer >= 10 && !mIdleAnim.empty())
			{
				mAnimTimer = 0;
				mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mIdleAnim.size());
			}
		}


		if (mEntranceTimer > 0) mEntranceTimer--;
		if (mEntranceTimer <= 0)
		{
			mState = State::Idle;
			mStateTimer = 0;
			mAnimFrame = 0;
			mAnimTimer = 0;
			mPostEntranceTimer = 60;
		}
		return;
	}

	if (mPostEntranceTimer > 0) mPostEntranceTimer--;
	if (mAttackCooldown > 0) mAttackCooldown--;
	if (mFlightCooldown > 0) mFlightCooldown--;

	// 火炎ブレスと地面の炎（8秒間持続ダメージ）の更新
	UpdateFireAndBurningGround(player);

	// ボス部屋の行動範囲制限 (X: 8080 ~ 9680)
	const float roomMinX = 8080.0f;
	const float roomMaxX = 9680.0f;

	float px = (player != nullptr) ? player->GetPosition().x : mvPosition.x;
	float dx = px - mvPosition.x;
	float absDx = std::fabs(dx);

	switch (mState)
	{
	case State::Idle:
	{
		mStateTimer++;
		mAnimTimer++;
		if (mAnimTimer >= 10 && !mIdleAnim.empty())
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mIdleAnim.size());
		}

		if (player && !player->GetIsDead())
		{
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;

			// 飛行クールダウンが明けたら離陸アニメーションへ移行
			if (mFlightCooldown <= 0 && mStateTimer >= 35)
			{
				mState = State::Takeoff;
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
				break;
			}

			// 通常アタックの距離圏内なら、攻撃へ移行
			if (absDx <= 360.0f && mAttackCooldown <= 0)
			{
				mState = State::Attack;
				mAttackPhase = 0; // Phase 0: 頭上に「！」マーク表示
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
				break;
			}

			// 一定時間待機したら移動開始
			if (mStateTimer >= 40)
			{
				mState = State::Move;
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
			}
		}
		break;
	}

	case State::Move:
	{
		// 通常の移動・アニメーション速度は現状のまま
		mStateTimer++;
		mAnimTimer++;
		if (mAnimTimer >= 8 && !mMoveAnim.empty())
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mMoveAnim.size());
		}

		if (player && !player->GetIsDead())
		{
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;

			// 飛行クールダウンが満ちていたら離脱して飛行(Takeoff)へ
			if (mFlightCooldown <= 0 && mStateTimer >= 35)
			{
				mState = State::Takeoff;
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
				break;
			}

			// 攻撃距離圏内に入ったら通常アタック(Attack)へ
			if (absDx <= 350.0f && mAttackCooldown <= 0)
			{
				mState = State::Attack;
				mAttackPhase = 0; // Phase 0: 頭上に「！」マーク表示
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
				break;
			}
		}

		float walkSpeed = 4.2f;
		mvPosition.x += mFacingDirection * walkSpeed;
		if (mvPosition.x < roomMinX) { mvPosition.x = roomMinX; mFacingDirection = 1.0f; }
		if (mvPosition.x > roomMaxX) { mvPosition.x = roomMaxX; mFacingDirection = -1.0f; }

		if (mStateTimer >= 110)
		{
			mState = State::Idle;
			mStateTimer = 0;
			mAnimFrame = 0;
			mAnimTimer = 0;
		}
		break;
	}

	case State::Attack:
	{
		// 通常アタック:
		// Phase 0: 頭の上に「！」マークを出して予兆する
		// Phase 1: 腕を前に突っ込み前方へ（5フレームでステップ約75px前方に飛び出る）
		// Phase 2: 1回目の攻撃（その場で爪を振り下ろす）
		// Phase 3: 吠えて前に突っ込み前方へ（5フレームでステップ約75px前方に飛び出る）
		// Phase 4: 2回目の攻撃
		mStateTimer++;
		mAnimTimer++;

		if (mAttackPhase == 0)
		{
			// 頭上に「！」マークを表示しながら待機
			if (mAnimTimer >= 8 && !mIdleAnim.empty())
			{
				mAnimTimer = 0;
				mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mIdleAnim.size());
			}
			if (mStateTimer >= 32)
			{
				mAttackPhase = 1;
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
			}
		}
		else if (mAttackPhase == 1 || mAttackPhase == 3)
		{
			// 腕を前に突っ込み前進（15px × 5フレーム = 75px 分のステップ）
			if (mAnimTimer >= 2 && !mMoveAnim.empty())
			{
				mAnimTimer = 0;
				mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mMoveAnim.size());
			}

			mvPosition.x += mFacingDirection * 15.0f;
			if (mvPosition.x < roomMinX) mvPosition.x = roomMinX;
			if (mvPosition.x > roomMaxX) mvPosition.x = roomMaxX;

			if (mStateTimer >= 5)
			{
				mAttackPhase++; // Phase 1 -> 2 (1回目攻撃), Phase 3 -> 4 (2回目攻撃)
				mStateTimer = 0;
				mAnimFrame = 2; // 溜めフレームから振り下ろし攻撃へ
				mAnimTimer = 0;
			}
		}
		else if (mAttackPhase == 2 || mAttackPhase == 4)
		{
			// 攻撃モーション再生（前進はせず、攻撃判定はその場で爪を振り下ろす）
			if (mAnimTimer >= 4)
			{
				mAnimTimer = 0;
				mAnimFrame++;
				int endFrame = (mAttackPhase == 2) ? 7 : static_cast<int>(mAttackAnim.size());
				if (mAnimFrame >= endFrame)
				{
					mAnimFrame = 0;
					mStateTimer = 0;
					if (mAttackPhase == 2)
					{
						// 1回目の攻撃終了 -> Phase 3（2回目の前進予兆）へ
						mAttackPhase = 3;
					}
					else
					{
						// 2回目の攻撃終了 -> Idleへ戻る
						mAttackPhase = 0;
						mState = State::Idle;
						mAttackCooldown = 70;
					}
				}
			}
		}
		break;
	}

	case State::Takeoff:
	{
		// 離陸アニメーション（sprite_takeoff1 ~ 7）
		mStateTimer++;
		mAnimTimer++;
		if (player && !player->GetIsDead())
		{
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;
		}

		// フレーム2以降で羽ばたいて空中へ浮上
		if (mAnimFrame >= 2)
		{
			mvPosition.y -= 6.5f;
			if (mvPosition.y < mHoverTargetY) mvPosition.y = mHoverTargetY;
		}

		if (mAnimTimer >= 6)
		{
			mAnimTimer = 0;
			mAnimFrame++;
			if (mAnimFrame >= static_cast<int>(mTakeoffAnim.size()))
			{
				mState = State::Fly;
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
			}
		}
		break;
	}

	case State::Fly:
	{
		// 飛行中アニメーション（sprite_fly1 ~ 7）
		mStateTimer++;
		mAnimTimer++;
		if (mAnimTimer >= 6 && !mFlyAnim.empty())
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mFlyAnim.size());
		}

		// 浮遊の高さを緩やかに維持しつつ、羽をばたつかせながら斜め上の間隔（約280～420px）へ移動
		float hoverWave = std::sin(mStateTimer * 0.1f) * 2.5f;
		mvPosition.y += (mHoverTargetY - mvPosition.y) * 0.12f + hoverWave;

		if (player && !player->GetIsDead())
		{
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;
			if (absDx > 400.0f)
			{
				mvPosition.x += mFacingDirection * 6.0f;
			}
			else if (absDx < 220.0f)
			{
				mvPosition.x -= mFacingDirection * 4.5f;
			}
		}
		if (mvPosition.x < roomMinX) mvPosition.x = roomMinX;
		if (mvPosition.x > roomMaxX) mvPosition.x = roomMaxX;

		// 一定時間飛行したら空中火炎ブレス攻撃（AirAttack）へ移行
		if (mStateTimer >= 65)
		{
			mState = State::AirAttack;
			mStateTimer = 0;
			mAnimFrame = 0;
			mAnimTimer = 0;
			if (player && !player->GetIsDead())
			{
				mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;
			}
		}
		break;
	}

	case State::AirAttack:
	{
		// 空中攻撃アニメーション（sprite_airAttack1 ~ 7）で空中に留まりながらプレイヤーに向けて火炎を放つ
		mStateTimer++;
		mAnimTimer++;

		// 空中で軽く浮遊を維持
		float hoverWave = std::sin(mStateTimer * 0.12f) * 1.5f;
		mvPosition.y += (mHoverTargetY - mvPosition.y) * 0.1f + hoverWave;

		if (player && !player->GetIsDead() && mAnimFrame <= 3)
		{
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;
		}

		// フレーム3～5（口を大きく開いて火炎をデモンストレーションする）の間、連続してプレイヤーに向かって火炎弾を発射
		if (mAnimFrame >= 3 && mAnimFrame <= 5)
		{
			if (mStateTimer % 4 == 0 && player)
			{
				float mouthX = mvPosition.x + mFacingDirection * (115.0f * mDrawScale);
				float mouthY = mvPosition.y - 110.0f;

				float targetX = player->GetPosition().x + static_cast<float>((GetRand(90) - 45));
				float targetY = 1903.0f; // プレイヤーの足元の地面まで届くように狙う
				float fdx = targetX - mouthX;
				float fdy = targetY - mouthY;
				if (fdy < 80.0f) fdy = 80.0f;
				float len = std::sqrt(fdx * fdx + fdy * fdy);
				if (len < 1.0f) len = 1.0f;

				float speed = 14.5f;
				FireProjectile fp;
				fp.x = mouthX;
				fp.y = mouthY;
				fp.vx = (fdx / len) * speed;
				fp.vy = (fdy / len) * speed;
				fp.radius = 24.0f;
				fp.animTick = 0;
				fp.active = true;
				mFireProjectiles.push_back(fp);
			}
		}

		// 火炎を放っているフレーム（3～5）は少し長めに維持してから次の動作へ
		int frameDuration = (mAnimFrame >= 3 && mAnimFrame <= 5) ? 12 : 6;
		if (mAnimTimer >= frameDuration)
		{
			mAnimTimer = 0;
			mAnimFrame++;
			if (mAnimFrame >= static_cast<int>(mAirAttackAnim.size()))
			{
				mState = State::Landing;
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
				velocityY = 4.0f;
			}
		}
		break;
	}

	case State::Landing:
	{
		// 着地アニメーション（sprite_landing1 ~ 7）
		mStateTimer++;
		mAnimTimer++;

		if (!isGrounded)
		{
			// 着地するまでのフレーム 0～2で降下
			if (mAnimTimer >= 6)
			{
				mAnimTimer = 0;
				if (mAnimFrame < 2) mAnimFrame++;
			}
		}
		else
		{
			// 着地後のフレーム 3～6（衝撃エフェクト付与など）を再生
			if (mAnimFrame < 3)
			{
				mAnimFrame = 3;
				mAnimTimer = 0;
			}
			if (mAnimTimer >= 6)
			{
				mAnimTimer = 0;
				mAnimFrame++;
				if (mAnimFrame >= static_cast<int>(mLandingAnim.size()))
				{
					mState = State::Idle;
					mStateTimer = 0;
					mAnimFrame = 0;
					mAnimTimer = 0;
					mFlightCooldown = 210;
					mAttackCooldown = 45;
				}
			}
		}
		break;
	}
	}
}

void BossEnemyDragon::EnemyMove(BlockMap& blockMap)
{
	mpBlockMap = &blockMap;

	// 離陸・飛行・空中攻撃中は重力を無効化し空中座標を維持する
	if (mState == State::Takeoff || mState == State::Fly || mState == State::AirAttack)
	{
		isGrounded = false;
		velocityY = 0.0f;
		return;
	}

	Enemy::EnemyMove(blockMap);
}

void BossEnemyDragon::Draw()
{
	if (isDeadBlinking && (deadBlinkTimer / 4) % 2 == 0)
	{
		return;
	}

	int handle = -1;
	float footOffsetFromCenter = 79.0f; // 256x256キャラクターキャンバス基準の足元オフセット

	switch (mState)
	{
	case State::EntranceDrop:
		if (!mLandingAnim.empty())
		{
			int idx = mAnimFrame;
			if (idx >= static_cast<int>(mLandingAnim.size())) idx = static_cast<int>(mLandingAnim.size()) - 1;
			handle = mLandingAnim[idx];
			footOffsetFromCenter = 57.0f; // 400x400キャラクターキャンバス基準
		}
		break;

	case State::EntranceWait:
		if (!mEntranceLandAnimDone && !mLandingAnim.empty())
		{
			int idx = mAnimFrame;
			if (idx >= static_cast<int>(mLandingAnim.size())) idx = static_cast<int>(mLandingAnim.size()) - 1;
			handle = mLandingAnim[idx];
			footOffsetFromCenter = 57.0f;
		}
		else if (!mIdleAnim.empty())
		{
			handle = mIdleAnim[mAnimFrame % mIdleAnim.size()];
			footOffsetFromCenter = 79.0f;
		}
		break;

	case State::Idle:
		if (!mIdleAnim.empty())
		{
			handle = mIdleAnim[mAnimFrame % mIdleAnim.size()];
			footOffsetFromCenter = 79.0f;
		}
		break;

	case State::Move:
		if (!mMoveAnim.empty())
		{
			handle = mMoveAnim[mAnimFrame % mMoveAnim.size()];
			footOffsetFromCenter = 79.0f;
		}
		break;

	case State::Attack:
		footOffsetFromCenter = 79.0f;
		if (mAttackPhase == 0)
		{
			if (!mIdleAnim.empty()) handle = mIdleAnim[mAnimFrame % mIdleAnim.size()];
		}
		else if (mAttackPhase == 1 || mAttackPhase == 3)
		{
			if (!mMoveAnim.empty()) handle = mMoveAnim[mAnimFrame % mMoveAnim.size()];
		}
		else
		{
			if (!mAttackAnim.empty()) handle = mAttackAnim[mAnimFrame % mAttackAnim.size()];
		}
		break;

	case State::Takeoff:
		if (!mTakeoffAnim.empty())
		{
			handle = mTakeoffAnim[mAnimFrame % mTakeoffAnim.size()];
			footOffsetFromCenter = 80.0f; // 350x350キャラクターキャンバス基準
		}
		break;

	case State::Fly:
		if (!mFlyAnim.empty())
		{
			handle = mFlyAnim[mAnimFrame % mFlyAnim.size()];
			footOffsetFromCenter = 57.0f; // 400x400キャラクターキャンバス基準
		}
		break;

	case State::AirAttack:
		if (!mAirAttackAnim.empty())
		{
			handle = mAirAttackAnim[mAnimFrame % mAirAttackAnim.size()];
			footOffsetFromCenter = 57.0f; // 400x400キャラクターキャンバス基準
		}
		break;

	case State::Landing:
		if (!mLandingAnim.empty())
		{
			handle = mLandingAnim[mAnimFrame % mLandingAnim.size()];
			footOffsetFromCenter = 57.0f; // 400x400キャラクターキャンバス基準
		}
		break;
	}

	if (handle != -1)
	{
		if (isDamaged)
		{
			SetDrawBright(255, 100, 100);
		}

		int screenX = static_cast<int>(ConvertToScreenX(mvPosition.x, mpBlockMap));
		float feetWorldY = mvPosition.y + (mfEnemyHeight * 0.5f);
		int screenY = static_cast<int>(ConvertToScreenY(feetWorldY - (footOffsetFromCenter * mDrawScale), mpBlockMap));

		// スプライトは元々左向きなので、右向き(mFacingDirection > 0)のときは左右反転描画
		bool reverse = (mFacingDirection > 0.0f);

		// 踏み込み（前進Phase 1, 3での突撃時）は移動の残像を描画してスピード感を演出
		if (mState == State::Attack && (mAttackPhase == 1 || mAttackPhase == 3))
		{
			int dirSign = (mFacingDirection > 0.0f) ? 1 : -1;
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 75);
			SetDrawBright(120, 200, 255);
			DrawRotaGraph(screenX - dirSign * 55, screenY, mDrawScale, 0.0, handle, TRUE, reverse);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 130);
			DrawRotaGraph(screenX - dirSign * 26, screenY, mDrawScale, 0.0, handle, TRUE, reverse);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			if (isDamaged) SetDrawBright(255, 100, 100);
			else SetDrawBright(255, 255, 255);
		}

		DrawRotaGraph(screenX, screenY, mDrawScale, 0.0, handle, TRUE, reverse);

		if (isDamaged)
		{
			SetDrawBright(255, 255, 255);
		}

		// 通常アタック前の予兆表示（Phase 0）：頭上の少し上の位置に「！」マークを表示
		if (mState == State::Attack && mAttackPhase == 0)
		{
			int exX = static_cast<int>(ConvertToScreenX(mvPosition.x + mFacingDirection * 85.0f, mpBlockMap));
			int exY = static_cast<int>(ConvertToScreenY(mvPosition.y - 165.0f, mpBlockMap));
			bool blinkOff = (mStateTimer / 4) % 2 != 0;
			if (!blinkOff)
			{
				// 目立つ赤色の「！」マーク
				DrawBox(exX - 10, exY - 62, exX + 10, exY - 13, GetColor(0, 0, 0), TRUE);
				DrawBox(exX - 10, exY - 7, exX + 10, exY + 12, GetColor(0, 0, 0), TRUE);
				DrawBox(exX - 8, exY - 60, exX + 8, exY - 15, GetColor(255, 20, 20), TRUE);
				DrawBox(exX - 8, exY - 5, exX + 8, exY + 10, GetColor(255, 20, 20), TRUE);
			}
		}
	}

	// 火炎ブレスおよび8秒間燃え盛る地面の描画
	DrawFireAndBurningGround();
}

void BossEnemyDragon::UpdateFireAndBurningGround(Player* player)
{
	const float floorY = 1903.0f;

	// 1. 空中から吐き出された火炎ブレス弾の移動・プレイヤーとの当たり判定・地面着弾処理
	for (auto& fp : mFireProjectiles)
	{
		if (!fp.active) continue;

		fp.x += fp.vx;
		fp.y += fp.vy;
		fp.animTick++;

		// プレイヤーへの直接ヒット判定
		if (player && !player->GetIsDead())
		{
			float px = player->GetPosition().x;
			float py = player->GetPosition().y;
			float pLeft = px - 16.0f;
			float pRight = px + 16.0f;
			float pTop = py - 50.0f;
			float pBot = py + 50.0f;

			if (fp.x + fp.radius > pLeft && fp.x - fp.radius < pRight &&
				fp.y + fp.radius > pTop && fp.y - fp.radius < pBot)
			{
				player->TakeDamage(mAttack);
			}
		}

		// 地面（Y >= 1903）または足場ブロックに着弾した判定
		bool hitGround = (fp.y >= floorY - 6.0f);
		if (!hitGround && mpBlockMap != nullptr && fp.y >= 1750.0f)
		{
			if (mpBlockMap->GetCollisionType(static_cast<int>(fp.x), static_cast<int>(fp.y)) == BlockMap::CollisionType::Block)
			{
				hitGround = true;
			}
		}

		if (hitGround)
		{
			fp.active = false;
			float burnX = fp.x;
			if (burnX < 7950.0f) burnX = 7950.0f;
			if (burnX > 9820.0f) burnX = 9820.0f;

			// すでに存在する近傍（70px以内）に燃えてエリアが生成されている場合のタイマーを8秒（480フレーム）に更新して統合
			bool merged = false;
			for (auto& bg : mBurningGrounds)
			{
				if (std::fabs(bg.x - burnX) < 70.0f)
				{
					bg.timer = 480; // 8秒間 (60fps ～ 8s)
					merged = true;
					break;
				}
			}

			if (!merged)
			{
				BurningGround bg;
				bg.x = burnX;
				bg.y = floorY;
				bg.halfWidth = 55.0f;
				bg.timer = 480;    // 8秒間 (60fps ～ 8s)
				bg.maxTimer = 480;
				mBurningGrounds.push_back(bg);
			}
		}
		else if (fp.x < 7800.0f || fp.x > 9900.0f || fp.y > 2100.0f)
		{
			fp.active = false;
		}
	}

	// 非アクティブになった火炎弾を削除
	for (auto it = mFireProjectiles.begin(); it != mFireProjectiles.end(); )
	{
		if (!it->active) it = mFireProjectiles.erase(it);
		else ++it;
	}

	// 2. 地面の燃えるエリア（8秒間持続・継続ダメージ）の更新
	for (auto it = mBurningGrounds.begin(); it != mBurningGrounds.end(); )
	{
		it->timer--;
		if (it->timer <= 0)
		{
			it = mBurningGrounds.erase(it);
			continue;
		}

		// 燃えている地面との接触ダメージ
		if (player && !player->GetIsDead())
		{
			float px = player->GetPosition().x;
			float py = player->GetPosition().y;
			float pLeft = px - 16.0f;
			float pRight = px + 16.0f;
			float pTop = py - 50.0f;
			float pBot = py + 50.0f;

			float fireLeft = it->x - it->halfWidth;
			float fireRight = it->x + it->halfWidth;
			float fireTop = it->y - 46.0f;
			float fireBot = it->y + 12.0f;

			if (pRight > fireLeft && pLeft < fireRight && pBot > fireTop && pTop < fireBot)
			{
				player->TakeDamage(mAttack);
			}
		}

		++it;
	}
}

void BossEnemyDragon::DrawFireAndBurningGround()
{
	if (mpBlockMap == nullptr) return;

	int now = GetNowCount();

	// 1. 空中火炎ブレスの予兆エフェクト（口元に発光する炎がだんだん集まるフラッシュ）
	if (mState == State::AirAttack)
	{
		int mouthSX = static_cast<int>(ConvertToScreenX(mvPosition.x + mFacingDirection * (115.0f * mDrawScale), mpBlockMap));
		int mouthSY = static_cast<int>(ConvertToScreenY(mvPosition.y - 110.0f, mpBlockMap));

		SetDrawBlendMode(DX_BLENDMODE_ADD, 210);
		if (mAnimFrame < 3)
		{
			// 溜め動作で口元に光が集まる表現
			int r = 10 + mAnimFrame * 6 + (now / 40) % 6;
			DrawCircle(mouthSX, mouthSY, r, GetColor(255, 80, 20), TRUE);
			DrawCircle(mouthSX, mouthSY, r / 2, GetColor(255, 220, 100), TRUE);
		}
		else if (mAnimFrame >= 3 && mAnimFrame <= 5)
		{
			// 火炎を放っている最中の口元の輝き
			int r = 28 + (now / 30) % 10;
			DrawCircle(mouthSX, mouthSY, r, GetColor(255, 60, 10), TRUE);
			DrawCircle(mouthSX, mouthSY, static_cast<int>(r * 0.7f), GetColor(255, 160, 30), TRUE);
			DrawCircle(mouthSX, mouthSY, static_cast<int>(r * 0.4f), GetColor(255, 255, 180), TRUE);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// 2. 8秒間燃え盛る地面の描画
	SetDrawBlendMode(DX_BLENDMODE_ADD, 220);
	for (const auto& bg : mBurningGrounds)
	{
		int cx = static_cast<int>(ConvertToScreenX(bg.x, mpBlockMap));
		int cy = static_cast<int>(ConvertToScreenY(bg.y, mpBlockMap));
		int hw = static_cast<int>(bg.halfWidth);

		// 残り1秒（60フレーム未満）で徐々に縮小しながら消えていく
		float scale = (bg.timer < 60) ? (static_cast<float>(bg.timer) / 60.0f) : 1.0f;
		if (scale < 0.15f) scale = 0.15f;

		// 地面に広がるマグマ・ベースの楕円
		DrawOval(cx, cy - 2, hw + 10, static_cast<int>(10 * scale) + 3, GetColor(220, 45, 10), TRUE);
		DrawOval(cx, cy - 2, hw, static_cast<int>(6 * scale) + 2, GetColor(255, 140, 20), TRUE);

		// 燃え盛る炎の列（複数の三角形を重ねて描画）
		const int numFlames = 9;
		for (int i = 0; i < numFlames; ++i)
		{
			float ratio = static_cast<float>(i) / static_cast<float>(numFlames - 1) - 0.5f; // -0.5 ~ +0.5
			int fx = cx + static_cast<int>(ratio * (hw * 1.75f));
			float wave1 = std::sin(now * 0.022f + i * 1.7f + bg.x * 0.05f);
			float wave2 = std::cos(now * 0.031f + i * 2.3f);

			int flameH = static_cast<int>((38.0f + 16.0f * wave1) * scale);
			int flameW = static_cast<int>(16.0f * scale) + 5;
			int tipX = fx + static_cast<int>(wave2 * 7.0f);

			// 外側の赤み
			DrawTriangle(
				fx - flameW, cy,
				fx + flameW, cy,
				tipX, cy - flameH,
				GetColor(255, 50, 10), TRUE
			);
			// 中間のオレンジの炎
			DrawTriangle(
				fx - (flameW * 2) / 3, cy,
				fx + (flameW * 2) / 3, cy,
				tipX, cy - (flameH * 3) / 4,
				GetColor(255, 150, 20), TRUE
			);
			// 芯の黄色・白熱の炎
			DrawTriangle(
				fx - flameW / 3, cy,
				fx + flameW / 3, cy,
				tipX, cy - flameH / 2,
				GetColor(255, 240, 130), TRUE
			);
		}

		// 舞い上がる火の粉パーティクル
		for (int p = 0; p < 4; ++p)
		{
			int seed = static_cast<int>(bg.x) + p * 97;
			int cycle = (now / 12 + seed) % 45;
			int px = cx + ((seed * 31) % (hw * 2)) - hw + static_cast<int>(std::sin(cycle * 0.25f) * 6.0f);
			int py = cy - static_cast<int>(cycle * 1.4f * scale) - 6;
			int pr = (cycle < 30) ? 3 : 2;
			DrawCircle(px, py, pr, GetColor(255, 190, 60), TRUE);
		}
	}

	// 3. 移動中の火炎ブレス弾の描画
	for (const auto& fp : mFireProjectiles)
	{
		if (!fp.active) continue;

		int sx = static_cast<int>(ConvertToScreenX(fp.x, mpBlockMap));
		int sy = static_cast<int>(ConvertToScreenY(fp.y, mpBlockMap));
		int r = static_cast<int>(fp.radius);

		// 進行方向反対に伸びる火炎の尾
		int tailX1 = static_cast<int>(ConvertToScreenX(fp.x - fp.vx * 1.2f, mpBlockMap));
		int tailY1 = static_cast<int>(ConvertToScreenY(fp.y - fp.vy * 1.2f, mpBlockMap));
		int tailX2 = static_cast<int>(ConvertToScreenX(fp.x - fp.vx * 2.4f, mpBlockMap));
		int tailY2 = static_cast<int>(ConvertToScreenY(fp.y - fp.vy * 2.4f, mpBlockMap));

		DrawCircle(tailX2, tailY2, static_cast<int>(r * 0.55f), GetColor(220, 40, 10), TRUE);
		DrawCircle(tailX1, tailY1, static_cast<int>(r * 0.8f), GetColor(255, 100, 15), TRUE);

		// 火炎弾本体（外郭・中層・白熱コア）
		DrawCircle(sx, sy, r, GetColor(255, 55, 10), TRUE);
		DrawCircle(sx, sy, static_cast<int>(r * 0.72f), GetColor(255, 165, 30), TRUE);
		DrawCircle(sx, sy, static_cast<int>(r * 0.42f), GetColor(255, 250, 170), TRUE);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void BossEnemyDragon::DrawHpBar() const
{
	// EntranceDrop中はHPバー非表示
	if (mState == State::EntranceDrop) return;
	// HP0かつSmoothHp0なら非表示
	if (mHp <= 0 && mSmoothHp <= 0.0f) return;

	// EntranceWait中は徐々に表示（後半120フレームでフェードイン）
	float fillMultiplier = 1.0f;
	if (mState == State::EntranceWait)
	{
		// mEntranceTimer は 240 -> 0 とカウントダウン
		int elapsed = 240 - mEntranceTimer;
		if (elapsed < 120) return; // 前半は非表示
		fillMultiplier = static_cast<float>(elapsed - 120) / 120.0f;
		if (fillMultiplier > 1.0f) fillMultiplier = 1.0f;
	}

	const int barX = ScreenSize::CenterX - 600;
	const int barY = ScreenSize::ScrrenHeight - 120;
	const int barW = 1200;
	const int barH = 40;

	int shakeX = 0, shakeY = 0;
	if (mHpShakeTimer > 0)
	{
		shakeX = (rand() % 11) - 5;
		shakeY = (rand() % 11) - 5;
	}
	int bx = barX + shakeX;
	int by = barY + shakeY;

	// 外枠（黒）
	DrawBox(bx - 6, by - 6, bx + barW + 6, by + barH + 6, GetColor(0, 0, 0), TRUE);
	// 空バー（暗いグレー）
	DrawBox(bx, by, bx + barW, by + barH, GetColor(50, 50, 50), TRUE);

	// スムーズ減少用バー（赤）
	float smoothRatio = (mSmoothHp / static_cast<float>(mMaxHp)) * fillMultiplier;
	if (smoothRatio > 1.0f) smoothRatio = 1.0f;
	if (smoothRatio > 0.0f)
	{
		DrawBox(bx, by, bx + static_cast<int>(barW * smoothRatio), by + barH, GetColor(220, 50, 50), TRUE);
	}

	// 実HPバー（緑）
	float actualRatio = (static_cast<float>(mHp) / static_cast<float>(mMaxHp)) * fillMultiplier;
	if (actualRatio > 1.0f) actualRatio = 1.0f;
	if (actualRatio > 0.0f)
	{
		DrawBox(bx, by, bx + static_cast<int>(barW * actualRatio), by + barH, GetColor(50, 220, 50), TRUE);
	}

	// 枠線（白）
	DrawBox(bx, by, bx + barW, by + barH, GetColor(255, 255, 255), FALSE);

	// 名前テキスト（バー上部）
	if (fillMultiplier > 0.0f && Master::mpGameManager)
	{
		int nameAlpha = static_cast<int>(255.0f * fillMultiplier);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, nameAlpha);
		const int nameFz = 48;
		Master::mpGameManager->GetFontManager()->DrawDotString(
			bx, by - 52, nameFz, GetColor(255, 100, 80), "%s", "\x83\x68\x83\x89\x83\x53\x83\x93"
		);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}
#include "BossEnemyDragon.h"
#include "DxLib.h"
#include "BlockMap.h"
#include "Master.h"
#include "GameManager.h"
#include "SceneManager.h"
#include "GameScene.h"
#include "Player.h"
#include "FontManager.h"
#include "GameConstants.h"
#include "Collision.h"
#include "StageClear.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>


BossEnemyDragon::BossEnemyDragon(VECTOR initPos)
	: Enemy("Resource/Enemy/Dragon/animation_dragon/new_idle/sprite_idle1.png", initPos)
	, mState(State::Idle)
	, mAnimFrame(0)
	, mAnimTimer(0)
	, mStateTimer(0)
	, mAttackPhase(0)
	, mDivePhase(0)
	, mCenterAttackPhase(0)
	, mRetreatPhase(0)
	, mHasRetreated(false)
	, mDiveVx(0.0f)
	, mDiveVy(0.0f)
	, mDiveAngle(0.0f)
	, mDiveTargetX(8900.0f)
	, mDiveTargetY(1803.0f)
	, mAirAttackCycle(0)
	, mAttackCooldown(60)
	, mFlightCooldown(130)
	, mEntranceTimer(0)
	, mPostEntranceTimer(0)
	, mHasBrokenCeiling(false)
	, mEntranceLandAnimDone(false)
	, mFacingDirection(-1.0f)
	, mDrawScale(1.6f)
	, mHoverTargetY(1480.0f)
	, mBreathTargetCenterX(8900.0f)
	, mSmoothHp(80.0f)
	, mHpShakeTimer(0)
	, mIsBattleActive(false)
{
	UpdateStatusByProgress(GameProgress::Tutorial1);
	LoadAnimations();

	// 物理・当たり判定サイズの設定
	mfEnemyWidth = 220.0f;
	mfEnemyHeight = 200.0f;
	moveSpeed = 0.0f;

}


void BossEnemyDragon::StartBattle()
{
	mIsBattleActive = true;

	// 戦闘開始時に各タイマーを適切な初期値にリセットする
	// (EntranceWait -> Fly 遷移後、ガード停止中にタイマーが残っているため)
	mStateTimer        = 0;
	mAnimFrame         = 0;
	mAnimTimer         = 0;
	// mPostEntranceTimer は「ドラゴン」文字のフェードアウト演出で使用中のためリセットしない
	mAttackCooldown    = 60;
	mFlightCooldown    = 0;  // 戦闘開始直後は即座に攻撃行動へ移行できるようにゼロにする
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

void BossEnemyDragon::LoadAnimations()
{
	// 1. Idle (4枚: sprite_idle1.png ~ sprite_idle4.png)
	for (int i = 1; i <= 4; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_idle/sprite_idle%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mIdleAnim.push_back(h);
	}

	// 2. Move (4枚: sprite_move1.png ~ sprite_move4.png)
	for (int i = 1; i <= 4; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_move/sprite_move%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mMoveAnim.push_back(h);
	}

	// 3. Attack (9枚: sprite_attack1.png ~ sprite_attack9.png)
	for (int i = 1; i <= 9; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_attack/sprite_attack%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mAttackAnim.push_back(h);
	}

	// 4. 離陸アニメーション (7枚: sprite_takeoff1.png ~ sprite_takeoff7.png)
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_takeoff/sprite_takeoff%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mTakeoffAnim.push_back(h);
	}

	// 5. 飛んでいるアニメーション (7枚: sprite_fly1.png ~ sprite_fly7.png)
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_flying/sprite_fly%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mFlyAnim.push_back(h);
	}

	// 6. 空中攻撃 (7枚: sprite_airAttack1.png ~ sprite_airAttack7.png)
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_airborne/sprite_airAttack%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mAirAttackAnim.push_back(h);
	}

	// 7. 着陸アニメーション (7枚: sprite_landing1.png ~ sprite_landing7.png)
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_landing/sprite_landing%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mLandingAnim.push_back(h);
	}
}

void BossEnemyDragon::UpdateStatusByProgress(GameProgress progress)
{
	mMaxHp = 80;
	mHp = 80;
	mAttack = 15;
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
	mRetreatPhase = 0;
	mHasRetreated = false;
	mAnimFrame = 0;
	mAnimTimer = 0;
	isGrounded = false;
	velocityY = 30.0f;
	mFireGimmick.ClearAll();
}

void BossEnemyDragon::TakeDamage(int damage)
{
	if (mState == State::EntranceDrop || mState == State::EntranceWait || mState == State::DefeatRetreat || mHp <= 0) return;
	mHp -= damage;
	if (mHp < 0) mHp = 0;
	mHpShakeTimer = 15;
}

bool BossEnemyDragon::CheckAttackCollision(const VECTOR& targetPos, const VECTOR& targetSize) const
{
	if (!IsAttacking()) return false;
	float left = 0.0f, top = 0.0f, right = 0.0f, bottom = 0.0f;
	GetAttackHitbox(left, top, right, bottom);
	VECTOR atkPos = VGet(left, top, 0.0f);
	VECTOR atkSize = VGet(right - left, bottom - top, 0.0f);
	return Collision::CheckRectToRect(targetPos, targetSize, atkPos, atkSize);
}

bool BossEnemyDragon::IsAttacking() const
{
	if (mState == State::Attack)
	{
		return ((mAttackPhase == 2 || mAttackPhase == 4) && mAnimFrame >= 3 && mAnimFrame <= 6);
	}
	if (mState == State::AirDiveAttack)
	{
		return (mDivePhase == 2 && mAnimFrame >= 3 && mAnimFrame <= 6);
	}
	return false;
}

void BossEnemyDragon::GetAttackHitbox(float& left, float& top, float& right, float& bottom) const
{
	float reach = (mState == State::AirDiveAttack) ? 185.0f : 170.0f;
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

void BossEnemyDragon::GetAirAttackMouthWorldPos(float& outMouthX, float& outMouthY) const
{
	// 400x400 キャンバス中心 (200, 200) から見た各フレームの正確な口元オフセット (左向き基準)
	static const float kMouthOffsetX[7] = { 106.0f, 104.0f, 97.0f, 105.0f, 115.0f, 106.0f, 105.0f };
	static const float kMouthOffsetY[7] = { -77.0f, -87.0f, -101.0f, -82.0f, -70.0f, -72.0f, -75.0f };

	int idx = mAnimFrame;
	if (idx < 0) idx = 0;
	if (idx > 6) idx = 6;

	// Draw() と完全に一致するスプライト中心のワールド座標
	float spriteCenterWorldX = mvPosition.x;
	float feetWorldY = mvPosition.y + (mfEnemyHeight * 0.5f);
	float spriteCenterWorldY = feetWorldY - (57.0f * mDrawScale);

	float localX = mFacingDirection * kMouthOffsetX[idx] * mDrawScale;
	float localY = kMouthOffsetY[idx] * mDrawScale;

	float angle = ((mState == State::AirDiveAttack && mDivePhase < 2) || (mState == State::CenterFireballAttack && mCenterAttackPhase == 1))
		? mDiveAngle
		: 0.0f;

	float cosA = std::cos(angle);
	float sinA = std::sin(angle);
	outMouthX = spriteCenterWorldX + (localX * cosA - localY * sinA);
	outMouthY = spriteCenterWorldY + (localX * sinA + localY * cosA);
}

void BossEnemyDragon::TriggerDefeatRetreat(GameScene* gameScene)
{
	mState = State::DefeatRetreat;
	mStateTimer = 0;
	mAnimFrame = 0;
	mAnimTimer = 0;
	mDiveAngle = 0.0f;
	mDiveVx = 0.0f;
	mDiveVy = 0.0f;
	velocityY = 0.0f;
	isGrounded = false;
	isDamaged = false;
	isDeadBlinking = false;

	// 残っている炎や火の玉・アーチ炎・画面揺れをすべて即座にクリア
	mFireGimmick.ClearAll();

	// 地上にいる場合は離陸（Phase 0）から、すでに空中にいる場合は穴の下への飛行移動（Phase 1）から開始
	mRetreatPhase = (mvPosition.y > 1580.0f) ? 0 : 1;
}

// =========================================================================
// メイン更新処理 (Update)
// =========================================================================
void BossEnemyDragon::Update()
{
	// BossEnemyDragon::Update() の冒頭に追加（一時デバッグ用）
	DrawFormatString(10, 100, GetColor(255, 255, 0),
		"Dragon State=%d BattleActive=%d StateTimer=%d",
		(int)mState, (int)mIsBattleActive, mStateTimer);

	Enemy::Update();

	// 戦闘開始フラグが立つまでは、位置やアニメーション以外のAI行動・クールタイムを進めない
	if (!mIsBattleActive && mState != State::EntranceDrop && mState != State::EntranceWait)
	{
		if (mPostEntranceTimer > 0) mPostEntranceTimer--;
		if (mState == State::Fly && !mFlyAnim.empty())
		{
			mAnimTimer++;
			if (mAnimTimer >= 6)
			{
				mAnimTimer = 0;
				mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mFlyAnim.size());
			}
		}
		return;
	}

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

	// 体力がゼロになったら消滅せず、最初に出てきた天井の穴（X=9500）から飛んで上に戻る演出へ移行
	if (mHp <= 0 && mState != State::DefeatRetreat)
	{
		TriggerDefeatRetreat(gameScene);
	}

	Player* player = (gameScene != nullptr) ? gameScene->GetPlayer() : nullptr;

	// 登場演出1: 天井を突き破って落下 (EntranceDrop)
	if (mState == State::EntranceDrop)
	{
		UpdateEntranceDrop(player, gameScene);
		return;
	}

	// 登場演出2: 地面に着地 → その後飛び上がって空中飛行から戦闘スタート (EntranceWait)
	if (mState == State::EntranceWait)
	{
		UpdateEntranceWait(player);
		return;
	}

	if (mPostEntranceTimer > 0) mPostEntranceTimer--;
	if (mAttackCooldown > 0) mAttackCooldown--;
	if (mFlightCooldown > 0) mFlightCooldown--;

	// 火炎ブレス弾・延焼地面・巨大火球・アーチ炎・火花の更新
	mFireGimmick.Update(player, mpBlockMap, mAttack);

	// ボス部屋の行動範囲制限 (X: 8080 ~ 9680)
	const float roomMinX = 8080.0f;
	const float roomMaxX = 9680.0f;

	float px = (player != nullptr) ? player->GetPosition().x : mvPosition.x;
	float dx = px - mvPosition.x;
	float absDx = std::fabs(dx);

	switch (mState)
	{
	case State::Idle:
		UpdateIdle(player, dx, absDx);
		break;
	case State::Move:
		UpdateMove(player, dx, absDx, roomMinX, roomMaxX);
		break;
	case State::Attack:
		UpdateGroundAttack(roomMinX, roomMaxX);
		break;
	case State::Takeoff:
		UpdateTakeoff(player, dx);
		break;
	case State::Fly:
		UpdateFly(player, dx, absDx, roomMinX, roomMaxX);
		break;
	case State::AirAttack:
		UpdateAirBreathAttack(player, dx);
		break;
	case State::AirDiveAttack:
		UpdateAirDiveAttack(player, dx, roomMinX, roomMaxX);
		break;
	case State::CenterFireballAttack:
		UpdateCenterFireballAttack(player, dx);
		break;
	case State::Landing:
		UpdateLanding();
		break;
	case State::DefeatRetreat:
		UpdateDefeatRetreat();
		break;
	default:
		break;
	}
}

// =========================================================================
// 各ステート別の行動・攻撃ロジック実装
// =========================================================================

void BossEnemyDragon::UpdateEntranceDrop(Player* player, GameScene* gameScene)
{
	if (player)
	{
		float dx = player->GetPosition().x - mvPosition.x;
		mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;
	}

	// 空中降下フレーム（着陸アニメーション前半 0..2）
	mAnimTimer++;
	if (mAnimTimer >= 6)
	{
		mAnimTimer = 0;
		if (mAnimFrame < 2) mAnimFrame++;
	}

	// 天井の高さ（Y >= 980）に到達した瞬間に天井を破壊して岩片を降らせる
	if (!mHasBrokenCeiling && mvPosition.y >= 980.0f && mpBlockMap != nullptr)
	{
		mHasBrokenCeiling = true;
	}

	// 床に着地したら EntranceWait へ移行し、着陸アニメーション後半（砂煙エフェクト 3..6）を再生
	if (isGrounded)
	{
		if (mpBlockMap != nullptr)
		{
			mpBlockMap->StartCameraShake(
				KingSlimeShakeConstants::ShakePower,
				KingSlimeShakeConstants::RatioX,
				KingSlimeShakeConstants::RatioY,
				KingSlimeShakeConstants::DurationFrames
			);
		}
		if (player != nullptr && !player->HasEvadedBossSpawn())
		{
			player->SetDeathReason(DeathReason::DeathType::NormalDeath);
			player->TakeDamage(player->GetHp());
		}
		mState = State::EntranceWait;
		mEntranceTimer = 240; // キングスライミーと同じ4秒間
		mEntranceLandAnimDone = false;
		mAnimFrame = 3;
		mAnimTimer = 0;
	}
}

void BossEnemyDragon::UpdateEntranceWait(Player* player)
{
	mAnimTimer++;
	if (!mEntranceLandAnimDone)
	{
		// まずは地面に着地するアニメーション後半（3..6）を再生
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
	else if (mEntranceTimer > 105)
	{
		// 地面に着地した状態で待機ポーズ
		if (mAnimTimer >= 10 && !mIdleAnim.empty())
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mIdleAnim.size());
		}
	}
	else if (mEntranceTimer > 63)
	{
		// 地面から離陸アニメーション（sprite_takeoff1 ~ 7）で空中へ飛び上がる
		if (mEntranceTimer == 105)
		{
			mAnimFrame = 0;
			mAnimTimer = 0;
			isGrounded = false;
			velocityY = 0.0f;
		}
		int takeoffIdx = (105 - mEntranceTimer) / 6;
		if (takeoffIdx < 0) takeoffIdx = 0;
		if (takeoffIdx >= static_cast<int>(mTakeoffAnim.size()))
		{
			takeoffIdx = static_cast<int>(mTakeoffAnim.size()) - 1;
		}
		mAnimFrame = takeoffIdx;

		if (takeoffIdx >= 2)
		{
			mvPosition.y -= 10.5f;
			if (mvPosition.y < mHoverTargetY) mvPosition.y = mHoverTargetY;
		}
	}
	else
	{
		// 空中で飛んでいるアニメーション（sprite_fly1 ~ 7）でホバリング待機
		if (mEntranceTimer == 63)
		{
			mAnimFrame = 0;
			mAnimTimer = 0;
		}
		mvPosition.y += (mHoverTargetY - mvPosition.y) * 0.2f;
		if (mAnimTimer >= 5 && !mFlyAnim.empty())
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mFlyAnim.size());
		}
	}


	if (mEntranceTimer > 0) mEntranceTimer--;
	if (mEntranceTimer <= 0)
	{
		// 空中で飛んでいる状態（State::Fly）からバトルスタート！
		mState = State::Fly;
		isGrounded = false;
		velocityY = 0.0f;
		mvPosition.y = mHoverTargetY;
		mStateTimer = 0;
		mAnimFrame = 0;
		mAnimTimer = 0;
		mPostEntranceTimer = 60;
	}
}

void BossEnemyDragon::UpdateIdle(Player* player, float dx, float absDx)
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

		// 飛行クールダウン完了時は離陸アニメーションへ移行
		if (mFlightCooldown <= 0 && mStateTimer >= 35)
		{
			mState = State::Takeoff;
			mStateTimer = 0;
			mAnimFrame = 0;
			mAnimTimer = 0;
			return;
		}

		// 通常アタックの間合いなら地上連続攻撃へ
		if (absDx <= 360.0f && mAttackCooldown <= 0)
		{
			mState = State::Attack;
			mAttackPhase = 0; // Phase 0: 頭上に「！」マーク表示
			mStateTimer = 0;
			mAnimFrame = 0;
			mAnimTimer = 0;
			return;
		}

		// 一定時間待機後に移動開始
		if (mStateTimer >= 40)
		{
			mState = State::Move;
			mStateTimer = 0;
			mAnimFrame = 0;
			mAnimTimer = 0;
		}
	}
}

void BossEnemyDragon::UpdateMove(Player* player, float dx, float absDx, float roomMinX, float roomMaxX)
{
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

		// 飛行クールダウンが溜まったら離陸（Takeoff）へ
		if (mFlightCooldown <= 0 && mStateTimer >= 35)
		{
			mState = State::Takeoff;
			mStateTimer = 0;
			mAnimFrame = 0;
			mAnimTimer = 0;
			return;
		}

		// 攻撃間合いに入ったら通常アタック（Attack）へ
		if (absDx <= 350.0f && mAttackCooldown <= 0)
		{
			mState = State::Attack;
			mAttackPhase = 0; // Phase 0: 頭上に「！」マーク表示
			mStateTimer = 0;
			mAnimFrame = 0;
			mAnimTimer = 0;
			return;
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
}

void BossEnemyDragon::UpdateGroundAttack(float roomMinX, float roomMaxX)
{
	// 通常アタック:
	// Phase 0: 頭の上に「！」マークを出して予備動作
	// Phase 1: 俊足で前に少し前進（5フレームでスッと約75pxだけ前へ出る）
	// Phase 2: 1回目の攻撃（その場で素早く斬りつける）
	// Phase 3: 再び俊足で前に少し前進（5フレームでスッと約75pxだけ前へ出る）
	// Phase 4: 2回目の攻撃
	mStateTimer++;
	mAnimTimer++;

	if (mAttackPhase == 0)
	{
		// 頭上に「！」マークを表示しながら予備動作
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
		// 俊足で前に少しだけ前進（15px × 5フレーム = 75px を一瞬でステップ）
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
			mAnimFrame = 2; // 溜めフレームを飛ばしてすぐ攻撃へ
			mAnimTimer = 0;
		}
	}
	else if (mAttackPhase == 2 || mAttackPhase == 4)
	{
		// 攻撃モーション再生（前進しすぎないよう、攻撃中はその場で斬りつける）
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
					// 1回目の攻撃完了 -> Phase 3（2回目の少し前進）へ
					mAttackPhase = 3;
				}
				else
				{
					// 2回目の攻撃完了 -> Idleへ戻る
					mAttackPhase = 0;
					mState = State::Idle;
					mAttackCooldown = 70;
				}
			}
		}
	}
}

void BossEnemyDragon::UpdateTakeoff(Player* player, float dx)
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
}

void BossEnemyDragon::UpdateFly(Player* player, float dx, float absDx, float roomMinX, float roomMaxX)
{
	// 飛んでいるアニメーション（sprite_fly1 ~ 7）
	mStateTimer++;
	mAnimTimer++;
	if (mAnimTimer >= 6 && !mFlyAnim.empty())
	{
		mAnimTimer = 0;
		mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mFlyAnim.size());
	}

	// ホバリング高度へ滑らかに維持しつつ、火を噴きやすい斜め上空の間合い（約280～420px）へ移動
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

	// 一定時間飛行した後に「中央火の玉大爆発（CenterFireballAttack → AirDiveAttack派生）」または「空中火炎ブレス（AirAttack）」へ移行
	if (mStateTimer >= 55)
	{
		mStateTimer = 0;
		mAnimFrame = 0;
		mAnimTimer = 0;
		if (player && !player->GetIsDead())
		{
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;
			mBreathTargetCenterX = player->GetPosition().x;
		}
		else
		{
			mBreathTargetCenterX = mvPosition.x + mFacingDirection * 320.0f;
		}

		if (mAirAttackCycle == 0)
		{
			mAirAttackCycle = 1;
			mState = State::CenterFireballAttack;
			mCenterAttackPhase = 0;
			mDiveAngle = 0.0f;
		}
		else
		{
			mAirAttackCycle = 0;
			mState = State::AirAttack;
		}
	}
}

void BossEnemyDragon::UpdateAirBreathAttack(Player* player, float dx)
{
	// 空中攻撃アニメーション（sprite_airAttack1 ~ 7）で空中に留まりながら豪快な火炎ブレスを吐き出す
	mStateTimer++;
	mAnimTimer++;

	// 空中で軽くホバリング維持
	float hoverWave = std::sin(mStateTimer * 0.12f) * 1.5f;
	mvPosition.y += (mHoverTargetY - mvPosition.y) * 0.1f + hoverWave;

	float mouthX = 0.0f, mouthY = 0.0f;
	GetAirAttackMouthWorldPos(mouthX, mouthY);

	if (mAnimFrame < 3)
	{
		// 予備動作（フレーム0～2）：プレイヤーを狙いながら口元に炎の粒子を吸い込む
		if (player && !player->GetIsDead())
		{
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;
			mBreathTargetCenterX = player->GetPosition().x;
		}

		mFireGimmick.SpawnBreathInhaleSparks(mouthX, mouthY);
	}
	else if (mAnimFrame >= 3 && mAnimFrame <= 5)
	{
		// 火炎ブレス放射中（フレーム3～5）：
		// 口元から地面へと広がるド迫力の火炎放射ストリームを毎フレーム連続生成し、着弾点をなぎ払う
		if (player && !player->GetIsDead())
		{
			// プレイヤーの現在位置へ少しずつ狙いを追従させる
			mBreathTargetCenterX += (player->GetPosition().x - mBreathTargetCenterX) * 0.06f;
		}

		// ブレスが地面をなめらかに薙ぎ払うスウィープ角
		float sweep = std::sin(mStateTimer * 0.18f) * 110.0f;
		float targetX = mBreathTargetCenterX + sweep + static_cast<float>(GetRand(40) - 20);
		float targetY = 1903.0f;
		mFireGimmick.SpawnBreathStream(mouthX, mouthY, targetX, targetY);
	}
	

	// 火を噴いているフレーム（3～5）は長めに維持して豪快な火炎放射を見せる
	int frameDuration = (mAnimFrame >= 3 && mAnimFrame <= 5) ? 14 : 7;
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
}

void BossEnemyDragon::UpdateAirDiveAttack(Player* player, float dx, float roomMinX, float roomMaxX)
{
	// 空中からプレイヤーの方を向いて照準を合わせ、斜め下へ超高速で突っ込んで通常攻撃を叩き込む！
	// Phase 0: 空中でプレイヤーの方へ身体の角度を傾けて照準ロックオン（0～54フレーム）
	// Phase 1: ロックオンした角度のまま超高速で斜め急降下ダイブ
	// Phase 2: 着地と同時に通常攻撃モーション（フレーム3～8）で豪快に斬り払う
	mStateTimer++;
	mAnimTimer++;

	if (mDivePhase == 0)
	{
		// 前半（0～26フレーム）はプレイヤーの位置を追尾して向きと角度を合わせる
		// 後半（27～54フレーム＝約0.47秒間）は照準位置を完全ロックして固定し、プレイヤーが避ける猶予をしっかり与える！
		if (mStateTimer <= 26 && player && !player->GetIsDead())
		{
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;
			mDiveTargetX = player->GetPosition().x - mFacingDirection * 95.0f;
			if (mDiveTargetX < roomMinX) mDiveTargetX = roomMinX;
			if (mDiveTargetX > roomMaxX) mDiveTargetX = roomMaxX;
			mDiveTargetY = 1803.0f;

			// ロックオン確定前の間だけ角度を更新
			float aimDx = std::fabs((mDiveTargetX + mFacingDirection * 95.0f) - mvPosition.x);
			if (aimDx < 120.0f) aimDx = 120.0f;
			float aimDy = 1853.0f - mvPosition.y;
			if (aimDy < 80.0f) aimDy = 80.0f;
			float rawAngle = std::atan2(aimDy, aimDx) * 0.72f;
			if (rawAngle > 0.58f) rawAngle = 0.58f;
			float desiredAngle = mFacingDirection * rawAngle;
			mDiveAngle += (desiredAngle - mDiveAngle) * 0.22f;
		}

		if (mStateTimer > 26)
		{
			// 照準ロック後：突撃直前のバネのような溜め（この間に照準から離れれば回避可能！）
			mvPosition.x -= mFacingDirection * 1.6f;
			mvPosition.y -= 1.2f;
			if (mvPosition.x < roomMinX) mvPosition.x = roomMinX;
			if (mvPosition.x > roomMaxX) mvPosition.x = roomMaxX;
			if (mvPosition.y < 1410.0f) mvPosition.y = 1410.0f;
		}
		else
		{
			mvPosition.y -= 0.8f;
			if (mvPosition.y < 1435.0f) mvPosition.y = 1435.0f;
		}

		if (mAnimTimer >= 16)
		{
			mAnimTimer = 0;
			if (mAnimFrame < 1) mAnimFrame++;
		}

		if (mStateTimer >= 54)
		{
			mDivePhase = 1;
			mStateTimer = 0;
			mAnimFrame = 2; // 斬りかかる直前のフォームで急降下
			mAnimTimer = 0;

			float ddx = mDiveTargetX - mvPosition.x;
			float ddy = mDiveTargetY - mvPosition.y;
			const float diveFrames = 14.0f; // 14フレームで強襲
			mDiveVx = ddx / diveFrames;
			if (mDiveVx > 30.0f) mDiveVx = 30.0f;
			if (mDiveVx < -30.0f) mDiveVx = -30.0f;
			mDiveVy = ddy / diveFrames;
			if (mDiveVy < 14.0f) mDiveVy = 14.0f;
		}
	}
	else if (mDivePhase == 1)
	{
		// 照準角度を維持したまま斜め下へ超高速ダイブ！
		mvPosition.x += mDiveVx;
		mvPosition.y += mDiveVy;
		if (mvPosition.x < roomMinX) mvPosition.x = roomMinX;
		if (mvPosition.x > roomMaxX) mvPosition.x = roomMaxX;

		// 突進軌道上に火花を散らす
		mFireGimmick.SpawnDiveTrailSparks(mvPosition.x, mvPosition.y, mDiveVx, mDiveVy);

		// 地上付近（Y >= 1795）に到達したら角度を水平に戻し、通常攻撃の斬撃フレーム（フレーム3～）を叩き込む！
		if (mvPosition.y >= 1795.0f || isGrounded)
		{
			mvPosition.y = 1803.0f;
			mDiveAngle = 0.0f;
			isGrounded = true;
			velocityY = 0.0f;
			mDivePhase = 2;
			mStateTimer = 0;
			mAnimFrame = 3; // 攻撃判定が発生する斬撃フレームへ即移行
			mAnimTimer = 0;

			// 強襲着地時の火花＆画面揺れ
			mFireGimmick.SpawnDiveLandingSparks(mvPosition.x, mFacingDirection);
		}
	}
	else if (mDivePhase == 2)
	{
		mDiveAngle = 0.0f;
		
		// 斬り払い中に少しだけ前へ踏み込む
		if (mAnimFrame >= 3 && mAnimFrame <= 5)
		{
			mvPosition.x += mFacingDirection * 2.0f;
			if (mvPosition.x < roomMinX) mvPosition.x = roomMinX;
			if (mvPosition.x > roomMaxX) mvPosition.x = roomMaxX;
		}

		if (mAnimTimer >= 3)
		{
			mAnimTimer = 0;
			mAnimFrame++;
			if (mAnimFrame >= static_cast<int>(mAttackAnim.size()))
			{
				mState = State::Idle;
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
				mFlightCooldown = 140;
				mAttackCooldown = 55;
			}
		}
	}
}

void BossEnemyDragon::UpdateCenterFireballAttack(Player* player, float dx)
{
	// 飛んでいるときに部屋の真ん中（X = 8900）へ飛んでいき、
	// 口元に炎をためて真ん中に巨大火の玉を吐いて周辺を大爆発させ、
	// 炎が地面を沿ってアーチ状で左右に広がったあと、少ししてから追尾空中攻撃（AirDiveAttack）へ派生する！
	const float arenaCenterX = 8900.0f;
	const float targetHoverY = 1430.0f;
	mStateTimer++;
	mAnimTimer++;

	if (mCenterAttackPhase == 0)
	{
		// Phase 0: 部屋の真ん中（X = 8900）の上空へ飛んで移動する
		if (mAnimTimer >= 5 && !mFlyAnim.empty())
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mFlyAnim.size());
		}

		float diffX = arenaCenterX - mvPosition.x;
		float diffY = targetHoverY - mvPosition.y;
		mvPosition.y += diffY * 0.14f;

		if (std::fabs(diffX) > 14.0f)
		{
			mFacingDirection = (diffX >= 0.0f) ? 1.0f : -1.0f;
			mvPosition.x += mFacingDirection * 12.0f;
		}
		else
		{
			// 真ん中に到着したら巨大火の玉のチャージ＆発射フェーズ（Phase 1）へ移行
			mvPosition.x = arenaCenterX;
			if (player && !player->GetIsDead())
			{
				mFacingDirection = (player->GetPosition().x >= mvPosition.x) ? 1.0f : -1.0f;
			}
			mCenterAttackPhase = 1;
			mStateTimer = 0;
			mAnimFrame = 0;
			mAnimTimer = 0;
		}
	}
	else if (mCenterAttackPhase == 1)
	{
		// Phase 1: 真ん中の上空でドラゴンの口元に巨大火球をチャージし、地面中央（8900, 1903）へ吐き出す！
		float hoverWave = std::sin(mStateTimer * 0.12f) * 2.0f;
		mvPosition.y += (targetHoverY - mvPosition.y) * 0.12f + hoverWave;

		// 傾きを0にしてスプライト本来の口元座標と完全一致させる
		mDiveAngle = 0.0f;

		float mouthX = 0.0f, mouthY = 0.0f;
		GetAirAttackMouthWorldPos(mouthX, mouthY);

		if (mAnimFrame < 3)
		{
			// チャージ中：ドラゴンの口元めがけて周囲から火の粉を激しく吸い込む
			mFireGimmick.SpawnCenterChargeSparks(mouthX, mouthY);

			if (mAnimTimer >= 11)
			{
				mAnimTimer = 0;
				mAnimFrame++;
				// フレーム3（口を大きく開けて吐き出す瞬間）で口元から地面中央（arenaCenterX, 1903.0f）へ巨大火の玉を発射！
				if (mAnimFrame == 3)
				{
					GetAirAttackMouthWorldPos(mouthX, mouthY);
					mFireGimmick.LaunchCenterFireball(mouthX, mouthY, arenaCenterX, 1903.0f);
				}
			}
		}
		else
		{
			// 巨大火の玉を吐き出した後のモーション再生
			if (mAnimTimer >= 8)
			{
				mAnimTimer = 0;
				mAnimFrame++;
				if (mAnimFrame >= static_cast<int>(mAirAttackAnim.size()))
				{
					// 吐き出しモーション完了後、すぐ着陸せず Phase 2（少し滞空してから追尾空中攻撃へ派生）に移行！
					mCenterAttackPhase = 2;
					mStateTimer = 0;
					mAnimFrame = 0;
					mAnimTimer = 0;
				}
			}
		}
	}
	else if (mCenterAttackPhase == 2)
	{
		// Phase 2: 火の玉が爆発してアーチ炎が広がるのを少しの間上空で羽ばたきながら見届け、
		// 続けてプレイヤーを狙う追尾空中攻撃（State::AirDiveAttack）へ突入する！
		if (mAnimTimer >= 6 && !mFlyAnim.empty())
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mFlyAnim.size());
		}

		float hoverWave = std::sin(mStateTimer * 0.12f) * 2.5f;
		mvPosition.y += (targetHoverY - mvPosition.y) * 0.12f + hoverWave;

		if (player && !player->GetIsDead())
		{
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;
		}

		if (mStateTimer >= 60)
		{
			mState = State::AirDiveAttack;
			mDivePhase = 0;
			mDiveAngle = 0.0f;
			mDiveVx = 0.0f;
			mDiveVy = 0.0f;
			mStateTimer = 0;
			mAnimFrame = 0;
			mAnimTimer = 0;
		}
	}
}

void BossEnemyDragon::UpdateLanding()
{
	// 着陸アニメーション（sprite_landing1 ~ 7）
	mStateTimer++;
	mAnimTimer++;

	if (!isGrounded)
	{
		// 着地するまではフレーム0～2で降下
		if (mAnimTimer >= 6)
		{
			mAnimTimer = 0;
			if (mAnimFrame < 2) mAnimFrame++;
		}
	}
	else
	{
		// 着地後はフレーム3～6（砂煙エフェクト付き着陸）を再生
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
				mFlightCooldown = 150;
				mAttackCooldown = 45;
			}
		}
	}
}

void BossEnemyDragon::UpdateDefeatRetreat()
{
	// 体力がゼロになった後、最初に出てきた天井の穴（X = 9500.0f）から上空へ飛んで帰還する演出
	const float holeX = 9500.0f;
	mStateTimer++;
	mAnimTimer++;

	if (mRetreatPhase == 0)
	{
		// Phase 0: 地上から離陸アニメーション（sprite_takeoff1 ~ 7）で飛び上がる
		if (std::fabs(holeX - mvPosition.x) > 15.0f)
		{
			mFacingDirection = (holeX >= mvPosition.x) ? 1.0f : -1.0f;
		}
		if (mAnimFrame >= 2)
		{
			mvPosition.y -= 8.5f;
			if (mvPosition.y < 1490.0f) mvPosition.y = 1490.0f;
		}
		if (mAnimTimer >= 6)
		{
			mAnimTimer = 0;
			mAnimFrame++;
			if (mAnimFrame >= static_cast<int>(mTakeoffAnim.size()))
			{
				mRetreatPhase = 1;
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
			}
		}
	}
	else if (mRetreatPhase == 1)
	{
		// Phase 1: 飛んでいるアニメーションで最初に出てきた天井の穴の真下（X = 9500.0f, Y = 1480.0f）へ移動
		if (mAnimTimer >= 5)
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % (mFlyAnim.empty() ? 1 : static_cast<int>(mFlyAnim.size()));
		}

		float diffX = holeX - mvPosition.x;
		float diffY = 1480.0f - mvPosition.y;
		if (std::fabs(diffX) > 12.0f)
		{
			mFacingDirection = (diffX >= 0.0f) ? 1.0f : -1.0f;
			mvPosition.x += (diffX > 0.0f ? 1.0f : -1.0f) * 11.0f;
		}
		else
		{
			mvPosition.x = holeX;
		}

		if (std::fabs(diffY) > 6.0f)
		{
			mvPosition.y += (diffY > 0.0f ? 1.0f : -1.0f) * 6.0f;
		}
		else
		{
			mvPosition.y = 1480.0f;
		}

		if (std::fabs(holeX - mvPosition.x) <= 12.0f && std::fabs(1480.0f - mvPosition.y) <= 12.0f)
		{
			mvPosition.x = holeX;
			mvPosition.y = 1480.0f;
			mFacingDirection = -1.0f; // ステージ中央（プレイヤー側）を向いて羽ばたく
			mRetreatPhase = 2;
			mStateTimer = 0;
			mAnimFrame = 0;
			mAnimTimer = 0;
		}
	}
	else
	{
		// Phase 2: 最初に出てきた天井の穴（X = 9500.0f）から真上へ力強く羽ばたいて帰っていく
		if (mAnimTimer >= 4)
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % (mFlyAnim.empty() ? 1 : static_cast<int>(mFlyAnim.size()));
		}

		if (mStateTimer > 12 && !mHasRetreated)
		{
			mvPosition.y -= 11.5f;
			if (mvPosition.y <= 700.0f)
			{
				mvPosition.y = 700.0f;
				mHasRetreated = true;
				StageClear::Trigger();
				SetDeleteFlag(true);
			}
		}
	}
}

// =========================================================================
// 物理移動連携 (EnemyMove)
// =========================================================================
void BossEnemyDragon::EnemyMove(BlockMap& blockMap)
{
	mpBlockMap = &blockMap;

	// 離陸・飛行・空中ブレス・空中強襲ダイブ中は重力落下させず空中軌道を維持する
	if (mState == State::Takeoff || mState == State::Fly || mState == State::AirAttack ||
		mState == State::CenterFireballAttack || (mState == State::AirDiveAttack && mDivePhase < 2) ||
		mState == State::DefeatRetreat || (mState == State::EntranceWait && mEntranceTimer <= 105))
	{
		isGrounded = false;
		velocityY = 0.0f;
		return;
	}

	// 登場落下中、天井ブロック領域（Y < 1350）を通過している間は天井に引っかからないよう直接落下させる
	if (mState == State::EntranceDrop && (mvPosition.y + mfEnemyHeight * 0.5f) < 1350.0f)
	{
		mvPosition.y += velocityY;
		isGrounded = false;
		return;
	}

	mCharacterPhysics.UpdateMoveAndCollision(
		mvPosition,
		velocityY,
		isGrounded,
		mbIsJumping,
		blockMap,
		mfEnemyWidth,
		mfEnemyHeight,
		gravity,
		0.0f,
		0.0f
	);
}

// =========================================================================
// 描画処理 (Draw)
// =========================================================================
void BossEnemyDragon::Draw()
{
	if (mHasRetreated)
	{
		return;
	}

	int handle = -1;
	float footOffsetFromCenter = 79.0f;
	SelectCurrentSprite(handle, footOffsetFromCenter);

	if (handle != -1)
	{
		if (isDamaged)
		{
			SetDrawBright(255, 100, 100);
		}

		int screenX = static_cast<int>(ConvertToScreenX(mvPosition.x, mpBlockMap));
		float feetWorldY = mvPosition.y + (mfEnemyHeight * 0.5f);
		int screenY = static_cast<int>(ConvertToScreenY(feetWorldY - (footOffsetFromCenter * mDrawScale), mpBlockMap));

		// 元スプライトは左向きなので、右向き(mFacingDirection > 0)のとき左右反転描画
		bool reverse = (mFacingDirection > 0.0f);
		double drawAngle = ((mState == State::AirDiveAttack && mDivePhase < 2) || mState == State::CenterFireballAttack) ? static_cast<double>(mDiveAngle) : 0.0;

		// 1. 空中急降下通常攻撃（AirDiveAttack Phase 0）：照準ロックオンサイト＆突撃予告ライン描画
		DrawDiveLockOnSight();

		// 2. 俊足前進中および空中急降下中の残像描画
		DrawAfterimages(screenX, screenY, handle, reverse, drawAngle);

		// 3. ドラゴン本体の描画
		DrawRotaGraph(screenX, screenY, mDrawScale, drawAngle, handle, TRUE, reverse);

		// 4. 空中ロックオン時の赤い十字眼光エフェクト
		DrawDiveEyeFlare(screenX, screenY);

		if (isDamaged)
		{
			SetDrawBright(255, 255, 255);
		}

		// 5. 攻撃予備動作の「！」マーク描画
		DrawWarningExclamation();
	}

	// 6. 火炎ブレス・延焼地面・中央巨大火球・大爆発・アーチ火炎波・火花の描画
	float mouthWX = 0.0f, mouthWY = 0.0f;
	GetAirAttackMouthWorldPos(mouthWX, mouthWY);
	bool isAirBreath = (mState == State::AirAttack);
	bool isCenterCharge = (mState == State::CenterFireballAttack && mCenterAttackPhase == 1);
	mFireGimmick.Draw(mpBlockMap, isAirBreath, isCenterCharge, mAnimFrame, mouthWX, mouthWY);
}

void BossEnemyDragon::SelectCurrentSprite(int& outHandle, float& outFootOffset) const
{
	outHandle = -1;
	outFootOffset = 79.0f; // 256x256キャンバス基準の足元オフセット

	switch (mState)
	{
	case State::EntranceDrop:
		if (!mLandingAnim.empty())
		{
			int idx = mAnimFrame;
			if (idx >= static_cast<int>(mLandingAnim.size())) idx = static_cast<int>(mLandingAnim.size()) - 1;
			outHandle = mLandingAnim[idx];
			outFootOffset = 57.0f; // 400x400キャンバス基準
		}
		break;

	case State::EntranceWait:
		if (!mEntranceLandAnimDone && !mLandingAnim.empty())
		{
			int idx = mAnimFrame;
			if (idx >= static_cast<int>(mLandingAnim.size())) idx = static_cast<int>(mLandingAnim.size()) - 1;
			outHandle = mLandingAnim[idx];
			outFootOffset = 57.0f;
		}
		else if (mEntranceTimer > 105)
		{
			if (!mIdleAnim.empty())
			{
				outHandle = mIdleAnim[mAnimFrame % mIdleAnim.size()];
				outFootOffset = 79.0f;
			}
		}
		else if (mEntranceTimer > 63)
		{
			if (!mTakeoffAnim.empty())
			{
				outHandle = mTakeoffAnim[mAnimFrame % mTakeoffAnim.size()];
				outFootOffset = 80.0f; // 350x350キャンバス基準
			}
		}
		else
		{
			if (!mFlyAnim.empty())
			{
				outHandle = mFlyAnim[mAnimFrame % mFlyAnim.size()];
				outFootOffset = 57.0f; // 400x400キャンバス基準
			}
		}
		break;

	case State::Idle:
		if (!mIdleAnim.empty())
		{
			outHandle = mIdleAnim[mAnimFrame % mIdleAnim.size()];
			outFootOffset = 79.0f;
		}
		break;

	case State::Move:
		if (!mMoveAnim.empty())
		{
			outHandle = mMoveAnim[mAnimFrame % mMoveAnim.size()];
			outFootOffset = 79.0f;
		}
		break;

	case State::Attack:
		outFootOffset = 79.0f;
		if (mAttackPhase == 0)
		{
			if (!mIdleAnim.empty()) outHandle = mIdleAnim[mAnimFrame % mIdleAnim.size()];
		}
		else if (mAttackPhase == 1 || mAttackPhase == 3)
		{
			if (!mMoveAnim.empty()) outHandle = mMoveAnim[mAnimFrame % mMoveAnim.size()];
		}
		else
		{
			if (!mAttackAnim.empty()) outHandle = mAttackAnim[mAnimFrame % mAttackAnim.size()];
		}
		break;

	case State::Takeoff:
		if (!mTakeoffAnim.empty())
		{
			outHandle = mTakeoffAnim[mAnimFrame % mTakeoffAnim.size()];
			outFootOffset = 80.0f; // 350x350キャンバス基準
		}
		break;

	case State::Fly:
		if (!mFlyAnim.empty())
		{
			outHandle = mFlyAnim[mAnimFrame % mFlyAnim.size()];
			outFootOffset = 57.0f; // 400x400キャンバス基準
		}
		break;

	case State::AirAttack:
		if (!mAirAttackAnim.empty())
		{
			outHandle = mAirAttackAnim[mAnimFrame % mAirAttackAnim.size()];
			outFootOffset = 57.0f; // 400x400キャンバス基準
		}
		break;

	case State::AirDiveAttack:
		if (!mAttackAnim.empty())
		{
			outHandle = mAttackAnim[mAnimFrame % mAttackAnim.size()];
			outFootOffset = 79.0f; // 通常攻撃(new_attack: 256x256キャンバス)基準
		}
		break;

	case State::CenterFireballAttack:
		outFootOffset = 57.0f; // 400x400キャンバス基準
		if (mCenterAttackPhase == 0 || mCenterAttackPhase == 2)
		{
			if (!mFlyAnim.empty()) outHandle = mFlyAnim[mAnimFrame % mFlyAnim.size()];
		}
		else
		{
			if (!mAirAttackAnim.empty()) outHandle = mAirAttackAnim[mAnimFrame % mAirAttackAnim.size()];
		}
		break;

	case State::Landing:
		if (!mLandingAnim.empty())
		{
			outHandle = mLandingAnim[mAnimFrame % mLandingAnim.size()];
			outFootOffset = 57.0f; // 400x400キャンバス基準
		}
		break;

	case State::DefeatRetreat:
		if (mRetreatPhase == 0)
		{
			if (!mTakeoffAnim.empty())
			{
				outHandle = mTakeoffAnim[mAnimFrame % mTakeoffAnim.size()];
				outFootOffset = 80.0f; // 350x350キャンバス基準
			}
		}
		else
		{
			if (!mFlyAnim.empty())
			{
				outHandle = mFlyAnim[mAnimFrame % mFlyAnim.size()];
				outFootOffset = 57.0f; // 400x400キャンバス基準
			}
		}
		break;
	}
}

void BossEnemyDragon::DrawDiveLockOnSight() const
{
	if (mState != State::AirDiveAttack || mDivePhase != 0 || mpBlockMap == nullptr) return;

	float aimWorldX = mDiveTargetX + mFacingDirection * 95.0f;
	float aimWorldY = 1865.0f;
	int startSX = static_cast<int>(ConvertToScreenX(mvPosition.x + mFacingDirection * 95.0f, mpBlockMap));
	int startSY = static_cast<int>(ConvertToScreenY(mvPosition.y - 40.0f, mpBlockMap));
	int endSX = static_cast<int>(ConvertToScreenX(aimWorldX, mpBlockMap));
	int endSY = static_cast<int>(ConvertToScreenY(aimWorldY, mpBlockMap));

	bool isLocked = (mStateTimer > 26);
	unsigned int sightCol = isLocked ? GetColor(255, 230, 120) : GetColor(255, 45, 55);
	unsigned int coreCol = isLocked ? GetColor(255, 255, 255) : GetColor(255, 130, 130);

	float ldx = static_cast<float>(endSX - startSX);
	float ldy = static_cast<float>(endSY - startSY);
	float lLen = std::sqrt(ldx * ldx + ldy * ldy);
	if (lLen <= 1.0f) return;

	float ux = ldx / lLen;
	float uy = ldy / lLen;
	float nx = -uy;
	float ny = ux;

	// 1. 突撃軌道の淡いレーザー帯
	SetDrawBlendMode(DX_BLENDMODE_ADD, isLocked ? 140 : 85);
	int bandW = isLocked ? 22 : 14;
	DrawQuadrangle(
		startSX + static_cast<int>(nx * bandW), startSY + static_cast<int>(ny * bandW),
		endSX + static_cast<int>(nx * bandW), endSY + static_cast<int>(ny * bandW),
		endSX - static_cast<int>(nx * bandW), endSY - static_cast<int>(ny * bandW),
		startSX - static_cast<int>(nx * bandW), startSY - static_cast<int>(ny * bandW),
		GetColor(220, 25, 35), TRUE
	);

	// 2. 中心の鋭いロックオンレーザー線
	SetDrawBlendMode(DX_BLENDMODE_ADD, 230);
	DrawLine(startSX, startSY, endSX, endSY, sightCol, isLocked ? 4 : 2);
	DrawLine(startSX, startSY, endSX, endSY, coreCol, 1);

	// 3. 軌道上を流れるシェブロン矢印（＞＞＞）
	int stepDist = 65;
	int offset = (mStateTimer * 8) % stepDist;
	for (int d = offset; d < static_cast<int>(lLen) - 25; d += stepDist)
	{
		int cx = startSX + static_cast<int>(ux * d);
		int cy = startSY + static_cast<int>(uy * d);
		int tipX = cx + static_cast<int>(ux * 16.0f);
		int tipY = cy + static_cast<int>(uy * 16.0f);
		int wX = static_cast<int>(nx * 14.0f);
		int wY = static_cast<int>(ny * 14.0f);
		DrawLine(cx + wX, cy + wY, tipX, tipY, sightCol, 3);
		DrawLine(cx - wX, cy - wY, tipX, tipY, sightCol, 3);
	}

	// 4. プレイヤー位置に収束する照準レティクル（ロックオンマーカー）
	int reticleR = isLocked ? 34 : (58 - mStateTimer);
	if (reticleR < 34) reticleR = 34;
	DrawCircle(endSX, endSY, reticleR, sightCol, FALSE);
	DrawCircle(endSX, endSY, reticleR - 6, coreCol, FALSE);
	// 十字照準ライン
	DrawLine(endSX - reticleR - 12, endSY, endSX - reticleR / 3, endSY, sightCol, 3);
	DrawLine(endSX + reticleR / 3, endSY, endSX + reticleR + 12, endSY, sightCol, 3);
	DrawLine(endSX, endSY - reticleR - 12, endSX, endSY - reticleR / 3, sightCol, 3);
	DrawLine(endSX, endSY + reticleR / 3, endSX, endSY + reticleR + 12, sightCol, 3);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void BossEnemyDragon::DrawAfterimages(int screenX, int screenY, int handle, bool reverse, double drawAngle) const
{
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
	else if (mState == State::AirDiveAttack && (mDivePhase == 1 || (mDivePhase == 2 && mAnimFrame <= 5)))
	{
		int ox1 = static_cast<int>(mDiveVx * 1.1f);
		int oy1 = static_cast<int>(mDiveVy * 1.1f);
		int ox2 = static_cast<int>(mDiveVx * 2.2f);
		int oy2 = static_cast<int>(mDiveVy * 2.2f);
		int ox3 = static_cast<int>(mDiveVx * 3.3f);
		int oy3 = static_cast<int>(mDiveVy * 3.3f);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 55);
		SetDrawBright(255, 70, 90);
		DrawRotaGraph(screenX - ox3, screenY - oy3, mDrawScale, drawAngle, handle, TRUE, reverse);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 95);
		SetDrawBright(100, 190, 255);
		DrawRotaGraph(screenX - ox2, screenY - oy2, mDrawScale, drawAngle, handle, TRUE, reverse);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
		DrawRotaGraph(screenX - ox1, screenY - oy1, mDrawScale, drawAngle, handle, TRUE, reverse);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		if (isDamaged) SetDrawBright(255, 100, 100);
		else SetDrawBright(255, 255, 255);
	}
}

void BossEnemyDragon::DrawDiveEyeFlare(int screenX, int screenY) const
{
	if (mState == State::AirDiveAttack && mDivePhase == 0)
	{
		int eyeSX = screenX + static_cast<int>(mFacingDirection * 115.0f);
		int eyeSY = screenY - 35 + static_cast<int>(std::fabs(mDiveAngle) * 45.0f);
		int flareLen = (mStateTimer > 26) ? 28 : 18;
		SetDrawBlendMode(DX_BLENDMODE_ADD, 230);
		DrawLine(eyeSX - flareLen, eyeSY, eyeSX + flareLen, eyeSY, GetColor(255, 40, 40), 3);
		DrawLine(eyeSX, eyeSY - flareLen, eyeSX, eyeSY + flareLen, GetColor(255, 40, 40), 3);
		DrawCircle(eyeSX, eyeSY, 6, GetColor(255, 220, 220), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

void BossEnemyDragon::DrawWarningExclamation() const
{
	if ((mState == State::Attack && mAttackPhase == 0) || (mState == State::AirDiveAttack && mDivePhase == 0))
	{
		int exX = static_cast<int>(ConvertToScreenX(mvPosition.x + mFacingDirection * 85.0f, mpBlockMap));
		int exY = static_cast<int>(ConvertToScreenY(mvPosition.y - 165.0f, mpBlockMap));
		bool blinkOff = (mStateTimer / 4) % 2 != 0;
		if (!blinkOff)
		{
			// 黒縁取り＋赤色の「！」マーク
			DrawBox(exX - 10, exY - 62, exX + 10, exY - 13, GetColor(0, 0, 0), TRUE);
			DrawBox(exX - 10, exY - 7, exX + 10, exY + 12, GetColor(0, 0, 0), TRUE);
			DrawBox(exX - 8, exY - 60, exX + 8, exY - 15, GetColor(255, 20, 20), TRUE);
			DrawBox(exX - 8, exY - 5, exX + 8, exY + 10, GetColor(255, 20, 20), TRUE);
		}
	}
}

// =========================================================================
// ボス登場時の「ドラゴン」文字演出＆画面下部HPバー描画
// =========================================================================
void BossEnemyDragon::DrawBossUI(Player* player, BlockMap& blockMap) const
{
	int waitT = mEntranceTimer;
	int postT = mPostEntranceTimer;

	// 1. 登場時の「ドラゴン」ネームロゴ表示＆溶解エフェクト
	if (mState == State::EntranceWait || postT > 0)
	{
		int charsToDraw = 0;
		int alpha = 255;
		if (mState == State::EntranceWait)
		{
			int elapsedAppear = 240 - waitT;
			if (elapsedAppear > 120) elapsedAppear = 120;
			charsToDraw = (elapsedAppear * 4) / 120;
		}
		else if (postT > 0)
		{
			charsToDraw = 4;
			alpha = (postT * 255) / 60;
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		}

		if (charsToDraw > 0 && player != nullptr)
		{
			std::string dragonName = "ドラゴン";
			int fontSize = 120;
			float playerScreenX = player->GetPosition().x - blockMap.GetScrollX();
			float bossScreenX = mvPosition.x - blockMap.GetScrollX();
			float centerX = (playerScreenX + bossScreenX) / 2.0f;
			int startX = static_cast<int>(centerX) - (2 * fontSize);
			int startY = 340;
			int now = GetNowCount();

			for (int i = 0; i < charsToDraw; i++)
			{
				std::string charStr = dragonName.substr(i * 2, 2);
				unsigned int color = (i < 2) ? GetColor(255, 80, 60) : GetColor(100, 200, 255);
				int charX = startX + (i * fontSize);
				int swayY = static_cast<int>(std::sin(now / 150.0f + i * 0.8f) * 15.0f);
				int cMeltY = 0;
				if (postT > 0)
				{
					float meltTime = (60.0f - postT) - (i * 2.5f);
					if (meltTime < 0.0f) meltTime = 0.0f;
					float mt = meltTime / 60.0f;
					cMeltY = static_cast<int>(mt * mt * 600.0f);
				}
				int charY = startY + swayY + cMeltY;
				Master::mpGameManager->GetFontManager()->DrawDotString(charX, charY, fontSize, color, "%s", charStr.c_str());
			}
		}
		if (postT > 0)
		{
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	// 2. ボスHPバー表示
	bool shouldDrawHpBar = true;
	float fillMultiplier = 1.0f;
	if (mState == State::EntranceDrop)
	{
		shouldDrawHpBar = false;
	}
	else if (mState == State::EntranceWait)
	{
		int elapsedAppear = 240 - mEntranceTimer;
		if (elapsedAppear < 120)
		{
			shouldDrawHpBar = false;
		}
		else
		{
			fillMultiplier = (elapsedAppear - 120) / 120.0f;
		}
	}
	if (mHp <= 0 && mSmoothHp <= 0.0f)
	{
		shouldDrawHpBar = false;
	}

	if (shouldDrawHpBar)
	{
		int barX = ScreenSize::CenterX - 600;
		int barY = ScreenSize::ScrrenHeight - 120;
		int barW = 1200;
		int barH = 40;
		int shakeX = 0, shakeY = 0;
		if (mHpShakeTimer > 0)
		{
			shakeX = (std::rand() % 11) - 5;
			shakeY = (std::rand() % 11) - 5;
		}
		int bx = barX + shakeX;
		int by = barY + shakeY;

		DrawBox(bx - 6, by - 6, bx + barW + 6, by + barH + 6, GetColor(0, 0, 0), TRUE);
		DrawBox(bx, by, bx + barW, by + barH, GetColor(50, 50, 50), TRUE);

		float smoothRatio = (mSmoothHp / static_cast<float>(mMaxHp)) * fillMultiplier;
		if (smoothRatio > 0.0f)
		{
			if (smoothRatio > 1.0f) smoothRatio = 1.0f;
			DrawBox(bx, by, bx + static_cast<int>(barW * smoothRatio), by + barH, GetColor(220, 50, 50), TRUE);
		}

		float actualRatio = (mHp / static_cast<float>(mMaxHp)) * fillMultiplier;
		if (actualRatio > 0.0f)
		{
			if (actualRatio > 1.0f) actualRatio = 1.0f;
			DrawBox(bx, by, bx + static_cast<int>(barW * actualRatio), by + barH, GetColor(50, 220, 50), TRUE);
		}

		DrawBox(bx, by, bx + barW, by + barH, GetColor(255, 255, 255), FALSE);

		if (fillMultiplier > 0.0f)
		{
			int nameFz = 48;
			int alpha = static_cast<int>(255 * fillMultiplier);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			Master::mpGameManager->GetFontManager()->DrawDotString(bx, by - 52, nameFz, GetColor(255, 100, 80), "%s", "ドラゴン");
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}
}

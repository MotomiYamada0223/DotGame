#include "Stage3BossDragon.h"
#include "Effekseer2DManager.h"
#include "Stage3Magma.h"
#include "Player.h"
#include "BlockMap.h"
#include "Collision.h"
#include "GameConstants.h"
#include "Master.h"
#include "GameManager.h"
#include "FontManager.h"
#include "SceneManager.h"
#include "GameScene.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>

Stage3BossDragon::Stage3BossDragon()
	: Stage3BossDragon(VGet(SPAWN_X, SPAWN_Y, 0.0f))
{
}

Stage3BossDragon::Stage3BossDragon(VECTOR pos)
	: Enemy("Resource/Enemy/Dragon/animation_dragon/new_idle/sprite_idle1.png", pos)
	, mState(State::CutsceneIdle)
	, mbCutsceneRoaring(false)
	, mRoarTimer(0)
	, mbIsPhase2(false)
	, mbIsGolden(false)
	, mbAwakeningMovieRequested(false)
	, mAwakenChargeRatio(0.0f)
	, mbTriggerFinalUltimate(false)
	, mStunTimer(0)
	, mDownPhase(0)
	, mDownTargetX(SPAWN_X)
	, mDownFloorY(1152.0f)
	, mDownVy(0.0f)
	, mAnimFrame(0)
	, mAnimTimer(0)
	, mStateTimer(0)
	, mFacingDirection(-1.0f)
	, mDrawScale(1.6f)
	, mSmoothHp(80.0f)
	, mHpShakeTimer(0)
	, mAttackCooldown(45)
	, mSpecialAttackCooldown(60)
	, mDefeatTimer(0)
	, mAttackPatternIndex(0)
	, mLastAttackId(-1)
	, mRoarAttackAnimType(0)
	, mRoarAttackAnimFrame(0)
	, mMagmaFireballAnimType(0)
	, mMagmaFireballAnimFrame(0)
	, mAirDashComboAnimType(0)
	, mAirDashComboAnimFrame(0)
	, mBounceFireballAnimType(0)
	, mBounceFireballAnimFrame(0)
	, mCloneDashAnimType(0)
	, mCloneDashAnimFrame(0)
	, mCloneMeteorDashAnimType(0)
	, mCloneMeteorDashAnimFrame(0)
	, mUltimateWindSlamAnimType(0)
	, mUltimateWindSlamAnimFrame(0)
	, mCloneWindRoarAnimType(0)
	, mCloneWindRoarAnimFrame(0)
	, mSsjFireTex{ -1, -1, -1, -1 }
	, mSsjThunderTex{ -1, -1, -1, -1, -1, -1, -1, -1 }
	, mEyeGlintTimer(0)
{
	UpdateStatusByProgress(GameProgress::Tutorial1);
	LoadAnimations();
	mRoarAttack.Initialize();
	mMagmaFireballAttack.Initialize();
	mAirDashComboAttack.Initialize();
	mBounceFireballAttack.Initialize();
	mCloneDashAttack.Initialize();
	mCloneMeteorDashAttack.Initialize();
	mUltimateWindSlamAttack.Initialize();
	mCloneWindRoarAttack.Initialize();

	mfEnemyWidth  = 220.0f;
	mfEnemyHeight = 200.0f;
	moveSpeed     = 0.0f;
}

Stage3BossDragon::~Stage3BossDragon()
{
	mUltimateWindSlamAttack.Reset(nullptr);
	mCloneWindRoarAttack.Reset(nullptr);

	for (int h : mIdleAnim)            if (h != -1) DeleteGraph(h);
	for (int h : mMoveAnim)            if (h != -1) DeleteGraph(h);
	for (int h : mAttackAnim)          if (h != -1) DeleteGraph(h);
	for (int h : mTakeoffAnim)         if (h != -1) DeleteGraph(h);
	for (int h : mFlyAnim)             if (h != -1) DeleteGraph(h);
	for (int h : mAirAttackAnim)       if (h != -1) DeleteGraph(h);
	for (int h : mLandingAnim)         if (h != -1) DeleteGraph(h);
	for (int h : mRoarAnim)            if (h != -1) DeleteGraph(h);
	for (int h : mChargeAnim)          if (h != -1) DeleteGraph(h);
	for (int h : mDownAnim)            if (h != -1) DeleteGraph(h);

	for (int h : mGoldenIdleAnim)      if (h != -1) DeleteGraph(h);
	for (int h : mGoldenMoveAnim)      if (h != -1) DeleteGraph(h);
	for (int h : mGoldenAttackAnim)    if (h != -1) DeleteGraph(h);
	for (int h : mGoldenTakeoffAnim)   if (h != -1) DeleteGraph(h);
	for (int h : mGoldenFlyAnim)       if (h != -1) DeleteGraph(h);
	for (int h : mGoldenAirAttackAnim) if (h != -1) DeleteGraph(h);
	for (int h : mGoldenLandingAnim)   if (h != -1) DeleteGraph(h);
	for (int h : mGoldenRoarAnim)      if (h != -1) DeleteGraph(h);
	for (int h : mGoldenChargeAnim)    if (h != -1) DeleteGraph(h);
	for (int h : mGoldenDownAnim)      if (h != -1) DeleteGraph(h);

	for (int i = 0; i < 4; ++i) if (mSsjFireTex[i] != -1)    DeleteGraph(mSsjFireTex[i]);
	for (int i = 0; i < 8; ++i) if (mSsjThunderTex[i] != -1) DeleteGraph(mSsjThunderTex[i]);
}

int Stage3BossDragon::LoadGoldenDragonGraph(const char* filePath)
{
	int softImg = LoadSoftImage(filePath);
	if (softImg == -1)
	{
		return LoadGraph(filePath);
	}

	int w = 0, h = 0;
	GetSoftImageSize(softImg, &w, &h);

	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			int r = 0, g = 0, b = 0, a = 0;
			GetPixelSoftImage(softImg, x, y, &r, &g, &b, &a);
			if (a <= 10) continue;

			// 本体の赤い目は真紅のルビー色に保ち、威厳を強調する
			if (a > 120 && r > g + 30 && r > b + 15 && r > 110)
			{
				DrawPixelSoftImage(softImg, x, y, 255, 35, 45, a);
				continue;
			}

			// 体の鱗・翼・輪郭の輝度を算出し、重厚かつ神々しいメタリックゴールドへ変換
			int lum = (r * 299 + g * 587 + b * 114) / 1000;
			int nr = 0, ng = 0, nb = 0;
			if (lum < 45)
			{
				// 輪郭・暗部：深いブロンズゴールド
				nr = 45 + (lum * 110) / 45;
				ng = 28 + (lum * 75) / 45;
				nb = 5  + (lum * 15) / 45;
			}
			else if (lum < 155)
			{
				// 中間調：鮮やかな黄金色
				int t = lum - 45;
				nr = 155 + (t * 100) / 110;
				ng = 103 + (t * 112) / 110;
				nb = 20  + (t * 35) / 110;
			}
			else
			{
				// ハイライト：輝く白金ゴールド
				int t = lum - 155;
				nr = 255;
				ng = 215 + (t * 38) / 100;
				nb = 55  + (t * 135) / 100;
			}
			if (nr > 255) nr = 255;
			if (ng > 255) ng = 255;
			if (nb > 255) nb = 255;

			DrawPixelSoftImage(softImg, x, y, nr, ng, nb, a);
		}
	}

	int graphHandle = CreateGraphFromSoftImage(softImg);
	DeleteSoftImage(softImg);
	return graphHandle;
}

void Stage3BossDragon::RecordEyeOffset(const char* filePath, int handle1, int handle2)
{
	int softImg = LoadSoftImage(filePath);
	if (softImg == -1) return;

	int w = 0, h = 0;
	GetSoftImageSize(softImg, &w, &h);

	std::vector<std::pair<int, int>> strictPts;
	std::vector<std::pair<int, int>> fallbackPts;
	int strictMinY = h;
	int fallbackMinY = h;
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			int r = 0, g = 0, b = 0, a = 0;
			GetPixelSoftImage(softImg, x, y, &r, &g, &b, &a);
			if (a > 120 && r >= 200 && g <= 75 && r > g + 60)
			{
				strictPts.push_back(std::make_pair(x, y));
				if (y < strictMinY) strictMinY = y;
			}
			else if (a > 120 && r > g + 30 && r > b + 15 && r > 110)
			{
				fallbackPts.push_back(std::make_pair(x, y));
				if (y < fallbackMinY) fallbackMinY = y;
			}
		}
	}
	DeleteSoftImage(softImg);

	const std::vector<std::pair<int, int>>& usePts = !strictPts.empty() ? strictPts : fallbackPts;
	int useMinY = !strictPts.empty() ? strictMinY : fallbackMinY;

	double sumX = 0.0;
	double sumY = 0.0;
	int count = 0;
	for (size_t i = 0; i < usePts.size(); ++i)
	{
		// 咆哮口元の赤い発光を除外し、最上部の目クラスタのみを集計する
		if (usePts[i].second <= useMinY + 14)
		{
			sumX += static_cast<double>(usePts[i].first);
			sumY += static_cast<double>(usePts[i].second);
			++count;
		}
	}

	if (count > 0)
	{
		float offX = static_cast<float>(sumX / count) - (static_cast<float>(w) * 0.5f);
		float offY = static_cast<float>(sumY / count) - (static_cast<float>(h) * 0.5f);
		if (handle1 != -1) mEyeOffsetsByHandle[handle1] = std::make_pair(offX, offY);
		if (handle2 != -1) mEyeOffsetsByHandle[handle2] = std::make_pair(offX, offY);
	}
}

void Stage3BossDragon::LoadAnimations()
{
	LoadDivGraph("Resource/Effect/Pierre01/Pierre01/Texture/Fire.png", 4, 2, 2, 256, 256, mSsjFireTex);
	LoadDivGraph("Resource/Effect/tktk02/tktk02/Parts/Thunder2.png", 8, 2, 4, 128, 64, mSsjThunderTex);
	for (int i = 1; i <= 4; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_idle/sprite_idle%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mIdleAnim.push_back(h);
		int gh = LoadGoldenDragonGraph(path);
		if (gh != -1) mGoldenIdleAnim.push_back(gh);
		RecordEyeOffset(path, h, gh);
	}
	for (int i = 1; i <= 4; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_move/sprite_move%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mMoveAnim.push_back(h);
		int gh = LoadGoldenDragonGraph(path);
		if (gh != -1) mGoldenMoveAnim.push_back(gh);
		RecordEyeOffset(path, h, gh);
	}
	for (int i = 1; i <= 9; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_attack/sprite_attack%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mAttackAnim.push_back(h);
		int gh = LoadGoldenDragonGraph(path);
		if (gh != -1) mGoldenAttackAnim.push_back(gh);
		RecordEyeOffset(path, h, gh);
	}
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_takeoff/sprite_takeoff%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mTakeoffAnim.push_back(h);
		int gh = LoadGoldenDragonGraph(path);
		if (gh != -1) mGoldenTakeoffAnim.push_back(gh);
		RecordEyeOffset(path, h, gh);
	}
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_flying/sprite_fly%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mFlyAnim.push_back(h);
		int gh = LoadGoldenDragonGraph(path);
		if (gh != -1) mGoldenFlyAnim.push_back(gh);
		RecordEyeOffset(path, h, gh);
	}
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_airborne/sprite_airAttack%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mAirAttackAnim.push_back(h);
		int gh = LoadGoldenDragonGraph(path);
		if (gh != -1) mGoldenAirAttackAnim.push_back(gh);
		RecordEyeOffset(path, h, gh);
	}
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_landing/sprite_landing%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mLandingAnim.push_back(h);
		int gh = LoadGoldenDragonGraph(path);
		if (gh != -1) mGoldenLandingAnim.push_back(gh);
		RecordEyeOffset(path, h, gh);
	}
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_roar/roar%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mRoarAnim.push_back(h);
		int gh = LoadGoldenDragonGraph(path);
		if (gh != -1) mGoldenRoarAnim.push_back(gh);
		RecordEyeOffset(path, h, gh);
	}
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_charge/dragon_charge%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mChargeAnim.push_back(h);
		int gh = LoadGoldenDragonGraph(path);
		if (gh != -1) mGoldenChargeAnim.push_back(gh);
		RecordEyeOffset(path, h, gh);
	}
	for (int i = 1; i <= 10; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_down/sprite_down%02d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mDownAnim.push_back(h);
		int gh = LoadGoldenDragonGraph(path);
		if (gh != -1) mGoldenDownAnim.push_back(gh);
	}
}

void Stage3BossDragon::UpdateStatusByProgress(GameProgress progress)
{
	(void)progress;
	mMaxHp    = 80;
	mHp       = 80;
	mAttack   = 15;
	mDefense  = 5;
	mMagic    = 10;
	mSmoothHp = 80.0f;
}

void Stage3BossDragon::Reset()
{
	GameScene* gameScene = nullptr;
	if (Master::mpGameManager && Master::mpGameManager->GetSceneManager())
	{
		gameScene = dynamic_cast<GameScene*>(Master::mpGameManager->GetSceneManager()->GetCurrentScene());
	}
	Player* player = (gameScene != nullptr) ? gameScene->GetPlayer() : nullptr;
	mbIsPhase2                = false;
	mbIsGolden                = false;
	mbAwakeningMovieRequested = false;
	mAwakenChargeRatio        = 0.0f;
	mbTriggerFinalUltimate    = false;
	mStunTimer                = 0;
	mDownPhase                = 0;
	mDownVy                   = 0.0f;
	mAttackPatternIndex       = 0;
	mLastAttackId             = -1;
	mRoarAttack.Reset();
	mMagmaFireballAttack.Reset();
	mAirDashComboAttack.Reset();
	mBounceFireballAttack.Reset();
	mCloneDashAttack.Reset();
	mCloneMeteorDashAttack.Reset();
	mUltimateWindSlamAttack.Reset(player);
	mCloneWindRoarAttack.Reset(player);
	mEyeGlintTimer = 0;
}

void Stage3BossDragon::SetCutsceneMode(bool inCutscene)
{
	if (inCutscene)
	{
		mState              = State::CutsceneIdle;
		mbCutsceneRoaring   = false;
		mRoarTimer          = 0;
		mAnimFrame          = 0;
		mAnimTimer          = 0;
		mStateTimer         = 0;
		mFacingDirection    = -1.0f;
		mRoarAttack.Reset();
		mMagmaFireballAttack.Reset();
		mAirDashComboAttack.Reset();
		mBounceFireballAttack.Reset();
		mCloneDashAttack.Reset();
		mCloneMeteorDashAttack.Reset();
		mUltimateWindSlamAttack.Reset();
		mCloneWindRoarAttack.Reset();
	}
	else
	{
		mState                 = State::Idle;
		mbCutsceneRoaring      = false;
		mAnimFrame             = 0;
		mAnimTimer             = 0;
		mStateTimer            = 0;
		mSpecialAttackCooldown = 40;
		mAttackPatternIndex    = 0;
		mLastAttackId          = -1;
	}
}

void Stage3BossDragon::TriggerCutsceneRoar()
{
	mbCutsceneRoaring = true;
	mRoarTimer        = 0;
	mAnimFrame        = 0;
	mAnimTimer        = 0;
}

bool Stage3BossDragon::ConsumeAwakeningMovieRequest()
{
	if (mbAwakeningMovieRequested)
	{
		mbAwakeningMovieRequested = false;
		return true;
	}
	return false;
}

void Stage3BossDragon::UpdateAwakeningCutscenePose(float x, float y, float facingDir, bool roaring, float chargeRatio)
{
	mState             = State::AwakeningCutscene;
	mvPosition.x       = x;
	mvPosition.y       = y;
	mFacingDirection   = facingDir;
	mAwakenChargeRatio = chargeRatio;
	mbCutsceneRoaring  = roaring;
}

void Stage3BossDragon::TriggerGoldenAwakening()
{
	mbIsPhase2        = true;
	mbIsGolden        = true;
	mbCutsceneRoaring = true;
	mRoarTimer        = 0;
	mAnimFrame        = 0;
	mAnimTimer        = 0;
	mMaxHp            = 80;
	mHp               = 80;
	mSmoothHp         = 80.0f;
}

void Stage3BossDragon::StartPhase2Battle()
{
	mbIsPhase2             = true;
	mbIsGolden             = true;
	mbCutsceneRoaring      = false;
	mAwakenChargeRatio     = 1.0f;
	mvPosition.x           = SPAWN_X;
	mvPosition.y           = SPAWN_Y;
	mFacingDirection       = -1.0f;
	isGrounded             = true;
	velocityY              = 0.0f;
	mState                 = State::Idle;
	mStateTimer            = 0;
	mAnimFrame             = 0;
	mAnimTimer             = 0;
	mSpecialAttackCooldown = 40;
	mAttackPatternIndex    = 0;
	mLastAttackId          = -1;
	mbTriggerFinalUltimate = false;
	mStunTimer             = 0;
	mDownPhase             = 0;
	mDownVy                = 0.0f;
}

bool Stage3BossDragon::IsActiveTarget() const
{
	if (mState == State::CutsceneIdle ||
		mState == State::AwakeningCutscene ||
		mState == State::UltimateWindSlamAttack ||
		mState == State::Defeated)
	{
		return false;
	}
	return (mHp > 0);
}

bool Stage3BossDragon::CheckAttackCollision(const VECTOR& targetPos, const VECTOR& targetSize) const
{
	if (!IsAttacking()) return false;
	float left = 0.0f, top = 0.0f, right = 0.0f, bottom = 0.0f;
	GetAttackHitbox(left, top, right, bottom);
	VECTOR atkPos = VGet(left, top, 0.0f);
	VECTOR atkSize = VGet(right - left, bottom - top, 0.0f);
	return Collision::CheckRectToRect(targetPos, targetSize, atkPos, atkSize);
}

bool Stage3BossDragon::CanDealContactDamage() const
{
	if (mState == State::CutsceneIdle ||
		mState == State::AwakeningCutscene ||
		mState == State::Down ||
		mState == State::FinalStun ||
		mState == State::Defeated)
	{
		return false;
	}
	return (mHp > 0);
}

bool Stage3BossDragon::IsChargingAirRoar() const
{
	if (mState == State::RoarAttack && mRoarAttack.IsChargingRoar())
	{
		return true;
	}
	if (mState == State::CloneWindRoarAttack && mCloneWindRoarAttack.IsChargingAirRoar())
	{
		return true;
	}
	return false;
}

void Stage3BossDragon::TakeMagicDamage(int damage)
{
	if (mState == State::CutsceneIdle ||
		mState == State::AwakeningCutscene ||
		mState == State::UltimateWindSlamAttack ||
		mState == State::Defeated ||
		mHp <= 0)
	{
		return;
	}

	bool  wasChargingAirRoar = IsChargingAirRoar();
	float roarTargetX        = mvPosition.x;
	// 【Stage3.png 5724x1692拡大版 対応】中央X=4353・左右足場床Y(1262/1289)に更新
	float roarFloorY         = (mvPosition.x < 4353.0f) ? 1262.0f : 1289.0f;
	if (mState == State::RoarAttack)
	{
		roarTargetX = mRoarAttack.GetTargetX();
		roarFloorY  = mRoarAttack.GetTargetFloorY();
	}
	else if (mState == State::CloneWindRoarAttack)
	{
		roarTargetX = mCloneWindRoarAttack.GetRealTargetX();
		roarFloorY  = mCloneWindRoarAttack.GetRealFloorY();
	}

	TakeDamage(damage);

	if (wasChargingAirRoar &&
		mState != State::AwakeningCutscene &&
		mState != State::UltimateWindSlamAttack &&
		mState != State::Defeated &&
		mState != State::FinalStun)
	{
		GameScene* gameScene = nullptr;
		if (Master::mpGameManager && Master::mpGameManager->GetSceneManager())
		{
			gameScene = dynamic_cast<GameScene*>(Master::mpGameManager->GetSceneManager()->GetCurrentScene());
		}
		Player* player = (gameScene != nullptr) ? gameScene->GetPlayer() : nullptr;

		mRoarAttack.Reset(player);
		mCloneWindRoarAttack.Reset(player);

		if (mpBlockMap != nullptr)
		{
			mpBlockMap->mShakeX = 0;
			mpBlockMap->mShakeY = 0;
		}

		mState       = State::Down;
		mDownPhase   = 0;
		mStateTimer  = 0;
		mStunTimer   = 0;
		mAnimFrame   = 0;
		mAnimTimer   = 0;
		mDownTargetX = roarTargetX;
		mDownFloorY  = roarFloorY;
		mDownVy      = -3.5f;
		isGrounded   = false;
		velocityY    = 0.0f;
	}
}

void Stage3BossDragon::TakeDamage(int damage)
{
	if (mState == State::CutsceneIdle ||
		mState == State::AwakeningCutscene ||
		mState == State::UltimateWindSlamAttack ||
		mState == State::Defeated ||
		mHp <= 0)
	{
		return;
	}

	GameScene* gameScene = nullptr;
	if (Master::mpGameManager && Master::mpGameManager->GetSceneManager())
	{
		gameScene = dynamic_cast<GameScene*>(Master::mpGameManager->GetSceneManager()->GetCurrentScene());
	}
	Player* player = (gameScene != nullptr) ? gameScene->GetPlayer() : nullptr;

	// 1. 最後の大技終了後の8秒間スタン中に攻撃された場合：トドメ成立で撃破・勝利！
	if (mState == State::FinalStun)
	{
		mHp           = 0;
		mSmoothHp     = 0.0f;
		mHpShakeTimer = 25;
		mState        = State::Defeated;
		mDefeatTimer  = 0;
		if (player != nullptr)
		{
			player->SetWindFloatMode(false);
		}
		if (gameScene != nullptr)
		{
			gameScene->GetTutorialText().OnBossDefeated();
		}
		if (mpBlockMap != nullptr)
		{
			mpBlockMap->mShakeX = 24;
			mpBlockMap->mShakeY = 24;
		}
		return;
	}

	// 2. 第1形態でHPが0になった場合：倒れずに中央へ行って黄金に覚醒するムービーへ移行！
	if (!mbIsPhase2)
	{
		mHp -= damage;
		if (mHp < 0) mHp = 0;
		mHpShakeTimer = 15;
		if (mHp <= 0)
		{
			mHp                       = 1; // Object2Dリストや当たり判定から消えないよう1を保持しつつ覚醒ムービーへ
			mSmoothHp                 = 0.0f;
			mState                    = State::AwakeningCutscene;
			mStateTimer               = 0;
			mAnimFrame                = 0;
			mAnimTimer                = 0;
			mbCutsceneRoaring         = false;
			mAwakenChargeRatio        = 0.0f;
			mbAwakeningMovieRequested = true;

			mRoarAttack.Reset();
			mMagmaFireballAttack.Reset();
			mAirDashComboAttack.Reset();
			mBounceFireballAttack.Reset();
			mCloneDashAttack.Reset();
			mCloneMeteorDashAttack.Reset();
			mUltimateWindSlamAttack.Reset(player);
			mCloneWindRoarAttack.Reset(player);
		}
		return;
	}

	// 3. 第2形態「アルティメットドラゴン」：HPが1以下になる時は強制的にHP1で残し、最後の風炎玉攻撃を発動！
	mHp -= damage;
	mHpShakeTimer = 15;
	if (mHp <= 1)
	{
		mHp = 1;
		if (!mbTriggerFinalUltimate)
		{
			mbTriggerFinalUltimate = true;
			// 現在実行中の分身攻撃をクリーンに終了して即座に最後の風炎玉攻撃へ移行！
			mCloneDashAttack.Reset();
			mCloneMeteorDashAttack.Reset();
			mCloneWindRoarAttack.Reset(player);
			mRoarAttack.Reset();
			mMagmaFireballAttack.Reset();
			mAirDashComboAttack.Reset();
			mBounceFireballAttack.Reset();

			mState      = State::UltimateWindSlamAttack;
			mStateTimer = 0;
			mAnimFrame  = 0;
			mAnimTimer  = 0;
			mUltimateWindSlamAttack.StartAttack(mvPosition, mFacingDirection, player);
		}
	}
}

bool Stage3BossDragon::IsAttacking() const
{
	if (mState == State::Attack)
	{
		return (mAnimFrame >= 3 && mAnimFrame <= 6);
	}
	if (mState == State::AirDashComboAttack)
	{
		return mAirDashComboAttack.IsSlashActive();
	}
	if (mState == State::CloneDashAttack)
	{
		return mCloneDashAttack.IsSlashActive();
	}
	if (mState == State::CloneMeteorDashAttack)
	{
		return mCloneMeteorDashAttack.IsSlashActive();
	}
	return false;
}

void Stage3BossDragon::GetAttackHitbox(float& left, float& top, float& right, float& bottom) const
{
	float reach = 175.0f;
	if (mFacingDirection < 0.0f)
	{
		right = mvPosition.x + 20.0f;
		left  = mvPosition.x - (mfEnemyWidth * 0.5f + reach);
	}
	else
	{
		left  = mvPosition.x - 20.0f;
		right = mvPosition.x + (mfEnemyWidth * 0.5f + reach);
	}
	top    = mvPosition.y - mfEnemyHeight * 0.48f;
	bottom = mvPosition.y + mfEnemyHeight * 0.52f;
}

void Stage3BossDragon::Update()
{
	if (IsDeleteFlag()) return;

	Enemy::Update();

	if (mHpShakeTimer > 0) mHpShakeTimer--;
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
	Player*        player  = (gameScene != nullptr) ? gameScene->GetPlayer() : nullptr;
	if (player != nullptr && player->GetIsDead())
	{
		player->SetWindFloatMode(false);
	}
	Stage3Gimmick* gimmick = (gameScene != nullptr) ? &gameScene->GetStage3Controller().GetGimmick() : nullptr;
	Stage3Magma*   magma   = (gameScene != nullptr) ? &gameScene->GetStage3Controller().GetMagma()   : nullptr;

	// 1. ムービー演出中（初登場 または 第2形態黄金覚醒）の待機・咆哮アニメーション
	if (mState == State::CutsceneIdle || mState == State::AwakeningCutscene)
	{
		mStateTimer++;
		mAnimTimer++;

		if (mbCutsceneRoaring)
		{
			mRoarTimer++;
			if (mRoarTimer < 12)      mAnimFrame = 0;
			else if (mRoarTimer < 24) mAnimFrame = 1;
			else if (mRoarTimer < 36) mAnimFrame = 2;
			else if (mRoarTimer < 48) mAnimFrame = 3;
			else if (mRoarTimer < 95)
			{
				mAnimFrame = ((mRoarTimer / 6) % 2 == 0) ? 4 : 5;
			}
			else
			{
				mAnimFrame = 6;
			}

			if (mState == State::CutsceneIdle)
			{
				if (mRoarTimer >= 44 && mRoarTimer <= 90 && mpBlockMap != nullptr)
				{
					mpBlockMap->mShakeX = GetRand(12) - 6;
					mpBlockMap->mShakeY = GetRand(12) - 6;
				}
				else if (mpBlockMap != nullptr)
				{
					mpBlockMap->mShakeX = 0;
					mpBlockMap->mShakeY = 0;
				}
			}
		}
		else
		{
			const size_t animCount = (mState == State::AwakeningCutscene) ? mFlyAnim.size() : mIdleAnim.size();
			if (mAnimTimer >= 6 && animCount > 0)
			{
				mAnimTimer = 0;
				mAnimFrame = (mAnimFrame + 1) % static_cast<int>(animCount);
			}
		}
		return;
	}

	// 2. 撃破・勝利時
	if (mState == State::Defeated)
	{
		mDefeatTimer++;
		if (mpBlockMap != nullptr)
		{
			if (mDefeatTimer < 65)
			{
				int amp = (65 - mDefeatTimer) / 3;
				if (amp < 2) amp = 2;
				mpBlockMap->mShakeX = GetRand(amp * 2) - amp;
				mpBlockMap->mShakeY = GetRand(amp * 2) - amp;
			}
			else
			{
				mpBlockMap->mShakeX = 0;
				mpBlockMap->mShakeY = 0;
			}
		}
		if (mDefeatTimer == 80 && gameScene != nullptr)
		{
			gameScene->TriggerStageClearFade();
		}
		return;
	}

	if (mAttackCooldown > 0) mAttackCooldown--;
	if (mSpecialAttackCooldown > 0) mSpecialAttackCooldown--;

	float px = (player != nullptr) ? player->GetPosition().x : 3600.0f;
	float dx = px - mvPosition.x;
	float absDx = std::fabs(dx);

	switch (mState)
	{
	case State::Idle:
		mStateTimer++;
		mAnimTimer++;
		if (mAnimTimer >= 10 && !mIdleAnim.empty())
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mIdleAnim.size());
		}
		mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;

		// 大技クールダウン完了時は攻撃パターンを順番に繰り出す！
		if (mSpecialAttackCooldown <= 0 && mStateTimer >= 35)
		{
			mStateTimer = 0;
			mAnimFrame  = 0;
			mAnimTimer  = 0;

			if (!mbIsPhase2)
			{
				// =========================================================
				// 第1形態の攻撃順序（分身系は第2形態専用）
				// 初回固定4連携:
				//   0: 咆哮 (RoarAttack)
				//   1: 2回突進攻撃 (AirDashComboAttack)
				//   2: 火を吐いて反射する玉攻撃 (BounceFireballAttack)
				//   3: 真ん中に巨大火玉を放つ攻撃 (MagmaFireballAttack)
				// 5回目以降は直前の攻撃と被らないように上記4種からランダム選択
				// =========================================================
				bool magmaGone = (magma != nullptr && magma->IsMagmaGone());
				int nextAttackId = 0;
				if (mAttackPatternIndex < 4)
				{
					nextAttackId = mAttackPatternIndex;
					mAttackPatternIndex++;
					if (magmaGone && nextAttackId == 3)
					{
						nextAttackId = 0;
					}
				}
				else
				{
					int candidates[4];
					int candidateCount = 0;
					for (int id = 0; id < 4; ++id)
					{
						if (id == mLastAttackId) continue;
						if (magmaGone && id == 3) continue;
						candidates[candidateCount++] = id;
					}
					if (candidateCount > 0)
					{
						nextAttackId = candidates[GetRand(candidateCount - 1)];
					}
				}
				mLastAttackId = nextAttackId;

				if (nextAttackId == 0)
				{
					mState = State::RoarAttack;
					mRoarAttack.StartAttack(mvPosition, mFacingDirection);
				}
				else if (nextAttackId == 1)
				{
					mState = State::AirDashComboAttack;
					mAirDashComboAttack.StartAttack(mvPosition, mFacingDirection, player);
				}
				else if (nextAttackId == 2)
				{
					mState = State::BounceFireballAttack;
					mBounceFireballAttack.StartAttack(mvPosition, mFacingDirection, player);
				}
				else
				{
					mState = State::MagmaFireballAttack;
					mMagmaFireballAttack.StartAttack(mvPosition, mFacingDirection);
				}
			}
			else
			{
				// =========================================================
				// 第2形態「アルティメットドラゴン」の攻撃順序（分身召喚攻撃）
				// 初回固定3連携:
				//   0: 分身突進攻撃 (CloneDashAttack)
				//   1: 風の刃＆咆哮攻撃 (CloneWindRoarAttack)
				//   2: 突進＆火の玉メテオ攻撃 (CloneMeteorDashAttack)
				// 4回目以降は直前の攻撃と被らないように上記3種からランダム選択
				// =========================================================
				int nextAttackId = 0;
				if (mAttackPatternIndex < 3)
				{
					nextAttackId = mAttackPatternIndex;
					mAttackPatternIndex++;
				}
				else
				{
					int candidates[3];
					int candidateCount = 0;
					for (int id = 0; id < 3; ++id)
					{
						if (id == mLastAttackId) continue;
						candidates[candidateCount++] = id;
					}
					if (candidateCount > 0)
					{
						nextAttackId = candidates[GetRand(candidateCount - 1)];
					}
				}
				mLastAttackId = nextAttackId;

				if (nextAttackId == 0)
				{
					mState = State::CloneDashAttack;
					mCloneDashAttack.StartAttack(mvPosition, mFacingDirection);
				}
				else if (nextAttackId == 1)
				{
					mState = State::CloneWindRoarAttack;
					mCloneWindRoarAttack.StartAttack(mvPosition, mFacingDirection, player);
				}
				else
				{
					mState = State::CloneMeteorDashAttack;
					mCloneMeteorDashAttack.StartAttack(mvPosition, mFacingDirection, player);
				}
			}
		}
		// 同じ足場にプレイヤーが近づいてきたら近接斬撃
		else if (absDx <= 320.0f && mAttackCooldown <= 0)
		{
			mState      = State::Attack;
			mStateTimer = 0;
			mAnimFrame  = 0;
			mAnimTimer  = 0;
		}
		break;

	case State::Attack:
		mStateTimer++;
		mAnimTimer++;
		if (mAnimTimer >= 6 && !mAttackAnim.empty())
		{
			mAnimTimer = 0;
			mAnimFrame++;
			if (mAnimFrame >= static_cast<int>(mAttackAnim.size()))
			{
				mState          = State::Idle;
				mStateTimer     = 0;
				mAnimFrame      = 0;
				mAnimTimer      = 0;
				mAttackCooldown = 65;
			}
		}
		break;

	case State::RoarAttack:
		mRoarAttack.Update(
			mvPosition,
			mFacingDirection,
			isGrounded,
			mRoarAttackAnimType,
			mRoarAttackAnimFrame,
			player,
			mpBlockMap,
			gimmick
		);

		if (mRoarAttack.IsFinished())
		{
			mState                 = State::Idle;
			mStateTimer            = 0;
			mAnimFrame             = 0;
			mAnimTimer             = 0;
			mSpecialAttackCooldown = 130;
		}
		break;

	case State::MagmaFireballAttack:
		mMagmaFireballAttack.UpdateDragon(
			mvPosition,
			mFacingDirection,
			isGrounded,
			mMagmaFireballAnimType,
			mMagmaFireballAnimFrame,
			mDrawScale,
			player
		);

		if (mMagmaFireballAttack.IsFinished())
		{
			mState                 = State::Idle;
			mStateTimer            = 0;
			mAnimFrame             = 0;
			mAnimTimer             = 0;
			mSpecialAttackCooldown = 140;
		}
		break;

	case State::AirDashComboAttack:
		mAirDashComboAttack.Update(
			mvPosition,
			mFacingDirection,
			isGrounded,
			mAirDashComboAnimType,
			mAirDashComboAnimFrame,
			player,
			mpBlockMap
		);

		if (mAirDashComboAttack.IsFinished())
		{
			mState                 = State::Idle;
			mStateTimer            = 0;
			mAnimFrame             = 0;
			mAnimTimer             = 0;
			mSpecialAttackCooldown = 130;
		}
		break;

	case State::BounceFireballAttack:
		mBounceFireballAttack.UpdateDragon(
			mvPosition,
			mFacingDirection,
			isGrounded,
			mBounceFireballAnimType,
			mBounceFireballAnimFrame,
			mDrawScale,
			player
		);

		if (mBounceFireballAttack.IsFinished() && !mBounceFireballAttack.HasActiveProjectiles())
		{
			mState                 = State::Idle;
			mStateTimer            = 0;
			mAnimFrame             = 0;
			mAnimTimer             = 0;
			mSpecialAttackCooldown = 95;
		}
		break;

	case State::CloneDashAttack:
		mCloneDashAttack.Update(
			mvPosition,
			mFacingDirection,
			isGrounded,
			mCloneDashAnimType,
			mCloneDashAnimFrame,
			mpBlockMap
		);

		if (mCloneDashAttack.IsFinished())
		{
			mState                 = State::Idle;
			mStateTimer            = 0;
			mAnimFrame             = 0;
			mAnimTimer             = 0;
			mSpecialAttackCooldown = 130;
		}
		break;

	case State::CloneMeteorDashAttack:
		mCloneMeteorDashAttack.UpdateDragon(
			mvPosition,
			mFacingDirection,
			isGrounded,
			mCloneMeteorDashAnimType,
			mCloneMeteorDashAnimFrame,
			mpBlockMap
		);

		if (mCloneMeteorDashAttack.IsFinished())
		{
			mState                 = State::Idle;
			mStateTimer            = 0;
			mAnimFrame             = 0;
			mAnimTimer             = 0;
			mSpecialAttackCooldown = 130;
		}
		break;

	case State::CloneWindRoarAttack:
		mCloneWindRoarAttack.UpdateDragon(
			mvPosition,
			mFacingDirection,
			isGrounded,
			mCloneWindRoarAnimType,
			mCloneWindRoarAnimFrame,
			player,
			mpBlockMap
		);

		if (mCloneWindRoarAttack.IsFinished())
		{
			mState                 = State::Idle;
			mStateTimer            = 0;
			mAnimFrame             = 0;
			mAnimTimer             = 0;
			mSpecialAttackCooldown = 130;
		}
		break;

	case State::UltimateWindSlamAttack:
		mUltimateWindSlamAttack.UpdateDragon(
			mvPosition,
			mFacingDirection,
			isGrounded,
			mUltimateWindSlamAnimType,
			mUltimateWindSlamAnimFrame,
			player,
			mpBlockMap,
			magma,
			gimmick
		);

		// 【Stage3.png 5724x1692拡大版 対応】最終スタン位置をピット中央（X=4353, 床Y=1477 -> 1353）へ更新
		if (mUltimateWindSlamAttack.IsFinished())
		{
			mvPosition.x     = 4353.0f;
			mvPosition.y     = 1353.0f;
			isGrounded       = true;
			velocityY        = 0.0f;
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;
			mState           = State::FinalStun;
			mStateTimer      = 0;
			mStunTimer       = 0;
			mAnimFrame       = 7;
			mAnimTimer       = 0;
			isDamaged        = false; // 勇者の一撃が確実に当たるよう無敵フレームをクリア
			if (player != nullptr)
			{
				player->SetWindFloatMode(false);
			}
		}
		break;

	case State::Down:
		mStateTimer++;
		mAnimTimer++;
		if (damageTimer > 15)
		{
			damageTimer = 15;
		}

		if (mDownPhase == 0)
		{
			mvPosition.x = mDownTargetX;
			mDownVy += 0.35f;
			mvPosition.y += mDownVy;

			if (mAnimTimer >= 4 && mAnimFrame < 3)
			{
				mAnimTimer = 0;
				mAnimFrame++;
			}

			if (mStateTimer >= 16)
			{
				mDownPhase  = 1;
				mStateTimer = 0;
				mAnimFrame  = 4;
				mAnimTimer  = 0;
				mDownVy     = 6.0f;
			}
		}
		else if (mDownPhase == 1)
		{
			mvPosition.x = mDownTargetX;
			mDownVy += 1.35f;
			if (mDownVy > 22.0f) mDownVy = 22.0f;
			mvPosition.y += mDownVy;

			if (mAnimTimer >= 5 && mAnimFrame < 6)
			{
				mAnimTimer = 0;
				mAnimFrame++;
			}

			float targetCenterY = mDownFloorY - (mfEnemyHeight * 0.5f);
			if (mvPosition.y >= targetCenterY)
			{
				mvPosition.y = targetCenterY;
				isGrounded   = true;
				velocityY    = 0.0f;
				mDownVy      = 0.0f;
				mDownPhase   = 2;
				mStateTimer  = 0;
				mStunTimer   = 0;
				mAnimFrame   = 7;
				mAnimTimer   = 0;
				if (mpBlockMap != nullptr)
				{
					mpBlockMap->mShakeX = 14;
					mpBlockMap->mShakeY = 16;
				}
			}
		}
		else
		{
			mStunTimer++;
			mvPosition.x = mDownTargetX;
			if (mStunTimer < 6)       mAnimFrame = 7;
			else if (mStunTimer < 12) mAnimFrame = 8;
			else                      mAnimFrame = 9;

			if (mpBlockMap != nullptr)
			{
				if (mStunTimer <= 12)
				{
					mpBlockMap->mShakeX = GetRand(10) - 5;
					mpBlockMap->mShakeY = GetRand(12) - 6;
				}
				else if (mStunTimer == 13)
				{
					mpBlockMap->mShakeX = 0;
					mpBlockMap->mShakeY = 0;
				}
			}

			if (mStunTimer >= DOWN_GROUND_FRAMES)
			{
				if (mpBlockMap != nullptr)
				{
					mpBlockMap->mShakeX = 0;
					mpBlockMap->mShakeY = 0;
				}
				mState                 = State::Idle;
				mDownPhase             = 0;
				mStateTimer            = 0;
				mStunTimer             = 0;
				mAnimFrame             = 0;
				mAnimTimer             = 0;
				mSpecialAttackCooldown = 90;
				mAttackCooldown        = 45;
				mFacingDirection       = (dx >= 0.0f) ? 1.0f : -1.0f;
			}
		}
		break;

	case State::FinalStun:
		mStateTimer++;
		mStunTimer++;
		mAnimTimer++;
		isDamaged    = false; // スタン中はいつでも勇者の攻撃が通るようにする
		mvPosition.x = 4353.0f;
		if (mStunTimer < 6)       mAnimFrame = 7;
		else if (mStunTimer < 12) mAnimFrame = 8;
		else                      mAnimFrame = 9;

		// 8秒間（480フレーム）経過してもトドメを刺されなかった場合は再び最後の風炎玉攻撃を発動
		if (mStunTimer >= 480)
		{
			mState      = State::UltimateWindSlamAttack;
			mStateTimer = 0;
			mStunTimer  = 0;
			mAnimFrame  = 0;
			mAnimTimer  = 0;
			mUltimateWindSlamAttack.StartAttack(mvPosition, mFacingDirection, player);
		}
		break;

	default:
		break;
	}

	// プレイヤーの咆哮ノックバック速度を毎フレーム反映
	mRoarAttack.UpdatePlayerKnockback(player, mpBlockMap);

	// 巨大火球・縦マグマ噴水・周囲へのマグマ飛散弾の更新と当たり判定
	mMagmaFireballAttack.UpdateProjectilesAndEffects(player, mpBlockMap, mAttack);

	// 地上反射火の玉・反射数連動マグマ噴水・周囲へのマグマ飛散弾の更新と当たり判定
	mBounceFireballAttack.UpdateProjectilesAndEffects(player, mpBlockMap, mAttack);

	// 分身メテオ8秒間・特大火球・疾走エフェクトの更新と当たり判定
	mCloneMeteorDashAttack.UpdateProjectilesAndEffects(player, mpBlockMap, mAttack);

	// 最後の大技：風の玉・吸い込まれるマグマ・叩きつけ衝撃波の更新と当たり判定
	mUltimateWindSlamAttack.UpdateProjectilesAndEffects(player, mpBlockMap, magma, mAttack);

	// 分身2体の風の刃・本体の逆方向溜め咆哮エフェクトの更新と当たり判定
	mCloneWindRoarAttack.UpdateProjectilesAndEffects(player, mpBlockMap, mAttack);

	// 攻撃発動の約14フレーム前（とんとんとん → キラン → ドーン！／戻り突進も含む）に鋭い赤い眼光タイマーを起動
	bool triggerEyeFlash =
		(mState == State::Attack && mStateTimer == 1) ||
		(mState == State::RoarAttack && mRoarAttack.IsEyeGlintMoment()) ||
		(mState == State::AirDashComboAttack && mAirDashComboAttack.IsEyeGlintMoment()) ||
		(mState == State::CloneDashAttack && mCloneDashAttack.IsEyeGlintMoment()) ||
		(mState == State::CloneMeteorDashAttack && mCloneMeteorDashAttack.IsEyeGlintMoment()) ||
		(mState == State::UltimateWindSlamAttack && mUltimateWindSlamAttack.IsEyeGlintMoment()) ||
		(mState == State::CloneWindRoarAttack && mCloneWindRoarAttack.IsEyeGlintMoment());

	if (triggerEyeFlash)
	{
		mEyeGlintTimer = 16;
	}
	else if (mEyeGlintTimer > 0)
	{
		mEyeGlintTimer--;
	}
}

void Stage3BossDragon::EnemyMove(BlockMap& blockMap)
{
	mpBlockMap = &blockMap;

	if (mState == State::AwakeningCutscene ||
		(mState == State::Down && mDownPhase < 2) ||
		(mState == State::RoarAttack && mRoarAttack.IsAirbornePhase()) ||
		(mState == State::MagmaFireballAttack && mMagmaFireballAttack.IsAirbornePhase()) ||
		(mState == State::AirDashComboAttack && mAirDashComboAttack.IsAirbornePhase()) ||
		(mState == State::CloneDashAttack && mCloneDashAttack.IsAirbornePhase()) ||
		(mState == State::CloneMeteorDashAttack && mCloneMeteorDashAttack.IsAirbornePhase()) ||
		(mState == State::UltimateWindSlamAttack && mUltimateWindSlamAttack.IsAirbornePhase()) ||
		(mState == State::CloneWindRoarAttack && mCloneWindRoarAttack.IsAirbornePhase()))
	{
		isGrounded = false;
		velocityY  = 0.0f;
		return;
	}

	Enemy::EnemyMove(blockMap);
}

void Stage3BossDragon::SelectCurrentSprite(int& outHandle, float& outFootOffset, bool forceGolden) const
{
	bool useGold = mbIsGolden || forceGolden;
	SelectSpriteFromSet(
		outHandle,
		outFootOffset,
		useGold ? mGoldenIdleAnim      : mIdleAnim,
		useGold ? mGoldenMoveAnim      : mMoveAnim,
		useGold ? mGoldenAttackAnim    : mAttackAnim,
		useGold ? mGoldenTakeoffAnim   : mTakeoffAnim,
		useGold ? mGoldenFlyAnim       : mFlyAnim,
		useGold ? mGoldenAirAttackAnim : mAirAttackAnim,
		useGold ? mGoldenLandingAnim   : mLandingAnim,
		useGold ? mGoldenRoarAnim      : mRoarAnim,
		useGold ? mGoldenChargeAnim    : mChargeAnim,
		useGold ? mGoldenDownAnim      : mDownAnim
	);
}

void Stage3BossDragon::SelectSpriteFromSet(
	int& outHandle,
	float& outFootOffset,
	const std::vector<int>& idleAnim,
	const std::vector<int>& moveAnim,
	const std::vector<int>& attackAnim,
	const std::vector<int>& takeoffAnim,
	const std::vector<int>& flyAnim,
	const std::vector<int>& airAttackAnim,
	const std::vector<int>& landingAnim,
	const std::vector<int>& roarAnim,
	const std::vector<int>& chargeAnim,
	const std::vector<int>& downAnim) const
{
	outHandle     = -1;
	outFootOffset = 79.0f;

	switch (mState)
	{
	case State::CutsceneIdle:
		if (mbCutsceneRoaring && !roarAnim.empty())
		{
			outHandle     = roarAnim[mRoarTimer > 0 ? (mAnimFrame % roarAnim.size()) : 0];
			outFootOffset = 80.0f;
		}
		else if (!idleAnim.empty())
		{
			outHandle     = idleAnim[mAnimFrame % idleAnim.size()];
			outFootOffset = 79.0f;
		}
		break;

	case State::AwakeningCutscene:
		if (mbCutsceneRoaring && !roarAnim.empty())
		{
			outHandle     = roarAnim[mRoarTimer > 0 ? (mAnimFrame % roarAnim.size()) : 0];
			outFootOffset = 80.0f;
		}
		else if (!flyAnim.empty())
		{
			outHandle     = flyAnim[mAnimFrame % flyAnim.size()];
			outFootOffset = 57.0f;
		}
		break;

	case State::Down:
	case State::FinalStun:
		if (!downAnim.empty())
		{
			int frameIdx = mAnimFrame % static_cast<int>(downAnim.size());
			outHandle = downAnim[frameIdx];
			if (frameIdx <= 4)      outFootOffset = 57.0f;
			else if (frameIdx == 5) outFootOffset = 95.0f;
			else if (frameIdx == 6) outFootOffset = 115.0f;
			else                    outFootOffset = 120.0f;
		}
		else if (!idleAnim.empty())
		{
			outHandle     = idleAnim[mAnimFrame % idleAnim.size()];
			outFootOffset = 79.0f;
		}
		break;

	case State::Idle:
	default:
		if (!idleAnim.empty())
		{
			outHandle     = idleAnim[mAnimFrame % idleAnim.size()];
			outFootOffset = 79.0f;
		}
		break;

	case State::Walk:
		if (!moveAnim.empty())
		{
			outHandle     = moveAnim[mAnimFrame % moveAnim.size()];
			outFootOffset = 79.0f;
		}
		break;

	case State::Attack:
		if (!attackAnim.empty())
		{
			outHandle     = attackAnim[mAnimFrame % attackAnim.size()];
			outFootOffset = 79.0f;
		}
		break;

	case State::RoarAttack:
		if (mRoarAttackAnimType == Stage3DragonRoarAttack::ANIM_TAKEOFF && !takeoffAnim.empty())
		{
			outHandle     = takeoffAnim[mRoarAttackAnimFrame % takeoffAnim.size()];
			outFootOffset = 80.0f;
		}
		else if (mRoarAttackAnimType == Stage3DragonRoarAttack::ANIM_FLY && !flyAnim.empty())
		{
			outHandle     = flyAnim[mRoarAttackAnimFrame % flyAnim.size()];
			outFootOffset = 57.0f;
		}
		else if (mRoarAttackAnimType == Stage3DragonRoarAttack::ANIM_ROAR && !roarAnim.empty())
		{
			outHandle     = roarAnim[mRoarAttackAnimFrame % roarAnim.size()];
			outFootOffset = 80.0f;
		}
		else if (mRoarAttackAnimType == Stage3DragonRoarAttack::ANIM_LANDING && !landingAnim.empty())
		{
			outHandle     = landingAnim[mRoarAttackAnimFrame % landingAnim.size()];
			outFootOffset = 57.0f;
		}
		else if (!idleAnim.empty())
		{
			outHandle     = idleAnim[mRoarAttackAnimFrame % idleAnim.size()];
			outFootOffset = 79.0f;
		}
		break;

	case State::MagmaFireballAttack:
		if (mMagmaFireballAnimType == Stage3DragonMagmaFireballAttack::ANIM_TAKEOFF && !takeoffAnim.empty())
		{
			outHandle     = takeoffAnim[mMagmaFireballAnimFrame % takeoffAnim.size()];
			outFootOffset = 80.0f;
		}
		else if (mMagmaFireballAnimType == Stage3DragonMagmaFireballAttack::ANIM_FLY && !flyAnim.empty())
		{
			outHandle     = flyAnim[mMagmaFireballAnimFrame % flyAnim.size()];
			outFootOffset = 57.0f;
		}
		else if (mMagmaFireballAnimType == Stage3DragonMagmaFireballAttack::ANIM_AIR_ATTACK && !airAttackAnim.empty())
		{
			outHandle     = airAttackAnim[mMagmaFireballAnimFrame % airAttackAnim.size()];
			outFootOffset = 74.0f;
		}
		else if (mMagmaFireballAnimType == Stage3DragonMagmaFireballAttack::ANIM_LANDING && !landingAnim.empty())
		{
			outHandle     = landingAnim[mMagmaFireballAnimFrame % landingAnim.size()];
			outFootOffset = 57.0f;
		}
		else if (!idleAnim.empty())
		{
			outHandle     = idleAnim[mMagmaFireballAnimFrame % idleAnim.size()];
			outFootOffset = 79.0f;
		}
		break;

	case State::AirDashComboAttack:
		if (mAirDashComboAnimType == Stage3DragonAirDashComboAttack::ANIM_TAKEOFF && !takeoffAnim.empty())
		{
			outHandle     = takeoffAnim[mAirDashComboAnimFrame % takeoffAnim.size()];
			outFootOffset = 80.0f;
		}
		else if (mAirDashComboAnimType == Stage3DragonAirDashComboAttack::ANIM_FLY && !flyAnim.empty())
		{
			outHandle     = flyAnim[mAirDashComboAnimFrame % flyAnim.size()];
			outFootOffset = 57.0f;
		}
		else if (mAirDashComboAnimType == Stage3DragonAirDashComboAttack::ANIM_ATTACK && !attackAnim.empty())
		{
			outHandle     = attackAnim[mAirDashComboAnimFrame % attackAnim.size()];
			outFootOffset = 79.0f;
		}
		else if (mAirDashComboAnimType == Stage3DragonAirDashComboAttack::ANIM_LANDING && !landingAnim.empty())
		{
			outHandle     = landingAnim[mAirDashComboAnimFrame % landingAnim.size()];
			outFootOffset = 57.0f;
		}
		else if (!idleAnim.empty())
		{
			outHandle     = idleAnim[mAirDashComboAnimFrame % idleAnim.size()];
			outFootOffset = 79.0f;
		}
		break;

	case State::BounceFireballAttack:
		if (mBounceFireballAnimType == Stage3DragonBounceFireballAttack::ANIM_CHARGE && !chargeAnim.empty())
		{
			outHandle     = chargeAnim[mBounceFireballAnimFrame % chargeAnim.size()];
			outFootOffset = 80.0f;
		}
		else if (mBounceFireballAnimType == Stage3DragonBounceFireballAttack::ANIM_ATTACK && !attackAnim.empty())
		{
			outHandle     = attackAnim[mBounceFireballAnimFrame % attackAnim.size()];
			outFootOffset = 79.0f;
		}
		else if (!idleAnim.empty())
		{
			outHandle     = idleAnim[mBounceFireballAnimFrame % idleAnim.size()];
			outFootOffset = 79.0f;
		}
		break;

	case State::CloneDashAttack:
		if (mCloneDashAnimType == Stage3DragonCloneDashAttack::ANIM_TAKEOFF && !takeoffAnim.empty())
		{
			outHandle     = takeoffAnim[mCloneDashAnimFrame % takeoffAnim.size()];
			outFootOffset = 80.0f;
		}
		else if (mCloneDashAnimType == Stage3DragonCloneDashAttack::ANIM_FLY && !flyAnim.empty())
		{
			outHandle     = flyAnim[mCloneDashAnimFrame % flyAnim.size()];
			outFootOffset = 57.0f;
		}
		else if (mCloneDashAnimType == Stage3DragonCloneDashAttack::ANIM_ATTACK && !attackAnim.empty())
		{
			outHandle     = attackAnim[mCloneDashAnimFrame % attackAnim.size()];
			outFootOffset = 79.0f;
		}
		else if (mCloneDashAnimType == Stage3DragonCloneDashAttack::ANIM_LANDING && !landingAnim.empty())
		{
			outHandle     = landingAnim[mCloneDashAnimFrame % landingAnim.size()];
			outFootOffset = 57.0f;
		}
		else if (!idleAnim.empty())
		{
			outHandle     = idleAnim[mCloneDashAnimFrame % idleAnim.size()];
			outFootOffset = 79.0f;
		}
		break;

	case State::CloneMeteorDashAttack:
		if (mCloneMeteorDashAnimType == Stage3DragonCloneMeteorDashAttack::ANIM_TAKEOFF && !takeoffAnim.empty())
		{
			outHandle     = takeoffAnim[mCloneMeteorDashAnimFrame % takeoffAnim.size()];
			outFootOffset = 80.0f;
		}
		else if (mCloneMeteorDashAnimType == Stage3DragonCloneMeteorDashAttack::ANIM_FLY && !flyAnim.empty())
		{
			outHandle     = flyAnim[mCloneMeteorDashAnimFrame % flyAnim.size()];
			outFootOffset = 57.0f;
		}
		else if (mCloneMeteorDashAnimType == Stage3DragonCloneMeteorDashAttack::ANIM_ATTACK && !attackAnim.empty())
		{
			outHandle     = attackAnim[mCloneMeteorDashAnimFrame % attackAnim.size()];
			outFootOffset = 79.0f;
		}
		else if (mCloneMeteorDashAnimType == Stage3DragonCloneMeteorDashAttack::ANIM_LANDING && !landingAnim.empty())
		{
			outHandle     = landingAnim[mCloneMeteorDashAnimFrame % landingAnim.size()];
			outFootOffset = 57.0f;
		}
		else if (!idleAnim.empty())
		{
			outHandle     = idleAnim[mCloneMeteorDashAnimFrame % idleAnim.size()];
			outFootOffset = 79.0f;
		}
		break;

	case State::CloneWindRoarAttack:
		if (mCloneWindRoarAnimType == Stage3DragonCloneWindRoarAttack::ANIM_TAKEOFF && !takeoffAnim.empty())
		{
			outHandle     = takeoffAnim[mCloneWindRoarAnimFrame % takeoffAnim.size()];
			outFootOffset = 80.0f;
		}
		else if (mCloneWindRoarAnimType == Stage3DragonCloneWindRoarAttack::ANIM_FLY && !flyAnim.empty())
		{
			outHandle     = flyAnim[mCloneWindRoarAnimFrame % flyAnim.size()];
			outFootOffset = 57.0f;
		}
		else if (mCloneWindRoarAnimType == Stage3DragonCloneWindRoarAttack::ANIM_ROAR && !roarAnim.empty())
		{
			outHandle     = roarAnim[mCloneWindRoarAnimFrame % roarAnim.size()];
			outFootOffset = 80.0f;
		}
		else if (mCloneWindRoarAnimType == Stage3DragonCloneWindRoarAttack::ANIM_LANDING && !landingAnim.empty())
		{
			outHandle     = landingAnim[mCloneWindRoarAnimFrame % landingAnim.size()];
			outFootOffset = 57.0f;
		}
		else if (!idleAnim.empty())
		{
			outHandle     = idleAnim[mCloneWindRoarAnimFrame % idleAnim.size()];
			outFootOffset = 79.0f;
		}
		break;

	case State::UltimateWindSlamAttack:
		if (mUltimateWindSlamAnimType == Stage3DragonUltimateWindSlamAttack::ANIM_TAKEOFF && !takeoffAnim.empty())
		{
			outHandle     = takeoffAnim[mUltimateWindSlamAnimFrame % takeoffAnim.size()];
			outFootOffset = 80.0f;
		}
		else if (mUltimateWindSlamAnimType == Stage3DragonUltimateWindSlamAttack::ANIM_FLY && !flyAnim.empty())
		{
			outHandle     = flyAnim[mUltimateWindSlamAnimFrame % flyAnim.size()];
			outFootOffset = 57.0f;
		}
		else if (mUltimateWindSlamAnimType == Stage3DragonUltimateWindSlamAttack::ANIM_ATTACK && !attackAnim.empty())
		{
			outHandle     = attackAnim[mUltimateWindSlamAnimFrame % attackAnim.size()];
			outFootOffset = 79.0f;
		}
		else if (mUltimateWindSlamAnimType == Stage3DragonUltimateWindSlamAttack::ANIM_LANDING && !landingAnim.empty())
		{
			outHandle     = landingAnim[mUltimateWindSlamAnimFrame % landingAnim.size()];
			outFootOffset = 57.0f;
		}
		else if (!idleAnim.empty())
		{
			outHandle     = idleAnim[mUltimateWindSlamAnimFrame % idleAnim.size()];
			outFootOffset = 79.0f;
		}
		break;
	}
}

void Stage3BossDragon::Draw()
{
	if (mState == State::Defeated && mDefeatTimer > 75)
	{
		return;
	}
	if (mState == State::Defeated && (mDefeatTimer / 3) % 2 == 1)
	{
		return;
	}

	GameScene* gameScene = nullptr;
	if (Master::mpGameManager && Master::mpGameManager->GetSceneManager())
	{
		gameScene = dynamic_cast<GameScene*>(Master::mpGameManager->GetSceneManager()->GetCurrentScene());
	}
	Player* player = (gameScene != nullptr) ? gameScene->GetPlayer() : nullptr;

	// 咆哮攻撃の範囲予告・衝撃波エフェクト描画
	mRoarAttack.Draw(mpBlockMap, mvPosition, mFacingDirection);

	int   handle               = -1;
	float footOffsetFromCenter = 79.0f;
	SelectCurrentSprite(handle, footOffsetFromCenter);

	// 空中2連続突進通常攻撃のロックオン予告・残像・斬撃エフェクト描画
	mAirDashComboAttack.Draw(mpBlockMap, mvPosition, mFacingDirection, mDrawScale, handle, footOffsetFromCenter, mfEnemyHeight);

	int   screenX    = static_cast<int>(ConvertToScreenX(mvPosition.x, mpBlockMap));
	float feetWorldY = mvPosition.y + (mfEnemyHeight * 0.5f);
	int   screenY    = static_cast<int>(ConvertToScreenY(feetWorldY - (footOffsetFromCenter * mDrawScale), mpBlockMap));
	bool  reverse    = (mFacingDirection > 0.0f);
	int   now        = GetNowCount();

	int drawOffsetY = 0;

	// 黄金覚醒ムービー中または第2形態「アルティメットドラゴン」の輪郭闘気オーラ＆黄金スパーク演出
	if (mState == State::AwakeningCutscene || mbIsGolden)
	{
		float auraStrength = mbIsGolden ? 1.0f : mAwakenChargeRatio;
		if (auraStrength > 0.05f && handle != -1)
		{
			// 1. スーパーサイヤ人風の激しく燃え上がる黄金の気オーラ（Fire.png 7本＋超軽量GPU描画）
			int fireTick = now / 45;
			float cutBoost = (mState == State::AwakeningCutscene) ? 1.25f : 1.0f;
			for (int f = 0; f < 7; ++f)
			{
				int fIdx = (fireTick + f) & 3;
				int fTex = mSsjFireTex[fIdx];
				if (fTex == -1) continue;

				float frac  = static_cast<float>(f) / 6.0f;
				float offX  = (frac - 0.5f) * 230.0f * cutBoost;
				float archY = -std::sin(frac * 3.14159265f) * 42.0f;
				float sway  = std::sin(now * 0.014f + f * 1.3f) * 8.0f;
				int   fx    = screenX + static_cast<int>(offX + sway);
				int   fy    = screenY + drawOffsetY + static_cast<int>(archY + 12.0f);
				double fSc  = (0.68 + std::sin(now * 0.018f + f * 2.1f) * 0.12) * cutBoost;
				double fAng = (frac - 0.5f) * 0.35;

				SetDrawBright(255, 185, 20);
				SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(auraStrength * 135.0f));
				DrawRotaGraph(fx, fy, fSc, fAng, fTex, TRUE);

				SetDrawBright(255, 235, 95);
				SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(auraStrength * 90.0f));
				DrawRotaGraph(fx, fy + 8, fSc * 0.72, fAng, fTex, TRUE);
			}

			// 2. スーパーサイヤ人2風の黄金ビリビリスパーク（3本・まばらに明滅）
			int elecTick = now / 35;
			for (int e = 0; e < 3; ++e)
			{
				int seed = elecTick * 29 + e * 47;
				if ((seed % 3) == 0) continue;
				int eTex = mSsjThunderTex[(seed & 0x7FFF) % 8];
				if (eTex == -1) continue;

				float eAng = (static_cast<float>(e) / 3.0f) * 6.2831853f + static_cast<float>(seed % 11) * 0.25f;
				int   ex   = screenX + static_cast<int>(std::cos(eAng) * 115.0f * cutBoost);
				int   ey   = screenY + drawOffsetY + static_cast<int>(std::sin(eAng) * 82.0f * cutBoost);
				double eRot = eAng + 1.5707963;

				SetDrawBright(255, 225, 70);
				SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(auraStrength * 220.0f));
				DrawRotaGraph(ex, ey, 0.85 * cutBoost, eRot, eTex, TRUE);
			}

			// 3. ドラゴンのシルエット（輪郭）に沿って立ち昇る黄金の闘気フレーム
			int outlineAlpha = static_cast<int>(auraStrength * 95.0f);
			SetDrawBlendMode(DX_BLENDMODE_ADD, outlineAlpha);
			SetDrawBright(255, 195, 35);
			for (int dir = 0; dir < 4; ++dir)
			{
				float ang  = dir * (3.14159265f * 0.5f) + (now * 0.008f);
				float dist = 9.0f + std::sin(now * 0.02f + dir * 1.1f) * 4.0f;
				int   ox   = static_cast<int>(std::cos(ang) * dist);
				int   oy   = static_cast<int>(std::sin(ang) * dist) - static_cast<int>(std::fabs(std::sin(now * 0.015f + dir)) * 10.0f);
				DrawRotaGraph(screenX + ox, screenY + drawOffsetY + oy, mDrawScale * 1.04, 0.0, handle, TRUE, reverse);
			}

			SetDrawBright(255, 255, 255);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	if (handle != -1)
	{
		if (isDamaged)
		{
			SetDrawBright(255, 100, 100);
		}

		DrawRotaGraph(screenX, screenY + drawOffsetY, mDrawScale, 0.0, handle, TRUE, reverse);

		// 覚醒チャージ中（まだ完全黄金化していない前半）は黄金スプライトを徐々にオーバーレイして金色に染まっていく演出
		if (mState == State::AwakeningCutscene && !mbIsGolden && mAwakenChargeRatio > 0.0f)
		{
			int   goldHandle = -1;
			float goldOffset = 79.0f;
			SelectCurrentSprite(goldHandle, goldOffset, true);
			if (goldHandle != -1)
			{
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(mAwakenChargeRatio * 255.0f));
				DrawRotaGraph(screenX, screenY, mDrawScale, 0.0, goldHandle, TRUE, reverse);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			}
		}

		if (isDamaged)
		{
			SetDrawBright(255, 255, 255);
		}
	}

	// 本体の鋭い赤色「キランッ！」十字閃光（目の実ピクセル座標に追従・爆発や炎ではなくシャープな星形閃光）
	if (mEyeGlintTimer > 0 && handle != -1)
	{
		auto itEye = mEyeOffsetsByHandle.find(handle);
		if (itEye != mEyeOffsetsByHandle.end())
		{
			int eyeSx = screenX + static_cast<int>((itEye->second.first * (reverse ? -1.0f : 1.0f)) * mDrawScale);
			int eyeSy = screenY + drawOffsetY + static_cast<int>(itEye->second.second * mDrawScale);

			float t   = static_cast<float>(mEyeGlintTimer) / 16.0f;
			float pop = std::sin(t * 3.14159265f);
			int   alpha = static_cast<int>(pop * 255.0f);

			int hLen  = static_cast<int>(64.0f * pop);
			int hHalf = static_cast<int>(7.0f * pop) + 1;
			int vLen  = static_cast<int>(46.0f * pop);
			int vHalf = static_cast<int>(6.0f * pop) + 1;
			int dLen  = static_cast<int>(20.0f * pop);

			unsigned int outerRed = GetColor(255, 15, 20);
			unsigned int coreHot  = GetColor(255, 230, 230);

			SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
			// 横方向の鋭いダイヤ光条
			DrawTriangle(eyeSx - hLen, eyeSy, eyeSx, eyeSy - hHalf, eyeSx, eyeSy + hHalf, outerRed, TRUE);
			DrawTriangle(eyeSx + hLen, eyeSy, eyeSx, eyeSy - hHalf, eyeSx, eyeSy + hHalf, outerRed, TRUE);
			// 縦方向の鋭いダイヤ光条
			DrawTriangle(eyeSx, eyeSy - vLen, eyeSx - vHalf, eyeSy, eyeSx + vHalf, eyeSy, outerRed, TRUE);
			DrawTriangle(eyeSx, eyeSy + vLen, eyeSx - vHalf, eyeSy, eyeSx + vHalf, eyeSy, outerRed, TRUE);
			// 内側の白熱コア光条
			DrawTriangle(eyeSx - (hLen * 2 / 3), eyeSy, eyeSx, eyeSy - 2, eyeSx, eyeSy + 2, coreHot, TRUE);
			DrawTriangle(eyeSx + (hLen * 2 / 3), eyeSy, eyeSx, eyeSy - 2, eyeSx, eyeSy + 2, coreHot, TRUE);
			DrawTriangle(eyeSx, eyeSy - (vLen * 2 / 3), eyeSx - 2, eyeSy, eyeSx + 2, eyeSy, coreHot, TRUE);
			DrawTriangle(eyeSx, eyeSy + (vLen * 2 / 3), eyeSx - 2, eyeSy, eyeSx + 2, eyeSy, coreHot, TRUE);
			// 斜めキラめき＋中心発光
			DrawLine(eyeSx - dLen, eyeSy - dLen, eyeSx + dLen, eyeSy + dLen, outerRed, 2);
			DrawLine(eyeSx - dLen, eyeSy + dLen, eyeSx + dLen, eyeSy - dLen, outerRed, 2);
			DrawCircle(eyeSx, eyeSy, static_cast<int>(6.0f * pop) + 1, outerRed, TRUE);
			DrawCircle(eyeSx, eyeSy, static_cast<int>(3.0f * pop) + 1, coreHot, TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	// ダウン中（Down Phase 2）および8秒間スタン中（FinalStun）のピヨり星＆放電スパーク＆カウントダウン演出
	if ((mState == State::Down && mDownPhase == 2) || mState == State::FinalStun)
	{
		int headX = screenX + (mFacingDirection < 0.0f ? -145 : 145);
		int headY = screenY + 45;

		SetDrawBlendMode(DX_BLENDMODE_ADD, 230);
		for (int s = 0; s < 4; ++s)
		{
			float ang = (mStunTimer * 0.11f) + s * (3.14159265f * 0.5f);
			int sx = headX + static_cast<int>(std::cos(ang) * 68.0f);
			int sy = headY + static_cast<int>(std::sin(ang) * 20.0f);
			DrawCircle(sx, sy, 10, GetColor(255, 230, 50), TRUE);
			DrawCircle(sx, sy, 5,  GetColor(255, 255, 220), TRUE);
			DrawLine(sx - 15, sy, sx + 15, sy, GetColor(255, 255, 160), 2);
			DrawLine(sx, sy - 15, sx, sy + 15, GetColor(255, 255, 160), 2);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		int totalFrames = (mState == State::FinalStun) ? 480 : DOWN_GROUND_FRAMES;
		float remainSec = (totalFrames - mStunTimer) / 60.0f;
		if (remainSec < 0.0f) remainSec = 0.0f;
		Master::mpGameManager->GetFontManager()->DrawDotString(
			headX - 105, headY - 58, 32, GetColor(255, 240, 60), "CHANCE! %.1fs", remainSec
		);
	}

	// 撃破時の黄金大爆発エフェクト
	if (mState == State::Defeated && mDefeatTimer <= 75)
	{
		SetDrawBlendMode(DX_BLENDMODE_ADD, 230);
		for (int i = 0; i < 4; ++i)
		{
			int ex = screenX + ((mDefeatTimer * 37 + i * 83) % 220) - 110;
			int ey = screenY + ((mDefeatTimer * 53 + i * 61) % 180) - 90;
			int er = 25 + ((mDefeatTimer * 9 + i * 19) % 55);
			DrawCircle(ex, ey, er, GetColor(255, 210, 50), TRUE);
			DrawCircle(ex, ey, er / 2, GetColor(255, 255, 230), TRUE);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// 分身2体（目の色違い・徐々にフェード）＋本体の3体同時横突進描画
	mCloneDashAttack.Draw(mpBlockMap, mvPosition, mFacingDirection, mDrawScale, handle, footOffsetFromCenter, mfEnemyHeight);

	// 分身1匹の上空特大火球8秒間メテオ＆本体＋分身1体の2秒ごと疾走突進描画
	mCloneMeteorDashAttack.Draw(mpBlockMap, mvPosition, mFacingDirection, mDrawScale, handle, footOffsetFromCenter, mfEnemyHeight);

	// 最後の大技：分身2匹の大きな風の玉＆吸い込まれるマグマ・叩きつけ大爆発描画
	mUltimateWindSlamAttack.Draw(mpBlockMap, player, mvPosition, mFacingDirection, mDrawScale, handle, footOffsetFromCenter, mfEnemyHeight);

	// 分身2体の120度ランダム風の刃＋本体の逆方向溜め咆哮描画
	mCloneWindRoarAttack.Draw(mpBlockMap, mvPosition, mFacingDirection, mDrawScale, handle, footOffsetFromCenter, mfEnemyHeight);

	// 中央マグマ大火球・縦マグマ噴水・周囲へのマグマ飛散弾の描画
	mMagmaFireballAttack.Draw(mpBlockMap, mvPosition, mFacingDirection, mDrawScale);

	// 地上バウンド火の玉・反射数連動マグマ噴水・周囲へのマグマ飛散弾の描画
	mBounceFireballAttack.Draw(mpBlockMap, mvPosition, mFacingDirection, mDrawScale);
}

void Stage3BossDragon::DrawBossUI(Player* player, BlockMap& blockMap) const
{
	(void)blockMap;

	if (player != nullptr && player->GetIsDead()) return;
	if (mState == State::CutsceneIdle || mState == State::AwakeningCutscene) return;
	if (mHp <= 0 && mSmoothHp <= 0.0f) return;

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

	float smoothRatio = mSmoothHp / static_cast<float>(mMaxHp);
	if (smoothRatio > 0.0f)
	{
		if (smoothRatio > 1.0f) smoothRatio = 1.0f;
		DrawBox(bx, by, bx + static_cast<int>(barW * smoothRatio), by + barH, GetColor(220, 50, 50), TRUE);
	}

	float actualRatio = static_cast<float>(mHp) / static_cast<float>(mMaxHp);
	if (actualRatio > 0.0f)
	{
		if (actualRatio > 1.0f) actualRatio = 1.0f;
		unsigned int hpColor = mbIsPhase2 ? GetColor(245, 200, 35) : GetColor(50, 220, 50);
		DrawBox(bx, by, bx + static_cast<int>(barW * actualRatio), by + barH, hpColor, TRUE);
	}

	unsigned int borderColor = mbIsPhase2 ? GetColor(255, 230, 90) : GetColor(255, 255, 255);
	DrawBox(bx, by, bx + barW, by + barH, borderColor, FALSE);

	if (mbIsPhase2)
	{
		Master::mpGameManager->GetFontManager()->DrawDotString(bx + 3, by - 50, 48, GetColor(0, 0, 0), "%s", "アルティメットドラゴン");
		Master::mpGameManager->GetFontManager()->DrawDotString(bx, by - 52, 48, GetColor(255, 225, 65), "%s", "アルティメットドラゴン");
	}
	else
	{
		Master::mpGameManager->GetFontManager()->DrawDotString(bx, by - 52, 48, GetColor(255, 100, 80), "%s", "ドラゴン");
	}
}

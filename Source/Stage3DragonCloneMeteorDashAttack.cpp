#include "Stage3DragonCloneMeteorDashAttack.h"
#include "Effekseer2DManager.h"
#include "Player.h"
#include "BlockMap.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>

namespace
{
	float ConvertWorldToScreenX(float worldX, BlockMap* blockMap)
	{
		if (blockMap != nullptr)
		{
			return worldX - blockMap->GetScrollX() + static_cast<float>(blockMap->mShakeX);
		}
		return worldX;
	}

	float ConvertWorldToScreenY(float worldY, BlockMap* blockMap)
	{
		if (blockMap != nullptr)
		{
			return worldY - blockMap->GetScrollY() + static_cast<float>(blockMap->mShakeY);
		}
		return worldY;
	}

	float GetSlotY(int slotIndex)
	{
		switch (slotIndex)
		{
		case 0:  return 550.0f;  // 上
		case 1:  return 795.0f;  // 真ん中
		default: return 1036.0f; // 下
		}
	}
}

Stage3DragonCloneMeteorDashAttack::Stage3DragonCloneMeteorDashAttack()
	: mPhase(Phase::Inactive)
	, mPhaseTimer(0)
	, mAnimType(ANIM_IDLE)
	, mAnimFrame(0)
	, mCasterCloneX(LEFT_LEDGE_STAND_X)
	, mCasterCloneY(LEFT_LEDGE_FLOOR_Y - BOSS_HALF_HEIGHT)
	, mCasterFacingDir(1.0f)
	, mCasterAnimType(ANIM_ROAR)
	, mCasterAnimFrame(0)
	, mCasterCloneAlpha(0)
	, mbGiantFireballLaunched(false)
	, mMeteorLoopTimer(0)
	, mCycleIndex(0)
	, mCycleTimer(0)
	, mDashStartFromLeft(false)
	, mRealDragonSlot(1)
	, mDashCloneSlot(0)
	, mMoveStartX(RIGHT_LEDGE_STAND_X)
	, mMoveStartY(800.0f)
	, mStartX(RIGHT_LEDGE_STAND_X)
	, mEndX(LEFT_LEDGE_STAND_X)
	, mCurrentDashX(RIGHT_LEDGE_STAND_X)
	, mDashCloneAlpha(0)
	, mCloneEyeGlintTimer(0)
	, mbSlashActiveInCycle(false)
	, mDashStreakEfk(-1)
	, mCloneStreakEfk(-1)
	, mFireTex{ -1, -1, -1, -1 }
{
	Reset();
}

Stage3DragonCloneMeteorDashAttack::~Stage3DragonCloneMeteorDashAttack()
{
	ReleaseCloneAnimations();
}

void Stage3DragonCloneMeteorDashAttack::Initialize()
{
	LoadCloneAnimations();
	Reset();
}

void Stage3DragonCloneMeteorDashAttack::Reset()
{
	mPhase                  = Phase::Inactive;
	mPhaseTimer             = 0;
	mAnimType               = ANIM_IDLE;
	mAnimFrame              = 0;
	mCasterCloneX           = LEFT_LEDGE_STAND_X;
	mCasterCloneY           = LEFT_LEDGE_FLOOR_Y - BOSS_HALF_HEIGHT;
	mCasterFacingDir        = 1.0f;
	mCasterAnimType         = ANIM_ROAR;
	mCasterAnimFrame        = 0;
	mCasterCloneAlpha       = 0;
	mbGiantFireballLaunched = false;
	mMeteorLoopTimer        = 0;
	mCycleIndex             = 0;
	mCycleTimer             = 0;
	mDashStartFromLeft      = false;
	mRealDragonSlot         = 1;
	mDashCloneSlot          = 0;
	mMoveStartX             = RIGHT_LEDGE_STAND_X;
	mMoveStartY             = 800.0f;
	mStartX                 = RIGHT_LEDGE_STAND_X;
	mEndX                   = LEFT_LEDGE_STAND_X;
	mCurrentDashX           = RIGHT_LEDGE_STAND_X;
	mDashCloneAlpha         = 0;
	mCloneEyeGlintTimer     = 0;
	mbSlashActiveInCycle    = false;

	mSkyGiantFireball.active = false;
	mMeteors.clear();
	ClearTrailPoints();
	mSpeedLines.clear();
	mSparks.clear();
	if (mDashStreakEfk != -1)
	{
		Effekseer2DManager::Stop(mDashStreakEfk);
		mDashStreakEfk = -1;
	}
	if (mCloneStreakEfk != -1)
	{
		Effekseer2DManager::Stop(mCloneStreakEfk);
		mCloneStreakEfk = -1;
	}
}

void Stage3DragonCloneMeteorDashAttack::ClearTrailPoints()
{
	for (int i = 0; i < MAX_TRAIL_POINTS; ++i)
	{
		mTrailPoints[i].active = false;
		mTrailPoints[i].alpha  = 0;
	}
}

void Stage3DragonCloneMeteorDashAttack::PushTrailPoint(float x)
{
	for (int i = MAX_TRAIL_POINTS - 1; i > 0; --i)
	{
		mTrailPoints[i] = mTrailPoints[i - 1];
	}
	mTrailPoints[0].active = true;
	mTrailPoints[0].x      = x;
	mTrailPoints[0].alpha  = 175;
}

void Stage3DragonCloneMeteorDashAttack::ReleaseCloneAnimations()
{
	for (int h : mCloneIdleAnim)   if (h != -1) DeleteGraph(h);
	for (int h : mCloneFlyAnim)    if (h != -1) DeleteGraph(h);
	for (int h : mCloneAttackAnim) if (h != -1) DeleteGraph(h);
	for (int h : mCloneRoarAnim)   if (h != -1) DeleteGraph(h);
	mCloneIdleAnim.clear();
	mCloneFlyAnim.clear();
	mCloneAttackAnim.clear();
	mCloneRoarAnim.clear();
	for (int i = 0; i < 4; ++i)
	{
		if (mFireTex[i] != -1)
		{
			DeleteGraph(mFireTex[i]);
			mFireTex[i] = -1;
		}
	}
}

// 本体（赤い目）に対して、分身は目のピクセルを鮮やかな「黄金～エメラルドグリーン」に書き換えたグラフィックを生成する
int Stage3DragonCloneMeteorDashAttack::LoadGraphWithRecoloredEyes(const char* filePath)
{
	int softImg = LoadSoftImage(filePath);
	if (softImg == -1)
	{
		return LoadGraph(filePath);
	}

	int w = 0, h = 0;
	GetSoftImageSize(softImg, &w, &h);

	std::vector< std::pair<int, int> > eyePixels;
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			int r = 0, g = 0, b = 0, a = 0;
			GetPixelSoftImage(softImg, x, y, &r, &g, &b, &a);
			if (a > 120 && r > g + 30 && r > b + 15 && r > 110)
			{
				eyePixels.push_back(std::make_pair(x, y));
			}
		}
	}

	for (size_t i = 0; i < eyePixels.size(); ++i)
	{
		int ex = eyePixels[i].first;
		int ey = eyePixels[i].second;
		for (int dy = -1; dy <= 1; ++dy)
		{
			for (int dx = -1; dx <= 1; ++dx)
			{
				int nx = ex + dx;
				int ny = ey + dy;
				if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;

				int r = 0, g = 0, b = 0, a = 0;
				GetPixelSoftImage(softImg, nx, ny, &r, &g, &b, &a);
				if (a > 80)
				{
					if (dx == 0 && dy == 0)
					{
						DrawPixelSoftImage(softImg, nx, ny, 80, 255, 180, a);
					}
					else
					{
						DrawPixelSoftImage(softImg, nx, ny, 40, 240, 140, a);
					}
				}
			}
		}
	}

	int graphHandle = CreateGraphFromSoftImage(softImg);
	DeleteSoftImage(softImg);
	return graphHandle;
}

void Stage3DragonCloneMeteorDashAttack::LoadCloneAnimations()
{
	ReleaseCloneAnimations();
	LoadDivGraph("Resource/Effect/Pierre01/Pierre01/Texture/Fire.png", 4, 2, 2, 256, 256, mFireTex);

	for (int i = 1; i <= 4; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_idle/sprite_idle%d.png", i);
		int h = LoadGraphWithRecoloredEyes(path);
		if (h != -1) mCloneIdleAnim.push_back(h);
	}
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_flying/sprite_fly%d.png", i);
		int h = LoadGraphWithRecoloredEyes(path);
		if (h != -1) mCloneFlyAnim.push_back(h);
	}
	for (int i = 1; i <= 9; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_attack/sprite_attack%d.png", i);
		int h = LoadGraphWithRecoloredEyes(path);
		if (h != -1) mCloneAttackAnim.push_back(h);
	}
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_roar/roar%d.png", i);
		int h = LoadGraphWithRecoloredEyes(path);
		if (h != -1) mCloneRoarAnim.push_back(h);
	}
}

// 上(0)・真ん中(1)・下(2) の3レーンからランダムに2つを選び、本体と突進分身1匹を配置する
void Stage3DragonCloneMeteorDashAttack::ShuffleTwoLanesForCycle()
{
	mRealDragonSlot = GetRand(2); // 0, 1, 2
	int offset      = 1 + GetRand(1); // 1 or 2
	mDashCloneSlot  = (mRealDragonSlot + offset) % 3;
}

void Stage3DragonCloneMeteorDashAttack::StartAttack(VECTOR& bossPos, float& facingDir, Player* player)
{
	mPhase                  = Phase::IntroCastGiantFireball;
	mPhaseTimer             = 0;
	mAnimType               = ANIM_TAKEOFF;
	mAnimFrame              = 0;
	mbGiantFireballLaunched = false;
	mMeteorLoopTimer        = 0;
	mCycleIndex             = 0;
	mCycleTimer             = 0;
	mDashCloneAlpha         = 0;
	mCloneEyeGlintTimer     = 0;
	mbSlashActiveInCycle    = false;

	mSkyGiantFireball.active = false;
	mMeteors.clear();
	ClearTrailPoints();
	mSpeedLines.clear();
	mSparks.clear();

	// 最初に特大火の玉を撃つ地上分身は、必ず「プレイヤーがいる方向の反対側」から出現させる！
	float playerX      = (player != nullptr) ? player->GetPosition().x : LEFT_LEDGE_STAND_X;
	// 【Stage3.png 5724x1692拡大版 対応】ボス部屋中央X(4353)に更新
	bool  playerOnLeft = (playerX < 4353.0f);
	mDashStartFromLeft = playerOnLeft; // プレイヤーが左なら分身は右足場に出現（プレイヤーが右なら分身は左足場に出現）

	if (mDashStartFromLeft)
	{
		mStartX          = LEFT_LEDGE_STAND_X;
		mEndX            = RIGHT_LEDGE_STAND_X;
		facingDir        = 1.0f;
		mCasterCloneX    = RIGHT_LEDGE_STAND_X;
		mCasterCloneY    = RIGHT_LEDGE_FLOOR_Y - BOSS_HALF_HEIGHT;
		mCasterFacingDir = -1.0f;
	}
	else
	{
		mStartX          = RIGHT_LEDGE_STAND_X;
		mEndX            = LEFT_LEDGE_STAND_X;
		facingDir        = -1.0f;
		mCasterCloneX    = LEFT_LEDGE_STAND_X;
		mCasterCloneY    = LEFT_LEDGE_FLOOR_Y - BOSS_HALF_HEIGHT;
		mCasterFacingDir = 1.0f;
	}

	mCasterAnimType   = ANIM_ROAR;
	mCasterAnimFrame  = 0;
	mCasterCloneAlpha = 0;

	// 1回目の突進用に上・真ん中・下をランダム抽選
	ShuffleTwoLanesForCycle();

	mMoveStartX   = bossPos.x;
	mMoveStartY   = bossPos.y;
	mCurrentDashX = mStartX;
}

bool Stage3DragonCloneMeteorDashAttack::IsSlashActive() const
{
	return (mPhase == Phase::MeteorAndDashLoop && mbSlashActiveInCycle);
}

void Stage3DragonCloneMeteorDashAttack::SpawnImpactSparks(float x, float y, int count, unsigned int mainCol)
{
	for (int i = 0; i < count; ++i)
	{
		float angle = (static_cast<float>(GetRand(360)) * 3.14159265f) / 180.0f;
		float spd   = 3.0f + static_cast<float>(GetRand(110)) * 0.1f;
		FireSpark sp;
		sp.x       = x;
		sp.y       = y;
		sp.vx      = std::cos(angle) * spd;
		sp.vy      = std::sin(angle) * spd - 2.0f;
		sp.length  = 16.0f + static_cast<float>(GetRand(16));
		sp.maxLife = 14 + GetRand(10);
		sp.life    = sp.maxLife;
		sp.color   = (i % 3 == 0) ? GetColor(255, 245, 150) : mainCol;
		mSparks.push_back(sp);
	}
}

void Stage3DragonCloneMeteorDashAttack::SpawnMeteorFireball()
{
	FallingMeteorFireball m;
	m.active    = true;
	// ボス部屋全域（左足場～中央浮遊台～右足場：X = 3450 .. 5620）のランダムな場所に降らせる
	m.x         = 3450.0f + static_cast<float>(GetRand(2170));
	m.y         = ROOM_CEILING_Y + 20.0f;
	m.vx        = (GetRand(20) - 10) * 0.08f;
	m.vy        = 11.5f + static_cast<float>(GetRand(45)) * 0.1f;
	m.radius    = 25.0f + static_cast<float>(GetRand(8));
	m.warnTimer = 16; // 天井でキラッと光ってから落下開始
	mMeteors.push_back(m);
}

void Stage3DragonCloneMeteorDashAttack::UpdateDragon(
	VECTOR& bossPos,
	float& facingDir,
	bool& isBossGrounded,
	int& outAnimType,
	int& outAnimFrame,
	BlockMap* blockMap)
{
	if (mPhase == Phase::Inactive || mPhase == Phase::Finished)
	{
		return;
	}

	mPhaseTimer++;

	if (IsEyeGlintMoment())
	{
		mCloneEyeGlintTimer = 16;
	}
	else if (mCloneEyeGlintTimer > 0)
	{
		mCloneEyeGlintTimer--;
	}

	switch (mPhase)
	{
	// =================================================================
	// 1. 分身1匹が地上で上空に特大火の玉を放ち、本体は最初の突進レーンへ移動
	// =================================================================
	case Phase::IntroCastGiantFireball:
	{
		isBossGrounded       = false;
		mbSlashActiveInCycle = false;

		// 地上ブレス役の分身1匹がフェードインして咆哮ブレスモーションで上空へ特大火の玉を発射！
		if (mPhaseTimer <= 15)
		{
			mCasterCloneAlpha = (mPhaseTimer * 225) / 15;
		}
		else
		{
			mCasterCloneAlpha = 225;
		}

		mCasterAnimType = ANIM_ROAR;
		if (mPhaseTimer < 14)      mCasterAnimFrame = 0;
		else if (mPhaseTimer < 26) mCasterAnimFrame = 1;
		else if (mPhaseTimer < 38) mCasterAnimFrame = 2;
		else if (mPhaseTimer < 46) mCasterAnimFrame = 3;
		else if (mPhaseTimer < 64) mCasterAnimFrame = ((mPhaseTimer / 4) % 2 == 0) ? 4 : 5;
		else                       mCasterAnimFrame = 6;

		// フレーム46で分身1匹が口元から上空へ向かって特大火の玉を打ち上げる！
		if (!mbGiantFireballLaunched && mPhaseTimer >= 46)
		{
			mbGiantFireballLaunched  = true;
			mSkyGiantFireball.active = true;
			mSkyGiantFireball.x      = mCasterCloneX + mCasterFacingDir * 120.0f;
			mSkyGiantFireball.y      = mCasterCloneY - 90.0f;
			mSkyGiantFireball.vy     = -25.0f;
			mSkyGiantFireball.radius = 125.0f;

			SpawnImpactSparks(mSkyGiantFireball.x, mSkyGiantFireball.y, 28, GetColor(255, 140, 40));
			if (blockMap != nullptr)
			{
				blockMap->mShakeX = GetRand(12) - 6;
				blockMap->mShakeY = GetRand(12) - 6;
			}
		}

		// 本体は離陸して最初の突進レーン（上・真ん中・下のどれか）へ移動しておく
		if (mPhaseTimer < 24)
		{
			mAnimType  = ANIM_TAKEOFF;
			mAnimFrame = (mPhaseTimer * 7) / 24;
			if (mAnimFrame > 6) mAnimFrame = 6;
			bossPos.y -= 6.0f;
			mMoveStartX = bossPos.x;
			mMoveStartY = bossPos.y;
		}
		else
		{
			mAnimType  = ANIM_FLY;
			mAnimFrame = (mPhaseTimer / 5) % 7;
			float t    = static_cast<float>(mPhaseTimer - 24) / static_cast<float>(INTRO_CAST_FRAMES - 24);
			if (t > 1.0f) t = 1.0f;
			float ease = t * (2.0f - t);

			float targetY = GetSlotY(mRealDragonSlot);
			bossPos.x     = mMoveStartX + (mStartX - mMoveStartX) * ease;
			bossPos.y     = mMoveStartY + (targetY - mMoveStartY) * ease;
			mCurrentDashX = bossPos.x;
		}

		facingDir = mDashStartFromLeft ? 1.0f : -1.0f;

		if (mPhaseTimer >= INTRO_CAST_FRAMES)
		{
			if (blockMap != nullptr)
			{
				blockMap->mShakeX = 0;
				blockMap->mShakeY = 0;
			}
			mPhase           = Phase::MeteorAndDashLoop;
			mPhaseTimer      = 0;
			mMeteorLoopTimer = 0;
			mCycleIndex      = 0;
			mCycleTimer      = 0;
			mMoveStartX      = bossPos.x;
			mMoveStartY      = bossPos.y;
		}
		break;
	}

	// =================================================================
	// 2. 8秒間（480フレーム）：
	//    ・上から火の玉がランダムなステージの場所へ降り注ぎ続ける
	//    ・本体＋もう1匹の分身が「2秒（120フレーム）に1回」上・真ん中・下のどれかから高速突進攻撃！
	// =================================================================
	case Phase::MeteorAndDashLoop:
	{
		isBossGrounded = false;
		mMeteorLoopTimer++;

		// 地上で特大火球を放った分身1匹目は、打ち上げ後に余韻を残してスッとフェードアウトして消える
		if (mCasterCloneAlpha > 0)
		{
			mCasterCloneAlpha -= 6;
			if (mCasterCloneAlpha < 0) mCasterCloneAlpha = 0;
		}

		// 8秒間（480フレーム）、約15フレームごとにランダムなステージの場所へ上空から火の玉を降らせる
		if (mMeteorLoopTimer % 15 == 1 && mMeteorLoopTimer <= METEOR_TOTAL_FRAMES - 30)
		{
			SpawnMeteorFireball();
			// 時々2個同時に降らせて密度を高める
			if (GetRand(2) == 0)
			{
				SpawnMeteorFireball();
			}
		}

		// -------------------------------------------------------------
		// 2秒（120フレーム）に1回の「本体＋もう1匹の分身」高速横突進サイクル制御
		// 0..17f  : 上・真ん中・下のシャッフル位置へ素早く移動
		// 18..63f : 本体＋分身1匹が出現し、通常攻撃を溜める（目の色の違いとレーン予告を表示）
		// 64..108f: あのかっこいい超高速横突進攻撃！（分身は徐々にフェードアウトして108fで消える）
		// 109..119f: 向かい側で次の2秒サイクルへ向けて反転＆再シャッフル準備
		// -------------------------------------------------------------
		mCycleTimer++;
		facingDir = mDashStartFromLeft ? 1.0f : -1.0f;

		if (mCycleTimer <= 18)
		{
			// (A) シャッフルされたレーン高さへ素早くセット
			mbSlashActiveInCycle = false;
			mDashCloneAlpha      = 0;
			mAnimType            = ANIM_FLY;
			mAnimFrame           = (mCycleTimer / 4) % 7;

			float t = static_cast<float>(mCycleTimer) / 18.0f;
			float ease = t * (2.0f - t);
			float targetY = GetSlotY(mRealDragonSlot);

			bossPos.x     = mStartX;
			bossPos.y     = mMoveStartY + (targetY - mMoveStartY) * ease;
			mCurrentDashX = mStartX;
		}
		else if (mCycleTimer <= 64)
		{
			// (B) もう1匹の分身が上・真ん中・下の別レーンに出現し、本体と一緒に通常攻撃を溜める！
			mbSlashActiveInCycle = false;
			mAnimType            = ANIM_ATTACK;

			int chargeTick = mCycleTimer - 18; // 0 .. 46
			if (chargeTick <= 12)
			{
				mDashCloneAlpha = (chargeTick * 255) / 12;
			}
			else
			{
				mDashCloneAlpha = 255;
			}

			if (chargeTick < 14)      mAnimFrame = 0;
			else if (chargeTick < 28) mAnimFrame = 1;
			else                      mAnimFrame = 2;

			float pullRatio = static_cast<float>(chargeTick) / 46.0f;
			mCurrentDashX   = mStartX - facingDir * (50.0f * pullRatio);
			bossPos.x       = mCurrentDashX;
			bossPos.y       = GetSlotY(mRealDragonSlot) + std::sin(chargeTick * 0.3f) * 5.0f;

			// 溜め中の大気吸引ライン
			if (chargeTick % 2 == 0)
			{
				int activeSlots[2] = { mRealDragonSlot, mDashCloneSlot };
				for (int i = 0; i < 2; ++i)
				{
					bool isClone = (i == 1);
					SpeedLine sl;
					sl.x         = mCurrentDashX + facingDir * (160.0f + static_cast<float>(GetRand(280)));
					sl.y         = GetSlotY(activeSlots[i]) + static_cast<float>(GetRand(100) - 50);
					sl.vx        = -facingDir * (22.0f + static_cast<float>(GetRand(14)));
					sl.length    = 60.0f + static_cast<float>(GetRand(60));
					sl.thickness = 2;
					sl.maxLife   = 10;
					sl.life      = 10;
					sl.isClone   = isClone;
					sl.color     = isClone ? GetColor(90, 255, 200) : GetColor(255, 110, 80);
					mSpeedLines.push_back(sl);
				}
			}

			if (mCycleTimer == 64)
			{
				mMoveStartX = mCurrentDashX;
				ClearTrailPoints();

				// 突進発射時：魔法陣リングは出さず、本体・分身の追従スピード斬撃ストリークのみ再生
				float dashRotZ = (facingDir >= 0.0f) ? 0.0f : 3.14159265f;
				if (mDashStreakEfk != -1)
				{
					Effekseer2DManager::Stop(mDashStreakEfk);
					mDashStreakEfk = -1;
				}
				if (mCloneStreakEfk != -1)
				{
					Effekseer2DManager::Stop(mCloneStreakEfk);
					mCloneStreakEfk = -1;
				}
				mDashStreakEfk = Effekseer2DManager::Play(
					Effect2DId::Stage3_FlameBurst,
					mCurrentDashX + facingDir * 95.0f,
					GetSlotY(mRealDragonSlot),
					1.45f,
					2.20f,
					dashRotZ,
					255, 205, 80, 245
				);
				mCloneStreakEfk = Effekseer2DManager::Play(
					Effect2DId::Stage3_SlashWind,
					mCurrentDashX + facingDir * 90.0f,
					GetSlotY(mDashCloneSlot),
					1.25f,
					2.20f,
					dashRotZ,
					85, 255, 220, 230
				);
			}
		}
		else if (mCycleTimer <= 108)
		{
			// (C) あのかっこいい超高速横突進攻撃！（本体＋分身1匹が同時に向かい側の地面まで疾走！）
			mAnimType = ANIM_ATTACK;
			int dashTick = mCycleTimer - 64; // 1 .. 44
			const int dashDuration = 44;

			mbSlashActiveInCycle = (dashTick >= 3 && dashTick <= dashDuration - 3);

			float t = static_cast<float>(dashTick) / static_cast<float>(dashDuration);
			if (t > 1.0f) t = 1.0f;
			float ease = 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);

			mCurrentDashX = mMoveStartX + (mEndX - mMoveStartX) * ease;
			bossPos.x     = mCurrentDashX;
			bossPos.y     = GetSlotY(mRealDragonSlot);

			PushTrailPoint(mCurrentDashX);

			// 2D Effekseer ストリークを本体＆分身の突進位置に毎フレーム追従させて超高速トレイルを描画
			float dashRotZ = (facingDir >= 0.0f) ? 0.0f : 3.14159265f;
			if (mDashStreakEfk != -1 && Effekseer2DManager::IsPlaying(mDashStreakEfk))
			{
				Effekseer2DManager::SetTransform(
					mDashStreakEfk,
					mCurrentDashX + facingDir * 95.0f,
					GetSlotY(mRealDragonSlot),
					1.45f,
					dashRotZ
				);
			}
			if (mCloneStreakEfk != -1 && Effekseer2DManager::IsPlaying(mCloneStreakEfk))
			{
				Effekseer2DManager::SetTransform(
					mCloneStreakEfk,
					mCurrentDashX + facingDir * 90.0f,
					GetSlotY(mDashCloneSlot),
					1.25f,
					dashRotZ
				);
			}

			// 中盤の超音速黄金雷スパーク（軽量化のため本体に1発のみ）
			if (dashTick == 12)
			{
				Effekseer2DManager::Play(
					Effect2DId::Stage3_Thunder,
					mCurrentDashX + facingDir * 105.0f,
					GetSlotY(mRealDragonSlot),
					1.30f,
					2.05f,
					dashRotZ,
					255, 235, 95, 240
				);
			}

			// 本体と分身1匹の2レーンに超軽量な高速スピードラインを間引き生成（4フレームごと）
			if (dashTick % 4 == 0)
			{
				int activeSlots[2] = { mRealDragonSlot, mDashCloneSlot };
				for (int i = 0; i < 2; ++i)
				{
					bool isClone = (i == 1);
					float laneY  = GetSlotY(activeSlots[i]);
					SpeedLine sl;
					sl.x         = mCurrentDashX + facingDir * static_cast<float>(GetRand(120) - 60);
					sl.y         = laneY + static_cast<float>(GetRand(120) - 60);
					sl.vx        = -facingDir * (44.0f + static_cast<float>(GetRand(24)));
					sl.length    = 200.0f + static_cast<float>(GetRand(160));
					sl.thickness = 2;
					sl.maxLife   = 9 + GetRand(4);
					sl.life      = sl.maxLife;
					sl.isClone   = isClone;
					sl.color     = isClone ? GetColor(130, 255, 215) : GetColor(255, 200, 95);
					mSpeedLines.push_back(sl);
				}
			}

			if (dashTick < 6)       mAnimFrame = 3;
			else if (dashTick < 13) mAnimFrame = 4;
			else if (dashTick < 21) mAnimFrame = 5;
			else if (dashTick < 29) mAnimFrame = 6;
			else if (dashTick < 36) mAnimFrame = 7;
			else                    mAnimFrame = 8;

			// 分身は突進攻撃しながら徐々にフェードアウトして最後に完全に消える
			mDashCloneAlpha = static_cast<int>(255.0f * (1.0f - t));
			if (mDashCloneAlpha < 0) mDashCloneAlpha = 0;

			if (dashTick >= 3 && dashTick <= 32 && blockMap != nullptr)
			{
				blockMap->mShakeX = GetRand(8) - 4;
				blockMap->mShakeY = GetRand(8) - 4;
			}
			else if (blockMap != nullptr)
			{
				blockMap->mShakeX = 0;
				blockMap->mShakeY = 0;
			}
		}
		else
		{
			// (D) 109..119f: 向かい側に到達して分身消滅、次の2秒サイクルへ向けて準備
			mbSlashActiveInCycle = false;
			mDashCloneAlpha      = 0;
			bossPos.x            = mEndX;
			mAnimType            = ANIM_FLY;
			mAnimFrame           = (mCycleTimer / 4) % 7;

			if (blockMap != nullptr)
			{
				blockMap->mShakeX = 0;
				blockMap->mShakeY = 0;
			}

			if (mCycleTimer >= DASH_CYCLE_FRAMES)
			{
				mCycleTimer        = 0;
				mCycleIndex++;
				mDashStartFromLeft = !mDashStartFromLeft;
				mStartX            = mDashStartFromLeft ? LEFT_LEDGE_STAND_X  : RIGHT_LEDGE_STAND_X;
				mEndX              = mDashStartFromLeft ? RIGHT_LEDGE_STAND_X : LEFT_LEDGE_STAND_X;
				mCurrentDashX      = mStartX;
				mMoveStartX        = bossPos.x;
				mMoveStartY        = bossPos.y;
				ClearTrailPoints();

				// 次の2秒突進のために上・真ん中・下を再度ランダムシャッフル！
				ShuffleTwoLanesForCycle();
			}
		}

		// 8秒間（480フレーム＝2秒突進×4回）完了したら着陸フェーズへ移行
		if (mMeteorLoopTimer >= METEOR_TOTAL_FRAMES)
		{
			if (blockMap != nullptr)
			{
				blockMap->mShakeX = 0;
				blockMap->mShakeY = 0;
			}
			mbSlashActiveInCycle = false;
			mDashCloneAlpha      = 0;
			mCasterCloneAlpha    = 0;
			ClearTrailPoints();
			mPhase      = Phase::LandAtLedge;
			mPhaseTimer = 0;
			mAnimFrame  = 0;
		}
		break;
	}

	// =================================================================
	// 3. 8秒間の攻撃終了後、近い方の足場へ本体が着陸して完了
	// =================================================================
	case Phase::LandAtLedge:
	{
		isBossGrounded       = false;
		mbSlashActiveInCycle = false;
		mDashCloneAlpha      = 0;
		mCasterCloneAlpha    = 0;
		mAnimType            = ANIM_LANDING;
		mAnimFrame           = (mPhaseTimer / 4) % 7;

		bool landLeft = (std::fabs(bossPos.x - LEFT_LEDGE_STAND_X) < std::fabs(bossPos.x - RIGHT_LEDGE_STAND_X));
		float targetX = landLeft ? LEFT_LEDGE_STAND_X : RIGHT_LEDGE_STAND_X;
		float floorY  = landLeft ? LEFT_LEDGE_FLOOR_Y : RIGHT_LEDGE_FLOOR_Y;
		float standY  = floorY - BOSS_HALF_HEIGHT;

		bossPos.x += (targetX - bossPos.x) * 0.25f;
		if (bossPos.y < standY)
		{
			bossPos.y += 18.0f;
			if (bossPos.y > standY) bossPos.y = standY;
		}
		else if (bossPos.y > standY)
		{
			bossPos.y -= 18.0f;
			if (bossPos.y < standY) bossPos.y = standY;
		}

		if (std::fabs(bossPos.y - standY) <= 1.5f && mPhaseTimer >= 16)
		{
			bossPos.x      = targetX;
			bossPos.y      = standY;
			isBossGrounded = true;
			facingDir      = landLeft ? 1.0f : -1.0f;
			mPhase         = Phase::Finished;
			mPhaseTimer    = 0;
		}
		break;
	}

	default:
		break;
	}

	outAnimType  = mAnimType;
	outAnimFrame = mAnimFrame;
}

void Stage3DragonCloneMeteorDashAttack::UpdateProjectilesAndEffects(Player* player, BlockMap* blockMap, int attackDamage)
{
	(void)blockMap;

	// 1. 残像・スピードライン・ソニックブーム・火花の更新
	for (int i = 0; i < MAX_TRAIL_POINTS; ++i)
	{
		if (mTrailPoints[i].active)
		{
			mTrailPoints[i].alpha -= 20;
			if (mTrailPoints[i].alpha <= 0)
			{
				mTrailPoints[i].active = false;
				mTrailPoints[i].alpha  = 0;
			}
		}
	}

	for (int i = static_cast<int>(mSpeedLines.size()) - 1; i >= 0; --i)
	{
		mSpeedLines[i].x += mSpeedLines[i].vx;
		mSpeedLines[i].life--;
		if (mSpeedLines[i].life <= 0)
		{
			mSpeedLines.erase(mSpeedLines.begin() + i);
		}
	}

	for (int i = static_cast<int>(mSparks.size()) - 1; i >= 0; --i)
	{
		mSparks[i].x  += mSparks[i].vx;
		mSparks[i].y  += mSparks[i].vy;
		mSparks[i].vy += 0.24f;
		mSparks[i].life--;
		if (mSparks[i].life <= 0)
		{
			mSparks.erase(mSparks.begin() + i);
		}
	}

	// 2. 分身1匹が地上から上空へ打ち上げた特大火の玉の更新
	if (mSkyGiantFireball.active)
	{
		mSkyGiantFireball.y += mSkyGiantFireball.vy;

		// 上昇中の炎トレイル
		for (int i = 0; i < 3; ++i)
		{
			FireSpark sp;
			sp.x       = mSkyGiantFireball.x + static_cast<float>(GetRand(70) - 35);
			sp.y       = mSkyGiantFireball.y + 20.0f;
			sp.vx      = static_cast<float>(GetRand(20) - 10) * 0.25f;
			sp.vy      = 4.0f + static_cast<float>(GetRand(30)) * 0.2f;
			sp.length  = 18.0f;
			sp.maxLife = 14;
			sp.life    = 14;
			sp.color   = (i == 0) ? GetColor(255, 240, 120) : GetColor(255, 95, 30);
			mSparks.push_back(sp);
		}

		// 天井付近（Y <= 240）に達したら大炸裂して上空の火球群になる！
		if (mSkyGiantFireball.y <= ROOM_CEILING_Y + 15.0f)
		{
			SpawnImpactSparks(mSkyGiantFireball.x, ROOM_CEILING_Y + 20.0f, 40, GetColor(255, 160, 40));
			mSkyGiantFireball.active = false;
		}
	}

	// 3. 8秒間上からランダムな場所へ落ちてくる火の玉（メテオ）の更新と当たり判定
	for (int i = static_cast<int>(mMeteors.size()) - 1; i >= 0; --i)
	{
		FallingMeteorFireball& m = mMeteors[i];
		if (!m.active)
		{
			mMeteors.erase(mMeteors.begin() + i);
			continue;
		}

		if (m.warnTimer > 0)
		{
			m.warnTimer--;
			continue;
		}

		m.x  += m.vx;
		m.y  += m.vy;
		m.vy += 0.16f; // 落下加速

		// 落下火球の炎の尾
		if (GetRand(1) == 0)
		{
			FireSpark sp;
			sp.x       = m.x + static_cast<float>(GetRand(24) - 12);
			sp.y       = m.y - m.radius * 0.6f;
			sp.vx      = static_cast<float>(GetRand(14) - 7) * 0.2f;
			sp.vy      = -3.0f - static_cast<float>(GetRand(20)) * 0.15f;
			sp.length  = 14.0f;
			sp.maxLife = 10;
			sp.life    = 10;
			sp.color   = GetColor(255, 150, 40);
			mSparks.push_back(sp);
		}

		// プレイヤーとの当たり判定
		if (player != nullptr && !player->GetIsDead())
		{
			float pLeft   = player->GetPosition().x - 20.0f;
			float pRight  = player->GetPosition().x + 20.0f;
			float pTop    = player->GetPosition().y - 44.0f;
			float pBottom = player->GetPosition().y + 44.0f;

			float hitR = m.radius * 0.78f;
			if (m.x + hitR > pLeft && m.x - hitR < pRight &&
				m.y + hitR > pTop  && m.y - hitR < pBottom)
			{
				player->TakeDamage(attackDamage, DeathReason::DeathType::DragonExplosion);
				SpawnImpactSparks(m.x, m.y, 18, GetColor(255, 110, 30));
				mMeteors.erase(mMeteors.begin() + i);
				continue;
			}
		}

		// 左右の足場の床またはマグマ表面に達したら炸裂して消える
		bool hitLeftLedge  = (m.x < LEFT_LEDGE_EDGE_X  && m.y + m.radius >= LEFT_LEDGE_FLOOR_Y);
		bool hitRightLedge = (m.x > RIGHT_LEDGE_EDGE_X && m.y + m.radius >= RIGHT_LEDGE_FLOOR_Y);
		bool hitMagma      = (m.x >= LEFT_LEDGE_EDGE_X && m.x <= RIGHT_LEDGE_EDGE_X && m.y + m.radius >= MAGMA_SURFACE_Y);

		if (hitLeftLedge || hitRightLedge || hitMagma)
		{
			SpawnImpactSparks(m.x, m.y, 16, GetColor(255, 120, 35));
			mMeteors.erase(mMeteors.begin() + i);
		}
	}
}

int Stage3DragonCloneMeteorDashAttack::SelectDashCloneSpriteHandle(float& outCloneFootOffset) const
{
	outCloneFootOffset = 79.0f;
	if (mAnimType == ANIM_ATTACK && !mCloneAttackAnim.empty())
	{
		outCloneFootOffset = 79.0f;
		return mCloneAttackAnim[mAnimFrame % mCloneAttackAnim.size()];
	}
	if (!mCloneFlyAnim.empty())
	{
		outCloneFootOffset = 57.0f;
		return mCloneFlyAnim[mAnimFrame % mCloneFlyAnim.size()];
	}
	return -1;
}

int Stage3DragonCloneMeteorDashAttack::SelectCasterCloneSpriteHandle(float& outCasterFootOffset) const
{
	outCasterFootOffset = 72.0f;
	if (mCasterAnimType == ANIM_ROAR && !mCloneRoarAnim.empty())
	{
		outCasterFootOffset = 72.0f;
		return mCloneRoarAnim[mCasterAnimFrame % mCloneRoarAnim.size()];
	}
	if (!mCloneIdleAnim.empty())
	{
		outCasterFootOffset = 79.0f;
		return mCloneIdleAnim[mCasterAnimFrame % mCloneIdleAnim.size()];
	}
	return -1;
}

void Stage3DragonCloneMeteorDashAttack::GetEyeWorldPosition(
	float posX,
	float posY,
	float facingDir,
	float drawScale,
	float footOffset,
	float enemyHeight,
	bool isAttackAnim,
	int animFrame,
	float& outEyeX,
	float& outEyeY) const
{
	static const float kAttackEyeOffsetX[9] = { -88.0f, -94.0f, -98.0f, -85.0f, -73.0f, -91.0f, -93.0f, -74.0f, -70.0f };
	static const float kAttackEyeOffsetY[9] = { -40.0f,   9.0f, -16.0f, -40.0f, -13.0f,   5.0f,  14.0f, -12.0f, -12.0f };

	float localOffX = -88.0f;
	float localOffY = -40.0f;
	if (isAttackAnim && animFrame >= 0 && animFrame < 9)
	{
		localOffX = kAttackEyeOffsetX[animFrame];
		localOffY = kAttackEyeOffsetY[animFrame];
	}

	float feetWorldY    = posY + (enemyHeight * 0.5f);
	float spriteCenterY = feetWorldY - (footOffset * drawScale);

	float dirSign = (facingDir > 0.0f) ? -1.0f : 1.0f;
	outEyeX = posX + (localOffX * dirSign) * drawScale;
	outEyeY = spriteCenterY + localOffY * drawScale;
}

void Stage3DragonCloneMeteorDashAttack::DrawEyeCrossGlint(int eyeSx, int eyeSy, int alpha, bool isClone) const
{
	(void)isClone;
	if (alpha <= 0 || mCloneEyeGlintTimer <= 0) return;

	float t   = static_cast<float>(mCloneEyeGlintTimer) / 16.0f;
	float pop = std::sin(t * 3.14159265f);
	int   a   = (alpha * static_cast<int>(pop * 255.0f)) / 255;
	if (a <= 0) return;

	int hLen  = static_cast<int>(64.0f * pop);
	int hHalf = static_cast<int>(7.0f * pop) + 1;
	int vLen  = static_cast<int>(46.0f * pop);
	int vHalf = static_cast<int>(6.0f * pop) + 1;
	int dLen  = static_cast<int>(20.0f * pop);

	unsigned int outerColor = GetColor(15, 255, 145);
	unsigned int coreHot    = GetColor(225, 255, 235);

	SetDrawBlendMode(DX_BLENDMODE_ADD, a);
	DrawTriangle(eyeSx - hLen, eyeSy, eyeSx, eyeSy - hHalf, eyeSx, eyeSy + hHalf, outerColor, TRUE);
	DrawTriangle(eyeSx + hLen, eyeSy, eyeSx, eyeSy - hHalf, eyeSx, eyeSy + hHalf, outerColor, TRUE);
	DrawTriangle(eyeSx, eyeSy - vLen, eyeSx - vHalf, eyeSy, eyeSx + vHalf, eyeSy, outerColor, TRUE);
	DrawTriangle(eyeSx, eyeSy + vLen, eyeSx - vHalf, eyeSy, eyeSx + vHalf, eyeSy, outerColor, TRUE);
	DrawTriangle(eyeSx - (hLen * 2 / 3), eyeSy, eyeSx, eyeSy - 2, eyeSx, eyeSy + 2, coreHot, TRUE);
	DrawTriangle(eyeSx + (hLen * 2 / 3), eyeSy, eyeSx, eyeSy - 2, eyeSx, eyeSy + 2, coreHot, TRUE);
	DrawTriangle(eyeSx, eyeSy - (vLen * 2 / 3), eyeSx - 2, eyeSy, eyeSx + 2, eyeSy, coreHot, TRUE);
	DrawTriangle(eyeSx, eyeSy + (vLen * 2 / 3), eyeSx - 2, eyeSy, eyeSx + 2, eyeSy, coreHot, TRUE);
	DrawLine(eyeSx - dLen, eyeSy - dLen, eyeSx + dLen, eyeSy + dLen, outerColor, 2);
	DrawLine(eyeSx - dLen, eyeSy + dLen, eyeSx + dLen, eyeSy - dLen, outerColor, 2);
	DrawCircle(eyeSx, eyeSy, static_cast<int>(6.0f * pop) + 1, outerColor, TRUE);
	DrawCircle(eyeSx, eyeSy, static_cast<int>(3.0f * pop) + 1, coreHot, TRUE);
}

void Stage3DragonCloneMeteorDashAttack::Draw(
	BlockMap* blockMap,
	const VECTOR& bossPos,
	float facingDir,
	float drawScale,
	int realSpriteHandle,
	float footOffset,
	float enemyHeight) const
{
	if (!IsActive() && mMeteors.empty() && mSparks.empty()) return;

	bool reverse = (facingDir > 0.0f);
	int  dirSign = (facingDir >= 0.0f) ? 1 : -1;

	// =================================================================
	// 1. 地上で上空へ特大火の玉を放つ分身1匹目（Caster Clone）の描画
	// =================================================================
	if (mCasterCloneAlpha > 0)
	{
		float casterFootOffset = 72.0f;
		int   casterHandle     = SelectCasterCloneSpriteHandle(casterFootOffset);
		if (casterHandle != -1)
		{
			int   sx         = static_cast<int>(ConvertWorldToScreenX(mCasterCloneX, blockMap));
			float feetWorldY = mCasterCloneY + (enemyHeight * 0.5f);
			int   sy         = static_cast<int>(ConvertWorldToScreenY(feetWorldY - (casterFootOffset * drawScale), blockMap));
			bool  casterRev  = (mCasterFacingDir > 0.0f);

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, mCasterCloneAlpha);
			DrawRotaGraph(sx, sy, drawScale, 0.0, casterHandle, TRUE, casterRev);

			// 打ち上げ前の口元特大火球チャージ演出
			if (mPhase == Phase::IntroCastGiantFireball && mPhaseTimer < 46)
			{
				float ratio  = static_cast<float>(mPhaseTimer) / 46.0f;
				int   mouthX = static_cast<int>(ConvertWorldToScreenX(mCasterCloneX + mCasterFacingDir * 120.0f, blockMap));
				int   mouthY = static_cast<int>(ConvertWorldToScreenY(mCasterCloneY - 90.0f, blockMap));
				int   r      = static_cast<int>(18.0f + 107.0f * ratio);

				int fTex = mFireTex[(mPhaseTimer / 2) & 3];
				if (fTex != -1)
				{
					double sc = (r * 2.8) / 256.0;
					float rot = mPhaseTimer * 0.12f;
					SetDrawBright(255, 110, 20);
					SetDrawBlendMode(DX_BLENDMODE_ADD, 220);
					DrawRotaGraph(mouthX, mouthY, sc, rot, fTex, TRUE);
					SetDrawBright(255, 220, 95);
					SetDrawBlendMode(DX_BLENDMODE_ADD, 185);
					DrawRotaGraph(mouthX, mouthY, sc * 0.62, -rot, fTex, TRUE);
					SetDrawBright(255, 255, 255);
				}
				else
				{
					SetDrawBlendMode(DX_BLENDMODE_ADD, 220);
					DrawCircle(mouthX, mouthY, r + 14, GetColor(255, 55, 15), TRUE);
					DrawCircle(mouthX, mouthY, r,      GetColor(255, 165, 40), TRUE);
					DrawCircle(mouthX, mouthY, r / 2,  GetColor(255, 250, 195), TRUE);
				}
			}
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	// =================================================================
	// 2. 上空へ打ち上げられる特大火の玉 ＆ 8秒間上から降り注ぐ火の玉（メテオ）の描画
	// =================================================================
	if (mSkyGiantFireball.active)
	{
		int sx = static_cast<int>(ConvertWorldToScreenX(mSkyGiantFireball.x, blockMap));
		int sy = static_cast<int>(ConvertWorldToScreenY(mSkyGiantFireball.y, blockMap));
		int r  = static_cast<int>(mSkyGiantFireball.radius);

		int fIdx = (mPhaseTimer / 2) & 3;
		int fTex = mFireTex[fIdx];
		int fTexTrail = mFireTex[(fIdx + 1) & 3];
		if (fTex != -1)
		{
			double sc = (mSkyGiantFireball.radius * 3.4) / 256.0;
			float rot = 0.0f;
			int ty = static_cast<int>(ConvertWorldToScreenY(mSkyGiantFireball.y - mSkyGiantFireball.vy * 1.6f, blockMap));
			if (fTexTrail != -1)
			{
				SetDrawBright(255, 65, 10);
				SetDrawBlendMode(DX_BLENDMODE_ADD, 160);
				DrawRotaGraph(sx, ty, sc * 0.85, rot, fTexTrail, TRUE);
			}
			SetDrawBright(255, 110, 20);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 225);
			DrawRotaGraph(sx, sy, sc, rot, fTex, TRUE);
			SetDrawBright(255, 220, 95);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 190);
			DrawRotaGraph(sx, sy, sc * 0.62, rot, fTex, TRUE);
			SetDrawBright(255, 255, 255);
		}
		else
		{
			SetDrawBlendMode(DX_BLENDMODE_ADD, 235);
			DrawCircle(sx, sy, r + 20, GetColor(255, 55, 15), TRUE);
			DrawCircle(sx, sy, r,      GetColor(255, 165, 35), TRUE);
			DrawCircle(sx, sy, static_cast<int>(r * 0.55f), GetColor(255, 250, 200), TRUE);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// 8秒間、天井一帯が赤熱してメテオ雲になっている演出
	if (mPhase == Phase::MeteorAndDashLoop)
	{
		// 【Stage3.png 5724x1692拡大版 対応】天井赤熱範囲をボス部屋(3016..5690)に更新
		int leftSx  = static_cast<int>(ConvertWorldToScreenX(3016.0f, blockMap));
		int rightSx = static_cast<int>(ConvertWorldToScreenX(5690.0f, blockMap));
		int ceilSy  = static_cast<int>(ConvertWorldToScreenY(ROOM_CEILING_Y, blockMap));

		SetDrawBlendMode(DX_BLENDMODE_ADD, 65);
		DrawBox(leftSx, ceilSy - 10, rightSx, ceilSy + 36, GetColor(255, 70, 20), TRUE);
		DrawBox(leftSx, ceilSy,      rightSx, ceilSy + 14, GetColor(255, 190, 70), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// 上から落ちてくる火の玉（メテオ）描画
	SetDrawBlendMode(DX_BLENDMODE_ADD, 225);
	for (size_t i = 0; i < mMeteors.size(); ++i)
	{
		const FallingMeteorFireball& m = mMeteors[i];
		if (!m.active) continue;

		int sx = static_cast<int>(ConvertWorldToScreenX(m.x, blockMap));
		int sy = static_cast<int>(ConvertWorldToScreenY(m.y, blockMap));
		int r  = static_cast<int>(m.radius);

		if (m.warnTimer > 0)
		{
			// 落下直前の天井発光マーカー
			int warnR = 12 + (16 - m.warnTimer) * 2;
			DrawCircle(sx, sy, warnR, GetColor(255, 140, 40), FALSE, 3);
			DrawLine(sx, sy, sx, sy + 110, GetColor(255, 90, 30), 2);
		}
		else
		{
			// 落下火の玉本体と尾（マグマ吸い込みと同じ2D Effekseer Fireテクスチャ演出）
			int fIdx = ((mPhaseTimer / 2) + static_cast<int>(i)) & 3;
			int fTex = mFireTex[fIdx];
			int fTexTrail = mFireTex[(fIdx + 1) & 3];
			if (fTex != -1)
			{
				float rot = std::atan2(m.vy, m.vx) + 1.5707963f;
				double sc = (m.radius * 3.0) / 256.0;
				int tx = static_cast<int>(ConvertWorldToScreenX(m.x - m.vx * 1.8f, blockMap));
				int ty = static_cast<int>(ConvertWorldToScreenY(m.y - m.vy * 1.8f, blockMap));
				if (fTexTrail != -1)
				{
					SetDrawBright(255, 65, 10);
					SetDrawBlendMode(DX_BLENDMODE_ADD, 155);
					DrawRotaGraph(tx, ty, sc * 0.85, rot, fTexTrail, TRUE);
				}
				SetDrawBright(255, 110, 20);
				SetDrawBlendMode(DX_BLENDMODE_ADD, 215);
				DrawRotaGraph(sx, sy, sc, rot, fTex, TRUE);
				SetDrawBright(255, 220, 95);
				SetDrawBlendMode(DX_BLENDMODE_ADD, 180);
				DrawRotaGraph(sx, sy, sc * 0.62, rot, fTex, TRUE);
				SetDrawBright(255, 255, 255);
			}
			else
			{
				DrawCircle(sx, sy - 24, static_cast<int>(r * 0.72f), GetColor(220, 50, 15), TRUE);
				DrawCircle(sx, sy - 12, static_cast<int>(r * 0.88f), GetColor(255, 100, 20), TRUE);
				DrawCircle(sx, sy, r + 6, GetColor(255, 65, 15), TRUE);
				DrawCircle(sx, sy, r,     GetColor(255, 175, 45), TRUE);
				DrawCircle(sx, sy, r / 2, GetColor(255, 250, 195), TRUE);
			}
		}
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// =================================================================
	// 3. 2秒ごとの「本体＋もう1匹の分身」上・真ん中・下 疾走突進の描画
	// =================================================================
	if (mPhase == Phase::MeteorAndDashLoop)
	{
		bool isChargingSubPhase = (mCycleTimer > 18 && mCycleTimer <= 64);
		bool isDashingSubPhase  = (mCycleTimer > 64 && mCycleTimer <= 108);

		float cloneFootOffset = 79.0f;
		int   cloneHandle     = SelectDashCloneSpriteHandle(cloneFootOffset);

		// (B) 突進中：間引き残像トレイル（2枚に削減してさらに軽量化）
		if (isDashingSubPhase)
		{

			for (int tIdx = 4; tIdx >= 2; tIdx -= 2)
			{
				if (!mTrailPoints[tIdx].active || mTrailPoints[tIdx].alpha <= 0) continue;

				int trailSx = static_cast<int>(ConvertWorldToScreenX(mTrailPoints[tIdx].x, blockMap));

				// 本体の残像
				if (realSpriteHandle != -1)
				{
					float realFeetY = GetSlotY(mRealDragonSlot) + (enemyHeight * 0.5f);
					int   realSy    = static_cast<int>(ConvertWorldToScreenY(realFeetY - (footOffset * drawScale), blockMap));

					SetDrawBlendMode(DX_BLENDMODE_ADD, mTrailPoints[tIdx].alpha);
					SetDrawBright(255, 95, 50);
					DrawRotaGraph(trailSx, realSy, drawScale, 0.0, realSpriteHandle, TRUE, reverse);
				}

				// もう1匹の突進分身の残像（フェード連動）
				if (cloneHandle != -1 && mDashCloneAlpha > 0)
				{
					int cTrailAlpha = (mTrailPoints[tIdx].alpha * mDashCloneAlpha) / 255;
					if (cTrailAlpha > 0)
					{
						float cFeetY = GetSlotY(mDashCloneSlot) + (enemyHeight * 0.5f);
						int   cSy    = static_cast<int>(ConvertWorldToScreenY(cFeetY - (cloneFootOffset * drawScale), blockMap));

						SetDrawBlendMode(DX_BLENDMODE_ADD, cTrailAlpha);
						SetDrawBright(70, 255, 200);
						DrawRotaGraph(trailSx, cSy, drawScale, 0.0, cloneHandle, TRUE, reverse);
					}
				}
			}
			SetDrawBright(255, 255, 255);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}

		// (C) もう1匹の突進分身スプライト＆斬撃エフェクト＆エメラルド十字眼光の描画
		if ((isChargingSubPhase || isDashingSubPhase) && mDashCloneAlpha > 0 && cloneHandle != -1)
		{
			float cloneX = mCurrentDashX;
			float cloneY = GetSlotY(mDashCloneSlot);
			if (isChargingSubPhase)
			{
				cloneY += std::sin((mCycleTimer + 12) * 0.3f) * 5.0f;
			}

			int   sx         = static_cast<int>(ConvertWorldToScreenX(cloneX, blockMap));
			float feetWorldY = cloneY + (enemyHeight * 0.5f);
			int   sy         = static_cast<int>(ConvertWorldToScreenY(feetWorldY - (cloneFootOffset * drawScale), blockMap));

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, mDashCloneAlpha);
			DrawRotaGraph(sx, sy, drawScale, 0.0, cloneHandle, TRUE, reverse);

			if (mCloneEyeGlintTimer > 0)
			{
				float cloneEyeX = 0.0f, cloneEyeY = 0.0f;
				GetEyeWorldPosition(
					cloneX,
					cloneY,
					facingDir,
					drawScale,
					cloneFootOffset,
					enemyHeight,
					(mAnimType == ANIM_ATTACK),
					mAnimFrame,
					cloneEyeX,
					cloneEyeY
				);
				int cEyeSx = static_cast<int>(ConvertWorldToScreenX(cloneEyeX, blockMap));
				int cEyeSy = static_cast<int>(ConvertWorldToScreenY(cloneEyeY, blockMap));
				DrawEyeCrossGlint(cEyeSx, cEyeSy, mDashCloneAlpha, true);
			}
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	// =================================================================
	// 4. 高速スピードライン・火花の描画
	// =================================================================
	for (size_t i = 0; i < mSpeedLines.size(); ++i)
	{
		const SpeedLine& sl = mSpeedLines[i];
		int alpha = (sl.life * 215) / sl.maxLife;
		if (sl.isClone && mDashCloneAlpha < 255 && mCycleTimer > 64)
		{
			alpha = (alpha * mDashCloneAlpha) / 255;
		}
		if (alpha <= 0) continue;

		int x1 = static_cast<int>(ConvertWorldToScreenX(sl.x, blockMap));
		int y1 = static_cast<int>(ConvertWorldToScreenY(sl.y, blockMap));
		int x2 = static_cast<int>(ConvertWorldToScreenX(sl.x - dirSign * sl.length, blockMap));

		SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
		DrawLine(x1, y1, x2, y1, sl.color, sl.thickness);
		DrawLine(x1, y1, (x1 + x2) / 2, y1, GetColor(255, 255, 240), 1);
	}

	for (size_t i = 0; i < mSparks.size(); ++i)
	{
		const FireSpark& sp = mSparks[i];
		int alpha = (sp.life * 230) / sp.maxLife;
		if (alpha <= 0) continue;

		int x1 = static_cast<int>(ConvertWorldToScreenX(sp.x, blockMap));
		int y1 = static_cast<int>(ConvertWorldToScreenY(sp.y, blockMap));
		float vLen = std::sqrt(sp.vx * sp.vx + sp.vy * sp.vy);
		if (vLen < 0.1f) vLen = 1.0f;
		int x2 = static_cast<int>(ConvertWorldToScreenX(sp.x - (sp.vx / vLen) * sp.length, blockMap));
		int y2 = static_cast<int>(ConvertWorldToScreenY(sp.y - (sp.vy / vLen) * sp.length, blockMap));

		SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
		DrawLine(x1, y1, x2, y2, sp.color, 3);
		DrawCircle(x1, y1, 3, GetColor(255, 255, 230), TRUE);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

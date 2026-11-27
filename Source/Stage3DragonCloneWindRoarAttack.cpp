#include "Stage3DragonCloneWindRoarAttack.h"
#include "Effekseer2DManager.h"
#include "BlockMap.h"
#include "Player.h"
#include "GameConstants.h"
#include <cmath>
#include <cstdio>

namespace
{
	constexpr float kPi = 3.1415926535f;
}

Stage3DragonCloneWindRoarAttack::Stage3DragonCloneWindRoarAttack()
	: mPhase(Phase::Inactive)
	, mPhaseTimer(0)
	, mAnimType(ANIM_IDLE)
	, mAnimFrame(0)
	, mbRealOnLeftEdge(false)
	, mRealFacingDir(-1.0f)
	, mStartBossX(RIGHT_LEDGE_STAND_X)
	, mStartBossY(RIGHT_LEDGE_FLOOR_Y - BOSS_HALF_HEIGHT)
	, mRealTargetX(RIGHT_LEDGE_STAND_X)
	, mRealTargetY(REAL_ROAR_Y)
	, mReturnTargetX(RIGHT_LEDGE_STAND_X)
	, mReturnTargetY(RIGHT_LEDGE_FLOOR_Y - BOSS_HALF_HEIGHT)
	, mClone1X(STAGE_CENTER_X - CLONE_CENTER_OFFSET_X)
	, mClone1Y(CLONE_TOP_Y)
	, mClone1FacingDir(-1.0f)
	, mClone2X(STAGE_CENTER_X + CLONE_CENTER_OFFSET_X)
	, mClone2Y(CLONE_TOP_Y)
	, mClone2FacingDir(1.0f)
	, mCloneAnimType(ANIM_FLY)
	, mCloneAnimFrame(0)
	, mCloneAlpha(0)
	, mCloneEyeGlintTimer(0)
	, mCloneMagicCircleEfk{ -1, -1 }
	, mClone1NextAngle(kPi)
	, mClone2NextAngle(0.0f)
	, mbHasKnockedBackInCycle(false)
	, mPlayerKnockbackVx(0.0f)
	, mKnockbackTimer(0)
{
}

Stage3DragonCloneWindRoarAttack::~Stage3DragonCloneWindRoarAttack()
{
	ReleaseCloneAnimations();
}

void Stage3DragonCloneWindRoarAttack::Initialize()
{
	LoadCloneAnimations();
	Reset(nullptr);
}

void Stage3DragonCloneWindRoarAttack::Reset(Player* player)
{
	if (mKnockbackTimer > 0 && player != nullptr)
	{
		player->SetInputDisabled(false);
	}
	mPhase                  = Phase::Inactive;
	mPhaseTimer             = 0;
	mAnimType               = ANIM_IDLE;
	mAnimFrame              = 0;
	mCloneAnimType          = ANIM_FLY;
	mCloneAnimFrame         = 0;
	mCloneAlpha             = 0;
	mCloneEyeGlintTimer     = 0;
	for (int i = 0; i < 2; ++i)
	{
		if (mCloneMagicCircleEfk[i] != -1)
		{
			Effekseer2DManager::Stop(mCloneMagicCircleEfk[i]);
			mCloneMagicCircleEfk[i] = -1;
		}
	}
	mbHasKnockedBackInCycle = false;
	mPlayerKnockbackVx      = 0.0f;
	mKnockbackTimer         = 0;

	for (size_t i = 0; i < mWindBlades.size(); ++i)
	{
		for (int h : mWindBlades[i].efkHandles)
		{
			if (h != -1) Effekseer2DManager::Stop(h);
		}
		mWindBlades[i].efkHandles.clear();
	}
	mWindBlades.clear();
	mWindLaunchRings.clear();
	mWindSparks.clear();
	mRoarShockRings.clear();
	mRoarWindLines.clear();
	mRoarChargeParticles.clear();
}

void Stage3DragonCloneWindRoarAttack::ReleaseCloneAnimations()
{
	for (int h : mCloneIdleAnim)      if (h != -1) DeleteGraph(h);
	for (int h : mCloneFlyAnim)       if (h != -1) DeleteGraph(h);
	for (int h : mCloneAirAttackAnim) if (h != -1) DeleteGraph(h);
	mCloneIdleAnim.clear();
	mCloneFlyAnim.clear();
	mCloneAirAttackAnim.clear();
	mCloneEyeOffsetsByHandle.clear();
}

int Stage3DragonCloneWindRoarAttack::LoadGraphWithRecoloredEyes(const char* filePath)
{
	int softImg = LoadSoftImage(filePath);
	if (softImg == -1)
	{
		return LoadGraph(filePath);
	}

	int w = 0, h = 0;
	GetSoftImageSize(softImg, &w, &h);

	std::vector<std::pair<int, int>> eyePixels;
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			int r = 0, g = 0, b = 0, a = 0;
			GetPixelSoftImage(softImg, x, y, &r, &g, &b, &a);
			if (a > 128 && r >= 170 && g <= 65 && b <= 65 && (r - g) >= 110)
			{
				eyePixels.push_back(std::make_pair(x, y));
				DrawPixelSoftImage(softImg, x, y, 30, 255, 200, a);
			}
		}
	}

	for (size_t i = 0; i < eyePixels.size(); ++i)
	{
		int ex = eyePixels[i].first;
		int ey = eyePixels[i].second;
		for (int dy = -2; dy <= 2; ++dy)
		{
			for (int dx = -2; dx <= 2; ++dx)
			{
				int nx = ex + dx;
				int ny = ey + dy;
				if (nx >= 0 && nx < w && ny >= 0 && ny < h)
				{
					int nr = 0, ng = 0, nb = 0, na = 0;
					GetPixelSoftImage(softImg, nx, ny, &nr, &ng, &nb, &na);
					if (na > 80 && ng < 200)
					{
						int boostG = (ng + 180 > 255) ? 255 : (ng + 180);
						int boostB = (nb + 150 > 255) ? 255 : (nb + 150);
						DrawPixelSoftImage(softImg, nx, ny, 20, boostG, boostB, na);
					}
				}
			}
		}
	}

	int graphHandle = CreateGraphFromSoftImage(softImg);
	DeleteSoftImage(softImg);
	if (graphHandle != -1 && !eyePixels.empty())
	{
		double sumX = 0.0;
		double sumY = 0.0;
		for (size_t i = 0; i < eyePixels.size(); ++i)
		{
			sumX += static_cast<double>(eyePixels[i].first);
			sumY += static_cast<double>(eyePixels[i].second);
		}
		float offX = static_cast<float>(sumX / eyePixels.size()) - (static_cast<float>(w) * 0.5f);
		float offY = static_cast<float>(sumY / eyePixels.size()) - (static_cast<float>(h) * 0.5f);
		mCloneEyeOffsetsByHandle[graphHandle] = std::make_pair(offX, offY);
	}
	return graphHandle;
}

void Stage3DragonCloneWindRoarAttack::LoadCloneAnimations()
{
	ReleaseCloneAnimations();
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
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_airborne/sprite_airAttack%d.png", i);
		int h = LoadGraphWithRecoloredEyes(path);
		if (h != -1) mCloneAirAttackAnim.push_back(h);
	}
}

int Stage3DragonCloneWindRoarAttack::SelectCloneSpriteHandle(float& outFootOffset) const
{
	outFootOffset = 57.0f;
	if (mCloneAnimType == ANIM_AIR_ATTACK && !mCloneAirAttackAnim.empty())
	{
		outFootOffset = 68.0f;
		return mCloneAirAttackAnim[mCloneAnimFrame % mCloneAirAttackAnim.size()];
	}
	if (!mCloneFlyAnim.empty())
	{
		outFootOffset = 57.0f;
		return mCloneFlyAnim[mCloneAnimFrame % mCloneFlyAnim.size()];
	}
	if (!mCloneIdleAnim.empty())
	{
		outFootOffset = 79.0f;
		return mCloneIdleAnim[mCloneAnimFrame % mCloneIdleAnim.size()];
	}
	return -1;
}

float Stage3DragonCloneWindRoarAttack::PickRandomAngleIn120DegCone(float cloneFacingDir) const
{
	// 分身が向いている方向（右向き: 0度、左向き: 180度）を中心に角度120度（-60度 ～ +60度）からランダム選択
	float baseAngle = (cloneFacingDir > 0.0f) ? 0.0f : kPi;
	// -60.0f ～ +60.0f 度（0.1度刻みで1201通り）
	float offsetDeg = (static_cast<float>(GetRand(1200)) - 600.0f) * 0.1f;
	float offsetRad = offsetDeg * (kPi / 180.0f);
	return baseAngle + offsetRad;
}

void Stage3DragonCloneWindRoarAttack::StartAttack(VECTOR& bossPos, float& facingDir, Player* player)
{
	Reset(player);

	// 分身2体はステージ真ん中の上（STAGE_CENTER_X ± 210, Y = 465）に左右を向いた状態で配置！
	mClone1X         = STAGE_CENTER_X - CLONE_CENTER_OFFSET_X;
	mClone1Y         = CLONE_TOP_Y;
	mClone1FacingDir = -1.0f; // 左側分身は左向き

	mClone2X         = STAGE_CENTER_X + CLONE_CENTER_OFFSET_X;
	mClone2Y         = CLONE_TOP_Y;
	mClone2FacingDir = 1.0f;  // 右側分身は右向き

	mCloneAlpha      = 0;
	mCloneAnimType   = ANIM_FLY;
	mCloneAnimFrame  = 0;

	// 本体は左端または右端の上空（REAL_ROAR_Y = 660）へ移動し、ステージ内側へ向かって咆哮を放つ
	mbRealOnLeftEdge = (GetRand(1) == 0);
	if (mbRealOnLeftEdge)
	{
		mRealTargetX   = LEFT_LEDGE_STAND_X;
		mRealTargetY   = REAL_ROAR_Y;
		mRealFacingDir = 1.0f;  // 左端から右向きに咆哮
		mReturnTargetX = LEFT_LEDGE_STAND_X;
		mReturnTargetY = LEFT_LEDGE_FLOOR_Y - BOSS_HALF_HEIGHT;
	}
	else
	{
		mRealTargetX   = RIGHT_LEDGE_STAND_X;
		mRealTargetY   = REAL_ROAR_Y;
		mRealFacingDir = -1.0f; // 右端から左向きに咆哮
		mReturnTargetX = RIGHT_LEDGE_STAND_X;
		mReturnTargetY = RIGHT_LEDGE_FLOOR_Y - BOSS_HALF_HEIGHT;
	}

	mStartBossX = bossPos.x;
	mStartBossY = bossPos.y;
	facingDir   = (mRealTargetX >= bossPos.x) ? 1.0f : -1.0f;

	mClone1NextAngle = PickRandomAngleIn120DegCone(mClone1FacingDir);
	mClone2NextAngle = PickRandomAngleIn120DegCone(mClone2FacingDir);

	mPhase      = Phase::SetupPositions;
	mPhaseTimer = 0;
	mAnimType   = ANIM_TAKEOFF;
	mAnimFrame  = 0;
}

void Stage3DragonCloneWindRoarAttack::SpawnWindBlade(float startX, float startY, float angle)
{
	WindBladeProjectile wb;
	wb.active  = true;
	wb.x       = startX;
	wb.y       = startY;
	float spd  = 16.5f;
	wb.vx      = std::cos(angle) * spd;
	wb.vy      = std::sin(angle) * spd;
	wb.angle   = angle;
	wb.radius      = 44.0f;
	wb.life        = 0;
	wb.maxLife     = 210;
	wb.bounceCount = 0;
	wb.fadeTimer   = 0;
	float leadX = startX + std::cos(angle) * 45.0f;
	float leadY = startY + std::sin(angle) * 45.0f;
	int h = Effekseer2DManager::Play(
		Effect2DId::Stage3_SlashWind,
		leadX,
		leadY,
		1.50f,
		1.65f,
		angle,
		85, 255, 220, 255
	);
	if (h != -1) wb.efkHandles.push_back(h);
	mWindBlades.push_back(wb);

	// 発射マズル魔法陣リング（軽量・高速）

	// 発射マズルソニックリング
	WindLaunchRing lr;
	lr.x       = startX;
	lr.y       = startY;
	lr.angle   = angle;
	lr.radius  = 18.0f;
	lr.life    = 0;
	lr.maxLife = 18;
	mWindLaunchRings.push_back(lr);

	SpawnWindSlashSparks(startX, startY, angle, 10);
}

void Stage3DragonCloneWindRoarAttack::SpawnWindSlashSparks(float x, float y, float baseAngle, int count)
{
	for (int i = 0; i < count; ++i)
	{
		WindSpark sp;
		sp.x       = x + static_cast<float>(GetRand(28) - 14);
		sp.y       = y + static_cast<float>(GetRand(28) - 14);
		float a    = baseAngle + (static_cast<float>(GetRand(140) - 70) * 0.01f);
		float spd  = 6.0f + static_cast<float>(GetRand(70)) * 0.16f;
		sp.vx      = std::cos(a) * spd;
		sp.vy      = std::sin(a) * spd;
		sp.length  = 18.0f + static_cast<float>(GetRand(22));
		sp.life    = 0;
		sp.maxLife = 14 + GetRand(10);
		sp.color   = (i % 3 == 0) ? GetColor(230, 255, 250) : ((i % 2 == 0) ? GetColor(60, 255, 200) : GetColor(120, 255, 160));
		mWindSparks.push_back(sp);
	}
}

void Stage3DragonCloneWindRoarAttack::UpdateDragon(
	VECTOR& bossPos,
	float& facingDir,
	bool& isBossGrounded,
	int& outAnimType,
	int& outAnimFrame,
	Player* player,
	BlockMap* blockMap)
{
	if (mPhase == Phase::Inactive || mPhase == Phase::Finished)
	{
		return;
	}

	mPhaseTimer++;
	isBossGrounded = false;

	switch (mPhase)
	{
	case Phase::SetupPositions:
	{
		float t = static_cast<float>(mPhaseTimer) / static_cast<float>(SETUP_DURATION_FRAMES);
		if (t > 1.0f) t = 1.0f;
		float ease = t * t * (3.0f - 2.0f * t);

		bossPos.x = mStartBossX + (mRealTargetX - mStartBossX) * ease;
		bossPos.y = mStartBossY + (mRealTargetY - mStartBossY) * ease;
		facingDir = mRealFacingDir;

		mCloneAlpha     = static_cast<int>(ease * 240.0f);
		mCloneAnimType  = ANIM_FLY;
		mCloneAnimFrame = (mPhaseTimer / 5) % 7;

		mAnimType  = (t < 0.35f) ? ANIM_TAKEOFF : ANIM_FLY;
		mAnimFrame = (mPhaseTimer / 5) % 7;

		if (mPhaseTimer >= SETUP_DURATION_FRAMES)
		{
			bossPos.x   = mRealTargetX;
			bossPos.y   = mRealTargetY;
			facingDir   = mRealFacingDir;
			mCloneAlpha = 240;
			mPhase      = Phase::WindBladeAndRoarLoop;
			mPhaseTimer = 0;

			mClone1NextAngle = PickRandomAngleIn120DegCone(mClone1FacingDir);
			mClone2NextAngle = PickRandomAngleIn120DegCone(mClone2FacingDir);
			mbHasKnockedBackInCycle = false;
		}
		break;
	}

	case Phase::WindBladeAndRoarLoop:
	{
		// 本体は端の上空でホバリングしながら3秒に1回溜め大咆哮
		facingDir = mRealFacingDir;
		bossPos.x = mRealTargetX;
		bossPos.y = mRealTargetY + std::sin(mPhaseTimer * 0.14f) * 14.0f;

		// 分身2体は中央上空で左右を向いてホバリング
		mClone1Y = CLONE_TOP_Y + std::sin(mPhaseTimer * 0.11f) * 10.0f;
		mClone2Y = CLONE_TOP_Y + std::cos(mPhaseTimer * 0.11f) * 10.0f;

		// =================================================================
		// A. 分身2体の風の刃：1.5秒（90フレーム）に1回発射！
		//    真ん中の上で左右を向き、それぞれ向いている方向の120度（±60度）からランダム発射
		// =================================================================
		int bladeCycle = mPhaseTimer % WIND_BLADE_INTERVAL; // 0 .. 89

		if (bladeCycle == 42)
		{
			mClone1NextAngle = PickRandomAngleIn120DegCone(mClone1FacingDir);
			mClone2NextAngle = PickRandomAngleIn120DegCone(mClone2FacingDir);
		}
		if (bladeCycle == 54)
		{
			mCloneEyeGlintTimer = 16;
		}
		else if (mCloneEyeGlintTimer > 0)
		{
			mCloneEyeGlintTimer--;
		}

		// 発射タイミング（bladeCycle == 68）にのみ魔法陣から2D Effekseerの風衝撃波を放つ
		if (bladeCycle == 68)
		{
			float cloneXs[2]   = { mClone1X,         mClone2X         };
			float cloneYs[2]   = { mClone1Y,         mClone2Y         };
			float cloneDirs[2] = { mClone1FacingDir, mClone2FacingDir };
			for (int c = 0; c < 2; ++c)
			{
				float mx = cloneXs[c] + cloneDirs[c] * 155.0f;
				float my = cloneYs[c] - 58.0f;
				if (mCloneMagicCircleEfk[c] != -1)
				{
					Effekseer2DManager::Stop(mCloneMagicCircleEfk[c]);
					mCloneMagicCircleEfk[c] = -1;
				}
				mCloneMagicCircleEfk[c] = Effekseer2DManager::Play(
					Effect2DId::Stage3_RoarWave,
					mx,
					my,
					1.45f,
					1.15f,
					0.0f,
					35, 255, 195, 240
				);
			}
		}

		if (bladeCycle >= 48 && bladeCycle < 68)
		{
			mCloneAnimType  = ANIM_AIR_ATTACK;
			mCloneAnimFrame = ((bladeCycle - 48) * 3) / 20; // 0..2
		}
		else if (bladeCycle == 68)
		{
			// 1.5秒に1回、左右の分身が同時に風の刃を発射！
			mCloneAnimType  = ANIM_AIR_ATTACK;
			mCloneAnimFrame = 3;

			float spawn1X = mClone1X + mClone1FacingDir * 155.0f;
			float spawn2X = mClone2X + mClone2FacingDir * 155.0f;
			SpawnWindBlade(spawn1X, mClone1Y - 58.0f, mClone1NextAngle);
			SpawnWindBlade(spawn2X, mClone2Y - 58.0f, mClone2NextAngle);
		}
		else if (bladeCycle > 68 && bladeCycle < 85)
		{
			mCloneAnimType  = ANIM_AIR_ATTACK;
			mCloneAnimFrame = 4 + ((bladeCycle - 69) * 3) / 16;
			if (mCloneAnimFrame > 6) mCloneAnimFrame = 6;
		}
		else
		{
			mCloneAnimType  = ANIM_FLY;
			mCloneAnimFrame = (mPhaseTimer / 5) % 7;
		}

		// =================================================================
		// B. 本体の咆哮：3秒（180フレーム）に1回、力を溜めて大咆哮！
		//    咆哮単体攻撃（Stage3DragonRoarAttack）と同様のド迫力リング＆暴風＆危険予兆演出
		// =================================================================
		int roarCycle = mPhaseTimer % ROAR_INTERVAL; // 0 .. 179

		if (roarCycle == 0)
		{
			mbHasKnockedBackInCycle = false;
		}

		if (roarCycle < ROAR_CHARGE_FRAMES)
		{
			// 【咆哮の溜め（0 .. 123F）】
			mAnimType = ANIM_ROAR;
			if (roarCycle < 30)       mAnimFrame = 0;
			else if (roarCycle < 62)  mAnimFrame = 1;
			else if (roarCycle < 94)  mAnimFrame = 2;
			else                      mAnimFrame = 3;

			float mouthX = bossPos.x + mRealFacingDir * 130.0f;
			float mouthY = bossPos.y - 70.0f;

			// 口元へ2D Effekseer溜め放電（魔法陣・赤い球は廃止）
			if (roarCycle == 45 || roarCycle == 95)
			{
				Effekseer2DManager::Play(
					Effect2DId::Stage3_Thunder,
					mouthX + mRealFacingDir * 15.0f,
					mouthY,
					(roarCycle == 45) ? 0.65f : 0.95f,
					1.65f,
					(mRealFacingDir >= 0.0f) ? 0.0f : 3.14159265f,
					255, 225, 95, 235
				);
			}
		}
		else if (roarCycle < ROAR_CHARGE_FRAMES + ROAR_BLAST_FRAMES)
		{
			// 【咆哮炸裂！（124 .. 171F：3秒に1回）】
			int blastTick = roarCycle - ROAR_CHARGE_FRAMES;
			mAnimType  = ANIM_ROAR;
			mAnimFrame = ((blastTick / 5) % 2 == 0) ? 4 : 5;

			if (blastTick == 0)
			{
				mRoarChargeParticles.clear();
			}

			// 画面を激しく揺らす
			if (blockMap != nullptr)
			{
				blockMap->mShakeX = GetRand(18) - 9;
				blockMap->mShakeY = GetRand(16) - 8;
			}

			float mouthX = bossPos.x + mRealFacingDir * 145.0f;
			float mouthY = bossPos.y - 75.0f;
			float roarRotZ = (mRealFacingDir >= 0.0f) ? 0.0f : 3.14159265f;

			// 2D Effekseer 前方放射衝撃波（魔法陣なし・口の前方から方向付き衝撃波）
			if (blastTick == 0)
			{
				Effekseer2DManager::Play(
					Effect2DId::Stage3_FlameBurst,
					mouthX + mRealFacingDir * 185.0f,
					mouthY + 10.0f,
					1.95f,
					1.55f,
					roarRotZ,
					255, 195, 80, 245
				);
				Effekseer2DManager::Play(
					Effect2DId::Stage3_SlashWind,
					mouthX + mRealFacingDir * 250.0f,
					mouthY + 10.0f,
					1.85f,
					1.55f,
					roarRotZ,
					255, 225, 110, 235
				);
			}
			else if (blastTick == 22)
			{
				Effekseer2DManager::Play(
					Effect2DId::Stage3_Thunder,
					mouthX + mRealFacingDir * 210.0f,
					mouthY + 10.0f,
					1.45f,
					1.75f,
					roarRotZ,
					255, 230, 105, 235
				);
			}

			// 空中から斜め下へ吹き荒れる暴風ライン（3フレームごとに間引き生成で軽量化）
			if (blastTick % 3 == 0)
			{
				RoarWindLine wl;
				wl.x       = mbRealOnLeftEdge ? (LEFT_SAFE_MAX_X - 40.0f) : (RIGHT_SAFE_MIN_X + 40.0f);
				wl.y       = 260.0f + static_cast<float>(GetRand(880));
				wl.vx      = mRealFacingDir * (34.0f + static_cast<float>(GetRand(18)));
				wl.vy      = 4.5f + static_cast<float>(GetRand(6));
				wl.length  = 120.0f + static_cast<float>(GetRand(100));
				wl.life    = 36;
				wl.maxLife = 36;
				mRoarWindLines.push_back(wl);
			}

			// 危険範囲にいるプレイヤーを大咆哮で吹き飛ばす（Stage3DragonRoarAttackと同じ挙動）
			if (!mbHasKnockedBackInCycle && player != nullptr && !player->GetIsDead())
			{
				VECTOR pPos = player->GetPosition();
				bool inDangerZone = mbRealOnLeftEdge
					? (pPos.x > LEFT_SAFE_MAX_X)
					: (pPos.x < RIGHT_SAFE_MIN_X);

				if (inDangerZone)
				{
					mbHasKnockedBackInCycle = true;
					mPlayerKnockbackVx      = mRealFacingDir * 56.0f;
					mKnockbackTimer         = 55;
					player->SetInputDisabled(true);
					player->SetIsGrounded(false);
					player->SetVelocityY(-10.0f);
					player->SetFacingRight(mRealFacingDir < 0.0f);
				}
			}
		}
		else
		{
			// 咆哮後の余韻（172 .. 179F）
			mAnimType  = ANIM_ROAR;
			mAnimFrame = 6;
			if (blockMap != nullptr)
			{
				blockMap->mShakeX = 0;
				blockMap->mShakeY = 0;
			}
		}

		// 10秒（600フレーム）経過したら終了フェーズへ
		if (mPhaseTimer >= ATTACK_DURATION_FRAMES)
		{
			for (int i = 0; i < 2; ++i)
			{
				if (mCloneMagicCircleEfk[i] != -1)
				{
					Effekseer2DManager::Stop(mCloneMagicCircleEfk[i]);
					mCloneMagicCircleEfk[i] = -1;
				}
			}
			if (blockMap != nullptr)
			{
				blockMap->mShakeX = 0;
				blockMap->mShakeY = 0;
			}
			mRoarChargeParticles.clear();
			mStartBossX = bossPos.x;
			mStartBossY = bossPos.y;
			mPhase      = Phase::ReturnToLedge;
			mPhaseTimer = 0;
		}
		break;
	}

	case Phase::ReturnToLedge:
	{
		const int RETURN_FRAMES = 42;
		float t = static_cast<float>(mPhaseTimer) / static_cast<float>(RETURN_FRAMES);
		if (t > 1.0f) t = 1.0f;
		float ease = t * t * (3.0f - 2.0f * t);

		bossPos.x   = mStartBossX + (mReturnTargetX - mStartBossX) * ease;
		bossPos.y   = mStartBossY + (mReturnTargetY - mStartBossY) * ease;
		facingDir   = mRealFacingDir;
		mCloneAlpha = static_cast<int>((1.0f - ease) * 240.0f);

		mAnimType  = ANIM_LANDING;
		mAnimFrame = static_cast<int>(t * 6.0f);

		if (mPhaseTimer >= RETURN_FRAMES)
		{
			bossPos.x      = mReturnTargetX;
			bossPos.y      = mReturnTargetY;
			isBossGrounded = true;
			mCloneAlpha    = 0;
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

void Stage3DragonCloneWindRoarAttack::UpdatePlayerKnockback(Player* player, BlockMap* blockMap)
{
	if (player == nullptr || blockMap == nullptr) return;
	if (player->GetIsDead())
	{
		if (mKnockbackTimer > 0)
		{
			player->SetInputDisabled(false);
		}
		mKnockbackTimer    = 0;
		mPlayerKnockbackVx = 0.0f;
		return;
	}

	if (mKnockbackTimer > 0)
	{
		mKnockbackTimer--;
		if (mKnockbackTimer == 15 || mKnockbackTimer == 0)
		{
			player->SetInputDisabled(false);
		}
	}

	if (std::fabs(mPlayerKnockbackVx) > 0.35f)
	{
		VECTOR pPos  = player->GetPosition();
		float pHalfW = PlayerConstants::PlayerCollisionWidth * 0.5f;
		float pHalfH = PlayerConstants::PlayerCollisionHeight * 0.5f;

		int steps   = static_cast<int>(std::fabs(mPlayerKnockbackVx)) + 1;
		float stepX = mPlayerKnockbackVx / static_cast<float>(steps);

		for (int s = 0; s < steps; ++s)
		{
			float nextX  = pPos.x + stepX;
			float checkX = (stepX > 0.0f) ? (nextX + pHalfW) : (nextX - pHalfW);
			bool hitWall = false;
			for (float cy = pPos.y - pHalfH + 20.0f; cy <= pPos.y + pHalfH - 20.0f; cy += 16.0f)
			{
				BlockMap::CollisionType ct = blockMap->GetCollisionType(static_cast<int>(checkX), static_cast<int>(cy));
				if (ct == BlockMap::CollisionType::Block || ct == BlockMap::CollisionType::BreakableBlock)
				{
					hitWall = true;
					break;
				}
			}
			if (hitWall)
			{
				if (mKnockbackTimer > 10)
				{
					mPlayerKnockbackVx = -mPlayerKnockbackVx * 0.88f;
					stepX              = -stepX * 0.88f;
				}
				else
				{
					mPlayerKnockbackVx = 0.0f;
					break;
				}
			}
			else
			{
				pPos.x = nextX;
			}
		}

		player->SetPosition(pPos);

		// カメラ追従
		int scrollX    = blockMap->GetScrollX();
		int screenX    = static_cast<int>(pPos.x) - scrollX;
		int maxScrollX = blockMap->GetCurrentWidth() - ScreenSize::ScrrenWidth;
		if (maxScrollX < 0) maxScrollX = 0;

		if (!blockMap->IsCameraLocked())
		{
			if (screenX > MapScrollConstants::ScrollStartRightX)
			{
				scrollX = static_cast<int>(pPos.x) - MapScrollConstants::ScrollStartRightX;
			}
			else if (screenX < MapScrollConstants::ScrollStartLeftX)
			{
				scrollX = static_cast<int>(pPos.x) - MapScrollConstants::ScrollStartLeftX;
			}
			if (scrollX < 0) scrollX = 0;
			if (scrollX > maxScrollX) scrollX = maxScrollX;
			blockMap->SetScrollX(scrollX);
		}

		mPlayerKnockbackVx *= 0.96f;
		if (std::fabs(mPlayerKnockbackVx) <= 0.35f)
		{
			mPlayerKnockbackVx = 0.0f;
			if (mKnockbackTimer > 0)
			{
				mKnockbackTimer = 0;
				player->SetInputDisabled(false);
			}
		}
	}
}

void Stage3DragonCloneWindRoarAttack::UpdateProjectilesAndEffects(
	Player* player,
	BlockMap* blockMap,
	int attackDamage)
{
	UpdatePlayerKnockback(player, blockMap);

	bool playerAlive = (player != nullptr && !player->GetIsDead());
	float pLeft = 0.0f, pTop = 0.0f, pRight = 0.0f, pBottom = 0.0f;
	if (playerAlive)
	{
		VECTOR pPos = player->GetPosition();
		pLeft   = pPos.x - 16.0f;
		pRight  = pPos.x + 16.0f;
		pTop    = pPos.y - 46.0f;
		pBottom = pPos.y + 46.0f;
	}

	auto stopBladeEffects = [](WindBladeProjectile& wb)
	{
		for (int h : wb.efkHandles)
		{
			if (h != -1) Effekseer2DManager::Stop(h);
		}
		wb.efkHandles.clear();
	};

	// 1. 風の刃の飛翔更新（2D Effekseer追従）とプレイヤー当たり判定
	for (auto it = mWindBlades.begin(); it != mWindBlades.end(); )
	{
		if (!it->active)
		{
			stopBladeEffects(*it);
			it = mWindBlades.erase(it);
			continue;
		}

		if (it->fadeTimer > 0)
		{
			it->x += it->vx * 0.25f;
			it->y += it->vy * 0.25f;
			it->fadeTimer--;
			float fadeRatio = static_cast<float>(it->fadeTimer) / 18.0f;
			float leadX = it->x + std::cos(it->angle) * 45.0f;
			float leadY = it->y + std::sin(it->angle) * 45.0f;
			for (int h : it->efkHandles)
			{
				if (h != -1 && Effekseer2DManager::IsPlaying(h))
				{
					Effekseer2DManager::SetTransform(h, leadX, leadY, 1.45f * fadeRatio, it->angle);
				}
			}
			if (it->fadeTimer <= 0)
			{
				stopBladeEffects(*it);
				it = mWindBlades.erase(it);
				continue;
			}
			++it;
			continue;
		}

		it->x += it->vx;
		it->y += it->vy;
		it->life++;

		// 【Stage3.png 5724x1692拡大版 対応】ボス部屋の壁・天井・左右足場・マグマ表面との当たり判定
		constexpr float wallR = 24.0f;
		constexpr float ROOM_LEFT_X   = 3016.0f;
		constexpr float ROOM_RIGHT_X  = 5690.0f;
		constexpr float ROOM_CEIL_Y   = 483.0f;
		constexpr float L_LEDGE_X     = 3667.0f;
		constexpr float L_LEDGE_Y     = 1262.0f;
		constexpr float R_LEDGE_X     = 4970.0f;
		constexpr float R_LEDGE_Y     = 1289.0f;
		constexpr float MAGMA_Y       = 1365.0f;
		bool hitWall = false;
		if (it->x - wallR <= ROOM_LEFT_X && it->vx < 0.0f)
		{
			it->x = ROOM_LEFT_X + wallR; it->vx = -it->vx; hitWall = true;
		}
		else if (it->x + wallR >= ROOM_RIGHT_X && it->vx > 0.0f)
		{
			it->x = ROOM_RIGHT_X - wallR; it->vx = -it->vx; hitWall = true;
		}
		else if (it->y > L_LEDGE_Y + 8.0f && it->x - wallR <= L_LEDGE_X && it->vx < 0.0f)
		{
			it->x = L_LEDGE_X + wallR; it->vx = -it->vx; hitWall = true;
		}
		else if (it->y > R_LEDGE_Y + 8.0f && it->x + wallR >= R_LEDGE_X && it->vx > 0.0f)
		{
			it->x = R_LEDGE_X - wallR; it->vx = -it->vx; hitWall = true;
		}
		if (it->y - wallR <= ROOM_CEIL_Y && it->vy < 0.0f)
		{
			it->y = ROOM_CEIL_Y + wallR; it->vy = -it->vy; hitWall = true;
		}
		else if (it->x < L_LEDGE_X && it->y + wallR >= L_LEDGE_Y && it->vy > 0.0f)
		{
			it->y = L_LEDGE_Y - wallR; it->vy = -it->vy; hitWall = true;
		}
		else if (it->x > R_LEDGE_X && it->y + wallR >= R_LEDGE_Y && it->vy > 0.0f)
		{
			it->y = R_LEDGE_Y - wallR; it->vy = -it->vy; hitWall = true;
		}
		else if (it->x >= L_LEDGE_X && it->x <= R_LEDGE_X && it->y + wallR >= MAGMA_Y && it->vy > 0.0f)
		{
			it->y = MAGMA_Y - wallR; it->vy = -it->vy; hitWall = true;
		}

		if (hitWall)
		{
			if (it->bounceCount == 0)
			{
				it->bounceCount = 1;
				it->angle = std::atan2(it->vy, it->vx);
				stopBladeEffects(*it);
				float leadX = it->x + std::cos(it->angle) * 45.0f;
				float leadY = it->y + std::sin(it->angle) * 45.0f;
				int h = Effekseer2DManager::Play(
					Effect2DId::Stage3_SlashWind,
					leadX,
					leadY,
					1.45f,
					1.65f,
					it->angle,
					85, 255, 220, 255
				);
				if (h != -1) it->efkHandles.push_back(h);
			}
			else
			{
				it->fadeTimer = 18;
				++it;
				continue;
			}
		}

		// 先端先行＋10フレームごとの高速オーバーラップ更新（全ハンドルを追従させて取り残し防止）
		float leadX = it->x + std::cos(it->angle) * 45.0f;
		float leadY = it->y + std::sin(it->angle) * 45.0f;
		for (auto hit = it->efkHandles.begin(); hit != it->efkHandles.end(); )
		{
			if (*hit == -1 || !Effekseer2DManager::IsPlaying(*hit))
			{
				hit = it->efkHandles.erase(hit);
			}
			else
			{
				Effekseer2DManager::SetTransform(*hit, leadX, leadY, 1.45f, it->angle);
				++hit;
			}
		}
		if (it->life % 10 == 0 || it->efkHandles.empty())
		{
			int h = Effekseer2DManager::Play(
				Effect2DId::Stage3_SlashWind,
				leadX,
				leadY,
				1.45f,
				1.65f,
				it->angle,
				85, 255, 220, 255
			);
			if (h != -1) it->efkHandles.push_back(h);
		}

		// プレイヤーとの当たり判定
		if (playerAlive)
		{
			float r = it->radius;
			if (it->x + r > pLeft && it->x - r < pRight &&
			    it->y + r > pTop  && it->y - r < pBottom)
			{
				player->TakeDamage(attackDamage, DeathReason::DeathType::NormalDeath);
				stopBladeEffects(*it);
				it = mWindBlades.erase(it);
				continue;
			}
		}

		// 寿命到達時はフェードアウト消滅
		if (it->life >= it->maxLife)
		{
			it->fadeTimer = 18;
		}

		++it;
	}

	// 2. 発射マズルリングの更新
	for (auto it = mWindLaunchRings.begin(); it != mWindLaunchRings.end(); )
	{
		it->radius += 5.5f;
		it->life++;
		if (it->life >= it->maxLife) it = mWindLaunchRings.erase(it);
		else ++it;
	}

	// 3. 風スパークの更新
	for (auto it = mWindSparks.begin(); it != mWindSparks.end(); )
	{
		it->x += it->vx;
		it->y += it->vy;
		it->life++;
		if (it->life >= it->maxLife) it = mWindSparks.erase(it);
		else ++it;
	}

	// 4. 咆哮衝撃波リングの更新（Stage3DragonRoarAttackと同じ補間拡大・収束）
	for (auto it = mRoarShockRings.begin(); it != mRoarShockRings.end(); )
	{
		it->radius += (it->maxRadius - it->radius) * 0.16f + 8.0f;
		it->life--;
		if (it->life <= 0) it = mRoarShockRings.erase(it);
		else ++it;
	}

	// 5. 咆哮暴風ラインの更新
	for (auto it = mRoarWindLines.begin(); it != mRoarWindLines.end(); )
	{
		it->x += it->vx;
		it->y += it->vy;
		it->life--;
		if (it->life <= 0) it = mRoarWindLines.erase(it);
		else ++it;
	}

	// 6. 咆哮チャージ粒子の更新
	for (auto it = mRoarChargeParticles.begin(); it != mRoarChargeParticles.end(); )
	{
		it->dist  -= it->speed;
		it->angle += 0.09f;
		if (it->dist <= 8.0f) it = mRoarChargeParticles.erase(it);
		else ++it;
	}
}

void Stage3DragonCloneWindRoarAttack::GetDragonEyeScreenPos(
	const VECTOR& pos,
	float facingDir,
	float drawScale,
	float footOffset,
	float enemyHeight,
	int scrollX,
	int scrollY,
	int& outEyeSx,
	int& outEyeSy) const
{
	float localOffX     = -88.0f;
	float localOffY     = -40.0f;
	float feetWorldY    = pos.y + (enemyHeight * 0.5f);
	float spriteCenterY = feetWorldY - (footOffset * drawScale);
	float dirSign       = (facingDir > 0.0f) ? -1.0f : 1.0f;

	float eyeWorldX = pos.x + (localOffX * dirSign) * drawScale;
	float eyeWorldY = spriteCenterY + localOffY * drawScale;

	outEyeSx = static_cast<int>(eyeWorldX) - scrollX;
	outEyeSy = static_cast<int>(eyeWorldY) - scrollY;
}

void Stage3DragonCloneWindRoarAttack::DrawEyeCrossGlint(int eyeSx, int eyeSy, int alpha, bool isClone) const
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

	unsigned int outerCol = GetColor(15, 255, 145);
	unsigned int coreHot  = GetColor(225, 255, 235);

	SetDrawBlendMode(DX_BLENDMODE_ADD, a);
	DrawTriangle(eyeSx - hLen, eyeSy, eyeSx, eyeSy - hHalf, eyeSx, eyeSy + hHalf, outerCol, TRUE);
	DrawTriangle(eyeSx + hLen, eyeSy, eyeSx, eyeSy - hHalf, eyeSx, eyeSy + hHalf, outerCol, TRUE);
	DrawTriangle(eyeSx, eyeSy - vLen, eyeSx - vHalf, eyeSy, eyeSx + vHalf, eyeSy, outerCol, TRUE);
	DrawTriangle(eyeSx, eyeSy + vLen, eyeSx - vHalf, eyeSy, eyeSx + vHalf, eyeSy, outerCol, TRUE);
	DrawTriangle(eyeSx - (hLen * 2 / 3), eyeSy, eyeSx, eyeSy - 2, eyeSx, eyeSy + 2, coreHot, TRUE);
	DrawTriangle(eyeSx + (hLen * 2 / 3), eyeSy, eyeSx, eyeSy - 2, eyeSx, eyeSy + 2, coreHot, TRUE);
	DrawTriangle(eyeSx, eyeSy - (vLen * 2 / 3), eyeSx - 2, eyeSy, eyeSx + 2, eyeSy, coreHot, TRUE);
	DrawTriangle(eyeSx, eyeSy + (vLen * 2 / 3), eyeSx - 2, eyeSy, eyeSx + 2, eyeSy, coreHot, TRUE);
	DrawLine(eyeSx - dLen, eyeSy - dLen, eyeSx + dLen, eyeSy + dLen, outerCol, 2);
	DrawLine(eyeSx - dLen, eyeSy + dLen, eyeSx + dLen, eyeSy - dLen, outerCol, 2);
	DrawCircle(eyeSx, eyeSy, static_cast<int>(6.0f * pop) + 1, outerCol, TRUE);
	DrawCircle(eyeSx, eyeSy, static_cast<int>(3.0f * pop) + 1, coreHot, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Stage3DragonCloneWindRoarAttack::Draw(
	BlockMap* blockMap,
	const VECTOR& bossPos,
	float facingDir,
	float drawScale,
	int realSpriteHandle,
	float footOffset,
	float enemyHeight) const
{
	(void)realSpriteHandle;
	if (blockMap == nullptr) return;
	if (mPhase == Phase::Inactive || mPhase == Phase::Finished) return;

	int scrollX = blockMap->GetScrollX() + blockMap->mShakeX;
	int scrollY = blockMap->GetScrollY() + blockMap->mShakeY;

	// -----------------------------------------------------------------
	// 1. 本体の咆哮チャージ危険範囲表示＆口元エネルギー球（Stage3DragonRoarAttackと同等＋強化）
	// -----------------------------------------------------------------
	if (mPhase == Phase::WindBladeAndRoarLoop)
	{
		int roarCycle = mPhaseTimer % ROAR_INTERVAL;

		if (roarCycle < ROAR_CHARGE_FRAMES)
		{
			float ratio    = static_cast<float>(roarCycle) / static_cast<float>(ROAR_CHARGE_FRAMES);
			int pulseAlpha = 20 + static_cast<int>((std::sin(mPhaseTimer * 0.28f) * 0.5f + 0.5f) * 38.0f);

			// ドラゴンがいる足場側「以外」の危険範囲を赤く半透明点滅（Stage3DragonRoarAttackと同じ演出）
			// 【Stage3.png 5724x1692拡大版 対応】危険範囲の壁(3016..5690)・天井(483)・底(1477)座標を更新
			int zoneLeftMapX  = mbRealOnLeftEdge ? static_cast<int>(LEFT_SAFE_MAX_X) : 3016;
			int zoneRightMapX = mbRealOnLeftEdge ? 5690 : static_cast<int>(RIGHT_SAFE_MIN_X);
			int sx1 = zoneLeftMapX  - scrollX;
			int sx2 = zoneRightMapX - scrollX;

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, pulseAlpha);
			DrawBox(sx1, 483 - scrollY, sx2, 1477 - scrollY, GetColor(255, 40, 40), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

			// 安全地帯との境界ライン表示
			int borderMapX = mbRealOnLeftEdge ? static_cast<int>(LEFT_SAFE_MAX_X) : static_cast<int>(RIGHT_SAFE_MIN_X);
			int borderSX   = borderMapX - scrollX;
			DrawLine(borderSX, 483 - scrollY, borderSX, 1477 - scrollY, GetColor(255, 90, 90), 3);
		}
	}

	// -----------------------------------------------------------------
	// 2. 分身2体の描画（真ん中の上に左右を向いて配置、120度コーンから風の刃を発射）
	// -----------------------------------------------------------------
	if (mCloneAlpha > 0)
	{
		float cloneFootOffset = 57.0f;
		int   cloneHandle     = SelectCloneSpriteHandle(cloneFootOffset);

		float cloneXs[2]   = { mClone1X,         mClone2X         };
		float cloneYs[2]   = { mClone1Y,         mClone2Y         };
		float cloneDirs[2] = { mClone1FacingDir, mClone2FacingDir };
		float nextAngs[2]  = { mClone1NextAngle, mClone2NextAngle };

		int bladeCycle = (mPhase == Phase::WindBladeAndRoarLoop) ? (mPhaseTimer % WIND_BLADE_INTERVAL) : 0;

		for (int c = 0; c < 2; ++c)
		{
			float cX      = cloneXs[c];
			float cY      = cloneYs[c];
			float cDir    = cloneDirs[c];
			bool  reverse = (cDir > 0.0f);

			int   cSx   = static_cast<int>(cX) - scrollX;
			float feetY = cY + (enemyHeight * 0.5f);
			int   cSy   = static_cast<int>(feetY - (cloneFootOffset * drawScale)) - scrollY;

			// 分身の顔の前あたりに常時展開する大型ルーン魔法陣（2D Effekseer魔法陣と連動）
			if (mPhase == Phase::WindBladeAndRoarLoop)
			{
				int muzzleX = static_cast<int>(cX + cDir * 155.0f) - scrollX;
				int muzzleY = static_cast<int>(cY - 58.0f) - scrollY;
				bool isFireBoost = (bladeCycle >= 52 && bladeCycle <= 74);
				float rot = mPhaseTimer * (isFireBoost ? 0.14f : 0.055f) * cDir;
				float baseR = (isFireBoost ? 112.0f : 94.0f) + std::sin(mPhaseTimer * 0.18f) * 3.0f;
				int rx = static_cast<int>(baseR * 0.45f);
				int ry = static_cast<int>(baseR * 1.18f);
				int mcAlpha = (mCloneAlpha * (isFireBoost ? 245 : 185)) / 255;

				SetDrawBlendMode(DX_BLENDMODE_ADD, mcAlpha);
				unsigned int outerCol = GetColor(25, 255, 190);
				unsigned int midCol   = GetColor(95, 255, 225);
				unsigned int coreCol  = GetColor(225, 255, 245);

				DrawOval(muzzleX, muzzleY, rx, ry, outerCol, FALSE, 3);
				DrawOval(muzzleX, muzzleY, static_cast<int>(rx * 0.80f), static_cast<int>(ry * 0.80f), midCol, FALSE, 2);
				DrawOval(muzzleX, muzzleY, static_cast<int>(rx * 0.48f), static_cast<int>(ry * 0.48f), coreCol, FALSE, 1);

				for (int rIdx = 0; rIdx < 8; ++rIdx)
				{
					float a = rot * 0.6f + rIdx * (kPi * 0.25f);
					int x1 = muzzleX + static_cast<int>(std::cos(a) * rx * 0.82f);
					int y1 = muzzleY + static_cast<int>(std::sin(a) * ry * 0.82f);
					int x2 = muzzleX + static_cast<int>(std::cos(a) * rx * 1.02f);
					int y2 = muzzleY + static_cast<int>(std::sin(a) * ry * 1.02f);
					DrawLine(x1, y1, x2, y2, midCol, 2);
				}

				for (int tri = 0; tri < 2; ++tri)
				{
					float baseRot = (tri == 0) ? rot : (-rot + kPi / 3.0f);
					for (int k = 0; k < 3; ++k)
					{
						float a1 = baseRot + k * (2.0f * kPi / 3.0f);
						float a2 = baseRot + ((k + 1) % 3) * (2.0f * kPi / 3.0f);
						int x1 = muzzleX + static_cast<int>(std::cos(a1) * rx * 0.80f);
						int y1 = muzzleY + static_cast<int>(std::sin(a1) * ry * 0.80f);
						int x2 = muzzleX + static_cast<int>(std::cos(a2) * rx * 0.80f);
						int y2 = muzzleY + static_cast<int>(std::sin(a2) * ry * 0.80f);
						DrawLine(x1, y1, x2, y2, (tri == 0) ? outerCol : coreCol, 2);
					}
				}
				if (isFireBoost)
				{
					DrawCircle(muzzleX, muzzleY, 12, outerCol, TRUE);
					DrawCircle(muzzleX, muzzleY, 5,  coreCol,  TRUE);
				}
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			}

			if (cloneHandle != -1)
			{
				SetDrawBlendMode(DX_BLENDMODE_ADD, mCloneAlpha / 2);
				SetDrawBright(60, 255, 200);
				DrawRotaGraph(cSx, cSy, drawScale * 1.05f, 0.0, cloneHandle, TRUE, reverse);

				SetDrawBlendMode(DX_BLENDMODE_ALPHA, mCloneAlpha);
				SetDrawBright(180, 255, 235);
				DrawRotaGraph(cSx, cSy, drawScale, 0.0, cloneHandle, TRUE, reverse);

				SetDrawBright(255, 255, 255);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

				if (mCloneEyeGlintTimer > 0)
				{
					int eyeSx = 0, eyeSy = 0;
					auto itEye = mCloneEyeOffsetsByHandle.find(cloneHandle);
					if (itEye != mCloneEyeOffsetsByHandle.end())
					{
						float dirSign = reverse ? -1.0f : 1.0f;
						eyeSx = cSx + static_cast<int>((itEye->second.first * dirSign) * drawScale);
						eyeSy = cSy + static_cast<int>(itEye->second.second * drawScale);
					}
					else
					{
						GetDragonEyeScreenPos(VGet(cX, cY, 0.0f), cDir, drawScale, cloneFootOffset, enemyHeight, scrollX, scrollY, eyeSx, eyeSy);
					}
					DrawEyeCrossGlint(eyeSx, eyeSy, mCloneAlpha, true);
				}
			}
		}
	}

	// -----------------------------------------------------------------
	// 3. 咆哮の暴風ライン描画
	// -----------------------------------------------------------------
	for (size_t i = 0; i < mRoarWindLines.size(); ++i)
	{
		const RoarWindLine& wl = mRoarWindLines[i];
		float dirSign = (wl.vx >= 0.0f) ? 1.0f : -1.0f;
		int wx1   = static_cast<int>(wl.x) - scrollX;
		int wy1   = static_cast<int>(wl.y) - scrollY;
		int wx2   = static_cast<int>(wl.x + dirSign * wl.length) - scrollX;
		int wy2   = static_cast<int>(wl.y + wl.length * 0.16f) - scrollY;
		int alpha = (wl.life * 215) / wl.maxLife;
		SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
		DrawLine(wx1, wy1, wx2, wy2, GetColor(255, 90, 100), 3);
		DrawLine(wx1, wy1 - 2, wx2, wy2 - 2, GetColor(255, 220, 220), 1);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// -----------------------------------------------------------------
	// 4. 風の刃の発射リング・風の刃本体・風スパークの描画
	// -----------------------------------------------------------------
	for (size_t i = 0; i < mWindLaunchRings.size(); ++i)
	{
		const WindLaunchRing& lr = mWindLaunchRings[i];
		int lx = static_cast<int>(lr.x) - scrollX;
		int ly = static_cast<int>(lr.y) - scrollY;
		int a  = ((lr.maxLife - lr.life) * 230) / lr.maxLife;
		SetDrawBlendMode(DX_BLENDMODE_ADD, a);
		DrawCircle(lx, ly, static_cast<int>(lr.radius), GetColor(70, 255, 200), FALSE, 3);
		DrawCircle(lx, ly, static_cast<int>(lr.radius * 0.7f), GetColor(220, 255, 245), FALSE, 1);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	for (size_t i = 0; i < mWindSparks.size(); ++i)
	{
		const WindSpark& sp = mWindSparks[i];
		int sx1 = static_cast<int>(sp.x) - scrollX;
		int sy1 = static_cast<int>(sp.y) - scrollY;
		int sx2 = static_cast<int>(sp.x - sp.vx * 2.4f) - scrollX;
		int sy2 = static_cast<int>(sp.y - sp.vy * 2.4f) - scrollY;
		int a   = ((sp.maxLife - sp.life) * 245) / sp.maxLife;
		SetDrawBlendMode(DX_BLENDMODE_ADD, a);
		DrawLine(sx1, sy1, sx2, sy2, sp.color, 2);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

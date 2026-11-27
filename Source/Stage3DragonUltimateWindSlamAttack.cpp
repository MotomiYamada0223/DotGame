#include "Stage3DragonUltimateWindSlamAttack.h"
#include "Stage3EffectManager.h"
#include "BlockMap.h"
#include "Player.h"
#include "Stage3Magma.h"
#include "Stage3Gimmick.h"
#include "GameConstants.h"
#include <cmath>
#include <cstdlib>
#include <utility>

Stage3DragonUltimateWindSlamAttack::Stage3DragonUltimateWindSlamAttack()
	: mPhase(Phase::Inactive)
	, mPhaseTimer(0)
	, mAnimType(ANIM_IDLE)
	, mAnimFrame(0)
	, mStartBossX(RIGHT_LEDGE_STAND_X)
	, mStartBossY(RIGHT_LEDGE_FLOOR_Y - BOSS_HALF_HEIGHT)
	, mReturnTargetX(RIGHT_LEDGE_STAND_X)
	, mReturnTargetY(RIGHT_LEDGE_FLOOR_Y - BOSS_HALF_HEIGHT)
	, mLeftCloneX(CENTER_X - CLONE_OFFSET_X)
	, mLeftCloneY(CLONE_FLAP_Y)
	, mRightCloneX(CENTER_X + CLONE_OFFSET_X)
	, mRightCloneY(CLONE_FLAP_Y)
	, mCloneAnimFrame(0)
	, mCloneAlpha(0)
	, mbSphereActive(false)
	, mPlayerFloatWindEfk(-1)
	, mSphereTexWindVortex(-1)
	, mSphereTexWindSoft(-1)
	, mSphereTexElectric(-1)
	, mSphereTexThunderArcs{ -1, -1, -1, -1, -1, -1, -1, -1 }
	, mExpTexFire{ -1, -1, -1, -1 }
	, mExpTexSmoke{ -1, -1, -1, -1 }
	, mSphereTexWindVortexSize(128)
	, mSphereTexWindSoftSize(128)
	, mSphereTexElectricSize(128)
	, mSphereX(CENTER_X)
	, mSphereY(WIND_SPHERE_INIT_Y)
	, mSphereRadius(0.0f)
	, mMagmaAbsorbRatio(0.0f)
	, mWhiteFlashAlpha(0)
	, mSphereElectrified(false)
	, mSphereElectricityLevel(0)
	, mSphereElectricTimer(0)
{
}

Stage3DragonUltimateWindSlamAttack::~Stage3DragonUltimateWindSlamAttack()
{
	ReleaseCloneAnimations();
}

void Stage3DragonUltimateWindSlamAttack::Initialize()
{
	LoadCloneAnimations();
	Reset(nullptr);
}

void Stage3DragonUltimateWindSlamAttack::Reset(Player* player)
{
	mPhase             = Phase::Inactive;
	mPhaseTimer        = 0;
	mAnimType          = ANIM_IDLE;
	mAnimFrame         = 0;
	mCloneAnimFrame    = 0;
	mCloneAlpha        = 0;
	mbSphereActive     = false;
	mSphereX           = CENTER_X;
	mSphereY           = WIND_SPHERE_INIT_Y;
	mSphereRadius      = 0.0f;
	mMagmaAbsorbRatio  = 0.0f;

	StopSphereEffects();
	mSuckedMagma.clear();
	mCrescentBlades.clear();
	mSideLightnings.clear();
	mSphereElectrified      = false;
	mSphereElectricityLevel = 0;
	mSphereElectricTimer    = 0;
	mGroundShocks.clear();
	mExplosionPlumes.clear();
	mExplosionSparks.clear();
	mWhiteFlashAlpha = 0;

	if (player != nullptr)
	{
		player->SetWindFloatMode(false);
	}
}

void Stage3DragonUltimateWindSlamAttack::ReleaseCloneAnimations()
{
	for (int h : mCloneIdleAnim) if (h != -1) DeleteGraph(h);
	for (int h : mCloneFlyAnim)  if (h != -1) DeleteGraph(h);
	mCloneIdleAnim.clear();
	mCloneFlyAnim.clear();

	if (mSphereTexWindVortex != -1) { DeleteGraph(mSphereTexWindVortex); mSphereTexWindVortex = -1; }
	if (mSphereTexWindSoft != -1)   { DeleteGraph(mSphereTexWindSoft);   mSphereTexWindSoft = -1; }
	if (mSphereTexElectric != -1)   { DeleteGraph(mSphereTexElectric);   mSphereTexElectric = -1; }
	for (int i = 0; i < 8; ++i)
	{
		if (mSphereTexThunderArcs[i] != -1)
		{
			DeleteGraph(mSphereTexThunderArcs[i]);
			mSphereTexThunderArcs[i] = -1;
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		if (mExpTexFire[i] != -1)  { DeleteGraph(mExpTexFire[i]);  mExpTexFire[i] = -1; }
		if (mExpTexSmoke[i] != -1) { DeleteGraph(mExpTexSmoke[i]); mExpTexSmoke[i] = -1; }
	}
}

int Stage3DragonUltimateWindSlamAttack::LoadGraphWithRecoloredEyes(const char* filePath)
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

void Stage3DragonUltimateWindSlamAttack::LoadCloneAnimations()
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

	auto loadTex = [](const char* path, int& outHandle, int& outSize)
	{
		outHandle = LoadGraph(path);
		int w = 0, h = 0;
		if (outHandle != -1)
		{
			GetGraphSize(outHandle, &w, &h);
		}
		outSize = (w > 0) ? w : 128;
	};
	loadTex("Resource/Effect/Effekseer01/Effekseer01/Texture/Wind01.png",     mSphereTexWindVortex, mSphereTexWindVortexSize);
	loadTex("Resource/Effect/Effekseer01/Effekseer01/Texture/wind02.png",     mSphereTexWindSoft,   mSphereTexWindSoftSize);
	loadTex("Resource/Effect/NextSoft01/NextSoft01/Texture/ElectricBall.png", mSphereTexElectric,   mSphereTexElectricSize);
	LoadDivGraph("Resource/Effect/tktk02/tktk02/Parts/Thunder2.png", 8, 2, 4, 128, 64, mSphereTexThunderArcs);
	LoadDivGraph("Resource/Effect/Pierre01/Pierre01/Texture/Fire.png", 4, 2, 2, 256, 256, mExpTexFire);
	LoadDivGraph("Resource/Effect/Pierre01/Pierre01/Texture/Smoke.png", 4, 2, 2, 256, 256, mExpTexSmoke);
}

void Stage3DragonUltimateWindSlamAttack::StartAttack(VECTOR& bossPos, float& facingDir, Player* player)
{
	mPhase             = Phase::SetupPositions;
	mPhaseTimer        = 0;
	mAnimType          = ANIM_TAKEOFF;
	mAnimFrame         = 0;
	mCloneAnimFrame    = 0;
	mCloneAlpha        = 0;
	mbSphereActive     = true;
	mSphereX           = CENTER_X;
	mSphereY           = WIND_SPHERE_INIT_Y;
	mSphereRadius      = 28.0f;
	mMagmaAbsorbRatio  = 0.0f;

	StopSphereEffects();
	mSuckedMagma.clear();
	mCrescentBlades.clear();
	mSideLightnings.clear();

	mStartBossX = bossPos.x;
	mStartBossY = bossPos.y;

	float distLeft = std::fabs(bossPos.x - LEFT_LEDGE_STAND_X);
	float distRight = std::fabs(bossPos.x - RIGHT_LEDGE_STAND_X);
	if (distLeft < distRight)
	{
		mReturnTargetX = LEFT_LEDGE_STAND_X;
		mReturnTargetY = LEFT_LEDGE_FLOOR_Y - BOSS_HALF_HEIGHT;
	}
	else
	{
		mReturnTargetX = RIGHT_LEDGE_STAND_X;
		mReturnTargetY = RIGHT_LEDGE_FLOOR_Y - BOSS_HALF_HEIGHT;
	}

	mLeftCloneX  = CENTER_X - CLONE_OFFSET_X;
	mLeftCloneY  = CLONE_FLAP_Y;
	mRightCloneX = CENTER_X + CLONE_OFFSET_X;
	mRightCloneY = CLONE_FLAP_Y;

	facingDir = (bossPos.x > CENTER_X) ? -1.0f : 1.0f;

	if (player != nullptr && !player->GetIsDead())
	{
		player->SetWindFloatMode(true);
	}
}

void Stage3DragonUltimateWindSlamAttack::SpawnSuckedMagmaGlob(float surfaceY)
{
	SuckedMagmaGlob g;
	g.active    = true;
	int span    = static_cast<int>(MAGMA_PIT_RIGHT_X - MAGMA_PIT_LEFT_X);
	g.startX    = MAGMA_PIT_LEFT_X + static_cast<float>(GetRand(span));
	g.startY    = surfaceY + 6.0f;
	g.ctrlX     = g.startX * 0.82f + CENTER_X * 0.18f + static_cast<float>(GetRand(90) - 45);
	g.ctrlY     = 660.0f;
	g.x         = g.startX;
	g.y         = g.startY;
	for (int i = 0; i < 5; ++i)
	{
		g.prevX[i] = g.startX;
		g.prevY[i] = g.startY;
	}
	g.radius    = 24.0f + static_cast<float>(GetRand(8));
	g.warnTimer = 18;
	g.timer     = 0;
	g.duration  = 72 + GetRand(14);
	mSuckedMagma.push_back(g);
}

void Stage3DragonUltimateWindSlamAttack::SpawnCrescentBlade(bool fromLeft)
{
	CrescentBlade b;
	b.active    = true;
	b.fromLeft  = fromLeft;
	b.startX    = fromLeft ? (mLeftCloneX + 85.0f) : (mRightCloneX - 85.0f);
	b.startY    = fromLeft ? mLeftCloneY : mRightCloneY;
	b.x         = b.startX;
	b.y         = b.startY;
	b.t         = 0.0f;
	b.speed     = 0.052f + (GetRand(10) / 1000.0f);
	b.angle     = std::atan2(mSphereY - b.startY, mSphereX - b.startX);
	b.scale     = 1.15f + (GetRand(4) * 0.1f);
	b.efkHandle = Stage3EffectManager::Play(
		Stage3EffectType::SlashWind, b.x, b.y, 1.45f, 1.55f, b.angle, 65, 255, 200, 240
	);
	mCrescentBlades.push_back(b);
}

void Stage3DragonUltimateWindSlamAttack::StopSphereEffects()
{
	auto stopHandle = [](int& h)
	{
		if (h != -1)
		{
			Stage3EffectManager::Stop(h);
			h = -1;
		}
	};
	stopHandle(mPlayerFloatWindEfk);
	for (auto& b  : mCrescentBlades)  stopHandle(b.efkHandle);
	for (auto& s  : mSideLightnings)  stopHandle(s.efkHandle);
	for (auto& gs : mGroundShocks)    stopHandle(gs.efkHandle);
}

void Stage3DragonUltimateWindSlamAttack::SpawnSideLightning(Player* player)
{
	SideLightningStrike s;
	s.active     = true;
	s.warnTimer  = 55; // 約0.92秒の予兆・回避猶予
	s.timer      = 0;
	s.hitGround  = false;
	s.burstTimer = 0;

	// 【Stage3.png 5724x1692拡大版 対応】左右外壁(3016..5690)と天井・床高度(500..1250)に合わせて電撃発射位置を更新
	float px = (player != nullptr) ? player->GetPosition().x : CENTER_X;
	float py = (player != nullptr) ? player->GetPosition().y : 980.0f;

	// 構えた瞬間のプレイヤーの高度に合わせて左右の壁から出現
	float spawnY = py + static_cast<float>(GetRand(80) - 40);
	if (spawnY < 500.0f)  spawnY = 500.0f;
	if (spawnY > 1250.0f) spawnY = 1250.0f;

	// プレイヤーの位置に合わせて左右の壁を選択
	if (px < 4050.0f)
	{
		s.fromLeft = (GetRand(3) != 0); // 75%で左、25%で右
	}
	else if (px > 4650.0f)
	{
		s.fromLeft = (GetRand(3) == 0); // 75%で右、25%で左
	}
	else
	{
		s.fromLeft = (GetRand(1) == 0);
	}

	s.startX  = s.fromLeft ? 3026.0f : 5680.0f;
	s.startY  = spawnY;

	// 【最重要】「電撃を構えた瞬間」のプレイヤー位置に完全に目標を固定！
	// 構え中や発射後にプレイヤーを追尾することは一切ありません
	s.targetX = px;
	s.targetY = py;

	s.x = s.startX;
	s.y = s.startY;

	float dx   = s.targetX - s.startX;
	float dy   = s.targetY - s.startY;
	float dist = std::sqrt(dx * dx + dy * dy);
	float spd  = 24.0f;
	if (dist > 1.0f)
	{
		s.vx = (dx / dist) * spd;
		s.vy = (dy / dist) * spd;
	}
	else
	{
		s.vx = s.fromLeft ? spd : -spd;
		s.vy = 0.0f;
	}

	mSideLightnings.push_back(s);
}

void Stage3DragonUltimateWindSlamAttack::SpawnGroundCrawlingShocks(float startX, float startY)
{
	// 左向き電撃（底面 -> 左崖を上へ -> 左足場 -> 左壁で消滅）
	GroundCrawlingShock sL;
	sL.active = true;
	sL.dir    = -1;
	sL.x      = startX - 25.0f;
	sL.y      = startY;
	sL.stage  = 0;
	sL.speed  = 24.0f;
	mGroundShocks.push_back(sL);

	// 右向き電撃（底面 -> 右崖を上へ -> 右足場 -> 右壁で消滅）
	GroundCrawlingShock sR;
	sR.active = true;
	sR.dir    = 1;
	sR.x      = startX + 25.0f;
	sR.y      = startY;
	sR.stage  = 0;
	sR.speed  = 24.0f;
	mGroundShocks.push_back(sR);
}

void Stage3DragonUltimateWindSlamAttack::DrawEyeCrossGlint(int eyeSx, int eyeSy, int alpha, float scale, bool megaFlash) const
{
	if (alpha <= 0) return;

	unsigned int outerColor = GetColor(255, 30, 20);
	unsigned int midColor   = GetColor(255, 140, 40);
	unsigned int innerColor = GetColor(255, 255, 230);

	SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);

	// コア発光球
	int coreR1 = static_cast<int>(14.0f * scale);
	int coreR2 = static_cast<int>(7.0f * scale);
	DrawCircle(eyeSx, eyeSy, coreR1, outerColor, TRUE);
	DrawCircle(eyeSx, eyeSy, coreR2, innerColor, TRUE);

	if (megaFlash)
	{
		// 【超絶キランッ！特大眼光演出】
		// 1. 横一直線に画面を切り裂く極太レーザーフレア (全幅640px)
		int beamW = static_cast<int>(320.0f * scale);
		int beamH = static_cast<int>(3.5f * scale);
		if (beamH < 2) beamH = 2;
		DrawBox(eyeSx - beamW, eyeSy - beamH, eyeSx + beamW, eyeSy + beamH, outerColor, TRUE);
		DrawLine(eyeSx - beamW, eyeSy, eyeSx + beamW, eyeSy, innerColor, 2);

		// 2. 縦の鋭利なレーザースパイク (全高360px)
		int vSpikeH = static_cast<int>(180.0f * scale);
		DrawLine(eyeSx, eyeSy - vSpikeH, eyeSx, eyeSy + vSpikeH, outerColor, 4);
		DrawLine(eyeSx, eyeSy - vSpikeH + 20, eyeSx, eyeSy + vSpikeH - 20, innerColor, 2);

		// 3. 斜め4方向のクロス光線 (140px)
		int diagL = static_cast<int>(70.0f * scale);
		DrawLine(eyeSx - diagL, eyeSy - diagL, eyeSx + diagL, eyeSy + diagL, midColor, 2);
		DrawLine(eyeSx - diagL, eyeSy + diagL, eyeSx + diagL, eyeSy - diagL, midColor, 2);

		// 4. 外周に広がる巨大衝撃リングオーラ
		int ringR = static_cast<int>(55.0f * scale);
		DrawCircle(eyeSx, eyeSy, ringR, outerColor, FALSE);
		DrawCircle(eyeSx, eyeSy, ringR + 3, midColor, FALSE);

		// 5. 中心のダイヤモンドスパーク
		int dSize = static_cast<int>(18.0f * scale);
		DrawTriangle(eyeSx, eyeSy - dSize, eyeSx - dSize / 2, eyeSy, eyeSx + dSize / 2, eyeSy, innerColor, TRUE);
		DrawTriangle(eyeSx, eyeSy + dSize, eyeSx - dSize / 2, eyeSy, eyeSx + dSize / 2, eyeSy, innerColor, TRUE);
	}
	else
	{
		// 通常の鋭利な十文字眼光
		int hLen = static_cast<int>(38.0f * scale);
		int vLen = static_cast<int>(26.0f * scale);
		int dLen = static_cast<int>(16.0f * scale);

		DrawLine(eyeSx - hLen, eyeSy, eyeSx + hLen, eyeSy, outerColor, 3);
		DrawLine(eyeSx, eyeSy - vLen, eyeSx, eyeSy + vLen, outerColor, 3);
		DrawLine(eyeSx - (hLen * 2 / 3), eyeSy, eyeSx + (hLen * 2 / 3), eyeSy, innerColor, 1);
		DrawLine(eyeSx, eyeSy - (vLen * 2 / 3), eyeSy, eyeSx + (vLen * 2 / 3), innerColor, 1);

		DrawLine(eyeSx - dLen, eyeSy - dLen, eyeSx + dLen, eyeSy + dLen, midColor, 1);
		DrawLine(eyeSx - dLen, eyeSy + dLen, eyeSx + dLen, eyeSy - dLen, midColor, 1);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Stage3DragonUltimateWindSlamAttack::GetDragonEyeScreenPos(
	const VECTOR& bossPos,
	float facingDir,
	float drawScale,
	float footOffset,
	float enemyHeight,
	int animFrame,
	int scrollX,
	int scrollY,
	int& outEyeSx,
	int& outEyeSy
) const
{
	static const float kAttackEyeOffsetX[9] = { -88.0f, -94.0f, -98.0f, -85.0f, -73.0f, -91.0f, -93.0f, -74.0f, -70.0f };
	static const float kAttackEyeOffsetY[9] = { -40.0f,   9.0f, -16.0f, -40.0f, -13.0f,   5.0f,  14.0f, -12.0f, -12.0f };

	float localOffX = -88.0f;
	float localOffY = -40.0f;
	if (animFrame >= 0 && animFrame < 9)
	{
		localOffX = kAttackEyeOffsetX[animFrame];
		localOffY = kAttackEyeOffsetY[animFrame];
	}

	float feetWorldY    = bossPos.y + (enemyHeight * 0.5f);
	float spriteCenterY = feetWorldY - (footOffset * drawScale);
	float dirSign       = (facingDir > 0.0f) ? -1.0f : 1.0f;

	float eyeWorldX = bossPos.x + (localOffX * dirSign) * drawScale;
	float eyeWorldY = spriteCenterY + localOffY * drawScale;

	outEyeSx = static_cast<int>(eyeWorldX) - scrollX;
	outEyeSy = static_cast<int>(eyeWorldY) - scrollY;
}

void Stage3DragonUltimateWindSlamAttack::UpdateDragon(
	VECTOR& bossPos,
	float& facingDir,
	bool& isBossGrounded,
	int& outAnimType,
	int& outAnimFrame,
	Player* player,
	BlockMap* blockMap,
	Stage3Magma* magma,
	Stage3Gimmick* gimmick)
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
		const int DURATION = 50;
		float t    = static_cast<float>(mPhaseTimer) / static_cast<float>(DURATION);
		if (t > 1.0f) t = 1.0f;
		float ease = t * t * (3.0f - 2.0f * t);

		bossPos.x = mStartBossX + (CENTER_X    - mStartBossX) * ease;
		bossPos.y = mStartBossY + (REAL_WAIT_Y - mStartBossY) * ease;
		facingDir = -1.0f;

		mCloneAlpha     = static_cast<int>(240.0f * ease);
		mCloneAnimFrame = (mPhaseTimer / 3) % 7;
		mSphereRadius   = 18.0f + 24.0f * ease;



		if (player != nullptr)
		{
			if (!player->GetIsDead())
			{
				player->SetWindFloatMode(true);

				// 一定の強さで少しずつ風玉の方向へ引き寄せる（壁・床めり込みは回避）
				VECTOR pPos = player->GetPosition();
				float dx = mSphereX - pPos.x;
				float dy = mSphereY - pPos.y;
				float dist = std::sqrt(dx * dx + dy * dy);
				if (dist > 48.0f)
				{
					const float PULL_SPEED = 2.45f;
					float nx = dx / dist;
					float ny = dy / dist;
					float nextX = pPos.x + nx * PULL_SPEED;
					float nextY = pPos.y + ny * PULL_SPEED;
					if (blockMap != nullptr)
					{
						if (blockMap->GetCollisionType(static_cast<int>(nextX), static_cast<int>(pPos.y)) == BlockMap::CollisionType::None)
						{
							pPos.x = nextX;
						}
						if (blockMap->GetCollisionType(static_cast<int>(pPos.x), static_cast<int>(nextY)) == BlockMap::CollisionType::None)
						{
							pPos.y = nextY;
						}
					}
					else
					{
						pPos.x = nextX;
						pPos.y = nextY;
					}
					player->SetPosition(pPos);
				}
			}
			else
			{
				player->SetWindFloatMode(false);
			}
		}

		mAnimType  = (mPhaseTimer < 18) ? ANIM_TAKEOFF : ANIM_FLY;
		mAnimFrame = (mPhaseTimer / 4) % 7;

		if (mPhaseTimer >= DURATION)
		{
			mPhase      = Phase::CreateWindSphereAndSuckMagma;
			mPhaseTimer = 0;
		}
		break;
	}

	case Phase::CreateWindSphereAndSuckMagma:
	{
		float progress = static_cast<float>(mPhaseTimer) / static_cast<float>(SUCTION_DURATION);
		if (progress > 1.0f) progress = 1.0f;

		bossPos.x = CENTER_X + std::sin(mPhaseTimer * 0.05f) * 22.0f;
		bossPos.y = REAL_WAIT_Y + std::cos(mPhaseTimer * 0.08f) * 12.0f;
		facingDir = (player != nullptr && player->GetPosition().x >= CENTER_X) ? 1.0f : -1.0f;

		mCloneAlpha     = 240;
		mCloneAnimFrame = (mPhaseTimer / 2) % 7;
		mLeftCloneX     = (CENTER_X - CLONE_OFFSET_X) + std::sin(mPhaseTimer * 0.14f) * 10.0f;
		mLeftCloneY     = CLONE_FLAP_Y + std::cos(mPhaseTimer * 0.18f) * 16.0f;
		mRightCloneX    = (CENTER_X + CLONE_OFFSET_X) - std::sin(mPhaseTimer * 0.14f) * 10.0f;
		mRightCloneY    = CLONE_FLAP_Y + std::cos(mPhaseTimer * 0.18f + 1.2f) * 16.0f;

		mSphereX          = CENTER_X;
		mSphereY          = WIND_SPHERE_INIT_Y + std::sin(mPhaseTimer * 0.12f) * 8.0f;
		mSphereRadius     = 42.0f + 143.0f * progress;
		mMagmaAbsorbRatio = progress;

		if (player != nullptr)
		{
			if (!player->GetIsDead())
			{
				player->SetWindFloatMode(true);

				// 一定の強さで少しずつ風玉の方向へ引き寄せる（壁・床めり込みは回避）
				VECTOR pPos = player->GetPosition();
				float dx = mSphereX - pPos.x;
				float dy = mSphereY - pPos.y;
				float dist = std::sqrt(dx * dx + dy * dy);
				if (dist > 48.0f)
				{
					const float PULL_SPEED = 2.45f;
					float nx = dx / dist;
					float ny = dy / dist;
					float nextX = pPos.x + nx * PULL_SPEED;
					float nextY = pPos.y + ny * PULL_SPEED;
					if (blockMap != nullptr)
					{
						if (blockMap->GetCollisionType(static_cast<int>(nextX), static_cast<int>(pPos.y)) == BlockMap::CollisionType::None)
						{
							pPos.x = nextX;
						}
						if (blockMap->GetCollisionType(static_cast<int>(pPos.x), static_cast<int>(nextY)) == BlockMap::CollisionType::None)
						{
							pPos.y = nextY;
						}
					}
					else
					{
						pPos.x = nextX;
						pPos.y = nextY;
					}
					player->SetPosition(pPos);
				}
			}
			else
			{
				player->SetWindFloatMode(false);
			}
		}

		float currentMagmaY = 1236.0f;
		if (magma != nullptr)
		{
			magma->SetDrainRatio(progress * 0.94f);
			currentMagmaY = magma->GetCurrentSurfaceY();
		}

		if (mPhaseTimer <= SUCTION_DURATION - 65)
		{
			if (mPhaseTimer % 22 == 1)
			{
				SpawnSuckedMagmaGlob(currentMagmaY);
			}
		}

		if (mPhaseTimer % 28 == 0)  SpawnCrescentBlade(true);
		if (mPhaseTimer % 28 == 14) SpawnCrescentBlade(false);

		// 足場側の安全地帯を狙い撃つサイド電撃（約1秒に1回）
		if (mPhaseTimer >= 30 && mPhaseTimer <= SUCTION_DURATION - 40 && (mPhaseTimer % 58 == 0))
		{
			SpawnSideLightning(player);
		}





		if (blockMap != nullptr && mPhaseTimer % 3 == 0)
		{
			int amp = 2 + static_cast<int>(progress * 5.0f);
			blockMap->mShakeX = GetRand(amp * 2) - amp;
			blockMap->mShakeY = GetRand(amp * 2) - amp;
		}

		mAnimType  = ANIM_FLY;
		mAnimFrame = (mPhaseTimer / 4) % 7;

		if (mPhaseTimer >= SUCTION_DURATION)
		{
			mPhase      = Phase::RealDragonSlamWindup;
			mPhaseTimer = 0;
			mSuckedMagma.clear();
			if (player != nullptr)
			{
				player->SetWindFloatMode(false);
			}
		}
		break;
	}

	case Phase::RealDragonSlamWindup:
	{
		const int DURATION = 46;
		float t = static_cast<float>(mPhaseTimer) / static_cast<float>(DURATION);
		if (t > 1.0f) t = 1.0f;

		// 風玉が最大に達し、分身2匹が羽ばたきながら風粒子となって球へ収束・フェードアウト！
		mCloneAlpha     = static_cast<int>(240.0f * (1.0f - t));
		mCloneAnimFrame = (mPhaseTimer / 3) % 7;



		bossPos.x     = CENTER_X;
		bossPos.y     = REAL_WAIT_Y + (mSphereY - 160.0f - REAL_WAIT_Y) * t;
		mSphereRadius = 185.0f + std::sin(mPhaseTimer * 0.6f) * 8.0f;

		mAnimType  = ANIM_ATTACK;
		mAnimFrame = (t < 0.5f) ? 1 : 2;



		if (blockMap != nullptr)
		{
			blockMap->mShakeX = GetRand(10) - 5;
			blockMap->mShakeY = GetRand(10) - 5;
		}

		if (mPhaseTimer >= DURATION)
		{
			mPhase      = Phase::RealDragonSlamDown;
			mPhaseTimer = 0;
			mCloneAlpha = 0;
		}
		break;
	}

	case Phase::RealDragonSlamDown:
	{
		const int DURATION = 16;
		float t    = static_cast<float>(mPhaseTimer) / static_cast<float>(DURATION);
		if (t > 1.0f) t = 1.0f;
		// 初速から一気に加速して玉を叩き落とす超高速急降下
		float ease = t * (0.45f + 0.55f * t);

		mSphereX = CENTER_X;
		mSphereY = WIND_SPHERE_INIT_Y + (PIT_FLOOR_Y - WIND_SPHERE_INIT_Y) * ease;

		// ドラゴン本体も玉を蹴り落としながら一緒に急降下！
		float startBossY = WIND_SPHERE_INIT_Y - 160.0f;
		float landBossY  = PIT_FLOOR_Y - BOSS_HALF_HEIGHT;
		bossPos.x = CENTER_X;
		bossPos.y = startBossY + (landBossY - startBossY) * ease;

		mAnimType  = ANIM_ATTACK;
		mAnimFrame = (t < 0.3f) ? 3 : ((t < 0.7f) ? 4 : 5);

		if (blockMap != nullptr)
		{
			int shakeAmp = 12 + static_cast<int>(t * 16.0f);
			blockMap->mShakeX = GetRand(shakeAmp * 2) - shakeAmp;
			blockMap->mShakeY = GetRand(shakeAmp * 2) - shakeAmp;
		}

		if (mPhaseTimer >= DURATION)
		{
			mPhase             = Phase::SlamImpactFinish;
			mPhaseTimer        = 0;
			mbSphereActive     = false;
			StopSphereEffects();

			// 1. 地面のマグマを完全に消滅
			if (magma != nullptr)
			{
				magma->ClearMagma();
			}

			// 2. 空中の4つの足場台をすべて大爆発で完全粉砕！
			if (gimmick != nullptr && blockMap != nullptr)
			{
				gimmick->DestroyAllPlatforms(*blockMap, player);
			}

			// 3. 画面全体を一瞬真っ白にする大爆発フラッシュ＋上空へ高く噴き上がる巨大爆煙・紅蓮炎・パラパラ舞う火の粉＋地走り電撃
			mWhiteFlashAlpha = 255;
			mExplosionPlumes.clear();
			mExplosionSparks.clear();
			for (int i = 0; i < 12; ++i)
			{
				ExplosionPlume sm;
				sm.active   = true;
				sm.isSmoke  = true;
				sm.texIdx   = i % 4;
				float dir   = (i % 2 == 0) ? -1.0f : 1.0f;
				int   tier  = i / 2; // 0..5
				if (tier >= 4)
				{
					// 上方向へ高く立ち昇る爆煙柱
					sm.x        = CENTER_X + dir * (65.0f + static_cast<float>(GetRand(140)));
					sm.y        = PIT_FLOOR_Y - 120.0f - static_cast<float>(GetRand(90));
					sm.vx       = dir * (4.0f + static_cast<float>(GetRand(8)));
					sm.vy       = -(22.0f + static_cast<float>(GetRand(14)));
					sm.scale    = 1.50f + (GetRand(5) * 0.14f);
					sm.maxLife  = 46 + GetRand(10);
				}
				else
				{
					sm.x        = CENTER_X + dir * (150.0f + tier * 64.0f);
					sm.y        = PIT_FLOOR_Y - 40.0f - static_cast<float>(GetRand(65));
					sm.vx       = dir * (14.0f + tier * 5.0f);
					sm.vy       = -(12.0f + static_cast<float>(GetRand(18)));
					sm.scale    = 1.35f + (GetRand(5) * 0.14f);
					sm.maxLife  = 40 + GetRand(10);
				}
				sm.scaleVel = 0.06f;
				sm.angle    = static_cast<float>(GetRand(628)) / 100.0f;
				sm.angleVel = dir * 0.05f;
				sm.life     = 0;
				if (tier % 2 == 0)
				{
					sm.r = 185; sm.g = 45; sm.b = 10; // 深紅の爆煙
				}
				else
				{
					sm.r = 65;  sm.g = 24; sm.b = 18; // 黒褐色の爆煙
				}
				mExplosionPlumes.push_back(sm);
			}
			for (int i = 0; i < 14; ++i)
			{
				ExplosionPlume fm;
				fm.active   = true;
				fm.isSmoke  = false;
				fm.texIdx   = i % 4;
				float frac  = static_cast<float>(i) / 13.0f; // 0..1
				float ang   = -2.55f + frac * 1.96f;         // 上方向中心 (-146° .. -34°)
				float spd   = (i >= 4 && i <= 9) ? (26.0f + static_cast<float>(GetRand(16))) : (19.0f + static_cast<float>(GetRand(14)));
				fm.x        = CENTER_X + std::cos(ang) * 140.0f;
				fm.y        = PIT_FLOOR_Y - 50.0f + std::sin(ang) * 90.0f;
				fm.vx       = std::cos(ang) * spd * 1.15f;
				fm.vy       = std::sin(ang) * spd * 1.25f;
				fm.scale    = 1.55f + (GetRand(5) * 0.15f);
				fm.scaleVel = 0.07f;
				fm.angle    = ang + 1.5707963f;
				fm.angleVel = (std::cos(ang) >= 0.0f) ? 0.02f : -0.02f;
				fm.life     = 0;
				fm.maxLife  = 34 + GetRand(12);
				fm.r        = 255;
				fm.g        = (i % 2 == 0) ? 95 : 55;
				fm.b        = 5;
				mExplosionPlumes.push_back(fm);
			}
			for (int i = 0; i < 64; ++i)
			{
				ExplosionSpark sp;
				sp.x       = CENTER_X + static_cast<float>(GetRand(1500) - 750);
				sp.y       = PIT_FLOOR_Y - 25.0f - static_cast<float>(GetRand(740));
				sp.prevX   = sp.x;
				sp.prevY   = sp.y;
				sp.vx      = static_cast<float>(GetRand(32) - 16);
				sp.vy      = -(2.0f + static_cast<float>(GetRand(12)));
				sp.life    = 0;
				sp.maxLife = 150 + GetRand(80); // 長時間パラパラ舞ってゆっくりフェード (2.5～3.8秒)
				sp.size    = 3 + GetRand(4);
				sp.r       = 255;
				sp.g       = (i % 3 == 0) ? 215 : 120;
				sp.b       = (i % 3 == 0) ? 65 : 15;
				mExplosionSparks.push_back(sp);
			}
			SpawnGroundCrawlingShocks(CENTER_X, PIT_FLOOR_Y);

			// 4. マグマ溜まり全体（X = 3850 .. 5220）を天井から床まで覆う即死大爆発判定！
			if (player != nullptr && !player->GetIsDead())
			{
				VECTOR pPos = player->GetPosition();
				// 【Stage3.png 5724x1692拡大版 対応】マグマピット爆発判定範囲を更新
				if (pPos.x >= 3667.0f && pPos.x <= 4970.0f && pPos.y >= 460.0f && pPos.y <= 1500.0f)
				{
					player->TakeDamage(9999, DeathReason::DeathType::DragonExplosion);
				}
			}

			if (blockMap != nullptr)
			{
				blockMap->mShakeX = 28;
				blockMap->mShakeY = 32;
			}
		}
		break;
	}

	case Phase::SlamImpactFinish:
	{
		const int DURATION = 75;
		// 画面全体の白爆発フラッシュが滑らかに溶けて消えていく
		if (mPhaseTimer < 36)
		{
			float ft  = static_cast<float>(mPhaseTimer) / 36.0f;
			float inv = 1.0f - ft;
			mWhiteFlashAlpha = static_cast<int>(255.0f * inv * inv);
		}
		else
		{
			mWhiteFlashAlpha = 0;
		}
		if (mPhaseTimer == 16 && mSphereElectricityLevel >= 2)
		{
			SpawnGroundCrawlingShocks(CENTER_X, PIT_FLOOR_Y);
		}

		// 玉と一緒に地面へ着地した姿勢から着地モーションへ
		bossPos.x  = CENTER_X;
		bossPos.y  = PIT_FLOOR_Y - BOSS_HALF_HEIGHT;
		mAnimType  = (mPhaseTimer < 28) ? ANIM_LANDING : ANIM_IDLE;
		mAnimFrame = (mPhaseTimer < 28) ? (mPhaseTimer / 7) : 0;

		if (blockMap != nullptr)
		{
			if (mPhaseTimer < 45)
			{
				int amp = 32 - (mPhaseTimer * 32 / 45);
				if (amp < 3) amp = 3;
				blockMap->mShakeX = GetRand(amp * 2) - amp;
				blockMap->mShakeY = GetRand(amp * 2) - amp;
			}
			else
			{
				blockMap->mShakeX = 0;
				blockMap->mShakeY = 0;
			}
		}

		if (mPhaseTimer >= DURATION)
		{
			bossPos.x      = CENTER_X;
			bossPos.y      = PIT_FLOOR_Y - BOSS_HALF_HEIGHT;
			isBossGrounded = true;
			mPhase         = Phase::Finished;
			mPhaseTimer    = 0;
			mAnimType      = ANIM_IDLE;
			mAnimFrame     = 0;
		}
		break;
	}

	case Phase::ReturnToLedge:
	{
		const int DURATION = 55;
		float t    = static_cast<float>(mPhaseTimer) / static_cast<float>(DURATION);
		if (t > 1.0f) t = 1.0f;
		float ease = t * t * (3.0f - 2.0f * t);

		bossPos.x = mStartBossX + (mReturnTargetX - mStartBossX) * ease;
		bossPos.y = mStartBossY + (mReturnTargetY - mStartBossY) * ease - std::sin(t * 3.14159265f) * 140.0f;
		facingDir = (mReturnTargetX > CENTER_X) ? -1.0f : 1.0f;

		mAnimType  = (t < 0.7f) ? ANIM_FLY : ANIM_LANDING;
		mAnimFrame = (mPhaseTimer / 4) % 7;

		if (mPhaseTimer >= DURATION)
		{
			bossPos.x      = mReturnTargetX;
			bossPos.y      = mReturnTargetY;
			isBossGrounded = true;
			mPhase         = Phase::Finished;
			mPhaseTimer    = 0;
			mAnimType      = ANIM_IDLE;
			mAnimFrame     = 0;
		}
		break;
	}

	default:
		break;
	}

	outAnimType  = mAnimType;
	outAnimFrame = mAnimFrame;
}

void Stage3DragonUltimateWindSlamAttack::UpdateProjectilesAndEffects(
	Player* player,
	BlockMap* blockMap,
	Stage3Magma* magma,
	int attackDamage)
{
	(void)blockMap;
	(void)magma;

	float pLeft = 0.0f, pTop = 0.0f, pRight = 0.0f, pBottom = 0.0f;
	bool  playerAlive = (player != nullptr && !player->GetIsDead());
	if (playerAlive)
	{
		VECTOR pPos = player->GetPosition();
		float halfW = PlayerConstants::PlayerCollisionWidth * 0.5f;
		float halfH = PlayerConstants::PlayerCollisionHeight * 0.5f;
		pLeft   = pPos.x - halfW;
		pRight  = pPos.x + halfW;
		pTop    = pPos.y - halfH;
		pBottom = pPos.y + halfH;
	}

	// 1. 吸い込まれるマグマの更新
	for (auto it = mSuckedMagma.begin(); it != mSuckedMagma.end(); )
	{
		if (!it->active)
		{
			it = mSuckedMagma.erase(it);
			continue;
		}

		if (it->warnTimer > 0)
		{
			it->warnTimer--;
			++it;
			continue;
		}

		for (int k = 4; k > 0; --k)
		{
			it->prevX[k] = it->prevX[k - 1];
			it->prevY[k] = it->prevY[k - 1];
		}
		it->prevX[0] = it->x;
		it->prevY[0] = it->y;

		it->timer++;
		float t = static_cast<float>(it->timer) / static_cast<float>(it->duration);
		if (t > 1.0f) t = 1.0f;
		float ease = t * t * (0.4f + 0.6f * t);
		float inv  = 1.0f - ease;

		it->x = inv * inv * it->startX + 2.0f * inv * ease * it->ctrlX + ease * ease * mSphereX;
		it->y = inv * inv * it->startY + 2.0f * inv * ease * it->ctrlY + ease * ease * mSphereY;

		// 炎エフェクトの見た目と一致する当たり判定（軽量化のため描画はGPUスプライトで実施）
		if (playerAlive && t < 0.92f)
		{
			float hitR = it->radius * 1.15f;
			if (it->x + hitR > pLeft && it->x - hitR < pRight &&
			    it->y + hitR > pTop  && it->y - hitR < pBottom)
			{
				player->TakeDamage(9999, DeathReason::DeathType::DragonExplosion);
			}
		}

		if (it->timer >= it->duration)
		{
			it = mSuckedMagma.erase(it);
		}
		else
		{
			++it;
		}
	}

	// 2. 巨大球叩きつけ接触判定（急降下中）
	if (mPhase == Phase::RealDragonSlamDown && mbSphereActive && playerAlive)
	{
		float hitR = mSphereRadius * 0.85f;
		if (mSphereX + hitR > pLeft && mSphereX - hitR < pRight &&
		    mSphereY + hitR > pTop  && mSphereY - hitR < pBottom)
		{
			player->TakeDamage(attackDamage, DeathReason::DeathType::DragonExplosion);
		}
	}

	// 2-2. 巨大風玉の最後の大爆発判定：
	// マグマがたまっていた場所（X=3852..5220）から天井の高さ（Y=180..1420）まで全域に当たり判定！
	if (mPhase == Phase::SlamImpactFinish && mPhaseTimer <= 52 && playerAlive)
	{
		const float PIT_HIT_LEFT_X  = 3850.0f;
		const float PIT_HIT_RIGHT_X = 5220.0f;
		const float CEIL_HIT_TOP_Y  = 160.0f;
		const float PIT_HIT_BOT_Y   = PIT_FLOOR_Y + 50.0f;

		if (pRight > PIT_HIT_LEFT_X && pLeft < PIT_HIT_RIGHT_X &&
		    pBottom > CEIL_HIT_TOP_Y && pTop < PIT_HIT_BOT_Y)
		{
			player->TakeDamage(9999, DeathReason::DeathType::DragonExplosion);
		}
	}

	// 4. 分身から風玉へ飛び込む2D Effekseer風魔法弾の追従更新
	for (auto it = mCrescentBlades.begin(); it != mCrescentBlades.end(); )
	{
		it->t += it->speed;
		float t = it->t;
		it->x = it->startX + (mSphereX - it->startX) * t;
		it->y = it->startY + (mSphereY - it->startY) * t + std::sin(t * 3.14159265f) * -20.0f;
		float rot = std::atan2(mSphereY - it->y, mSphereX - it->x);
		if (it->efkHandle != -1)
		{
			Stage3EffectManager::SetTransform(it->efkHandle, it->x, it->y, 1.45f * (1.0f - t * 0.25f), rot);
		}
		if (it->t >= 1.0f)
		{
			if (it->efkHandle != -1)
			{
				Stage3EffectManager::Stop(it->efkHandle);
				it->efkHandle = -1;
			}
			it = mCrescentBlades.erase(it);
		}
		else
		{
			++it;
		}
	}

	// 6-2. プレイヤー浮遊中の緑色2D Effekseer引き寄せ風エフェクト（風玉の方向へ勢いよく流れる）
	if (playerAlive && player->IsWindFloatMode())
	{
		VECTOR pPos = player->GetPosition();
		float dx = mSphereX - pPos.x;
		float dy = mSphereY - pPos.y;
		float pullRot = std::atan2(dy, dx);
		float fx = pPos.x + std::cos(pullRot) * 28.0f;
		float fy = pPos.y + std::sin(pullRot) * 28.0f;
		float fScale = 0.56f;
		if (!Stage3EffectManager::IsPlaying(mPlayerFloatWindEfk))
		{
			mPlayerFloatWindEfk = Stage3EffectManager::Play(
				Stage3EffectType::SlashWind, fx, fy, fScale, 1.45f, pullRot, 60, 255, 195, 215
			);
		}
		else
		{
			Stage3EffectManager::SetTransform(mPlayerFloatWindEfk, fx, fy, fScale, pullRot);
		}
	}
	else if (mPlayerFloatWindEfk != -1)
	{
		Stage3EffectManager::Stop(mPlayerFloatWindEfk);
		mPlayerFloatWindEfk = -1;
	}

	// 5-2. 側側サイド電撃の更新と風玉による吸引・吸収
	for (auto it = mSideLightnings.begin(); it != mSideLightnings.end(); )
	{
		if (!it->active)
		{
			it = mSideLightnings.erase(it);
			continue;
		}

		// 予兆段階：電撃を構えている間（約0.92秒間）
		// 【最重要】プレイヤーの現在位置へは一切追尾しない！構え開始時の目標地点に完全固定。
		// プレイヤーは赤レーザー線と照準を見て、横や上へ移動することで確実に回避可能！
		if (it->warnTimer > 0)
		{
			it->warnTimer--;
			++it;
			continue;
		}

		if (!it->hitGround)
		{
			// 直進飛行（プレイヤーへの追尾は一切なし）
			// 中央の巨大風玉の引力によって電気の球が風玉の中心へ引き寄せられ、接触すると吸収される
			if (mbSphereActive && mSphereRadius > 20.0f)
			{
				float toSx = mSphereX - it->x;
				float toSy = mSphereY - it->y;
				float dSphere = std::sqrt(toSx * toSx + toSy * toSy);

				// 風玉への吸い込み引力（風玉中心方向へ）
				if (dSphere > 1.0f && dSphere < 750.0f)
				{
					float pull = (1.0f - (dSphere / 750.0f)) * 1.8f;
					it->vx += (toSx / dSphere) * pull;
					it->vy += (toSy / dSphere) * pull;
				}

				// 風玉に接触すると吸収！風玉が帯電してビリビリを放つ
				if (dSphere <= (mSphereRadius + 45.0f))
				{
					mSphereElectrified = true;
					mSphereElectricityLevel++;
					mSphereElectricTimer = 220;

					if (it->efkHandle != -1)
					{
						Stage3EffectManager::Stop(it->efkHandle);
						it->efkHandle = -1;
					}
					it->active = false;
					it = mSideLightnings.erase(it);
					continue;
				}
			}

			it->x += it->vx;
			it->y += it->vy;

			// 2D Effekseerの電撃エフェクトを短くコンパクトに飛翔方向へ追従させる
			float lRot = std::atan2(it->vy, it->vx);
			if (!Stage3EffectManager::IsPlaying(it->efkHandle))
			{
				it->efkHandle = Stage3EffectManager::Play(
					Stage3EffectType::Thunder, it->x, it->y, 0.42f, 1.8f, lRot, 90, 235, 255, 255
				);
			}
			else
			{
				Stage3EffectManager::SetTransform(it->efkHandle, it->x, it->y, 0.42f, lRot);
			}

			// 飛翔中の電撃全体（短くした電撃の横幅に沿った線分当たり判定）
			if (playerAlive)
			{
				VECTOR pPos = player->GetPosition();
				float spdLen = std::sqrt(it->vx * it->vx + it->vy * it->vy);
				float dirX   = (spdLen > 0.01f) ? (it->vx / spdLen) : 1.0f;
				float dirY   = (spdLen > 0.01f) ? (it->vy / spdLen) : 0.0f;
				float halfSpan = 42.0f;
				float ax = it->x - dirX * halfSpan;
				float ay = it->y - dirY * halfSpan;
				float bx = it->x + dirX * halfSpan;
				float by = it->y + dirY * halfSpan;
				float abx = bx - ax;
				float aby = by - ay;
				float abLenSq = abx * abx + aby * aby;
				float proj = (abLenSq > 0.01f) ? (((pPos.x - ax) * abx + (pPos.y - ay) * aby) / abLenSq) : 0.5f;
				if (proj < 0.0f) proj = 0.0f;
				if (proj > 1.0f) proj = 1.0f;
				float cx = ax + abx * proj;
				float cy = ay + aby * proj;
				float hitR = 30.0f;
				if (cx + hitR > pLeft && cx - hitR < pRight &&
				    cy + hitR > pTop  && cy - hitR < pBottom)
				{
					player->TakeDamage(attackDamage, DeathReason::DeathType::NormalDeath);
					if (it->efkHandle != -1)
					{
						Stage3EffectManager::Stop(it->efkHandle);
						it->efkHandle = -1;
					}
					it->hitGround  = true;
					it->burstTimer = 0;
				}
			}

			// 地面・天井・外壁に到達
			it->timer++;
			if (it->y >= PIT_FLOOR_Y || it->y <= 460.0f ||
			    it->x <= 2990.0f || it->x >= 5710.0f || it->timer >= 90)
			{
				if (it->efkHandle != -1)
				{
					Stage3EffectManager::Stop(it->efkHandle);
					it->efkHandle = -1;
				}
				it->hitGround  = true;
				it->burstTimer = 0;
			}
		}
		else
		{
			it->burstTimer++;
			if (playerAlive && it->burstTimer <= 6)
			{
				float bR = 60.0f;
				if (it->x + bR > pLeft && it->x - bR < pRight &&
				    it->y + bR > pTop  && it->y - bR < pBottom)
				{
					player->TakeDamage(attackDamage, DeathReason::DeathType::NormalDeath);
				}
			}

			if (it->burstTimer >= 18)
			{
				it = mSideLightnings.erase(it);
				continue;
			}
		}

		++it;
	}

	// 13. 地面を這う電撃（カタクリズム後、中心から崖を登り足場へ流れ、壁で消滅）
	for (auto it = mGroundShocks.begin(); it != mGroundShocks.end(); )
	{
		if (!it->active)
		{
			it = mGroundShocks.erase(it);
			continue;
		}

		it->life++;

		// 移動処理
		if (it->dir == -1) // 左方向へ流れる電撃
		{
			if (it->stage == 0) // 底面：中心から左崖へ (X: 4536 -> 3875, Y: 1380)
			{
				it->x -= it->speed;
				it->y = PIT_FLOOR_Y;
				if (it->x <= MAGMA_PIT_LEFT_X)
				{
					it->x = MAGMA_PIT_LEFT_X;
					it->stage = 1; // 崖登り開始！
				}
			}
			else if (it->stage == 1) // 崖登り：底面から左足場へ (Y: 1380 -> 1116, X: 3875)
			{
				it->x = MAGMA_PIT_LEFT_X;
				it->y -= it->speed;
				if (it->y <= LEFT_LEDGE_FLOOR_Y)
				{
					it->y = LEFT_LEDGE_FLOOR_Y;
					it->stage = 2; // 左足場到達！
				}
			}
			// 【Stage3.png 5724x1692拡大版 対応】左外壁(X=3016)まで地面這い電撃を到達させる
			else if (it->stage == 2) // 左足場：崖上から左外壁へ (X: 3690 -> 3016, Y: 1262)
			{
				it->x -= it->speed;
				it->y = LEFT_LEDGE_FLOOR_Y;
				if (it->x <= 3016.0f) // 壁にぶつかると消滅！
				{
					if (it->efkHandle != -1)
					{
						Stage3EffectManager::Stop(it->efkHandle);
						it->efkHandle = -1;
					}
					it->active = false;
					it = mGroundShocks.erase(it);
					continue;
				}
			}
		}
		else // 右方向へ流れる電撃 (dir == +1)
		{
			if (it->stage == 0) // 底面：中心から右崖へ (X: 4536 -> 5195, Y: 1380)
			{
				it->x += it->speed;
				it->y = PIT_FLOOR_Y;
				if (it->x >= MAGMA_PIT_RIGHT_X)
				{
					it->x = MAGMA_PIT_RIGHT_X;
					it->stage = 1; // 崖登り開始！
				}
			}
			else if (it->stage == 1) // 崖登り：底面から右足場へ (Y: 1380 -> 1152, X: 5195)
			{
				it->x = MAGMA_PIT_RIGHT_X;
				it->y -= it->speed;
				if (it->y <= RIGHT_LEDGE_FLOOR_Y)
				{
					it->y = RIGHT_LEDGE_FLOOR_Y;
					it->stage = 2; // 右足場到達！
				}
			}
			// 【Stage3.png 5724x1692拡大版 対応】右外壁(X=5690)まで地面這い電撃を到達させる
			else if (it->stage == 2) // 右足場：崖上から右外壁へ (X: 4946 -> 5690, Y: 1289)
			{
				it->x += it->speed;
				it->y = RIGHT_LEDGE_FLOOR_Y;
				if (it->x >= 5690.0f) // 壁にぶつかると消滅！
				{
					if (it->efkHandle != -1)
					{
						Stage3EffectManager::Stop(it->efkHandle);
						it->efkHandle = -1;
					}
					it->active = false;
					it = mGroundShocks.erase(it);
					continue;
				}
			}
		}

		// 2D Effekseerの地面這い電撃を現在位置・進行方向へ追従させる
		float gRot = (it->stage == 1) ? -1.5707963f : (it->dir < 0 ? 3.14159265f : 0.0f);
		float gx   = it->x;
		float gy   = it->y - 16.0f;
		if (!Stage3EffectManager::IsPlaying(it->efkHandle))
		{
			it->efkHandle = Stage3EffectManager::Play(
				Stage3EffectType::Thunder, gx, gy, 0.50f, 1.8f, gRot, 85, 235, 255, 255
			);
		}
		else
		{
			Stage3EffectManager::SetTransform(it->efkHandle, gx, gy, 0.50f, gRot);
		}

		// 地面・壁を這う電撃エフェクトの見た目と一致する当たり判定
		if (playerAlive)
		{
			VECTOR pPos = player->GetPosition();
			float hitCenterX = (it->stage == 1) ? (it->x - it->dir * 12.0f) : (it->x - it->dir * 20.0f);
			float hitCenterY = (it->stage == 1) ? (it->y + 20.0f) : (it->y - 18.0f);
			float dx = std::abs(pPos.x - hitCenterX);
			float dy = std::abs(pPos.y - hitCenterY);
			float hitW = (it->stage == 1) ? 34.0f : 62.0f;
			float hitH = (it->stage == 1) ? 62.0f : 36.0f;
			if (dx < hitW && dy < hitH)
			{
				player->TakeDamage(attackDamage, DeathReason::DeathType::NormalDeath);
			}
		}

		++it;
	}

	for (auto it = mExplosionPlumes.begin(); it != mExplosionPlumes.end();)
	{
		it->life++;
		if (it->life >= it->maxLife)
		{
			it = mExplosionPlumes.erase(it);
			continue;
		}
		it->x += it->vx;
		it->y += it->vy;
		it->vx *= 0.93f;
		it->vy *= 0.93f;
		it->scale += it->scaleVel;
		it->angle += it->angleVel;
		++it;
	}

	for (auto it = mExplosionSparks.begin(); it != mExplosionSparks.end();)
	{
		it->life++;
		if (it->life >= it->maxLife)
		{
			it = mExplosionSparks.erase(it);
			continue;
		}
		it->prevX = it->x;
		it->prevY = it->y;
		// パラパラと広く揺れながらゆっくり舞い落ちる火の粉
		float flutter = std::sin(static_cast<float>(it->life) * 0.13f + it->y * 0.025f) * 1.45f;
		it->x    += it->vx + flutter;
		it->y    += it->vy;
		it->vx   *= 0.96f;
		it->vy   += 0.08f;
		if (it->vy > 1.55f) it->vy = 1.55f;
		++it;
	}
}

void Stage3DragonUltimateWindSlamAttack::Draw(
	BlockMap* blockMap,
	Player* player,
	const VECTOR& bossPos,
	float facingDir,
	float drawScale,
	int realSpriteHandle,
	float footOffset,
	float enemyHeight) const
{
	if (mPhase == Phase::Inactive || blockMap == nullptr)
	{
		return;
	}
	bool onlyTrailingExplosion = (mPhase == Phase::Finished);
	if (onlyTrailingExplosion && mExplosionPlumes.empty() && mExplosionSparks.empty())
	{
		return;
	}

	int scrollX = blockMap->GetScrollX() + blockMap->mShakeX;
	int scrollY = blockMap->GetScrollY() + blockMap->mShakeY;

	if (!onlyTrailingExplosion)
	{

	// -----------------------------------------------------------------
	// 2. 勇者の風浮遊エフェクト（落ち着いた足元の緑風＋風玉へ向かう引き寄せ気流）
	// -----------------------------------------------------------------
	if (player != nullptr && !player->GetIsDead() && player->IsWindFloatMode())
	{
		VECTOR pPos = player->GetPosition();
		int px = static_cast<int>(pPos.x) - scrollX;
		int py = static_cast<int>(pPos.y) - scrollY;
		int footY = py + 42;
		float timeF = static_cast<float>(mPhaseTimer) * 0.06f;

		// 1) 足元の下に静かに広がる横向き（扁平）の緑風クッション
		if (mSphereTexWindSoft != -1)
		{
			double ex = 64.0 / static_cast<double>(mSphereTexWindSoftSize);
			double ey = ex * 0.28;
			int cx = mSphereTexWindSoftSize / 2;
			SetDrawBright(50, 235, 150);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 105);
			DrawRotaGraph3(px, footY + 4, cx, cx, ex, ey, timeF, mSphereTexWindSoft, TRUE);
		}

		// 2) プレイヤーの周囲から風玉へ向かって収束しながら吸い込まれる7本の高速気流ストリーム
		float dx = mSphereX - pPos.x;
		float dy = mSphereY - pPos.y;
		float dist = std::sqrt(dx * dx + dy * dy);
		if (dist > 20.0f)
		{
			float nx = dx / dist;
			float ny = dy / dist;
			float perpX = -ny;
			float perpY =  nx;
			float maxSpan = (dist < 260.0f) ? dist : 260.0f;
			SetDrawBright(255, 255, 255);
			for (int s = 0; s < 7; ++s)
			{
				int cycle = (mPhaseTimer * 6 + s * 29) % 100;
				float t = static_cast<float>(cycle) / 100.0f;
				float travel = -45.0f + t * (maxSpan + 45.0f);
				float funnel = 1.0f - t * 0.72f; // 風玉に近づくほど細く収束
				float lat    = static_cast<float>(s - 3) * 14.0f * funnel;
				float len    = 36.0f + t * 28.0f;
				float bx = static_cast<float>(px) + perpX * lat + nx * travel;
				float by = static_cast<float>(py) + perpY * lat + ny * travel;
				float tx = bx + nx * len;
				float ty = by + ny * len;
				int sAlpha = static_cast<int>(std::sin(t * 3.14159265f) * 215.0f);
				if (sAlpha <= 0) continue;
				SetDrawBlendMode(DX_BLENDMODE_ADD, sAlpha);
				DrawLine(static_cast<int>(bx), static_cast<int>(by), static_cast<int>(tx), static_cast<int>(ty), GetColor(45, 240, 175), 3);
				DrawLine(static_cast<int>(bx), static_cast<int>(by), static_cast<int>(tx), static_cast<int>(ty), GetColor(200, 255, 235), 1);
			}
		}
		SetDrawBright(255, 255, 255);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// -----------------------------------------------------------------
	// 3. 分身2匹（神々しい翡翠オーラ＋三日月烈風ブレード）
	// -----------------------------------------------------------------
	if (mCloneAlpha > 0 && !mCloneFlyAnim.empty())
	{
		int cloneHandle = mCloneFlyAnim[mCloneAnimFrame % mCloneFlyAnim.size()];
		float cloneFootOffset = 57.0f;

		for (int c = 0; c < 2; ++c)
		{
			float cx   = (c == 0) ? mLeftCloneX  : mRightCloneX;
			float cy   = (c == 0) ? mLeftCloneY  : mRightCloneY;
			bool  rev  = (c == 0);

			int sx     = static_cast<int>(cx) - scrollX;
			float fWY  = cy + (enemyHeight * 0.5f);
			int sy     = static_cast<int>(fWY - (cloneFootOffset * drawScale)) - scrollY;

			// 分身の外周発光オーラ
			SetDrawBlendMode(DX_BLENDMODE_ADD, (mCloneAlpha * 120) / 255);
			SetDrawBright(80, 255, 195);
			DrawRotaGraph(sx, sy, drawScale * 1.08f, 0.0, cloneHandle, TRUE, rev);

			// 分身本体
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, mCloneAlpha);
			SetDrawBright(220, 255, 240);
			DrawRotaGraph(sx, sy, drawScale, 0.0, cloneHandle, TRUE, rev);
		}
		SetDrawBright(255, 255, 255);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// 3-2. 足場側サイド電撃（予兆レーザー線・飛翔稲妻ボルト・着弾放電ドーム）
	for (size_t i = 0; i < mSideLightnings.size(); ++i)
	{
		const SideLightningStrike& s = mSideLightnings[i];
		if (!s.active) continue;

		int sx = static_cast<int>(s.startX) - scrollX;
		int sy = static_cast<int>(s.startY) - scrollY;
		int tx = static_cast<int>(s.targetX) - scrollX;
		int ty = static_cast<int>(s.targetY) - scrollY;

		// 予兆：壁の放電スパークとプレイヤー空中追尾ロックオン照準
		if (s.warnTimer > 0)
		{
			int a = ((55 - s.warnTimer) * 255) / 55;
			if (a < 0) a = 0; if (a > 255) a = 255;
			SetDrawBlendMode(DX_BLENDMODE_ADD, a);
			// 壁の放電核
			DrawCircle(sx, sy, 18, GetColor(255, 60, 60), TRUE);
			DrawCircle(sx, sy, 9,  GetColor(255, 240, 200), TRUE);

			// プレイヤー追尾中のロックオン照準（リング＆十字レティクル）
			DrawOval(tx, ty, 38, 26, GetColor(255, 60, 60), FALSE);
			DrawOval(tx, ty, 20, 14, GetColor(255, 230, 100), FALSE);
			DrawLine(tx - 30, ty, tx + 30, ty, GetColor(255, 80, 80), 2);
			DrawLine(tx, ty - 22, tx, ty + 22, GetColor(255, 80, 80), 2);

			// 照準レーザー電光
			DrawLine(sx, sy, tx, ty, GetColor(255, 60, 60), 2);
			if (s.warnTimer % 4 < 2)
			{
				DrawLine(sx, sy, tx, ty, GetColor(255, 255, 220), 1);
			}
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		// 飛翔中：短いギザギザ電撃ボルト（2D Effekseerと連動）
		else if (!s.hitGround)
		{
			int arcIdx = ((mPhaseTimer / 2) + static_cast<int>(i) * 3) & 7;
			int arcH   = mSphereTexThunderArcs[arcIdx];
			if (arcH != -1)
			{
				int cx = static_cast<int>(s.x) - scrollX;
				int cy = static_cast<int>(s.y) - scrollY;
				float rot = std::atan2(s.vy, s.vx);
				SetDrawBright(65, 220, 255);
				SetDrawBlendMode(DX_BLENDMODE_ADD, 235);
				DrawRotaGraph(cx, cy, 0.72, rot, arcH, TRUE);
				SetDrawBright(220, 252, 255);
				SetDrawBlendMode(DX_BLENDMODE_ADD, 220);
				DrawRotaGraph(cx, cy, 0.60, rot, arcH, TRUE);
				SetDrawBright(255, 255, 255);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			}
		}
	}

	// 3-3. 分身から玉へ飛び込む風渦トレイル（2D EffekseerのSlashWindと連動）
	if (mSphereTexWindVortex != -1)
	{
		for (size_t i = 0; i < mCrescentBlades.size(); ++i)
		{
			const CrescentBlade& b = mCrescentBlades[i];
			if (!b.active) continue;

			int bx = static_cast<int>(b.x) - scrollX;
			int by = static_cast<int>(b.y) - scrollY;
			int alpha = static_cast<int>(std::sin(b.t * 3.14159265f) * 210.0f);
			if (alpha <= 0) continue;

			float sc = (78.0f * b.scale * (1.0f - b.t * 0.25f)) / static_cast<float>(mSphereTexWindVortexSize);
			float rot = (b.fromLeft ? 1.0f : -1.0f) * (b.t * 10.0f);
			SetDrawBright(70, 255, 210);
			SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
			DrawRotaGraph(bx, by, sc, rot, mSphereTexWindVortex, TRUE);
		}
		SetDrawBright(255, 255, 255);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// -----------------------------------------------------------------
	// 4. 吸い込まれるマグマ塊（彗星コメット状の灼熱トレイル）
	// -----------------------------------------------------------------
	for (size_t i = 0; i < mSuckedMagma.size(); ++i)
	{
		const SuckedMagmaGlob& g = mSuckedMagma[i];
		if (!g.active) continue;

		if (g.warnTimer > 0)
		{
			int wx = static_cast<int>(g.startX) - scrollX;
			int wy = static_cast<int>(g.startY) - scrollY;
			SetDrawBlendMode(DX_BLENDMODE_ADD, 230);
			DrawOval(wx, wy, 36 - g.warnTimer, 12, GetColor(255, 120, 20), TRUE);
			DrawOval(wx, wy, 18, 5, GetColor(255, 240, 160), TRUE);
			DrawLine(wx, wy, wx, wy - 90, GetColor(255, 160, 40), 3);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			continue;
		}

		int gx = static_cast<int>(g.x) - scrollX;
		int gy = static_cast<int>(g.y) - scrollY;
		int fIdx = ((mPhaseTimer / 2) + static_cast<int>(i)) & 3;
		int fTex = mExpTexFire[fIdx];
		int fTexTrail = mExpTexFire[(fIdx + 1) & 3];
		if (fTex != -1)
		{
			float rot = std::atan2(g.y - g.prevY[0], g.x - g.prevX[0]) + 1.5707963f;
			double sc = (g.radius * 2.8) / 256.0;
			int tx = static_cast<int>(g.prevX[2]) - scrollX;
			int ty = static_cast<int>(g.prevY[2]) - scrollY;
			if (fTexTrail != -1)
			{
				SetDrawBright(255, 65, 10);
				SetDrawBlendMode(DX_BLENDMODE_ADD, 155);
				DrawRotaGraph(tx, ty, sc * 0.85, rot, fTexTrail, TRUE);
			}
			SetDrawBright(255, 110, 20);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 215);
			DrawRotaGraph(gx, gy, sc, rot, fTex, TRUE);
			SetDrawBright(255, 220, 95);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 180);
			DrawRotaGraph(gx, gy, sc * 0.62, rot, fTex, TRUE);
			SetDrawBright(255, 255, 255);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	// 永続的に回転し、小->大の成長が途切れず視認できる螺旋風炎球（白い爆発なし・周囲をビリビリ電撃が走る）
	if (mbSphereActive && mSphereRadius > 4.0f)
	{
		int   cx    = static_cast<int>(mSphereX) - scrollX;
		int   cy    = static_cast<int>(mSphereY) - scrollY;
		float diam  = mSphereRadius * 2.0f;
		float timeA = static_cast<float>(mPhaseTimer) * 0.14f;

		// 1. 濃い風のベース渦（常に存在して大きさがはっきり伝わる）
		if (mSphereTexWindSoft != -1)
		{
			float sSoft = (diam * 1.10f) / static_cast<float>(mSphereTexWindSoftSize);
			SetDrawBright(35, 230, 190);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 215);
			DrawRotaGraph(cx, cy, sSoft, timeA * 0.65f, mSphereTexWindSoft, TRUE);
		}

		// 2. 螺旋丸風の高速回転スワール（2層重ねで立体感のある風の渦に）
		if (mSphereTexWindVortex != -1)
		{
			float sVortex1 = (diam * 1.15f) / static_cast<float>(mSphereTexWindVortexSize);
			float sVortex2 = (diam * 0.88f) / static_cast<float>(mSphereTexWindVortexSize);
			SetDrawBright(65, 255, 220);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 230);
			DrawRotaGraph(cx, cy, sVortex1, timeA, mSphereTexWindVortex, TRUE);
			SetDrawBright(120, 255, 235);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 185);
			DrawRotaGraph(cx, cy, sVortex2, timeA * 1.60f + 1.8f, mSphereTexWindVortex, TRUE);
		}

		// 3. マグマ吸引が進むほど炎の渦が外周を覆う
		if (mMagmaAbsorbRatio > 0.02f)
		{
			int flameA = static_cast<int>(220.0f * mMagmaAbsorbRatio);
			if (mSphereTexWindSoft != -1)
			{
				float sFlameSoft = (diam * 1.16f) / static_cast<float>(mSphereTexWindSoftSize);
				SetDrawBright(255, 80, 15);
				SetDrawBlendMode(DX_BLENDMODE_ADD, flameA);
				DrawRotaGraph(cx, cy, sFlameSoft, -timeA * 0.8f, mSphereTexWindSoft, TRUE);
			}
			if (mSphereTexWindVortex != -1)
			{
				float sFlameVortex = (diam * 1.22f) / static_cast<float>(mSphereTexWindVortexSize);
				SetDrawBright(255, 130, 25);
				SetDrawBlendMode(DX_BLENDMODE_ADD, flameA);
				DrawRotaGraph(cx, cy, sFlameVortex, -timeA * 1.35f + 0.7f, mSphereTexWindVortex, TRUE);
			}
		}

		// 4. 玉を包み込む電撃スパーク（ElectricBall）
		if (mSphereTexElectric != -1)
		{
			int elecA = mSphereElectrified ? 200 : (95 + static_cast<int>(80.0f * mMagmaAbsorbRatio));
			float sElec = (diam * 1.18f) / static_cast<float>(mSphereTexElectricSize);
			SetDrawBright(90, 225, 255);
			SetDrawBlendMode(DX_BLENDMODE_ADD, elecA);
			DrawRotaGraph(cx, cy, sElec, timeA * 2.3f, mSphereTexElectric, TRUE);
		}

		// 5. 玉の周囲・表面をまばらに走るシャープなビリビリ電撃アーク（先端を細く、間引いて放電感を出す）
		const int arcCount = 6;
		int stepTick = mPhaseTimer / 2;
		for (int i = 0; i < arcCount; ++i)
		{
			int seed   = stepTick * 29 + i * 43;
			int arcIdx = (seed & 0x7FFF) % 8;
			int arcH   = mSphereTexThunderArcs[arcIdx];
			if (arcH == -1) continue;

			// まばらに明滅させる
			if (((seed / 2) % 3) == 0) continue;

			bool  isSurface = (i >= 4);
			float baseAngle = (static_cast<float>(i) / static_cast<float>(arcCount)) * 6.2831853f
			                + static_cast<float>(stepTick) * (isSurface ? -0.35f : 0.28f)
			                + static_cast<float>((seed % 11) - 5) * 0.14f;
			float orbitR    = isSurface
			                ? (mSphereRadius * (0.25f + static_cast<float>(seed % 4) * 0.14f))
			                : (mSphereRadius * (0.86f + static_cast<float>(seed % 4) * 0.06f));
			int   ax        = cx + static_cast<int>(std::cos(baseAngle) * orbitR);
			int   ay        = cy + static_cast<int>(std::sin(baseAngle) * orbitR);
			float tangent   = isSurface
			                ? (baseAngle + 0.7f + static_cast<float>(seed % 7) * 0.4f)
			                : (baseAngle + 1.5707963f + (((seed % 2) == 0) ? 0.16f : -0.16f));
			float arcScale  = (mSphereRadius * (0.82f + static_cast<float>(seed % 4) * 0.12f)) / 128.0f;
			if (arcScale < 0.22f) arcScale = 0.22f;

			SetDrawBright(60, 210, 255);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 230);
			DrawRotaGraph(ax, ay, arcScale * 1.12f, tangent, arcH, TRUE);

			SetDrawBright(220, 252, 255);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 210);
			DrawRotaGraph(ax, ay, arcScale * 0.85f, tangent, arcH, TRUE);
		}

		// 6. 先端に向かって細くなる、まばらで鋭利な放電スパーク（四角い太線なし）
		int forkCount = mSphereElectrified ? 5 : 4;
		for (int f = 0; f < forkCount; ++f)
		{
			int fSeed = stepTick * 47 + f * 61;
			if ((fSeed % 2) == 0) continue;
			float fAng = (static_cast<float>(f) / static_cast<float>(forkCount)) * 6.2831853f
			           + static_cast<float>(fSeed % 23) * 0.27f;
			float r0 = mSphereRadius * 0.84f;
			float r1 = mSphereRadius * (1.03f + static_cast<float>(fSeed % 3) * 0.05f);
			float r2 = mSphereRadius * (1.20f + static_cast<float>((fSeed / 3) % 3) * 0.06f);
			float a1 = fAng + (((fSeed % 2) == 0) ? 0.16f : -0.16f);
			float a2 = fAng + ((((fSeed / 2) % 2) == 0) ? -0.12f : 0.12f);
			int x0 = cx + static_cast<int>(std::cos(fAng) * r0);
			int y0 = cy + static_cast<int>(std::sin(fAng) * r0);
			int x1 = cx + static_cast<int>(std::cos(a1)   * r1);
			int y1 = cy + static_cast<int>(std::sin(a1)   * r1);
			int x2 = cx + static_cast<int>(std::cos(a2)   * r2);
			int y2 = cy + static_cast<int>(std::sin(a2)   * r2);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 230);
			DrawLine(x0, y0, x1, y1, GetColor(70, 220, 255), 2);
			DrawLine(x1, y1, x2, y2, GetColor(215, 252, 255), 1);
		}

		SetDrawBright(255, 255, 255);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// -----------------------------------------------------------------
	// 6. 分身突進攻撃ゆずりの超ド迫力縦突進エフェクト（スピードライン・斬撃コーン・3連巨爪・残像・閃光）
	// -----------------------------------------------------------------
	if ((mPhase == Phase::RealDragonSlamWindup || mPhase == Phase::RealDragonSlamDown) && realSpriteHandle != -1)
	{
		int realSX   = static_cast<int>(bossPos.x) - scrollX;
		float fWY    = bossPos.y + (enemyHeight * 0.5f);
		int realSY   = static_cast<int>(fWY - (footOffset * drawScale)) - scrollY;
		int sphereSY = static_cast<int>(mSphereY) - scrollY;
		bool realRev = (facingDir > 0.0f);



		if (mPhase == Phase::RealDragonSlamWindup && mPhaseTimer >= 23 && mPhaseTimer <= 30)
		{
			// 画面全体に一瞬走る紅蓮の緊張感フラッシュ
			SetDrawBlendMode(DX_BLENDMODE_ADD, 45 - (mPhaseTimer - 23) * 5);
			DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(255, 30, 20), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	} // !onlyTrailingExplosion

	// 7. 着弾時の大爆発（白飛び・魔法陣ゼロ、上空へ高く噴き上がる爆煙・紅蓮炎・パラパラ散ってゆっくりフェードする火の粉）
	// -----------------------------------------------------------------
	if (!mExplosionPlumes.empty() || !mExplosionSparks.empty())
	{
		// 1) 深紅・黒褐色の爆煙（ALPHA描画のみなので絶対に白飛びしない）
		for (const auto& p : mExplosionPlumes)
		{
			if (!p.isSmoke) continue;
			int tex = mExpTexSmoke[p.texIdx & 3];
			if (tex == -1) continue;
			float t = static_cast<float>(p.life) / static_cast<float>(p.maxLife);
			int alpha = static_cast<int>((1.0f - t * t) * 190.0f);
			if (alpha <= 0) continue;
			int px = static_cast<int>(p.x) - scrollX;
			int py = static_cast<int>(p.y) - scrollY;
			SetDrawBright(p.r, p.g, p.b);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawRotaGraph(px, py, p.scale, p.angle, tex, TRUE);
		}
		// 2) 上空へ大きく立ち昇る14本の紅蓮の炎（低アルファ＋B=5で白飛び防止）
		for (const auto& p : mExplosionPlumes)
		{
			if (p.isSmoke) continue;
			int tex = mExpTexFire[p.texIdx & 3];
			if (tex == -1) continue;
			float t = static_cast<float>(p.life) / static_cast<float>(p.maxLife);
			int alpha = static_cast<int>((1.0f - t) * 105.0f);
			if (alpha <= 0) continue;
			int px = static_cast<int>(p.x) - scrollX;
			int py = static_cast<int>(p.y) - scrollY;
			SetDrawBright(p.r, p.g, p.b);
			SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
			DrawRotaGraph(px, py, p.scale, p.angle, tex, TRUE);
		}
		SetDrawBright(255, 255, 255);
		// 3) パラパラと舞い落ちながらゆっくりフェードする火の粉
		for (size_t i = 0; i < mExplosionSparks.size(); ++i)
		{
			const ExplosionSpark& sp = mExplosionSparks[i];
			float t = static_cast<float>(sp.life) / static_cast<float>(sp.maxLife);
			float fade = 1.0f - t;
			float twinkle = 0.72f + 0.28f * std::sin(static_cast<float>(sp.life) * 0.35f + static_cast<float>(i));
			int alpha = static_cast<int>(fade * fade * twinkle * 235.0f);
			if (alpha <= 0) continue;
			int sx  = static_cast<int>(sp.x) - scrollX;
			int sy  = static_cast<int>(sp.y) - scrollY;
			unsigned int col = GetColor(sp.r, sp.g, sp.b);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawCircle(sx, sy, sp.size, col, TRUE);
			if (sp.size >= 4)
			{
				DrawCircle(sx, sy, sp.size - 2, GetColor(255, 240, 150), TRUE);
			}
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// -----------------------------------------------------------------
	// 7. 地面・崖・足場をピタッと伝って走る高圧電撃波（2D Effekseer＋地形密着アーク）
	// -----------------------------------------------------------------
	for (size_t i = 0; i < mGroundShocks.size(); ++i)
	{
		const GroundCrawlingShock& gs = mGroundShocks[i];
		if (!gs.active) continue;

		int gsx = static_cast<int>(gs.x) - scrollX;
		int gsy = static_cast<int>(gs.y) - scrollY;
		int stepTick = mPhaseTimer / 2;
		int seed = stepTick * 37 + static_cast<int>(i) * 53;

		float baseRot = (gs.stage == 1) ? -1.5707963f : (gs.dir < 0 ? 3.14159265f : 0.0f);
		int arcIdx1 = (seed & 0x7FFF) % 8;
		int arcIdx2 = ((seed + 5) & 0x7FFF) % 8;
		int arcH1 = mSphereTexThunderArcs[arcIdx1];
		int arcH2 = mSphereTexThunderArcs[arcIdx2];

		// 1) 地形面にピタッと沿う2連のギザギザ電撃テクスチャ（前方＋後続トレイル）
		if (arcH1 != -1 && arcH2 != -1)
		{
			int bx1 = gsx, by1 = gsy;
			int bx2 = gsx, by2 = gsy;
			if (gs.stage == 1)
			{
				// 垂直の崖面に密着して駆け上がる
				bx1 = gsx - static_cast<int>(gs.dir * 10.0f);
				by1 = gsy + 8;
				bx2 = gsx - static_cast<int>(gs.dir * 12.0f);
				by2 = gsy + 58;
			}
			else
			{
				// 水平の床・足場に密着して走る
				bx1 = gsx - static_cast<int>(gs.dir * 12.0f);
				by1 = gsy - 10;
				bx2 = gsx - static_cast<int>(gs.dir * 66.0f);
				by2 = gsy - 8;
			}

			SetDrawBright(50, 210, 255);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 235);
			DrawRotaGraph(bx1, by1, 1.05, baseRot, arcH1, TRUE);
			DrawRotaGraph(bx2, by2, 0.85, baseRot, arcH2, TRUE);

			SetDrawBright(225, 252, 255);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 215);
			DrawRotaGraph(bx1, by1, 0.85, baseRot, arcH1, TRUE);
			DrawRotaGraph(bx2, by2, 0.65, baseRot, arcH2, TRUE);
			SetDrawBright(255, 255, 255);
		}

		// 2) 地面・壁の表面を走る細かな這い電撃＆低い跳ねスパーク（高さ10～24pxで地面を伝う）
		SetDrawBlendMode(DX_BLENDMODE_ADD, 230);
		const int segs = 6;
		int px[segs + 1];
		int py[segs + 1];
		for (int k = 0; k <= segs; ++k)
		{
			float frac = static_cast<float>(k) / static_cast<float>(segs);
			float dist = -115.0f + frac * 135.0f; // 後方-115pxから前方+20pxまで地形を伝う
			float hop  = (k == 0 || k == segs) ? 2.0f : (6.0f + static_cast<float>((seed + k * 13) % 18));
			if (gs.stage == 1)
			{
				px[k] = gsx - static_cast<int>(gs.dir * hop);
				py[k] = gsy - static_cast<int>(dist);
			}
			else
			{
				px[k] = gsx + static_cast<int>(gs.dir * dist);
				py[k] = gsy - static_cast<int>(hop);
			}
		}
		for (int k = 0; k < segs; ++k)
		{
			DrawLine(px[k], py[k], px[k + 1], py[k + 1], GetColor(60, 220, 255), 2);
			DrawLine(px[k], py[k], px[k + 1], py[k + 1], GetColor(225, 252, 255), 1);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// 8. 大爆発の瞬間、画面全体を一瞬真っ白にしてから溶けるようにフェードアウト
	if (mWhiteFlashAlpha > 0)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, mWhiteFlashAlpha);
		DrawBox(0, 0, 3000, 2000, GetColor(255, 255, 255), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

}

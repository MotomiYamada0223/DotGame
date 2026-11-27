#include "Stage3DragonBounceFireballAttack.h"
#include "Player.h"
#include "BlockMap.h"
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
}

Stage3DragonBounceFireballAttack::Stage3DragonBounceFireballAttack()
	: mPhase(Phase::Inactive)
	, mPhaseTimer(0)
	, mAnimFrame(0)
	, mbFireballShot(false)
	, mFireSingleTex(-1)
	, mBurstRedTex(-1)
	, mSplashTex(-1)
{
	Reset();
}

Stage3DragonBounceFireballAttack::~Stage3DragonBounceFireballAttack()
{
	if (mFireSingleTex != -1) { DeleteGraph(mFireSingleTex); mFireSingleTex = -1; }
	if (mBurstRedTex != -1)   { DeleteGraph(mBurstRedTex);   mBurstRedTex   = -1; }
	if (mSplashTex != -1)
	{
		DeleteGraph(mSplashTex);
		mSplashTex = -1;
	}
}

void Stage3DragonBounceFireballAttack::Initialize()
{
	if (mFireSingleTex == -1)
	{
		mFireSingleTex = LoadGraph("Resource/Effect/Pierre01/Pierre01/Texture/Fire_Single.png");
	}
	if (mBurstRedTex == -1)
	{
		mBurstRedTex = LoadGraph("Resource/Effect/tktk01/tktk01/Texture/burst02_red.png");
	}
	if (mSplashTex == -1)
	{
		mSplashTex = LoadGraph("Resource/Effect/Effekseer01/Effekseer01/Texture/Splash01.png");
	}
	Reset();
}

void Stage3DragonBounceFireballAttack::Reset()
{
	mPhase         = Phase::Inactive;
	mPhaseTimer    = 0;
	mAnimFrame     = 0;
	mbFireballShot = false;

	mFireball.active           = false;
	mFireball.bounceCount      = 0;
	mFireball.gravityY         = 0.0f;
	mFireball.isHeadingToMagma = false;
	mFireball.radius           = GetRadiusForBounce(0);
	mFireball.speed            = GetSpeedForBounce(0);

	mFountain.active        = false;
	mFountain.currentHeight = 0.0f;
	mFountain.timer         = 0;
	mFountain.hasSplashed   = false;
	mFountain.bouncePower   = 4;

	mMagmaGlobs.clear();
	mSparks.clear();
}

bool Stage3DragonBounceFireballAttack::HasActiveProjectiles() const
{
	if (mFireball.active || mFountain.active) return true;
	for (const auto& g : mMagmaGlobs)
	{
		if (g.active) return true;
	}
	return false;
}

// 1回跳ね返るごとに火の玉の大きさ（半径）が段階的に大きくなる（4反射で中央巨大火球と同サイズ）
float Stage3DragonBounceFireballAttack::GetRadiusForBounce(int bounceCount)
{
	switch (bounceCount)
	{
	case 0:  return 32.0f; // 初期サイズ
	case 1:  return 44.0f; // 1反射目
	case 2:  return 56.0f; // 2反射目
	case 3:  return 68.0f; // 3反射目
	default: return 82.0f; // 4反射目（超大迫力サイズ！）
	}
}

// 1回跳ね返るごとに火の玉の速度が段階的に速くなる
float Stage3DragonBounceFireballAttack::GetSpeedForBounce(int bounceCount)
{
	switch (bounceCount)
	{
	case 0:  return 14.5f; // 初期速度
	case 1:  return 18.0f; // 1反射目
	case 2:  return 21.5f; // 2反射目
	case 3:  return 25.0f; // 3反射目
	default: return 28.5f; // 4反射目（最高速）
	}
}

void Stage3DragonBounceFireballAttack::GetGroundMouthWorldPos(
	const VECTOR& bossPos,
	float facingDir,
	float drawScale,
	float& outMouthX,
	float& outMouthY) const
{
	// 地上咆哮・ブレス姿勢（roarスプライト）における口元のワールド座標
	const float footOffset = 72.0f;
	float feetWorldY       = bossPos.y + BOSS_HALF_HEIGHT;
	float spriteCenterY    = feetWorldY - (footOffset * drawScale);

	outMouthX = bossPos.x + facingDir * (92.0f * drawScale);
	outMouthY = spriteCenterY - (42.0f * drawScale);
}

void Stage3DragonBounceFireballAttack::StartAttack(const VECTOR& bossPos, float& facingDir, Player* player)
{
	mPhase         = Phase::ChargeBreath;
	mPhaseTimer    = 0;
	mAnimFrame     = 0;
	mbFireballShot = false;

	if (player != nullptr)
	{
		facingDir = (player->GetPosition().x >= bossPos.x) ? 1.0f : -1.0f;
	}
}

void Stage3DragonBounceFireballAttack::SpawnBurstSparks(float x, float y, int count, float speedScale, unsigned int mainColor)
{
	for (int i = 0; i < count; ++i)
	{
		float angle = (static_cast<float>(GetRand(360)) * 3.14159265f) / 180.0f;
		float spd   = (2.0f + static_cast<float>(GetRand(100)) * 0.08f) * speedScale;
		FireSpark sp;
		sp.x       = x;
		sp.y       = y;
		sp.vx      = std::cos(angle) * spd;
		sp.vy      = std::sin(angle) * spd - 1.5f;
		sp.size    = 4.0f + static_cast<float>(GetRand(6));
		sp.maxLife = 16 + GetRand(14);
		sp.life    = sp.maxLife;
		sp.color   = (i % 3 == 0) ? GetColor(255, 240, 130) : mainColor;
		mSparks.push_back(sp);
	}
}

void Stage3DragonBounceFireballAttack::UpdateDragon(
	const VECTOR& bossPos,
	float& facingDir,
	bool& isBossGrounded,
	int& outAnimType,
	int& outAnimFrame,
	float drawScale,
	Player* player)
{
	if (mPhase == Phase::Inactive || mPhase == Phase::Finished)
	{
		return;
	}

	mPhaseTimer++;
	isBossGrounded = true;

	switch (mPhase)
	{
	// -----------------------------------------------------------------
	// 1. 地上でプレイヤーの方を向き、口元に火の玉をチャージ
	// -----------------------------------------------------------------
	case Phase::ChargeBreath:
	{
		outAnimType = ANIM_CHARGE;
		mAnimFrame  = (mPhaseTimer * 6) / CHARGE_FRAMES;
		if (mAnimFrame > 5) mAnimFrame = 5;
		outAnimFrame = mAnimFrame;

		if (player != nullptr)
		{
			facingDir = (player->GetPosition().x >= bossPos.x) ? 1.0f : -1.0f;
		}

		// 口元に炎粒子を吸い込む演出
		float mouthX = 0.0f, mouthY = 0.0f;
		GetGroundMouthWorldPos(bossPos, facingDir, drawScale, mouthX, mouthY);
		for (int i = 0; i < 2; ++i)
		{
			float angle = (static_cast<float>(GetRand(360)) * 3.14159265f) / 180.0f;
			float dist  = 45.0f + static_cast<float>(GetRand(55));
			FireSpark sp;
			sp.x       = mouthX + std::cos(angle) * dist;
			sp.y       = mouthY + std::sin(angle) * dist;
			sp.vx      = (mouthX - sp.x) / 10.0f;
			sp.vy      = (mouthY - sp.y) / 10.0f;
			sp.size    = 4.0f + static_cast<float>(GetRand(4));
			sp.maxLife = 10;
			sp.life    = 10;
			sp.color   = (GetRand(1) == 0) ? GetColor(255, 210, 80) : GetColor(255, 95, 30);
			mSparks.push_back(sp);
		}

		if (mPhaseTimer >= CHARGE_FRAMES)
		{
			mPhase      = Phase::SpitFireball;
			mPhaseTimer = 0;
		}
		break;
	}

	// -----------------------------------------------------------------
	// 2. プレイヤーの現在位置に向かって真っ直ぐ火の玉を1つ吐き出す！
	// -----------------------------------------------------------------
	case Phase::SpitFireball:
	{
		outAnimType  = ANIM_CHARGE;
		mAnimFrame   = 6;
		outAnimFrame = mAnimFrame;

		if (!mbFireballShot)
		{
			mbFireballShot = true;

			float mouthX = 0.0f, mouthY = 0.0f;
			GetGroundMouthWorldPos(bossPos, facingDir, drawScale, mouthX, mouthY);

			// 固定軌道ではなく、発射瞬間のプレイヤーの座標に向かって真っ直ぐ撃つ
			float targetX = mouthX + facingDir * 900.0f;
			float targetY = mouthY;
			if (player != nullptr)
			{
				targetX = player->GetPosition().x;
				targetY = player->GetPosition().y;
			}

			float dx   = targetX - mouthX;
			float dy   = targetY - mouthY;
			float dist = std::sqrt(dx * dx + dy * dy);
			if (dist < 1.0f)
			{
				dx   = facingDir;
				dy   = 0.0f;
				dist = 1.0f;
			}

			mFireball.active           = true;
			mFireball.x                = mouthX;
			mFireball.y                = mouthY;
			mFireball.bounceCount      = 0;
			mFireball.gravityY         = 0.0f;
			mFireball.isHeadingToMagma = false;
			mFireball.radius           = GetRadiusForBounce(0);
			mFireball.speed            = GetSpeedForBounce(0);
			mFireball.vx               = (dx / dist) * mFireball.speed;
			mFireball.vy               = (dy / dist) * mFireball.speed;

			SpawnBurstSparks(mouthX, mouthY, 18, 1.2f, GetColor(255, 140, 40));
		}

		if (mPhaseTimer >= SPIT_FRAMES)
		{
			mPhase      = Phase::Recovery;
			mPhaseTimer = 0;
		}
		break;
	}

	// -----------------------------------------------------------------
	// 3. 吐き出し後の地上フォロースルー
	// -----------------------------------------------------------------
	case Phase::Recovery:
	{
		outAnimType  = ANIM_CHARGE;
		mAnimFrame   = (mPhaseTimer < 10) ? 6 : 0;
		outAnimFrame = mAnimFrame;

		if (mPhaseTimer >= RECOVERY_FRAMES)
		{
			mPhase      = Phase::Finished;
			mPhaseTimer = 0;
		}
		break;
	}

	default:
		break;
	}
}

// マグマに火の玉が入ったときの縦噴水発生（反射回数 0～4 に応じて噴水の高さ・太さ・持続時間が変化！）
void Stage3DragonBounceFireballAttack::TriggerMagmaFountain(float impactX, int bounceCount, BlockMap* blockMap)
{
	if (bounceCount < 0) bounceCount = 0;
	if (bounceCount > MAX_BOUNCES) bounceCount = MAX_BOUNCES;

	// マグマピット内（X = 3910 .. 5160）に収める
	if (impactX < LEFT_LEDGE_EDGE_X + 58.0f)  impactX = LEFT_LEDGE_EDGE_X + 58.0f;
	if (impactX > RIGHT_LEDGE_EDGE_X - 58.0f) impactX = RIGHT_LEDGE_EDGE_X - 58.0f;

	// 反射回数 0～4 ごとの噴水パラメータ（4反射で中央巨大火球攻撃と同じ高さ 760px・太さ 76px！）
	// 【Stage3.png 5724x1692拡大版 対応】天井(Y=483)に合わせて噴水高さを調整
	static const float kMaxHeights[5]  = { 260.0f, 390.0f, 520.0f, 650.0f, 780.0f };
	static const float kHalfWidths[5] = {  30.0f,  42.0f,  54.0f,  65.0f,  76.0f };
	static const float kRiseSpeeds[5] = {  22.0f,  26.0f,  30.0f,  34.0f,  38.0f };
	static const int   kHoldFrames[5] = {     14,     17,     20,     23,     26 };

	mFountain.active        = true;
	mFountain.x             = impactX;
	mFountain.baseY         = MAGMA_SURFACE_Y;
	mFountain.currentHeight = 20.0f;
	mFountain.maxHeight     = kMaxHeights[bounceCount];
	mFountain.halfWidth     = kHalfWidths[bounceCount];
	mFountain.riseSpeed     = kRiseSpeeds[bounceCount];
	mFountain.holdFrames    = kHoldFrames[bounceCount];
	mFountain.timer         = 0;
	mFountain.bouncePower   = bounceCount;
	mFountain.hasSplashed   = false;

	SpawnBurstSparks(impactX, MAGMA_SURFACE_Y - 12.0f, 16 + bounceCount * 8, 1.0f + bounceCount * 0.25f, GetColor(255, 110, 30));

	if (blockMap != nullptr)
	{
		int shakeAmp = 6 + bounceCount * 3;
		blockMap->mShakeX = GetRand(shakeAmp * 2) - shakeAmp;
		blockMap->mShakeY = GetRand(shakeAmp * 2) - shakeAmp;
	}
}

// 噴水が吹き上がったあとに周囲へ飛び散るマグマ塊を生成（反射回数 0～4 に応じて個数・飛距離・大きさが変化！）
void Stage3DragonBounceFireballAttack::SpawnMagmaSplashGlobs(float originX, float fountainTopY, int bounceCount)
{
	if (bounceCount < 0) bounceCount = 0;
	if (bounceCount > MAX_BOUNCES) bounceCount = MAX_BOUNCES;

	mMagmaGlobs.clear();

	// 反射回数ごとの左右それぞれの飛散数（0反射:片側2個=計4個 ～ 4反射:片側8個=計16個で中央火球攻撃と同等！）
	static const int   kGlobsPerSide[5] = { 2, 3, 5, 6, 8 };
	static const float kMaxSpeedX[5]    = { 5.2f, 7.8f, 10.5f, 13.0f, 15.5f };
	static const float kBaseUpVy[5]     = { -10.5f, -12.5f, -14.5f, -16.5f, -18.5f };
	static const float kBaseRadius[5]   = { 20.0f, 23.0f, 26.0f, 28.0f, 30.0f };

	int   perSide    = kGlobsPerSide[bounceCount];
	float maxSpeedX  = kMaxSpeedX[bounceCount];
	float baseUpVy   = kBaseUpVy[bounceCount];
	float baseRadius = kBaseRadius[bounceCount];

	float launchBaseY = MAGMA_SURFACE_Y - (MAGMA_SURFACE_Y - fountainTopY) * 0.48f;

	for (int side = -1; side <= 1; side += 2)
	{
		for (int i = 0; i < perSide; ++i)
		{
			MagmaGlob glob;
			glob.active = true;
			glob.x      = originX + side * (16.0f + i * 7.0f);
			glob.y      = launchBaseY - (i % 3) * 32.0f;

			float ratio = (perSide > 1) ? (static_cast<float>(i) / static_cast<float>(perSide - 1)) : 0.5f;
			float speedX = 2.2f + ratio * (maxSpeedX - 2.2f) + (GetRand(20) - 10) * 0.08f;
			float speedY = baseUpVy - (i % 4) * (1.2f + bounceCount * 0.25f) - GetRand(25) * 0.1f;

			glob.vx     = side * speedX;
			glob.vy     = speedY;
			glob.radius = baseRadius + (i % 3) * (1.8f + bounceCount * 0.45f);
			mMagmaGlobs.push_back(glob);
		}
	}
}

void Stage3DragonBounceFireballAttack::UpdateProjectilesAndEffects(Player* player, BlockMap* blockMap, int attackDamage)
{
	// -----------------------------------------------------------------
	// 1. 火花パーティクルの更新
	// -----------------------------------------------------------------
	for (int i = static_cast<int>(mSparks.size()) - 1; i >= 0; --i)
	{
		mSparks[i].x  += mSparks[i].vx;
		mSparks[i].y  += mSparks[i].vy;
		mSparks[i].vy += 0.15f;
		mSparks[i].life--;
		if (mSparks[i].life <= 0)
		{
			mSparks.erase(mSparks.begin() + i);
		}
	}

	// -----------------------------------------------------------------
	// 2. 壁に跳ね返る火の玉の移動・自然反射・床消滅・マグマ着弾判定
	// -----------------------------------------------------------------
	if (mFireball.active)
	{
		mFireball.x  += mFireball.vx;
		mFireball.y  += mFireball.vy;
		mFireball.vy += mFireball.gravityY;

		// 飛行中の炎トレイル火花
		for (int i = 0; i < 2; ++i)
		{
			FireSpark sp;
			sp.x       = mFireball.x + (GetRand(20) - 10);
			sp.y       = mFireball.y + (GetRand(20) - 10);
			sp.vx      = -mFireball.vx * 0.18f + (GetRand(10) - 5) * 0.3f;
			sp.vy      = -mFireball.vy * 0.18f + (GetRand(10) - 5) * 0.3f;
			sp.size    = 4.0f + mFireball.bounceCount * 1.5f;
			sp.maxLife = 12;
			sp.life    = 12;
			sp.color   = (mFireball.bounceCount >= 3) ? GetColor(255, 230, 90) : GetColor(255, 110, 30);
			mSparks.push_back(sp);
		}

		// プレイヤーとの直接接触判定
		if (player != nullptr)
		{
			float pLeft   = player->GetPosition().x - 20.0f;
			float pRight  = player->GetPosition().x + 20.0f;
			float pTop    = player->GetPosition().y - 44.0f;
			float pBottom = player->GetPosition().y + 44.0f;

			float hitR = mFireball.radius * 0.8f;
			if (mFireball.x + hitR > pLeft && mFireball.x - hitR < pRight &&
				mFireball.y + hitR > pTop  && mFireball.y - hitR < pBottom)
			{
				player->TakeDamage(attackDamage, DeathReason::DeathType::DragonExplosion);
			}
		}

		bool handledImpact = false;

		// (A) マグマ（X = 3852 .. 5220, Y = 1236）に入ったかチェック！
		if (mFireball.x >= LEFT_LEDGE_EDGE_X && mFireball.x <= RIGHT_LEDGE_EDGE_X &&
			mFireball.y + mFireball.radius >= MAGMA_SURFACE_Y && mFireball.vy > 0.0f)
		{
			float impactX    = mFireball.x;
			int   bounces    = mFireball.bounceCount;
			mFireball.active = false;
			TriggerMagmaFountain(impactX, bounces, blockMap);
			handledImpact = true;
		}

		// (B) 左右の足場の「床」にぶつかったら消滅する（床にぶつかっても消滅する）
		if (!handledImpact && mFireball.vy > 0.0f)
		{
			bool hitLeftFloor  = (mFireball.x < LEFT_LEDGE_EDGE_X  && mFireball.y + mFireball.radius >= LEFT_LEDGE_FLOOR_Y);
			bool hitRightFloor = (mFireball.x > RIGHT_LEDGE_EDGE_X && mFireball.y + mFireball.radius >= RIGHT_LEDGE_FLOOR_Y);
			if (hitLeftFloor || hitRightFloor)
			{
				SpawnBurstSparks(mFireball.x, mFireball.y, 22, 1.3f, GetColor(255, 120, 40));
				mFireball.active = false;
				handledImpact    = true;
			}
		}

		// (C) ボス部屋の壁・天井への衝突と自然反射判定（最大4回まで反射、4回目以降に壁にぶつかると消滅）
		if (!handledImpact)
		{
			bool hitLeftWall  = (mFireball.x - mFireball.radius <= ROOM_LEFT_WALL_X  && mFireball.vx < 0.0f);
			bool hitRightWall = (mFireball.x + mFireball.radius >= ROOM_RIGHT_WALL_X && mFireball.vx > 0.0f);
			bool hitCeiling   = (mFireball.y - mFireball.radius <= ROOM_CEILING_Y    && mFireball.vy < 0.0f);

			// マグマピット側面の垂直壁との衝突
			if (!hitLeftWall && mFireball.y > LEFT_LEDGE_FLOOR_Y + 8.0f &&
				mFireball.x - mFireball.radius <= LEFT_LEDGE_EDGE_X && mFireball.vx < 0.0f)
			{
				hitLeftWall = true;
			}
			if (!hitRightWall && mFireball.y > RIGHT_LEDGE_FLOOR_Y + 8.0f &&
				mFireball.x + mFireball.radius >= RIGHT_LEDGE_EDGE_X && mFireball.vx > 0.0f)
			{
				hitRightWall = true;
			}

			if (hitLeftWall || hitRightWall || hitCeiling)
			{
				// すでに4回跳ね返った状態（bounceCount >= 4）で壁にぶつかったら消滅！
				if (mFireball.bounceCount >= MAX_BOUNCES)
				{
					SpawnBurstSparks(mFireball.x, mFireball.y, 30, 1.6f, GetColor(255, 80, 30));
					mFireball.active = false;
				}
				else
				{
					// 反射回数 +1、1回跳ね返るごとに速度と大きさが変化！
					mFireball.bounceCount++;
					mFireball.radius = GetRadiusForBounce(mFireball.bounceCount);
					mFireball.speed  = GetSpeedForBounce(mFireball.bounceCount);

					SpawnBurstSparks(mFireball.x, mFireball.y, 16 + mFireball.bounceCount * 4, 1.1f, GetColor(255, 180, 60));
					if (blockMap != nullptr)
					{
						blockMap->mShakeX = GetRand(8) - 4;
						blockMap->mShakeY = GetRand(8) - 4;
					}

					// 壁めり込み補正
					if (hitLeftWall)
					{
						float wallX = (mFireball.y > LEFT_LEDGE_FLOOR_Y + 8.0f) ? LEFT_LEDGE_EDGE_X : ROOM_LEFT_WALL_X;
						mFireball.x = wallX + mFireball.radius + 2.0f;
					}
					else if (hitRightWall)
					{
						float wallX = (mFireball.y > RIGHT_LEDGE_FLOOR_Y + 8.0f) ? RIGHT_LEDGE_EDGE_X : ROOM_RIGHT_WALL_X;
						mFireball.x = wallX - mFireball.radius - 2.0f;
					}
					if (hitCeiling)
					{
						mFireball.y = ROOM_CEILING_Y + mFireball.radius + 2.0f;
					}

					// 反射するごとに：
					// ・50%の確率で「プレイヤーの現在座標を取って飛ぶ軌道」
					// ・50%の確率で「マグマに向かって飛ぶ軌道」
					bool aimAtPlayer = (GetRand(99) < 50);

					if (aimAtPlayer && player != nullptr)
					{
						mFireball.isHeadingToMagma = false;
						mFireball.gravityY         = 0.0f;

						float targetX = player->GetPosition().x;
						float targetY = player->GetPosition().y - 20.0f;
						float dx      = targetX - mFireball.x;
						float dy      = targetY - mFireball.y;

						// 同じ壁に即座に再衝突しないよう反射方向の符号を保証
						if (hitLeftWall  && dx < 60.0f)  dx = 60.0f;
						if (hitRightWall && dx > -60.0f) dx = -60.0f;
						if (hitCeiling   && dy < 60.0f)  dy = 60.0f;

						// 足場のすぐ上で反射した際、プレイヤーが反対側にいるのに目の前の足場床へ即刺さるのを防ぐ最小角度補正
						float absDx = std::fabs(dx);
						// 【Stage3.png 5724x1692拡大版 対応】床高度に合わせて最小角度補正を調整
						if (!hitCeiling && mFireball.y > 1080.0f && dy > 0.0f && absDx > 500.0f)
						{
							float maxSafeDy = (LEFT_LEDGE_FLOOR_Y - mFireball.radius - 10.0f) - mFireball.y;
							if (dy > maxSafeDy) dy = maxSafeDy;
						}

						float dist = std::sqrt(dx * dx + dy * dy);
						if (dist < 1.0f) dist = 1.0f;
						mFireball.vx = (dx / dist) * mFireball.speed;
						mFireball.vy = (dy / dist) * mFireball.speed;
					}
					else
					{
						// 50%でマグマ（X = 4100 .. 4970, Y = 1236）に向かって飛ぶ軌道！
						mFireball.isHeadingToMagma = true;

						// 【Stage3.png 5724x1692拡大版 対応】マグマピット幅(3667..4970)に合わせて着水目標Xを更新
						float aimMagmaX = 3900.0f + static_cast<float>(GetRand(840));
						float aimMagmaY = MAGMA_SURFACE_Y + 15.0f;

						// 壁の低い位置（足場のすぐ上）で反射したときは、手前の足場角に引っかからずマグマへ飛び込むよう放物線軌道にする
						if (!hitCeiling && (mFireball.x < LEFT_LEDGE_EDGE_X || mFireball.x > RIGHT_LEDGE_EDGE_X) && mFireball.y > 1000.0f)
						{
							float dx     = aimMagmaX - mFireball.x;
							float frames = std::fabs(dx) / (mFireball.speed * 0.92f);
							if (frames < 18.0f) frames = 18.0f;

							mFireball.gravityY = 0.34f;
							mFireball.vx       = dx / frames;
							mFireball.vy       = ((aimMagmaY - mFireball.y) - 0.5f * mFireball.gravityY * frames * frames) / frames;
						}
						else
						{
							// 高い位置や天井からの反射時はマグマへ向かって直線的に飛び込む
							mFireball.gravityY = 0.0f;
							float dx           = aimMagmaX - mFireball.x;
							float dy           = aimMagmaY - mFireball.y;
							if (hitCeiling && dy < 80.0f) dy = 80.0f;
							float dist = std::sqrt(dx * dx + dy * dy);
							if (dist < 1.0f) dist = 1.0f;
							mFireball.vx = (dx / dist) * mFireball.speed;
							mFireball.vy = (dy / dist) * mFireball.speed;
						}
					}
				}
			}
		}
	}

	// -----------------------------------------------------------------
	// 3. 着弾地点から縦に吹き上がるマグマ噴水の更新
	// -----------------------------------------------------------------
	if (mFountain.active)
	{
		mFountain.timer++;

		int riseFrames = static_cast<int>(mFountain.maxHeight / mFountain.riseSpeed) + 2;
		int splashTick = riseFrames + (mFountain.holdFrames / 2);
		int endTick    = riseFrames + mFountain.holdFrames;

		if (mFountain.timer <= riseFrames)
		{
			mFountain.currentHeight += mFountain.riseSpeed;
			if (mFountain.currentHeight > mFountain.maxHeight)
			{
				mFountain.currentHeight = mFountain.maxHeight;
			}
			if (blockMap != nullptr)
			{
				int shake = 5 + mFountain.bouncePower * 2;
				blockMap->mShakeX = GetRand(shake * 2) - shake;
				blockMap->mShakeY = GetRand(shake * 2) - shake;
			}
		}
		else if (mFountain.timer <= endTick)
		{
			mFountain.currentHeight = mFountain.maxHeight + std::sin(mFountain.timer * 0.7f) * 14.0f;
			if (blockMap != nullptr)
			{
				int shake = 3 + mFountain.bouncePower * 2;
				blockMap->mShakeX = GetRand(shake * 2) - shake;
				blockMap->mShakeY = GetRand(shake * 2) - shake;
			}

			// 縦のマグマ噴水がしっかり立ち昇ったあと、周囲へマグマ塊が一斉に飛び散る！
			if (!mFountain.hasSplashed && mFountain.timer >= splashTick)
			{
				mFountain.hasSplashed = true;
				SpawnMagmaSplashGlobs(mFountain.x, mFountain.baseY - mFountain.currentHeight, mFountain.bouncePower);
			}
		}
		else
		{
			mFountain.currentHeight -= (mFountain.riseSpeed * 0.75f);
			if (blockMap != nullptr)
			{
				blockMap->mShakeX = 0;
				blockMap->mShakeY = 0;
			}
			if (mFountain.currentHeight <= 0.0f)
			{
				mFountain.currentHeight = 0.0f;
				mFountain.active        = false;
			}
		}

		// 縦マグマ噴水とプレイヤーの当たり判定
		if (player != nullptr && mFountain.currentHeight > 20.0f)
		{
			float pLeft   = player->GetPosition().x - 20.0f;
			float pRight  = player->GetPosition().x + 20.0f;
			float pTop    = player->GetPosition().y - 44.0f;
			float pBottom = player->GetPosition().y + 44.0f;

			float fLeft   = mFountain.x - mFountain.halfWidth * 0.75f;
			float fRight  = mFountain.x + mFountain.halfWidth * 0.75f;
			float fTop    = mFountain.baseY - mFountain.currentHeight;
			float fBottom = mFountain.baseY;

			if (pRight > fLeft && pLeft < fRight && pBottom > fTop && pTop < fBottom)
			{
				player->TakeDamage(attackDamage, DeathReason::DeathType::DragonExplosion);
			}
		}
	}

	// -----------------------------------------------------------------
	// 4. 周囲へ飛び散るマグマ塊の放物線移動と当たり判定
	// -----------------------------------------------------------------
	for (size_t i = 0; i < mMagmaGlobs.size(); ++i)
	{
		if (!mMagmaGlobs[i].active) continue;

		mMagmaGlobs[i].x  += mMagmaGlobs[i].vx;
		mMagmaGlobs[i].y  += mMagmaGlobs[i].vy;
		mMagmaGlobs[i].vy += 0.50f; // 重力加速度

		if (player != nullptr)
		{
			float pLeft   = player->GetPosition().x - 20.0f;
			float pRight  = player->GetPosition().x + 20.0f;
			float pTop    = player->GetPosition().y - 44.0f;
			float pBottom = player->GetPosition().y + 44.0f;

			float hitR = mMagmaGlobs[i].radius * 0.75f;
			if (mMagmaGlobs[i].x + hitR > pLeft && mMagmaGlobs[i].x - hitR < pRight &&
				mMagmaGlobs[i].y + hitR > pTop  && mMagmaGlobs[i].y - hitR < pBottom)
			{
				player->TakeDamage(attackDamage, DeathReason::DeathType::DragonExplosion);
				mMagmaGlobs[i].active = false;
				continue;
			}
		}

		if (mMagmaGlobs[i].y > MAGMA_SURFACE_Y + 90.0f ||
			mMagmaGlobs[i].x < ROOM_LEFT_WALL_X - 100.0f ||
			mMagmaGlobs[i].x > ROOM_RIGHT_WALL_X + 100.0f)
		{
			mMagmaGlobs[i].active = false;
		}
	}
}

void Stage3DragonBounceFireballAttack::Draw(
	BlockMap* blockMap,
	const VECTOR& bossPos,
	float facingDir,
	float drawScale) const
{
	// -----------------------------------------------------------------
	// 1. 口元チャージ中の火の玉描画
	// -----------------------------------------------------------------
	if (mPhase == Phase::ChargeBreath)
	{
		float mouthX = 0.0f, mouthY = 0.0f;
		GetGroundMouthWorldPos(bossPos, facingDir, drawScale, mouthX, mouthY);
		int sx = static_cast<int>(ConvertWorldToScreenX(mouthX, blockMap));
		int sy = static_cast<int>(ConvertWorldToScreenY(mouthY, blockMap));

		float ratio = static_cast<float>(mPhaseTimer) / static_cast<float>(CHARGE_FRAMES);
		if (ratio > 1.0f) ratio = 1.0f;
		int r = static_cast<int>(10.0f + 24.0f * ratio);

		if (mBurstRedTex != -1)
		{
			double sc = (r * 3.8) / 765.0;
			float rot = mPhaseTimer * 0.12f;
			SetDrawBright(255, 130, 35);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 220);
			DrawRotaGraph(sx, sy, sc, rot, mBurstRedTex, TRUE);
			SetDrawBright(255, 235, 115);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 195);
			DrawRotaGraph(sx, sy, sc * 0.65, -rot * 1.4f, mBurstRedTex, TRUE);
			SetDrawBright(255, 255, 255);
		}
		else
		{
			SetDrawBlendMode(DX_BLENDMODE_ADD, 210);
			DrawCircle(sx, sy, r + 8, GetColor(255, 60, 15), TRUE);
			DrawCircle(sx, sy, r, GetColor(255, 160, 40), TRUE);
			DrawCircle(sx, sy, r / 2, GetColor(255, 250, 190), TRUE);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// -----------------------------------------------------------------
	// 2. 火花パーティクル描画
	// -----------------------------------------------------------------
	SetDrawBlendMode(DX_BLENDMODE_ADD, 210);
	for (const auto& sp : mSparks)
	{
		int sx = static_cast<int>(ConvertWorldToScreenX(sp.x, blockMap));
		int sy = static_cast<int>(ConvertWorldToScreenY(sp.y, blockMap));
		DrawCircle(sx, sy, static_cast<int>(sp.size), sp.color, TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// -----------------------------------------------------------------
	// 3. 壁に跳ね返る火の玉の描画（反射回数に応じてサイズと輝きが強化！）
	// -----------------------------------------------------------------
	if (mFireball.active)
	{
		int sx = static_cast<int>(ConvertWorldToScreenX(mFireball.x, blockMap));
		int sy = static_cast<int>(ConvertWorldToScreenY(mFireball.y, blockMap));
		int r  = static_cast<int>(mFireball.radius);

		// 反射回数に応じた外郭カラー（0反射:赤橙 → 4反射:灼熱の黄金・深紅オーラ）
		unsigned int outerColor = GetColor(255, 60 + mFireball.bounceCount * 20, 15);
		unsigned int midColor   = GetColor(255, 140 + mFireball.bounceCount * 22, 35 + mFireball.bounceCount * 15);

		SetDrawBlendMode(DX_BLENDMODE_ADD, 225);
		if (mBurstRedTex != -1 || mFireSingleTex != -1)
		{
			float moveRot = std::atan2(mFireball.vy, mFireball.vx) + 1.5707963f;
			float spinRot = mPhaseTimer * 0.16f;
			int tx = static_cast<int>(ConvertWorldToScreenX(mFireball.x - mFireball.vx * 1.8f, blockMap));
			int ty = static_cast<int>(ConvertWorldToScreenY(mFireball.y - mFireball.vy * 1.8f, blockMap));
			if (mFireSingleTex != -1)
			{
				double fSc = (mFireball.radius * 3.1) / 256.0;
				SetDrawBright(255, 75 + mFireball.bounceCount * 15, 12);
				DrawRotaGraph(tx, ty, fSc, moveRot, mFireSingleTex, TRUE);
			}
			if (mBurstRedTex != -1)
			{
				double bSc = (mFireball.radius * 3.8) / 765.0;
				SetDrawBright(255, 125 + mFireball.bounceCount * 20, 25);
				DrawRotaGraph(sx, sy, bSc, spinRot, mBurstRedTex, TRUE);
				SetDrawBright(255, 235, 125);
				DrawRotaGraph(sx, sy, bSc * 0.62, -spinRot * 1.35f, mBurstRedTex, TRUE);
			}
			DrawCircle(sx, sy, static_cast<int>(r * 0.45f), GetColor(255, 250, 205), TRUE);
			SetDrawBright(255, 255, 255);
		}
		else
		{
			DrawCircle(sx, sy, r + 12 + mFireball.bounceCount * 3, outerColor, TRUE);
			DrawCircle(sx, sy, r, midColor, TRUE);
			DrawCircle(sx, sy, static_cast<int>(r * 0.58f), GetColor(255, 250, 195), TRUE);
		}

		// 反射回数分だけ周囲に火炎リングを描画して段階強化を視覚的にわかりやすく表示
		for (int b = 0; b < mFireball.bounceCount; ++b)
		{
			DrawCircle(sx, sy, r + 5 + b * 5, GetColor(255, 220, 90), FALSE);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// -----------------------------------------------------------------
	// 4. 縦に吹き上がるマグマ噴水の描画（反射回数に応じた高さと太さ）
	// -----------------------------------------------------------------
	if (mFountain.active && mFountain.currentHeight > 2.0f)
	{
		int centerSx = static_cast<int>(ConvertWorldToScreenX(mFountain.x, blockMap));
		int bottomSy = static_cast<int>(ConvertWorldToScreenY(mFountain.baseY, blockMap));
		int topSy    = static_cast<int>(ConvertWorldToScreenY(mFountain.baseY - mFountain.currentHeight, blockMap));
		int halfW    = static_cast<int>(mFountain.halfWidth);
		int coreW    = static_cast<int>(mFountain.halfWidth * 0.54f);
		int innerW   = static_cast<int>(mFountain.halfWidth * 0.25f);

		SetDrawBlendMode(DX_BLENDMODE_ADD, 215);

		DrawBox(centerSx - halfW / 2, topSy + 20, centerSx + halfW / 2, bottomSy, GetColor(210, 40, 8), TRUE);
		DrawBox(centerSx - coreW,     topSy + 10, centerSx + coreW,     bottomSy, GetColor(255, 135, 25), TRUE);
		DrawBox(centerSx - innerW,    topSy,      centerSx + innerW,    bottomSy, GetColor(255, 240, 160), TRUE);

		DrawCircle(centerSx, topSy + 12, halfW + 8, GetColor(255, 95, 20), TRUE);
		DrawCircle(centerSx, topSy + 8,  coreW + 4, GetColor(255, 215, 80), TRUE);

		int stepY = 48;
		for (int y = bottomSy - 20; y > topSy; y -= stepY)
		{
			int waveOffset = static_cast<int>(std::sin((y + mFountain.timer * 14) * 0.05f) * (10.0f + mFountain.bouncePower * 2.5f));
			DrawCircle(centerSx + waveOffset, y, coreW, GetColor(255, 165, 45), TRUE);
		}

		double wRatio = static_cast<double>(mFountain.halfWidth) / 55.0;
		float surge = std::fmod(mFountain.timer * 0.06f, 1.0f);
		if (mFireSingleTex != -1)
		{
			for (int fi = 0; fi < 8; ++fi)
			{
				float fyRatio = std::fmod((static_cast<float>(fi) / 8.0f) + surge, 1.0f);
				int fy = bottomSy - static_cast<int>(mFountain.currentHeight * fyRatio);
				int xSway = static_cast<int>(std::sin(mFountain.timer * 0.28f + fi * 1.5f) * 18.0f);
				double sc = (mFountain.halfWidth * (3.1 + fyRatio * 1.0)) / 256.0;
				SetDrawBright(255, 100 + static_cast<int>(fyRatio * 95), 18);
				DrawRotaGraph(centerSx + xSway, fy, sc, (fi % 2 == 0) ? 0.16 : -0.16, mFireSingleTex, TRUE);
			}
		}
		if (mBurstRedTex != -1)
		{
			float rot = mFountain.timer * 0.10f;
			SetDrawBright(255, 95, 15);
			DrawRotaGraph(centerSx, bottomSy - 18, 0.56 * wRatio, rot, mBurstRedTex, TRUE);
			SetDrawBright(255, 200, 55);
			DrawRotaGraph(centerSx, topSy + 12, 0.62 * wRatio, -rot * 1.25f, mBurstRedTex, TRUE);
		}
		if (mSplashTex != -1)
		{
			// 火口ベース5方向噴出プルーム
			SetDrawBright(255, 75, 10);
			DrawRotaGraph3(centerSx - 54, bottomSy - 85, 256, 256, 0.68 * wRatio, 0.70, -0.58, mSplashTex, TRUE);
			DrawRotaGraph3(centerSx + 54, bottomSy - 85, 256, 256, 0.68 * wRatio, 0.70,  0.58, mSplashTex, TRUE);
			SetDrawBright(255, 105, 15);
			DrawRotaGraph3(centerSx - 32, bottomSy - 110, 256, 256, 0.78 * wRatio, 0.84, -0.28, mSplashTex, TRUE);
			DrawRotaGraph3(centerSx + 32, bottomSy - 110, 256, 256, 0.78 * wRatio, 0.84,  0.28, mSplashTex, TRUE);
			SetDrawBright(255, 165, 25);
			DrawRotaGraph3(centerSx, bottomSy - 135, 256, 256, 1.05 * wRatio, 0.95, 0.0, mSplashTex, TRUE);
			// 噴火頂上5方向火山クラウン
			SetDrawBright(255, 145, 25);
			DrawRotaGraph3(centerSx - 68, topSy - 14, 256, 256, 0.76 * wRatio, 0.48, -0.68, mSplashTex, TRUE);
			DrawRotaGraph3(centerSx + 68, topSy - 14, 256, 256, 0.76 * wRatio, 0.48,  0.68, mSplashTex, TRUE);
			SetDrawBright(255, 185, 40);
			DrawRotaGraph3(centerSx - 38, topSy - 28, 256, 256, 0.92 * wRatio, 0.58, -0.34, mSplashTex, TRUE);
			DrawRotaGraph3(centerSx + 38, topSy - 28, 256, 256, 0.92 * wRatio, 0.58,  0.34, mSplashTex, TRUE);
			SetDrawBright(255, 235, 95);
			DrawRotaGraph3(centerSx, topSy - 48, 256, 256, 1.24 * wRatio, 0.70, 0.0, mSplashTex, TRUE);
		}
		float rippleT = std::fmod(mFountain.timer * 0.05f, 1.0f);
		DrawOval(centerSx, bottomSy + 2, static_cast<int>(mFountain.halfWidth * (1.8f + rippleT * 1.4f)), static_cast<int>(16.0f + rippleT * 10.0f), GetColor(255, 120, 20), FALSE, 4);
		DrawOval(centerSx, bottomSy + 2, static_cast<int>(mFountain.halfWidth * 1.6f), 16, GetColor(255, 210, 75), FALSE, 3);
		SetDrawBright(255, 255, 255);
		SetDrawBright(255, 255, 255);

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// -----------------------------------------------------------------
	// 5. 周囲へ飛び散るマグマ塊の描画
	// -----------------------------------------------------------------
	SetDrawBlendMode(DX_BLENDMODE_ADD, 220);
	for (const auto& glob : mMagmaGlobs)
	{
		if (!glob.active) continue;

		int sx = static_cast<int>(ConvertWorldToScreenX(glob.x, blockMap));
		int sy = static_cast<int>(ConvertWorldToScreenY(glob.y, blockMap));
		int r  = static_cast<int>(glob.radius);

		int tailSx = static_cast<int>(ConvertWorldToScreenX(glob.x - glob.vx * 1.4f, blockMap));
		int tailSy = static_cast<int>(ConvertWorldToScreenY(glob.y - glob.vy * 1.4f, blockMap));
		if (mBurstRedTex != -1 || mFireSingleTex != -1)
		{
			float rot = std::atan2(glob.vy, glob.vx) + 1.5707963f;
			if (mFireSingleTex != -1)
			{
				double sc = (glob.radius * 2.8) / 256.0;
				SetDrawBright(255, 85, 14);
				DrawRotaGraph(tailSx, tailSy, sc, rot, mFireSingleTex, TRUE);
			}
			if (mBurstRedTex != -1)
			{
				double bSc = (glob.radius * 3.4) / 765.0;
				SetDrawBright(255, 165, 35);
				DrawRotaGraph(sx, sy, bSc, mPhaseTimer * 0.18f, mBurstRedTex, TRUE);
			}
			DrawCircle(sx, sy, static_cast<int>(r * 0.45f), GetColor(255, 245, 175), TRUE);
			SetDrawBright(255, 255, 255);
		}
		else
		{
			DrawCircle(tailSx, tailSy, static_cast<int>(r * 0.85f), GetColor(220, 50, 10), TRUE);
			DrawCircle(sx, sy, r + 4, GetColor(240, 65, 15), TRUE);
			DrawCircle(sx, sy, r, GetColor(255, 160, 35), TRUE);
			DrawCircle(sx, sy, static_cast<int>(r * 0.5f), GetColor(255, 245, 175), TRUE);
		}
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

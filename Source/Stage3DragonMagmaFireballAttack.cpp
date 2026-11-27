#include "Stage3DragonMagmaFireballAttack.h"
#include "BlockMap.h"
#include "Player.h"
#include "GameConstants.h"
#include <cmath>

Stage3DragonMagmaFireballAttack::Stage3DragonMagmaFireballAttack()
	: mPhase(Phase::Inactive)
	, mTimer(0)
	, mAnimTimer(0)
	, mAnimFrame(0)
	, mbReturnToLeftLedge(false)
	, mReturnTargetX(RIGHT_LEDGE_STAND_X)
	, mReturnFloorY(RIGHT_LEDGE_FLOOR_Y)
	, mFireSingleTex(-1)
	, mBurstRedTex(-1)
	, mSplashTex(-1)
{
}

Stage3DragonMagmaFireballAttack::~Stage3DragonMagmaFireballAttack()
{
	if (mFireSingleTex != -1) { DeleteGraph(mFireSingleTex); mFireSingleTex = -1; }
	if (mBurstRedTex != -1)   { DeleteGraph(mBurstRedTex);   mBurstRedTex   = -1; }
	if (mSplashTex != -1)
	{
		DeleteGraph(mSplashTex);
		mSplashTex = -1;
	}
}

void Stage3DragonMagmaFireballAttack::Initialize()
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

void Stage3DragonMagmaFireballAttack::Reset()
{
	mPhase              = Phase::Inactive;
	mTimer              = 0;
	mAnimTimer          = 0;
	mAnimFrame          = 0;
	mbReturnToLeftLedge = false;
	mReturnTargetX      = RIGHT_LEDGE_STAND_X;
	mReturnFloorY       = RIGHT_LEDGE_FLOOR_Y;

	mFireball.active        = false;
	mFountain.active        = false;
	mFountain.timer         = 0;
	mFountain.currentHeight = 0.0f;
	mFountain.hasSplashed   = false;
	mSplashGlobs.clear();
	mSparks.clear();
}

void Stage3DragonMagmaFireballAttack::GetAirAttackMouthWorldPos(
	const VECTOR& bossPos,
	float facingDir,
	float drawScale,
	int animFrame,
	float& outMouthX,
	float& outMouthY) const
{
	// Stage 2 (BossEnemyDragon) と完全同一の空中攻撃スプライト口元座標テーブル
	static const float kMouthOffsetX[7] = { 106.0f, 104.0f, 97.0f, 105.0f, 115.0f, 106.0f, 105.0f };
	static const float kMouthOffsetY[7] = { -77.0f, -87.0f, -101.0f, -82.0f, -70.0f, -72.0f, -75.0f };

	int idx = animFrame;
	if (idx < 0) idx = 0;
	if (idx > 6) idx = 6;

	float spriteCenterWorldX = bossPos.x;
	float feetWorldY         = bossPos.y + 100.0f; // mfEnemyHeight(200) * 0.5f
	float spriteCenterWorldY = feetWorldY - (57.0f * drawScale);

	outMouthX = spriteCenterWorldX + facingDir * kMouthOffsetX[idx] * drawScale;
	outMouthY = spriteCenterWorldY + kMouthOffsetY[idx] * drawScale;
}

void Stage3DragonMagmaFireballAttack::SpawnChargeSparks(float mouthX, float mouthY)
{
	for (int i = 0; i < 3; ++i)
	{
		float ang  = (GetRand(360) * 3.14159265f) / 180.0f;
		float dist = 55.0f + static_cast<float>(GetRand(55));
		LavaSpark sp;
		sp.x       = mouthX + std::cos(ang) * dist;
		sp.y       = mouthY + std::sin(ang) * dist;
		sp.vx      = (mouthX - sp.x) * 0.16f;
		sp.vy      = (mouthY - sp.y) * 0.16f;
		sp.size    = 4.0f + static_cast<float>(GetRand(4));
		sp.life    = 10;
		sp.maxLife = 10;
		sp.color   = (i % 2 == 0) ? GetColor(255, 90, 20) : GetColor(255, 240, 120);
		mSparks.push_back(sp);
	}
}

void Stage3DragonMagmaFireballAttack::LaunchGiantFireball(float mouthX, float mouthY)
{
	mFireball.active = true;
	mFireball.x      = mouthX;
	mFireball.y      = mouthY;
	mFireball.radius = 58.0f;

	// 中央のマグマ表面（CENTER_X = 4536, MAGMA_SURFACE_Y = 1236）へ向けて撃ち下ろす
	const float travelFrames = 28.0f;
	mFireball.vx = (CENTER_X - mouthX) / travelFrames;
	mFireball.vy = (MAGMA_SURFACE_Y - mouthY) / travelFrames;

}

void Stage3DragonMagmaFireballAttack::StartMagmaFountain(float impactX)
{
	mFountain.active        = true;
	mFountain.x             = impactX;
	mFountain.baseY         = MAGMA_SURFACE_Y;
	mFountain.currentHeight = 40.0f;
	// 【Stage3.png 5724x1692拡大版 対応】天井(Y=483)に合わせて噴水高さを調整
	mFountain.maxHeight     = 760.0f; // 画面上部（Y=605）まで豪快に突き上がるマグマの噴水！
	mFountain.halfWidth     = 58.0f;
	mFountain.timer         = 0;
	mFountain.maxTimer      = 68;
	mFountain.hasSplashed   = false;

	// 着弾瞬間の水しぶきならぬマグマしぶきスパーク
	for (int s = 0; s < 36; ++s)
	{
		float ang = -(GetRand(180) * 3.14159265f) / 180.0f;
		float spd = 6.0f + static_cast<float>(GetRand(130)) * 0.1f;
		LavaSpark sp;
		sp.x       = impactX + static_cast<float>(GetRand(60) - 30);
		sp.y       = MAGMA_SURFACE_Y - 6.0f;
		sp.vx      = std::cos(ang) * spd * 0.7f;
		sp.vy      = std::sin(ang) * spd * 1.3f;
		sp.size    = 5.0f + static_cast<float>(GetRand(6));
		sp.life    = 22 + GetRand(18);
		sp.maxLife = sp.life;
		sp.color   = (s % 3 == 0) ? GetColor(255, 250, 160) : ((s % 2 == 0) ? GetColor(255, 130, 20) : GetColor(220, 40, 10));
		mSparks.push_back(sp);
	}
}

void Stage3DragonMagmaFireballAttack::SpawnMagmaSplashWave(float originX, float originY, int waveIndex)
{
	// 縦のマグマ噴水が吹き上がったあと、噴水の頂上～中腹から周囲（左右の台・足場）へマグマが飛び散る！
	// 左右対称＋少しランダムな放物線で、ステージ全体に迫力よく降り注ぐ
	static const float kHorizontalSpeeds[7] = { 3.2f, 5.8f, 8.5f, 11.2f, 14.0f, 16.8f, 19.5f };
	static const float kUpwardSpeeds[7]     = { -13.5f, -12.5f, -11.5f, -10.5f, -9.5f, -8.5f, -7.5f };

	for (int side = -1; side <= 1; side += 2)
	{
		for (int i = 0; i < 7; ++i)
		{
			// 第1波と第2波で少し軌道と速度をずらして密度と回避の面白さを両立
			if (waveIndex == 1 && (i % 2 == 1)) continue;

			MagmaSplashGlob glob;
			glob.active = true;
			glob.x      = originX + side * (18.0f + i * 4.0f);
			glob.y      = originY + static_cast<float>(GetRand(50) - 25);

			float jitterVx = static_cast<float>(GetRand(16) - 8) * 0.1f;
			float jitterVy = static_cast<float>(GetRand(16) - 8) * 0.1f;
			float waveOffset = (waveIndex == 1) ? 1.12f : 1.0f;

			glob.vx     = side * (kHorizontalSpeeds[i] * waveOffset + jitterVx);
			glob.vy     = kUpwardSpeeds[i] + jitterVy;
			glob.radius = 28.0f + static_cast<float>(GetRand(8));
			glob.age    = 0;
			mSplashGlobs.push_back(glob);
		}
	}

	// 噴水が弾けた瞬間の大量のマグマ飛沫パーティクル
	for (int s = 0; s < 32; ++s)
	{
		float ang = -(GetRand(180) * 3.14159265f) / 180.0f;
		float spd = 7.0f + static_cast<float>(GetRand(140)) * 0.1f;
		LavaSpark sp;
		sp.x       = originX + static_cast<float>(GetRand(70) - 35);
		sp.y       = originY + static_cast<float>(GetRand(60) - 30);
		sp.vx      = std::cos(ang) * spd;
		sp.vy      = std::sin(ang) * spd;
		sp.size    = 4.0f + static_cast<float>(GetRand(5));
		sp.life    = 24 + GetRand(16);
		sp.maxLife = sp.life;
		sp.color   = (s % 2 == 0) ? GetColor(255, 200, 60) : GetColor(255, 70, 15);
		mSparks.push_back(sp);
	}
}

void Stage3DragonMagmaFireballAttack::StartAttack(VECTOR& bossPos, float& facingDir)
{
	mPhase     = Phase::Takeoff;
	mTimer     = 0;
	mAnimTimer = 0;
	mAnimFrame = 0;

	// 攻撃終了後に戻る足場（左端 or 右端）をランダムに決めておく
	mbReturnToLeftLedge = (GetRand(1) == 0);
	if (mbReturnToLeftLedge)
	{
		mReturnTargetX = LEFT_LEDGE_STAND_X;
		mReturnFloorY  = LEFT_LEDGE_FLOOR_Y;
	}
	else
	{
		mReturnTargetX = RIGHT_LEDGE_STAND_X;
		mReturnFloorY  = RIGHT_LEDGE_FLOOR_Y;
	}

	facingDir = (CENTER_X >= bossPos.x) ? 1.0f : -1.0f;
}

void Stage3DragonMagmaFireballAttack::UpdateDragon(
	VECTOR& bossPos,
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

	mTimer++;
	mAnimTimer++;

	switch (mPhase)
	{
	case Phase::Takeoff:
		// 1. 離陸して上空へ舞い上がる
		isBossGrounded = false;
		outAnimType    = ANIM_TAKEOFF;
		if (mAnimTimer >= 5)
		{
			mAnimTimer = 0;
			if (mAnimFrame < 6) mAnimFrame++;
		}
		outAnimFrame = mAnimFrame;

		if (mAnimFrame >= 1)
		{
			bossPos.y += (TARGET_HOVER_Y - bossPos.y) * 0.15f - 5.0f;
			if (bossPos.y < TARGET_HOVER_Y) bossPos.y = TARGET_HOVER_Y;
		}
		if (mTimer >= 28)
		{
			mPhase     = Phase::FlyToCenter;
			mTimer     = 0;
			mAnimTimer = 0;
			mAnimFrame = 0;
		}
		break;

	case Phase::FlyToCenter:
	{
		// 2. ステージ中央（CENTER_X = 4536, Y = 600）の上空へ飛んで移動する
		isBossGrounded = false;
		outAnimType    = ANIM_FLY;
		if (mAnimTimer >= 5)
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % 7;
		}
		outAnimFrame = mAnimFrame;

		float diffX = CENTER_X - bossPos.x;
		float diffY = TARGET_HOVER_Y - bossPos.y;
		bossPos.y += diffY * 0.16f;

		const float flySpeed = 20.0f;
		if (std::fabs(diffX) > flySpeed)
		{
			facingDir = (diffX >= 0.0f) ? 1.0f : -1.0f;
			bossPos.x += facingDir * flySpeed;
		}
		else
		{
			// 中央上空に到着！でっかい火の玉のチャージ＆発射へ移行
			bossPos.x = CENTER_X;
			bossPos.y = TARGET_HOVER_Y;
			if (player != nullptr && !player->GetIsDead())
			{
				facingDir = (player->GetPosition().x >= bossPos.x) ? 1.0f : -1.0f;
			}
			mPhase     = Phase::ChargeAndShoot;
			mTimer     = 0;
			mAnimTimer = 0;
			mAnimFrame = 0;
		}
		break;
	}

	case Phase::ChargeAndShoot:
	{
		// 3. 中央上空で口元にでっかい火の玉をチャージし、真下のマグマへ撃ち込む！
		isBossGrounded = false;
		outAnimType    = ANIM_AIR_ATTACK;
		bossPos.x      = CENTER_X;
		bossPos.y      = TARGET_HOVER_Y + std::sin(mTimer * 0.14f) * 6.0f;

		float mouthX = 0.0f, mouthY = 0.0f;
		GetAirAttackMouthWorldPos(bossPos, facingDir, drawScale, mAnimFrame, mouthX, mouthY);

		if (mAnimFrame < 3)
		{
			// チャージ中：口元へ火の粉を吸い込む
			SpawnChargeSparks(mouthX, mouthY);

			if (mAnimTimer >= 12)
			{
				mAnimTimer = 0;
				mAnimFrame++;
				if (mAnimFrame == 3)
				{
					// フレーム3（口を大きく開けて吐き出す瞬間）ででっかい火の玉を発射！
					GetAirAttackMouthWorldPos(bossPos, facingDir, drawScale, mAnimFrame, mouthX, mouthY);
					LaunchGiantFireball(mouthX, mouthY);
				}
			}
		}
		else
		{
			if (mAnimTimer >= 8)
			{
				mAnimTimer = 0;
				mAnimFrame++;
				if (mAnimFrame >= 7)
				{
					mPhase     = Phase::HoverWatch;
					mTimer     = 0;
					mAnimTimer = 0;
					mAnimFrame = 0;
				}
			}
		}
		outAnimFrame = (mAnimFrame < 7) ? mAnimFrame : 6;
		break;
	}

	case Phase::HoverWatch:
		// 4. 火の玉がマグマに着弾して縦の噴水が上がり、マグマが周囲に飛び散り始めるまで上空で羽ばたく
		isBossGrounded = false;
		outAnimType    = ANIM_FLY;
		if (mAnimTimer >= 5)
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % 7;
		}
		outAnimFrame = mAnimFrame;
		bossPos.y    = TARGET_HOVER_Y + std::sin(mTimer * 0.14f) * 10.0f;

		if (mTimer >= 65)
		{
			mPhase     = Phase::FlyToLedge;
			mTimer     = 0;
			mAnimTimer = 0;
			mAnimFrame = 0;
		}
		break;

	case Phase::FlyToLedge:
	{
		// 5. 左端または右端の足場の上空へ飛んで戻る
		isBossGrounded = false;
		outAnimType    = ANIM_FLY;
		if (mAnimTimer >= 5)
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % 7;
		}
		outAnimFrame = mAnimFrame;

		float dx = mReturnTargetX - bossPos.x;
		facingDir = (dx >= 0.0f) ? 1.0f : -1.0f;
		bossPos.y += (640.0f - bossPos.y) * 0.15f;

		const float flySpeed = 24.0f;
		if (std::fabs(dx) <= flySpeed)
		{
			bossPos.x  = mReturnTargetX;
			facingDir  = mbReturnToLeftLedge ? 1.0f : -1.0f;
			mPhase     = Phase::LandAtLedge;
			mTimer     = 0;
			mAnimTimer = 0;
			mAnimFrame = 0;
		}
		else
		{
			bossPos.x += (dx > 0.0f) ? flySpeed : -flySpeed;
		}
		break;
	}

	case Phase::LandAtLedge:
		// 6. 足場へ着陸する
		outAnimType = ANIM_LANDING;
		bossPos.x   = mReturnTargetX;
		facingDir   = mbReturnToLeftLedge ? 1.0f : -1.0f;

		if (!isBossGrounded && bossPos.y < mReturnFloorY - 100.0f)
		{
			bossPos.y += 16.0f;
			if (bossPos.y >= mReturnFloorY - 100.0f)
			{
				bossPos.y      = mReturnFloorY - 100.0f;
				isBossGrounded = true;
			}
			if (mAnimTimer >= 5 && mAnimFrame < 2)
			{
				mAnimTimer = 0;
				mAnimFrame++;
			}
		}
		else
		{
			bossPos.y      = mReturnFloorY - 100.0f;
			isBossGrounded = true;
			if (mAnimFrame < 3)
			{
				mAnimFrame = 3;
				mAnimTimer = 0;
			}
			if (mAnimTimer >= 4)
			{
				mAnimTimer = 0;
				mAnimFrame++;
				if (mAnimFrame >= 7)
				{
					mPhase = Phase::Finished;
					mTimer = 0;
				}
			}
		}
		outAnimFrame = (mAnimFrame < 7) ? mAnimFrame : 6;
		break;

	default:
		break;
	}
}

void Stage3DragonMagmaFireballAttack::UpdateProjectilesAndEffects(Player* player, BlockMap* blockMap, int attackDamage)
{
	// 1. 火の粉・マグマ飛沫パーティクルの更新
	for (auto it = mSparks.begin(); it != mSparks.end(); )
	{
		it->x += it->vx;
		it->y += it->vy;
		it->vy += 0.24f;
		it->life--;
		if (it->life <= 0) it = mSparks.erase(it);
		else ++it;
	}

	// 2. 落下中のでっかい火の玉の更新
	if (mFireball.active)
	{
		mFireball.x += mFireball.vx;
		mFireball.y += mFireball.vy;

		// 落下中に火の粉を撒き散らす
		for (int i = 0; i < 2; ++i)
		{
			LavaSpark sp;
			sp.x       = mFireball.x + static_cast<float>(GetRand(70) - 35);
			sp.y       = mFireball.y - 20.0f;
			sp.vx      = static_cast<float>(GetRand(60) - 30) * 0.1f;
			sp.vy      = -static_cast<float>(20 + GetRand(40)) * 0.1f;
			sp.size    = 4.0f + static_cast<float>(GetRand(4));
			sp.life    = 14 + GetRand(10);
			sp.maxLife = sp.life;
			sp.color   = (i == 0) ? GetColor(255, 225, 90) : GetColor(255, 80, 15);
			mSparks.push_back(sp);
		}

		// 落下中の巨大火の玉とプレイヤーの当たり判定
		if (player != nullptr && !player->GetIsDead())
		{
			float dx = player->GetPosition().x - mFireball.x;
			float dy = player->GetPosition().y - mFireball.y;
			float hitR = mFireball.radius + 22.0f;
			if (dx * dx + dy * dy <= hitR * hitR)
			{
				player->TakeDamage(attackDamage, DeathReason::DeathType::DragonExplosion);
			}
		}

		// マグマ表面（Y >= 1236）に着弾したら、まず着弾地点に縦のマグマ噴水を発生させる！
		if (mFireball.y >= MAGMA_SURFACE_Y - 10.0f)
		{
			mFireball.active = false;
			StartMagmaFountain(mFireball.x);
		}
	}

	// 3. 着弾地点から縦に吹き上がるマグマ噴水の更新
	if (mFountain.active)
	{
		mFountain.timer++;

		// 前半（0..14フレーム）：マグマ表面から真上へ向かって一気に噴水が高く伸び上がる！
		if (mFountain.timer <= 14)
		{
			float t = static_cast<float>(mFountain.timer) / 14.0f;
			float easeOut = 1.0f - (1.0f - t) * (1.0f - t);
			mFountain.currentHeight = mFountain.maxHeight * easeOut;
		}
		// 中盤（15..48フレーム）：最高高度で激しく噴き上がり続け、頂上から周囲へマグマが飛び散る！
		else if (mFountain.timer <= 48)
		{
			mFountain.currentHeight = mFountain.maxHeight + std::sin(mFountain.timer * 0.45f) * 28.0f;

			// 縦にマグマ噴水が吹き上がったあと、その頂上付近から周囲一帯へマグマ塊を飛び散らせる！
			if (mFountain.timer == 22)
			{
				SpawnMagmaSplashWave(mFountain.x, mFountain.baseY - mFountain.currentHeight * 0.78f, 0);
			}
			else if (mFountain.timer == 36)
			{
				SpawnMagmaSplashWave(mFountain.x, mFountain.baseY - mFountain.currentHeight * 0.65f, 1);
				mFountain.hasSplashed = true;
			}
		}
		// 後半（49..68フレーム）：噴水がマグマの中へ収まっていく
		else
		{
			float t = static_cast<float>(mFountain.timer - 48) / static_cast<float>(mFountain.maxTimer - 48);
			if (t > 1.0f) t = 1.0f;
			mFountain.currentHeight = mFountain.maxHeight * (1.0f - t);
		}

		// 噴水噴出中の画面揺れと飛沫スパーク
		if (blockMap != nullptr)
		{
			if (mFountain.timer < 52)
			{
				blockMap->mShakeX = GetRand(16) - 8;
				blockMap->mShakeY = GetRand(16) - 8;
			}
			else
			{
				blockMap->mShakeX = 0;
				blockMap->mShakeY = 0;
			}
		}

		if (mFountain.timer % 2 == 0 && mFountain.currentHeight > 120.0f)
		{
			float crestY = mFountain.baseY - mFountain.currentHeight;
			for (int i = 0; i < 3; ++i)
			{
				LavaSpark sp;
				sp.x       = mFountain.x + static_cast<float>(GetRand(90) - 45);
				sp.y       = crestY + static_cast<float>(GetRand(60));
				sp.vx      = static_cast<float>(GetRand(160) - 80) * 0.1f;
				sp.vy      = -static_cast<float>(30 + GetRand(70)) * 0.1f;
				sp.size    = 4.0f + static_cast<float>(GetRand(5));
				sp.life    = 16 + GetRand(12);
				sp.maxLife = sp.life;
				sp.color   = (i == 0) ? GetColor(255, 250, 170) : GetColor(255, 110, 20);
				mSparks.push_back(sp);
			}
		}

		// 縦のマグマ噴水柱とプレイヤーの当たり判定
		if (mFountain.currentHeight > 60.0f && player != nullptr && !player->GetIsDead())
		{
			float px     = player->GetPosition().x;
			float py     = player->GetPosition().y;
			float pLeft  = px - 16.0f;
			float pRight = px + 16.0f;
			float pTop   = py - 50.0f;
			float pBot   = py + 50.0f;

			float colLeft  = mFountain.x - (mFountain.halfWidth * 0.82f);
			float colRight = mFountain.x + (mFountain.halfWidth * 0.82f);
			float colTop   = mFountain.baseY - mFountain.currentHeight + 20.0f;
			float colBot   = mFountain.baseY + 20.0f;

			if (pRight > colLeft && pLeft < colRight && pBot > colTop && pTop < colBot)
			{
				player->TakeDamage(attackDamage, DeathReason::DeathType::DragonExplosion);
			}
		}

		if (mFountain.timer >= mFountain.maxTimer)
		{
			mFountain.active = false;
			if (blockMap != nullptr)
			{
				blockMap->mShakeX = 0;
				blockMap->mShakeY = 0;
			}
		}
	}

	// 4. 周囲に飛び散るマグマ塊（MagmaSplashGlob）の放物線移動＆当たり判定
	for (auto it = mSplashGlobs.begin(); it != mSplashGlobs.end(); )
	{
		if (!it->active)
		{
			it = mSplashGlobs.erase(it);
			continue;
		}

		it->age++;
		it->x  += it->vx;
		it->y  += it->vy;
		it->vy += 0.34f; // 重力で綺麗な放物線を描いて周囲へ降り注ぐ

		// 軌道上に小さなマグマの火の粉を残す
		if (it->age % 2 == 0)
		{
			LavaSpark sp;
			sp.x       = it->x + static_cast<float>(GetRand(16) - 8);
			sp.y       = it->y + static_cast<float>(GetRand(16) - 8);
			sp.vx      = -it->vx * 0.15f;
			sp.vy      = -it->vy * 0.15f;
			sp.size    = 3.0f + static_cast<float>(GetRand(3));
			sp.life    = 10 + GetRand(8);
			sp.maxLife = sp.life;
			sp.color   = GetColor(255, 140, 25);
			mSparks.push_back(sp);
		}

		// プレイヤーとの当たり判定
		if (player != nullptr && !player->GetIsDead())
		{
			float px     = player->GetPosition().x;
			float py     = player->GetPosition().y;
			float pLeft  = px - 16.0f;
			float pRight = px + 16.0f;
			float pTop   = py - 50.0f;
			float pBot   = py + 50.0f;

			float hitR = it->radius * 0.78f;
			if (it->x + hitR > pLeft && it->x - hitR < pRight &&
			    it->y + hitR > pTop  && it->y - hitR < pBot)
			{
				player->TakeDamage(attackDamage, DeathReason::DeathType::DragonExplosion);
			}
		}

		// マグマ表面または左右の足場に着弾したら飛沫を散らして消滅
		bool hitSurface = (it->vy > 0.0f && it->y >= MAGMA_SURFACE_Y);
		if (!hitSurface && it->vy > 0.0f && blockMap != nullptr && it->y >= 1080.0f)
		{
			if (blockMap->GetCollisionType(static_cast<int>(it->x), static_cast<int>(it->y)) == BlockMap::CollisionType::Block)
			{
				hitSurface = true;
			}
		}

		if (hitSurface || it->x < 3350.0f || it->x > 5720.0f)
		{
			for (int s = 0; s < 5; ++s)
			{
				LavaSpark sp;
				sp.x       = it->x;
				sp.y       = it->y - 4.0f;
				sp.vx      = static_cast<float>(GetRand(100) - 50) * 0.1f;
				sp.vy      = -static_cast<float>(25 + GetRand(55)) * 0.1f;
				sp.size    = 3.0f + static_cast<float>(GetRand(4));
				sp.life    = 12 + GetRand(10);
				sp.maxLife = sp.life;
				sp.color   = (s % 2 == 0) ? GetColor(255, 210, 70) : GetColor(255, 80, 15);
				mSparks.push_back(sp);
			}
			it = mSplashGlobs.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void Stage3DragonMagmaFireballAttack::Draw(BlockMap* blockMap, const VECTOR& bossPos, float facingDir, float drawScale) const
{
	if (blockMap == nullptr) return;

	int drawScrollX = blockMap->GetScrollX() + blockMap->mShakeX;
	int drawScrollY = blockMap->GetScrollY() + blockMap->mShakeY;
	int now         = GetNowCount();

	SetDrawBlendMode(DX_BLENDMODE_ADD, 235);

	// 1. 口元ででっかい火の玉を溜めているチャージ演出（ステージ2のドラゴンと同じ演出）
	if (mPhase == Phase::ChargeAndShoot && mAnimFrame < 3)
	{
		float mouthWX = 0.0f, mouthWY = 0.0f;
		GetAirAttackMouthWorldPos(bossPos, facingDir, drawScale, mAnimFrame, mouthWX, mouthWY);
		int mouthSX = static_cast<int>(mouthWX) - drawScrollX;
		int mouthSY = static_cast<int>(mouthWY) - drawScrollY;
		int chargeR = 20 + mAnimFrame * 18 + (now / 25) % 8;

		if (mBurstRedTex != -1)
		{
			double sc = (chargeR * 3.8) / 765.0;
			float rot = now * 0.005f;
			SetDrawBright(255, 130, 30);
			DrawRotaGraph(mouthSX, mouthSY, sc, rot, mBurstRedTex, TRUE);
			SetDrawBright(255, 235, 115);
			DrawRotaGraph(mouthSX, mouthSY, sc * 0.65, -rot * 1.4f, mBurstRedTex, TRUE);
			SetDrawBright(255, 255, 255);
		}
		else
		{
			DrawCircle(mouthSX, mouthSY, chargeR + 14, GetColor(200, 30, 5), TRUE);
			DrawCircle(mouthSX, mouthSY, chargeR, GetColor(255, 85, 15), TRUE);
			DrawCircle(mouthSX, mouthSY, static_cast<int>(chargeR * 0.68f), GetColor(255, 195, 45), TRUE);
			DrawCircle(mouthSX, mouthSY, static_cast<int>(chargeR * 0.38f), GetColor(255, 255, 210), TRUE);
		}
	}

	// 2. 中央へ落下するでっかい火の玉の描画
	if (mFireball.active)
	{
		int cx = static_cast<int>(mFireball.x) - drawScrollX;
		int cy = static_cast<int>(mFireball.y) - drawScrollY;
		int r  = static_cast<int>(mFireball.radius) + (now / 20) % 8;

		if (mBurstRedTex != -1 || mFireSingleTex != -1)
		{
			float moveRot = std::atan2(mFireball.vy, mFireball.vx) + 1.5707963f;
			float spinRot = now * 0.007f;
			int tx = static_cast<int>(mFireball.x - mFireball.vx * 1.8f) - drawScrollX;
			int ty = static_cast<int>(mFireball.y - mFireball.vy * 1.8f) - drawScrollY;
			if (mFireSingleTex != -1)
			{
				double fSc = (mFireball.radius * 3.1) / 256.0;
				SetDrawBright(255, 80, 12);
				DrawRotaGraph(tx, ty, fSc, moveRot, mFireSingleTex, TRUE);
			}
			if (mBurstRedTex != -1)
			{
				double bSc = (mFireball.radius * 3.8) / 765.0;
				SetDrawBright(255, 130, 25);
				DrawRotaGraph(cx, cy, bSc, spinRot, mBurstRedTex, TRUE);
				SetDrawBright(255, 235, 120);
				DrawRotaGraph(cx, cy, bSc * 0.62, -spinRot * 1.35f, mBurstRedTex, TRUE);
			}
			DrawCircle(cx, cy, static_cast<int>(r * 0.45f), GetColor(255, 250, 205), TRUE);
			SetDrawBright(255, 255, 255);
		}
		else
		{
			DrawOval(cx, cy - 38, static_cast<int>(r * 0.78f), r + 34, GetColor(200, 30, 5), TRUE);
			DrawCircle(cx, cy, r + 14, GetColor(190, 25, 5), TRUE);
			DrawCircle(cx, cy, r, GetColor(255, 75, 10), TRUE);
			DrawCircle(cx, cy, static_cast<int>(r * 0.72f), GetColor(255, 185, 35), TRUE);
			DrawCircle(cx, cy, static_cast<int>(r * 0.42f), GetColor(255, 255, 210), TRUE);
		}
	}

	// 3. 着弾地点から縦に高く吹き上がる「マグマの噴水」の描画！
	if (mFountain.active && mFountain.currentHeight > 10.0f)
	{
		int fx    = static_cast<int>(mFountain.x) - drawScrollX;
		int baseY = static_cast<int>(mFountain.baseY) - drawScrollY;
		int topY  = static_cast<int>(mFountain.baseY - mFountain.currentHeight) - drawScrollY;
		int hw    = static_cast<int>(mFountain.halfWidth);

		// (A) 着弾地点（マグマ表面）の巨大な沸騰グロウドーム
		DrawOval(fx, baseY, hw + 75, 34, GetColor(200, 30, 5), TRUE);
		DrawOval(fx, baseY, hw + 42, 22, GetColor(255, 100, 15), TRUE);
		DrawOval(fx, baseY, hw + 15, 12, GetColor(255, 220, 90), TRUE);

		// (B) 下から上へ脈動しながら垂直に突き上がる多層マグマ柱
		const int segments = 18;
		float segH = mFountain.currentHeight / static_cast<float>(segments);
		for (int i = 0; i < segments; ++i)
		{
			float ratio = static_cast<float>(i) / static_cast<float>(segments); // 0.0(底) -> 1.0(頂上)
			int y1 = baseY - static_cast<int>(i * segH);
			int y2 = baseY - static_cast<int>((i + 1) * segH) - 2;

			// 噴水らしく、根元が太く、中間が少し締まり、頂上（噴水の傘）で大きく左右に広がるシルエット
			float fountainProfile = 1.0f - std::sin(ratio * 3.14159265f) * 0.22f;
			if (ratio > 0.75f)
			{
				fountainProfile += (ratio - 0.75f) * 2.4f; // 頂上部で噴水のようにブワッと広がる
			}
			float pulse = std::sin(now * 0.035f - i * 0.7f) * 9.0f;
			int curHW = static_cast<int>(hw * fountainProfile + pulse);
			if (curHW < 16) curHW = 16;

			// 外層：深紅の溶岩流
			DrawBox(fx - curHW - 10, y2, fx + curHW + 10, y1, GetColor(190, 25, 5), TRUE);
			// 中層：灼熱のオレンジマグマ
			DrawBox(fx - curHW, y2, fx + curHW, y1, GetColor(255, 80, 12), TRUE);
			// 内層：黄金マグマ
			DrawBox(fx - static_cast<int>(curHW * 0.62f), y2, fx + static_cast<int>(curHW * 0.62f), y1, GetColor(255, 185, 35), TRUE);
			// 中心核：白熱マグマコア
			DrawBox(fx - static_cast<int>(curHW * 0.30f), y2, fx + static_cast<int>(curHW * 0.30f), y1, GetColor(255, 250, 195), TRUE);
		}

		// (C) 噴水の頂上部で左右へアーチ状に吹きこぼれる噴水クラウン（傘）演出
		int crestRadiusX = static_cast<int>(hw * 1.65f + std::sin(now * 0.04f) * 14.0f);
		int crestRadiusY = 42;
		DrawOval(fx, topY + 12, crestRadiusX + 16, crestRadiusY + 10, GetColor(210, 35, 5), TRUE);
		DrawOval(fx, topY + 12, crestRadiusX, crestRadiusY, GetColor(255, 110, 20), TRUE);
		DrawOval(fx, topY + 10, static_cast<int>(crestRadiusX * 0.68f), static_cast<int>(crestRadiusY * 0.68f), GetColor(255, 215, 70), TRUE);

		// (D) 2D Effekseer による火山噴火マグマ演出
		float surge = std::fmod(mFountain.timer * 0.06f, 1.0f);
		if (mFireSingleTex != -1)
		{
			for (int fi = 0; fi < 8; ++fi)
			{
				float fyRatio = std::fmod((static_cast<float>(fi) / 8.0f) + surge, 1.0f);
				int fy = baseY - static_cast<int>(mFountain.currentHeight * fyRatio);
				int xSway = static_cast<int>(std::sin(mFountain.timer * 0.28f + fi * 1.5f) * 18.0f);
				double sc = 0.90 + fyRatio * 0.40;
				SetDrawBright(255, 100 + static_cast<int>(fyRatio * 95), 18);
				DrawRotaGraph(fx + xSway, fy, sc, (fi % 2 == 0) ? 0.16 : -0.16, mFireSingleTex, TRUE);
			}
		}
		if (mBurstRedTex != -1)
		{
			float rot = mFountain.timer * 0.10f;
			SetDrawBright(255, 95, 15);
			DrawRotaGraph(fx, baseY - 20, 0.62, rot, mBurstRedTex, TRUE);
			SetDrawBright(255, 200, 55);
			DrawRotaGraph(fx, topY + 12, 0.68, -rot * 1.25f, mBurstRedTex, TRUE);
		}
		if (mSplashTex != -1)
		{
			SetDrawBright(255, 75, 10);
			DrawRotaGraph3(fx - 60, baseY - 90, 256, 256, 0.74, 0.74, -0.58, mSplashTex, TRUE);
			DrawRotaGraph3(fx + 60, baseY - 90, 256, 256, 0.74, 0.74,  0.58, mSplashTex, TRUE);
			SetDrawBright(255, 105, 15);
			DrawRotaGraph3(fx - 36, baseY - 115, 256, 256, 0.84, 0.88, -0.28, mSplashTex, TRUE);
			DrawRotaGraph3(fx + 36, baseY - 115, 256, 256, 0.84, 0.88,  0.28, mSplashTex, TRUE);
			SetDrawBright(255, 165, 25);
			DrawRotaGraph3(fx, baseY - 140, 256, 256, 1.12, 1.00, 0.0, mSplashTex, TRUE);
			SetDrawBright(255, 145, 25);
			DrawRotaGraph3(fx - 72, topY - 14, 256, 256, 0.82, 0.52, -0.68, mSplashTex, TRUE);
			DrawRotaGraph3(fx + 72, topY - 14, 256, 256, 0.82, 0.52,  0.68, mSplashTex, TRUE);
			SetDrawBright(255, 185, 40);
			DrawRotaGraph3(fx - 42, topY - 28, 256, 256, 0.98, 0.62, -0.34, mSplashTex, TRUE);
			DrawRotaGraph3(fx + 42, topY - 28, 256, 256, 0.98, 0.62,  0.34, mSplashTex, TRUE);
			SetDrawBright(255, 235, 95);
			DrawRotaGraph3(fx, topY - 48, 256, 256, 1.30, 0.74, 0.0, mSplashTex, TRUE);
		}
		float rippleT = std::fmod(mFountain.timer * 0.05f, 1.0f);
		DrawOval(fx, baseY + 2, static_cast<int>(hw * (2.0f + rippleT * 1.5f)), static_cast<int>(18.0f + rippleT * 11.0f), GetColor(255, 120, 20), FALSE, 4);
		DrawOval(fx, baseY + 2, static_cast<int>(hw * 1.7f), 18, GetColor(255, 210, 75), FALSE, 3);
		SetDrawBright(255, 255, 255);
	}

	// 4. 噴水から周囲へ放物線を描いて飛び散るマグマ塊の描画
	for (const auto& glob : mSplashGlobs)
	{
		if (!glob.active) continue;

		int gx = static_cast<int>(glob.x) - drawScrollX;
		int gy = static_cast<int>(glob.y) - drawScrollY;
		int gr = static_cast<int>(glob.radius);

		// 進行方向逆側へ引くマグマの尾
		int tailX = static_cast<int>(glob.x - glob.vx * 1.8f) - drawScrollX;
		int tailY = static_cast<int>(glob.y - glob.vy * 1.8f) - drawScrollY;

		if (mBurstRedTex != -1 || mFireSingleTex != -1)
		{
			float rot = std::atan2(glob.vy, glob.vx) + 1.5707963f;
			if (mFireSingleTex != -1)
			{
				double sc = (glob.radius * 2.8) / 256.0;
				SetDrawBright(255, 85, 14);
				DrawRotaGraph(tailX, tailY, sc, rot, mFireSingleTex, TRUE);
			}
			if (mBurstRedTex != -1)
			{
				double bSc = (glob.radius * 3.4) / 765.0;
				SetDrawBright(255, 165, 35);
				DrawRotaGraph(gx, gy, bSc, glob.age * 0.18f, mBurstRedTex, TRUE);
			}
			DrawCircle(gx, gy, static_cast<int>(gr * 0.45f), GetColor(255, 245, 175), TRUE);
			SetDrawBright(255, 255, 255);
		}
		else
		{
			DrawCircle(tailX, tailY, static_cast<int>(gr * 0.78f), GetColor(190, 25, 5), TRUE);
			DrawCircle(gx, gy, gr + 5, GetColor(190, 25, 5), TRUE);
			DrawCircle(gx, gy, gr, GetColor(255, 80, 12), TRUE);
			DrawCircle(gx, gy, static_cast<int>(gr * 0.68f), GetColor(255, 190, 40), TRUE);
			DrawCircle(gx, gy, static_cast<int>(gr * 0.36f), GetColor(255, 255, 200), TRUE);
		}
	}

	// 5. 火の粉・マグマ飛沫パーティクルの描画
	for (const auto& sp : mSparks)
	{
		int sx = static_cast<int>(sp.x) - drawScrollX;
		int sy = static_cast<int>(sp.y) - drawScrollY;
		int sr = static_cast<int>(sp.size * (static_cast<float>(sp.life) / static_cast<float>(sp.maxLife)));
		if (sr < 1) sr = 1;
		DrawCircle(sx, sy, sr, sp.color, TRUE);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

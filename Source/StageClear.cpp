#include "StageClear.h"
#include "GameConstants.h"
#include "Master.h"
#include "GameManager.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "FontManager.h"
#include "Player.h"
#include "BlockMap.h"
#include "DeathReason.h"
#include "DxLib.h"
#include <cmath>
#include <algorithm>

StageClear* StageClear::sInstance = nullptr;

StageClear::StageClear()
	: mState(State::Inactive)
	, mTimer(0)
	, mBurstTimer(-1)
	, mCrackerGraph(-1)
	, mAimedCrackerIndex(0)
	, mAimLocked(false)
	, mnLogoHandle(-1)
	, mLogoY(StageClearConstants::LogoInitialY)
	, mLogoVy(0.0f)
	, mLogoTargetY(0.0f)
	, mLogoScale(1.38f)
	, mbLogoLanded(false)
	, mLogoImpactTimer(0)
	, mLogoReboundOffset(0.0f)
	, mDoorWorldX(0.0f)
	, mDoorWorldY(0.0f)
	, mDoorAnimProgress(0.0f)
	, mDoorCurrentFrame(0)
	, mDoorPopScale(0.0f)
	, mbDoorDrawnThisFrame(false)
	, mpBlockMap(nullptr)
{
	sInstance = this;
	mCrackerGraph = LoadGraph("Resource/Image/Cracker.png");
	mnLogoHandle = LoadGraph(BackGroundGraphPath::TitleLogoGraphs.c_str());

	for (int i = 0; i < StageClearConstants::FlashFrameCount; i++)
	{
		mFlashGraphs[i] = -1;
	}
	LoadDivGraph(
		"Resource/Image/Flash.png",
		StageClearConstants::FlashFrameCount,
		StageClearConstants::FlashFrameCount,
		1,
		StageClearConstants::FlashFrameWidth,
		StageClearConstants::FlashFrameHeight,
		mFlashGraphs
	);

	for (int i = 0; i < StageClearConstants::DoorFrameCount; i++)
	{
		mDoorGraphs[i] = -1;
	}
	LoadDivGraph(
		"Resource/Image/OpenDoor.png",
		StageClearConstants::DoorFrameCount,
		StageClearConstants::DoorFrameCount,
		1,
		StageClearConstants::DoorFrameWidth,
		StageClearConstants::DoorFrameHeight,
		mDoorGraphs
	);
}

StageClear::~StageClear()
{
	if (mCrackerGraph != -1)
	{
		DeleteGraph(mCrackerGraph);
		mCrackerGraph = -1;
	}
	if (mnLogoHandle != -1)
	{
		DeleteGraph(mnLogoHandle);
		mnLogoHandle = -1;
	}
	for (int i = 0; i < StageClearConstants::FlashFrameCount; i++)
	{
		if (mFlashGraphs[i] != -1)
		{
			DeleteGraph(mFlashGraphs[i]);
			mFlashGraphs[i] = -1;
		}
	}
	for (int i = 0; i < StageClearConstants::DoorFrameCount; i++)
	{
		if (mDoorGraphs[i] != -1)
		{
			DeleteGraph(mDoorGraphs[i]);
			mDoorGraphs[i] = -1;
		}
	}
	if (sInstance == this)
	{
		sInstance = nullptr;
	}
}

void StageClear::Start()
{
	if (mState == State::Inactive)
	{
		mState = State::PreCrackerDelay;
		mTimer = 0;
		mBurstTimer = -1;
		mAimedCrackerIndex = GetRand(5); // 0〜5からランダムで1基を狙撃手に選出
		mAimLocked = false;
		InitCrackers();
		mConfetti.clear();
		mBullets.clear();
		mImpactDusts.clear();
		mDoorWorldX = 0.0f;
		mDoorWorldY = 0.0f;
		mDoorAnimProgress = 0.0f;
		mDoorCurrentFrame = 0;
		mDoorPopScale = 0.0f;
		mDoorSmoke.clear();
		InitLogo();
	}
}

void StageClear::Reset()
{
	mState = State::Inactive;
	mTimer = 0;
	mBurstTimer = -1;
	mAimedCrackerIndex = 0;
	mAimLocked = false;
	mCrackers.clear();
	mConfetti.clear();
	mBullets.clear();
	mImpactDusts.clear();
	mLogoY = StageClearConstants::LogoInitialY;
	mLogoVy = 0.0f;
	mbLogoLanded = false;
	mLogoImpactTimer = 0;
	mLogoReboundOffset = 0.0f;
	mDoorWorldX = 0.0f;
	mDoorWorldY = 0.0f;
	mDoorAnimProgress = 0.0f;
	mDoorCurrentFrame = 0;
	mDoorPopScale = 0.0f;
	mbDoorDrawnThisFrame = false;
	mDoorSmoke.clear();
	mpBlockMap = nullptr;
}

void StageClear::InitCrackers()
{
	mCrackers.clear();
	mCrackers.resize(6);

	const float kPi = 3.14159265f;

	// 左側 3基（上空へ綺麗な扇状に配置：-28度, -45度, -62度）
	// 通常画像は-45度を向いているため、描画角度 angle = shootAngleRad + 45度
	mCrackers[0] = { StageClearConstants::CrackerLeftStartX, StageClearConstants::CrackerPosYTop,
		StageClearConstants::CrackerLeftStartX, StageClearConstants::CrackerLeftTargetX, StageClearConstants::CrackerPosYTop,
		(-28.0f + 45.0f) * kPi / 180.0f, -28.0f * kPi / 180.0f, false, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f };

	mCrackers[1] = { StageClearConstants::CrackerLeftStartX, StageClearConstants::CrackerPosYMid,
		StageClearConstants::CrackerLeftStartX, StageClearConstants::CrackerLeftTargetX, StageClearConstants::CrackerPosYMid,
		(-45.0f + 45.0f) * kPi / 180.0f, -45.0f * kPi / 180.0f, false, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f };

	mCrackers[2] = { StageClearConstants::CrackerLeftStartX, StageClearConstants::CrackerPosYBottom,
		StageClearConstants::CrackerLeftStartX, StageClearConstants::CrackerLeftTargetX, StageClearConstants::CrackerPosYBottom,
		(-62.0f + 45.0f) * kPi / 180.0f, -62.0f * kPi / 180.0f, false, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f };

	// 右側 3基（左右対称に上空へ扇状に配置：-152度, -135度, -118度）
	// 水平反転(TurnFlag=TRUE)画像は-135度を向くため、描画角度 angle = shootAngleRad + 135度
	mCrackers[3] = { StageClearConstants::CrackerRightStartX, StageClearConstants::CrackerPosYTop,
		StageClearConstants::CrackerRightStartX, StageClearConstants::CrackerRightTargetX, StageClearConstants::CrackerPosYTop,
		(-152.0f + 135.0f) * kPi / 180.0f, -152.0f * kPi / 180.0f, true, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f };

	mCrackers[4] = { StageClearConstants::CrackerRightStartX, StageClearConstants::CrackerPosYMid,
		StageClearConstants::CrackerRightStartX, StageClearConstants::CrackerRightTargetX, StageClearConstants::CrackerPosYMid,
		(-135.0f + 135.0f) * kPi / 180.0f, -135.0f * kPi / 180.0f, true, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f };

	mCrackers[5] = { StageClearConstants::CrackerRightStartX, StageClearConstants::CrackerPosYBottom,
		StageClearConstants::CrackerRightStartX, StageClearConstants::CrackerRightTargetX, StageClearConstants::CrackerPosYBottom,
		(-118.0f + 135.0f) * kPi / 180.0f, -118.0f * kPi / 180.0f, true, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f };

	for (auto& c : mCrackers)
	{
		c.mouthX = c.x + 125.0f * std::cos(c.shootAngleRad);
		c.mouthY = c.y + 125.0f * std::sin(c.shootAngleRad);
	}
}

void StageClear::TrackPlayerWithCracker(int crackerIdx, Player* player, BlockMap* pBlockMap)
{
	if (crackerIdx < 0 || crackerIdx >= static_cast<int>(mCrackers.size()) || player == nullptr)
	{
		return;
	}

	const float kPi = 3.14159265f;
	auto& c = mCrackers[crackerIdx];

	float scrollX = (pBlockMap != nullptr) ? static_cast<float>(pBlockMap->GetScrollX()) : 0.0f;
	float scrollY = (pBlockMap != nullptr) ? static_cast<float>(pBlockMap->GetScrollY()) : 0.0f;

	float pScreenX = player->GetPosition().x - scrollX;
	float pScreenY = player->GetPosition().y - scrollY;

	float dx = pScreenX - c.x;
	float dy = pScreenY - c.y;

	float targetShootAngle = std::atan2(dy, dx);
	c.shootAngleRad = targetShootAngle;

	if (!c.isRightSide)
	{
		c.angle = targetShootAngle + (45.0f * kPi / 180.0f);
	}
	else
	{
		c.angle = targetShootAngle + (135.0f * kPi / 180.0f);
	}

	c.mouthX = c.x + 125.0f * std::cos(c.shootAngleRad);
	c.mouthY = c.y + 125.0f * std::sin(c.shootAngleRad);
}

void StageClear::BurstCrackers()
{
	const float kPi = 3.14159265f;

	const int kColors[6][3] = {
		{ 255, 60,  60  },
		{ 255, 215, 0   },
		{ 60,  140, 255 },
		{ 60,  220, 80  },
		{ 255, 120, 220 },
		{ 100, 240, 240 }
	};

	mConfetti.clear();
	mConfetti.reserve(StageClearConstants::ConfettiPerCracker * 6);
	mBullets.clear();
	mBullets.reserve(mCrackers.size() * StageClearConstants::BulletsPerCracker);

	for (size_t i = 0; i < mCrackers.size(); i++)
	{
		auto& c = mCrackers[i];

		c.recoilOffset = 14.0f;
		c.squishScaleX = 1.25f;
		c.squishScaleY = 0.85f;

		for (int bIdx = 0; bIdx < StageClearConstants::BulletsPerCracker; bIdx++)
		{
			SilverBullet b;
			b.x = c.mouthX;
			b.y = c.mouthY;
			b.radius = StageClearConstants::BulletRadius;
			b.isActive = true;

			for (int k = 0; k < 3; k++)
			{
				b.prevX[k] = b.x;
				b.prevY[k] = b.y;
			}

			if (static_cast<int>(i) == mAimedCrackerIndex)
			{
				b.isAimed = true;
				float fanAngle = (bIdx - 1) * 0.05f;
				float randAngle = static_cast<float>(GetRand(60) - 30) / 1000.0f;
				float shootAngle = c.shootAngleRad + fanAngle + randAngle;

				float speedRand = static_cast<float>(GetRand(30) - 15) / 10.0f;
				float speed = StageClearConstants::BulletAimedSpeed + speedRand;

				float addVx = static_cast<float>(GetRand(20) - 10) / 10.0f;
				float addVy = static_cast<float>(GetRand(20) - 10) / 10.0f;

				b.vx = speed * std::cos(shootAngle) + addVx;
				b.vy = speed * std::sin(shootAngle) + addVy;
				b.gravity = StageClearConstants::BulletAimedGravity;
			}
			else
			{
				b.isAimed = false;
				float fanAngle = (bIdx - 1) * 0.12f;
				float randAngle = static_cast<float>(GetRand(120) - 60) / 1000.0f;
				float shootAngle = c.shootAngleRad + fanAngle + randAngle;

				float speedRand = static_cast<float>(GetRand(60) - 30) / 10.0f;
				float speed = StageClearConstants::BulletSkySpeed + speedRand;

				float addVx = static_cast<float>(GetRand(24) - 12) / 10.0f;
				float addVy = static_cast<float>(GetRand(24) - 12) / 10.0f;

				b.vx = speed * std::cos(shootAngle) + addVx;
				b.vy = speed * std::sin(shootAngle) + addVy;

				float randGrav = static_cast<float>(GetRand(6) - 3) * 0.015f;
				b.gravity = StageClearConstants::BulletSkyGravity + randGrav;
			}

			mBullets.push_back(b);
		}

		for (int j = 0; j < StageClearConstants::ConfettiPerCracker; j++)
		{
			ConfettiParticle p;
			p.x = c.mouthX;
			p.y = c.mouthY;

			float spreadDeg = static_cast<float>(GetRand(440) - 220) / 10.0f;
			float shootRad = c.shootAngleRad + (spreadDeg * kPi / 180.0f);
			float speed = 18.0f + static_cast<float>(GetRand(120)) / 10.0f;

			p.vx = speed * std::cos(shootRad);
			p.vy = speed * std::sin(shootRad);
			p.angle = shootRad;

			p.spinAngle = static_cast<float>(GetRand(628)) / 100.0f;
			p.spinSpeed = 0.14f + static_cast<float>(GetRand(180)) / 1000.0f;

			p.sizeW = 10.0f + static_cast<float>(GetRand(60)) / 10.0f;
			p.sizeH = 16.0f + static_cast<float>(GetRand(80)) / 10.0f;

			int colIdx = GetRand(5);
			p.r = kColors[colIdx][0];
			p.g = kColors[colIdx][1];
			p.b = kColors[colIdx][2];

			p.shapeType = (GetRand(3) == 0) ? 1 : 0;

			p.timer = 0;
			p.life = StageClearConstants::ConfettiLifeFrames - GetRand(30);
			p.isActive = true;

			mConfetti.push_back(p);
		}
	}
}

void StageClear::UpdateCrackers()
{
	if (mState == State::CrackerSlideIn)
	{
		float t = static_cast<float>(mTimer) / static_cast<float>(StageClearConstants::CrackerSlideFrames);
		if (t > 1.0f) t = 1.0f;
		float ease = 1.0f - std::pow(1.0f - t, 3.0f);

		for (auto& c : mCrackers)
		{
			c.x = c.startX + (c.targetX - c.startX) * ease;
			c.y = c.targetY;
			c.mouthX = c.x + 125.0f * std::cos(c.shootAngleRad);
			c.mouthY = c.y + 125.0f * std::sin(c.shootAngleRad);
		}
	}
	else
	{
		for (auto& c : mCrackers)
		{
			c.recoilOffset *= 0.80f;
			c.squishScaleX += (1.0f - c.squishScaleX) * 0.20f;
			c.squishScaleY += (1.0f - c.squishScaleY) * 0.20f;

			c.mouthX = c.x + 125.0f * std::cos(c.shootAngleRad);
			c.mouthY = c.y + 125.0f * std::sin(c.shootAngleRad);
		}
	}
}

void StageClear::UpdateConfetti()
{
	for (auto& p : mConfetti)
	{
		if (!p.isActive) continue;

		p.x += p.vx;
		p.y += p.vy;

		p.vx *= 0.93f;
		p.vy += 0.18f;
		p.spinAngle += p.spinSpeed;
		p.x += std::sin(static_cast<float>(p.timer) * 0.12f) * 1.5f;

		p.timer++;
		if (p.timer >= p.life || p.y > 1150.0f)
		{
			p.isActive = false;
		}
	}
}

void StageClear::UpdateBullets(Player* player, BlockMap* pBlockMap)
{
	float scrollX = (pBlockMap != nullptr) ? static_cast<float>(pBlockMap->GetScrollX()) : 0.0f;
	float scrollY = (pBlockMap != nullptr) ? static_cast<float>(pBlockMap->GetScrollY()) : 0.0f;

	float pScreenX = (player != nullptr) ? (player->GetPosition().x - scrollX) : -9999.0f;
	float pScreenY = (player != nullptr) ? (player->GetPosition().y - scrollY) : -9999.0f;
	float halfW = (player != nullptr) ? (player->GetPlayerWidth() * 0.40f) : 20.0f;
	float halfH = (player != nullptr) ? (player->GetPlayerHeight() * 0.45f) : 40.0f;

	for (auto& b : mBullets)
	{
		if (!b.isActive) continue;

		b.prevX[2] = b.prevX[1];
		b.prevY[2] = b.prevY[1];
		b.prevX[1] = b.prevX[0];
		b.prevY[1] = b.prevY[0];
		b.prevX[0] = b.x;
		b.prevY[0] = b.y;

		b.x += b.vx;
		b.y += b.vy;
		b.vy += b.gravity;

		if (player != nullptr && !player->GetIsDead())
		{
			auto clampVal = [](float val, float minV, float maxV) {
				return (val < minV) ? minV : ((val > maxV) ? maxV : val);
			};
			float cx = clampVal(b.x, pScreenX - halfW, pScreenX + halfW);
			float cy = clampVal(b.y, pScreenY - halfH, pScreenY + halfH);
			float dx = b.x - cx;
			float dy = b.y - cy;
			float hitRadius = b.radius * 0.85f;
			if ((dx * dx + dy * dy) <= (hitRadius * hitRadius))
			{
				b.isActive = false;

				// PostTutorial1およびTutorial2の時は10ダメージ、それ以外は即死
				GameProgress prog = player->GetGameProgress();
				bool isTenDmg = (prog == GameProgress::PostTutorial1 || prog == GameProgress::Tutorial2);
				int dmg = isTenDmg ? StageClearConstants::BulletDamagePostTutorial : player->GetHp();

				player->SetDeathReason(DeathReason::DeathType::EnemyHit);
				player->TakeBulletDamage(dmg);

				if (player->GetHp() <= 0 || player->GetIsDead())
				{
					mState = State::Inactive;
					return;
				}
			}
		}

		if (b.y > 1200.0f || b.x < -200.0f || b.x > 2120.0f)
		{
			b.isActive = false;
		}
	}
}

void StageClear::InitLogo()
{
	mLogoY = StageClearConstants::LogoInitialY;
	mLogoVy = 4.0f;
	mbLogoLanded = false;
	mLogoImpactTimer = 0;
	mLogoReboundOffset = 0.0f;
	mpBlockMap = nullptr;
	mImpactDusts.clear();

	int textWidth = 840;
	if (Master::mpGameManager)
	{
		FontManager* fontMgr = Master::mpGameManager->GetFontManager();
		if (fontMgr)
		{
			int w = fontMgr->GetDotStringWidth(StageClearConstants::FontSize, "ステージクリア");
			if (w > 0) textWidth = w;
		}
	}

	mLogoScale = static_cast<float>(textWidth) / 611.0f;

	float sinA = std::sin(StageClearConstants::LogoTiltAngleRad);
	float cosA = std::cos(StageClearConstants::LogoTiltAngleRad);

	// 画像内の可視先端（非透明ピクセル px=42, py=209）を基点として接地目標を計算
	float rx = (42.0f - 305.5f) * mLogoScale;
	float ry = (209.0f - 167.0f) * mLogoScale;
	float tipRelY = rx * sinA + ry * cosA;

	float targetTipY = StageClearConstants::LogoGroundY + StageClearConstants::LogoEmbedDepth;
	mLogoTargetY = targetTipY - tipRelY;
}

void StageClear::SpawnImpactDust(float x, float y)
{
	mImpactDusts.clear();
	mImpactDusts.reserve(24);

	for (int i = 0; i < 20; i++)
	{
		ImpactDust d;
		d.x = x + static_cast<float>(GetRand(30) - 15);
		d.y = y + static_cast<float>(GetRand(10) - 5);

		float speedX = static_cast<float>(GetRand(240) - 120) / 10.0f;
		float speedY = -static_cast<float>(GetRand(80) + 20) / 10.0f;
		d.vx = speedX;
		d.vy = speedY;
		d.size = 12.0f + static_cast<float>(GetRand(14));
		d.alpha = 210 + GetRand(45);
		d.life = 25 + GetRand(15);
		d.timer = 0;
		d.isActive = true;
		mImpactDusts.push_back(d);
	}
}

void StageClear::UpdateImpactDust()
{
	for (auto& d : mImpactDusts)
	{
		if (!d.isActive) continue;

		d.x += d.vx;
		d.y += d.vy;
		d.vx *= 0.92f;
		d.vy += 0.28f;
		d.size += 0.45f;
		d.timer++;

		if (d.timer >= d.life)
		{
			d.isActive = false;
		}
	}
}

void StageClear::DrawImpactDust() const
{
	const BlockMap* blockMap = mpBlockMap;
	int shakeX = (blockMap != nullptr) ? blockMap->GetShakeOffsetX() : 0;
	int shakeY = (blockMap != nullptr) ? blockMap->GetShakeOffsetY() : 0;

	for (const auto& d : mImpactDusts)
	{
		if (!d.isActive) continue;

		float progress = static_cast<float>(d.timer) / static_cast<float>(d.life);
		int alpha = static_cast<int>(static_cast<float>(d.alpha) * (1.0f - progress));

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawCircleAA(d.x - shakeX, d.y - shakeY, d.size, 16, GetColor(200, 185, 165), TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void StageClear::UpdateLogo(Player* player, BlockMap* pBlockMap)
{
	float sinA = std::sin(StageClearConstants::LogoTiltAngleRad);
	float cosA = std::cos(StageClearConstants::LogoTiltAngleRad);

	if (!mbLogoLanded)
	{
		mLogoY += mLogoVy;
		mLogoVy += StageClearConstants::LogoFallGravity;
		if (mLogoVy > StageClearConstants::LogoFallMaxSpeed)
		{
			mLogoVy = StageClearConstants::LogoFallMaxSpeed;
		}

		float scrollX = (pBlockMap != nullptr) ? static_cast<float>(pBlockMap->GetScrollX()) : 0.0f;
		float scrollY = (pBlockMap != nullptr) ? static_cast<float>(pBlockMap->GetScrollY()) : 0.0f;

		// プレイヤー押し潰し判定（案B: 即死トラップ）
		// 画像の透明余白を除外し、狭く適正化した当たり判定（幅480px, 高320px）で判定
		if (player != nullptr && !player->GetIsDead())
		{
			float pScreenX = player->GetPosition().x - scrollX;
			float pScreenY = player->GetPosition().y - scrollY;
			float halfW = player->GetPlayerWidth() * 0.40f;
			float halfH = player->GetPlayerHeight() * 0.45f;

			float logoLeft = ScreenSize::CenterX - StageClearConstants::LogoHitboxHalfWidth;
			float logoRight = ScreenSize::CenterX + StageClearConstants::LogoHitboxHalfWidth;
			float logoBottom = mLogoY + StageClearConstants::LogoHitboxHalfHeight;
			float logoTop = mLogoY - StageClearConstants::LogoHitboxHalfHeight;

			bool xOverlap = (pScreenX + halfW >= logoLeft) && (pScreenX - halfW <= logoRight);
			bool yOverlap = (pScreenY + halfH >= logoTop) && (pScreenY - halfH <= logoBottom);

			if (xOverlap && yOverlap)
			{
				player->SetDeathReason(DeathReason::DeathType::EnemyHit);
				player->TakeDamage(player->GetHp());
				mState = State::Inactive;
				return;
			}
		}

		if (mLogoY >= mLogoTargetY)
		{
			mLogoY = mLogoTargetY;
			mLogoVy = 0.0f;
			mbLogoLanded = true;
			mLogoImpactTimer = 0;

			if (pBlockMap != nullptr)
			{
				pBlockMap->StartCameraShake(
					StageClearConstants::LogoShakePower,
					0.4f,
					1.0f,
					StageClearConstants::LogoShakeDuration
				);
			}

			// 可視先端の着地点に土煙エフェクトを発生
			float rx = (42.0f - 305.5f) * mLogoScale;
			float ry = (209.0f - 167.0f) * mLogoScale;
			float tipRelX = rx * cosA - ry * sinA;
			SpawnImpactDust(ScreenSize::CenterX + tipRelX, StageClearConstants::LogoGroundY);
		}
	}
	else
	{
		mLogoImpactTimer++;

		if (mLogoImpactTimer < 15)
		{
			float progress = static_cast<float>(mLogoImpactTimer) / 15.0f;
			mLogoReboundOffset = -8.0f * std::exp(-progress * 4.0f) * std::cos(progress * 10.0f);
		}
		else
		{
			mLogoReboundOffset = 0.0f;
	mpBlockMap = nullptr;
		}
	}
}

void StageClear::DrawBehindStage(const BlockMap* pBlockMap) const
{
	if (mState == State::Inactive)
	{
		return;
	}

	// 扉出現フェーズ以降はロゴを描画しない
	if (mState >= State::DoorAppearWait)
	{
		return;
	}

	if (mState == State::ClearCleanUp)
	{
		float t = static_cast<float>(mTimer) / static_cast<float>(StageClearConstants::ClearCleanUpFrames);
		int alpha = static_cast<int>(255.0f * (1.0f - t));
		if (alpha <= 0) return;

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawLogo(pBlockMap);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		return;
	}

	// ステージ（ブロックマップ）の背面に突き刺さるロゴ本体を描画
	DrawLogo(pBlockMap);
}

void StageClear::DrawLogo(const BlockMap* pBlockMap) const
{
	if (mnLogoHandle == -1) return;
	if (mLogoY < -550.0f) return;

	const BlockMap* blockMap = (pBlockMap != nullptr) ? pBlockMap : mpBlockMap;
	int shakeX = (blockMap != nullptr) ? blockMap->GetShakeOffsetX() : 0;
	int shakeY = (blockMap != nullptr) ? blockMap->GetShakeOffsetY() : 0;

	float currentY = mLogoY + mLogoReboundOffset;
	DrawRotaGraph(
		ScreenSize::CenterX - shakeX,
		static_cast<int>(currentY - shakeY),
		mLogoScale,
		StageClearConstants::LogoTiltAngleRad,
		mnLogoHandle,
		TRUE
	);
}

void StageClear::DrawLogoGroundImpact(const BlockMap* pBlockMap) const
{
	if (!mbLogoLanded) return;

	const BlockMap* blockMap = (pBlockMap != nullptr) ? pBlockMap : mpBlockMap;
	int shakeX = (blockMap != nullptr) ? blockMap->GetShakeOffsetX() : 0;
	int shakeY = (blockMap != nullptr) ? blockMap->GetShakeOffsetY() : 0;

	float sinA = std::sin(StageClearConstants::LogoTiltAngleRad);
	float cosA = std::cos(StageClearConstants::LogoTiltAngleRad);
	float rx = (42.0f - 305.5f) * mLogoScale;
	float ry = (209.0f - 167.0f) * mLogoScale;
	float tipRelX = rx * cosA - ry * sinA;

	int tipX = static_cast<int>(ScreenSize::CenterX + tipRelX - shakeX);
	int groundY = static_cast<int>(StageClearConstants::LogoGroundY - shakeY);

}
void StageClear::Update(Player* player, BlockMap* pBlockMap)
{
	mpBlockMap = pBlockMap;
	mbDoorDrawnThisFrame = false;
	if (mState == State::Inactive || mState == State::Finished)
	{
		return;
	}

	if (player != nullptr && player->GetIsDead())
	{
		mState = State::Inactive;
		return;
	}

	mTimer++;

	switch (mState)
	{
	case State::PreCrackerDelay:
		if (mTimer >= StageClearConstants::StartDelayFrames)
		{
			mState = State::CrackerSlideIn;
			mTimer = 0;
		}
		break;

	case State::CrackerSlideIn:
		UpdateCrackers();
		if (player != nullptr)
		{
			TrackPlayerWithCracker(mAimedCrackerIndex, player, pBlockMap);
		}
		if (mTimer >= StageClearConstants::CrackerSlideFrames)
		{
			mState = State::CrackerAimWait;
			mTimer = 0;
			mAimLocked = false;
		}
		break;

	case State::CrackerAimWait:
		UpdateCrackers();
		if (mTimer < StageClearConstants::CrackerAimWaitFrames - StageClearConstants::AimLockFramesBeforeBurst)
		{
			if (player != nullptr)
			{
				TrackPlayerWithCracker(mAimedCrackerIndex, player, pBlockMap);
			}
		}
		else
		{
			mAimLocked = true;
		}

		if (mTimer >= StageClearConstants::CrackerAimWaitFrames)
		{
			BurstCrackers();
			mState = State::CrackerBurstWait;
			mTimer = 0;
			mBurstTimer = 0;
		}
		break;

	case State::CrackerBurstWait:
		mBurstTimer++;
		UpdateCrackers();
		UpdateConfetti();
		UpdateBullets(player, pBlockMap);
		if (mState == State::Inactive) return;

		// すべての弾丸が画面外へ抜けるか被弾して消滅したかを監視
		{
			bool allBulletsGone = !mBullets.empty();
			for (const auto& b : mBullets)
			{
				if (b.isActive)
				{
					allBulletsGone = false;
					break;
				}
			}

			// 全弾消滅、または念のための上限時間経過で次のフェーズへ遷移
			if (allBulletsGone || mTimer >= StageClearConstants::CrackerBurstMaxFrames)
			{
				mState = State::PostBulletDelay;
				mTimer = 0;
			}
		}
		break;

	case State::PostBulletDelay:
		mBurstTimer++;
		UpdateCrackers();
		UpdateConfetti();

		// 弾丸がすべて消えた後、少し時間が経ってから
		if (mTimer >= StageClearConstants::PostBulletWaitFrames)
		{
			mState = State::LogoPreFallShake;
			mTimer = 0;
			if (pBlockMap != nullptr)
			{
				// 落下直前に画面横揺れ
				pBlockMap->StartCameraShake(
					StageClearConstants::LogoPreFallShakePower,
					1.0f,
					0.0f,
					StageClearConstants::LogoPreFallShakeDuration
				);
			}
		}
		break;

	case State::LogoPreFallShake:
		mBurstTimer++;
		UpdateCrackers();
		UpdateConfetti();

		// 落下直前の横揺れ演出が完了したらタイトルロゴ落下へ
		if (mTimer >= StageClearConstants::LogoPreFallShakeFrames)
		{
			mState = State::LogoFall;
			mTimer = 0;
			InitLogo();
		}
		break;

	case State::LogoFall:
		mBurstTimer++;
		UpdateCrackers();
		UpdateConfetti();
		UpdateBullets(player, pBlockMap);
		if (mState == State::Inactive) return;

		UpdateLogo(player, pBlockMap);
		if (mState == State::Inactive) return;

		UpdateImpactDust();

		if (mbLogoLanded && mLogoImpactTimer >= StageClearConstants::LogoPostImpactWaitFrames)
		{
			mState = State::Blink;
			mTimer = 0;
		}
		break;

	case State::Blink:
		mBurstTimer++;
		UpdateCrackers();
		UpdateConfetti();
		UpdateBullets(player, pBlockMap);
		if (mState == State::Inactive) return;
		UpdateLogo(player, pBlockMap);
		UpdateImpactDust();

		if (mTimer >= StageClearConstants::BlinkDurationFrames)
		{
			mState = State::Display;
			mTimer = 0;
		}
		if (InputManager::CheckDownKey(KEY_INPUT_RETURN) == 1)
		{
			mState = State::ClearCleanUp;
			mTimer = 0;
		}
		break;

	case State::Display:
		mBurstTimer++;
		UpdateCrackers();
		UpdateConfetti();
		UpdateBullets(player, pBlockMap);
		if (mState == State::Inactive) return;
		UpdateLogo(player, pBlockMap);
		UpdateImpactDust();

		if (mTimer >= StageClearConstants::DisplayDurationFrames ||
			InputManager::CheckDownKey(KEY_INPUT_RETURN) == 1)
		{
			mState = State::ClearCleanUp;
			mTimer = 0;
		}
		break;

	case State::ClearCleanUp:
	{
		mBurstTimer++;
		UpdateConfetti();
		UpdateBullets(player, pBlockMap);
		if (mState == State::Inactive) return;
		UpdateImpactDust();

		float t = static_cast<float>(mTimer) / static_cast<float>(StageClearConstants::ClearCleanUpFrames);
		if (t > 1.0f) t = 1.0f;
		float ease = t * t;

		// クラッカーが画面外へ退場（スライドアウト）
		for (auto& c : mCrackers)
		{
			c.x = c.targetX + (c.startX - c.targetX) * ease;
			c.mouthX = c.x + 125.0f * std::cos(c.shootAngleRad);
			c.mouthY = c.y + 125.0f * std::sin(c.shootAngleRad);
		}

		if (mTimer >= StageClearConstants::ClearCleanUpFrames)
		{
			mState = State::DoorAppearWait;
			mTimer = 0;
		}
	}
	break;

	case State::DoorAppearWait:
		UpdateConfetti();
		UpdateImpactDust();

		// 全てのクリア演出が終了し、少ししたら扉が出現
		if (mTimer >= StageClearConstants::DoorAppearWaitFrames)
		{
			mState = State::DoorAppear;
			mTimer = 0;

			float scrollX = (pBlockMap != nullptr) ? static_cast<float>(pBlockMap->GetScrollX()) : 0.0f;
			float scrollY = (pBlockMap != nullptr) ? static_cast<float>(pBlockMap->GetScrollY()) : 0.0f;

			// 扉の配置ワールド座標を設定
			mDoorWorldX = StageClearConstants::DoorSpawnScreenX + scrollX;

			// 地面Y座標の検出（地形コリジョンから正確な接地高さを特定して宙浮きを解消）
			float foundGroundY = -1.0f;
			if (pBlockMap != nullptr)
			{
				int checkX = static_cast<int>(mDoorWorldX);
				int startY = static_cast<int>(scrollY + 200.0f);
				int maxY = (std::min)(static_cast<int>(scrollY + 1080.0f), 2160);
				for (int y = startY; y < maxY; y++)
				{
					BlockMap::CollisionType col = pBlockMap->GetCollisionType(checkX, y);
					if (col == BlockMap::CollisionType::Block || col == BlockMap::CollisionType::BossArea)
					{
						foundGroundY = static_cast<float>(y);
						break;
					}
				}
			}

			if (foundGroundY > 0.0f)
			{
				mDoorWorldY = foundGroundY;
			}
			else if (player != nullptr && !player->GetIsDead() && std::abs(player->GetVelocityY()) < 0.01f)
			{
				mDoorWorldY = player->GetPosition().y + player->GetPlayerHeight() * 0.5f;
			}
			else
			{
				mDoorWorldY = StageClearConstants::DoorGroundFallbackY + scrollY;
			}

			mDoorAnimProgress = 0.0f;
			mDoorCurrentFrame = 0;
			mDoorPopScale = 0.0f;

			// 煙演出の発生
			SpawnDoorSmoke(mDoorWorldX, mDoorWorldY);
		}
		break;

	case State::DoorAppear:
	{
		UpdateDoorSmoke();

		float t = static_cast<float>(mTimer) / static_cast<float>(StageClearConstants::DoorAppearFrames);
		if (t > 1.0f) t = 1.0f;

		// 煙の中からポンッと飛び出すバウンススケール (0.0 -> 1.2 -> 1.0)
		mDoorPopScale = t * (1.0f + 0.20f * std::sin(t * 3.14159f));

		if (mTimer >= StageClearConstants::DoorAppearFrames)
		{
			mDoorPopScale = 1.0f;
			mState = State::DoorActive;
			mTimer = 0;
		}
	}
	break;

	case State::DoorActive:
	{
		UpdateDoorSmoke();

		bool isPlayerInRange = false;
		if (player != nullptr && !player->GetIsDead())
		{
			float playerHalfH = player->GetPlayerHeight() * 0.5f;
			float dx = std::abs(player->GetPosition().x - mDoorWorldX);
			float dy = std::abs(player->GetPosition().y - (mDoorWorldY - playerHalfH));

			if (dx <= StageClearConstants::DoorInteractRangeX && dy <= StageClearConstants::DoorInteractRangeY)
			{
				isPlayerInRange = true;
			}
		}

		if (isPlayerInRange)
		{
			// 範囲内：アニメーションを順再生（上限3.0fで停止、ループなし）
			mDoorAnimProgress += StageClearConstants::DoorAnimSpeed;
			if (mDoorAnimProgress > 3.0f)
			{
				mDoorAnimProgress = 3.0f;
			}
		}
		else
		{
			// 離れた：現在の画像の枚数のところから逆再生（下限0.0fで停止、ループなし）
			mDoorAnimProgress -= StageClearConstants::DoorAnimSpeed;
			if (mDoorAnimProgress < 0.0f)
			{
				mDoorAnimProgress = 0.0f;
			}
		}

		// 現在のコマ番号（0〜3）を決定
		int frame = static_cast<int>(mDoorAnimProgress);
		if (frame < 0) frame = 0;
		if (frame > 3) frame = 3;
		mDoorCurrentFrame = frame;

		// 扉へのインタラクト：プレイヤーが扉範囲内にいて、Enterキーが押された時のみ遷移（上キーや範囲外でのEnterは無効）
		bool enterKeyAtDoor = isPlayerInRange && (InputManager::CheckDownKey(KEY_INPUT_RETURN) == 1);

		if (enterKeyAtDoor)
		{
			mState = State::Finished;
			if (Master::mpGameManager && Master::mpGameManager->GetSceneManager())
			{
				Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_WINRESULT);
			}
		}
	}
	break;

	default:
		break;
	}
}

void StageClear::DrawCrackers() const
{
	if (mCrackerGraph == -1) return;

	for (const auto& c : mCrackers)
	{
		float drawX = c.x;
		float drawY = c.y;

		if (c.recoilOffset > 0.05f)
		{
			drawX -= c.recoilOffset * std::cos(c.shootAngleRad);
			drawY -= c.recoilOffset * std::sin(c.shootAngleRad);
		}

		// クラッカーの持ち手根元を回転ピボットに設定
		// 左側: cx=6, cy=31 
		// 右側: cx=30, cy=31
		DrawRotaGraph3(
			static_cast<int>(drawX),
			static_cast<int>(drawY),
			c.isRightSide ? 30 : 6,
			31,
			c.squishScaleX * StageClearConstants::CrackerScale,
			c.squishScaleY * StageClearConstants::CrackerScale,
			c.angle,
			mCrackerGraph,
			TRUE,
			c.isRightSide ? TRUE : FALSE
		);
	}
}

void StageClear::DrawMuzzleFlashes() const
{
	if (mBurstTimer < 0) return;

	int frameIdx = mBurstTimer / StageClearConstants::FlashAnimInterval;
	if (frameIdx >= StageClearConstants::FlashFrameCount) return;
	if (mFlashGraphs[frameIdx] == -1) return;

	// 加算ブレンドで自発光・強い閃光感を演出
	SetDrawBlendMode(DX_BLENDMODE_ADD, 255);

	for (const auto& c : mCrackers)
	{
		float drawMouthX = c.mouthX;
		float drawMouthY = c.mouthY;
		if (c.recoilOffset > 0.05f)
		{
			drawMouthX -= c.recoilOffset * std::cos(c.shootAngleRad);
			drawMouthY -= c.recoilOffset * std::sin(c.shootAngleRad);
		}

		DrawRotaGraph(
			static_cast<int>(drawMouthX),
			static_cast<int>(drawMouthY),
			StageClearConstants::FlashScale,
			0.0,
			mFlashGraphs[frameIdx],
			TRUE
		);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void StageClear::DrawBullets() const
{
	for (const auto& b : mBullets)
	{
		if (!b.isActive) continue;

		int ix = static_cast<int>(b.x);
		int iy = static_cast<int>(b.y);
		int r = static_cast<int>(b.radius);

		for (int k = 2; k >= 0; k--)
		{
			int trAlpha = 35 + (2 - k) * 35;
			int trRadius = static_cast<int>(b.radius * (0.45f + (2 - k) * 0.18f));
			int px = static_cast<int>(b.prevX[k]);
			int py = static_cast<int>(b.prevY[k]);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, trAlpha);
			DrawCircle(px, py, trRadius, GetColor(190, 205, 225), TRUE);
		}

		SetDrawBlendMode(DX_BLENDMODE_ADD, 90);
		DrawCircle(ix, iy, r + 4, GetColor(190, 215, 245), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		DrawCircle(ix, iy, r + 2, GetColor(15, 18, 24), TRUE);
		DrawCircle(ix, iy, r, GetColor(230, 235, 245), TRUE);
		DrawCircle(ix + static_cast<int>(r * 0.22f), iy + static_cast<int>(r * 0.22f), static_cast<int>(r * 0.65f), GetColor(165, 172, 185), TRUE);
		DrawCircle(ix - static_cast<int>(r * 0.35f), iy - static_cast<int>(r * 0.35f), static_cast<int>(r * 0.38f), GetColor(255, 255, 255), TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void StageClear::DrawConfetti() const
{
	for (const auto& p : mConfetti)
	{
		if (!p.isActive) continue;

		int alpha = 255;
		if (p.timer > p.life - 30)
		{
			alpha = static_cast<int>(255.0f * (static_cast<float>(p.life - p.timer) / 30.0f));
		}
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

		if (p.shapeType == 0)
		{
			float cosSpin = std::cos(p.spinAngle);
			bool isBack = (cosSpin < 0.0f);
			float halfW = (p.sizeW * 0.5f) * std::abs(cosSpin);
			float halfH = p.sizeH * 0.5f;
			if (halfW < 1.0f) halfW = 1.0f;

			unsigned int color = isBack
				? GetColor(static_cast<int>(p.r * 0.7f), static_cast<int>(p.g * 0.7f), static_cast<int>(p.b * 0.7f))
				: GetColor(p.r, p.g, p.b);

			float cosA = std::cos(p.angle);
			float sinA = std::sin(p.angle);

			float x1 = p.x - halfW * cosA + halfH * sinA;
			float y1 = p.y - halfW * sinA - halfH * cosA;
			float x2 = p.x + halfW * cosA + halfH * sinA;
			float y2 = p.y + halfW * sinA - halfH * cosA;
			float x3 = p.x + halfW * cosA - halfH * sinA;
			float y3 = p.y + halfW * sinA + halfH * cosA;
			float x4 = p.x - halfW * cosA - halfH * sinA;
			float y4 = p.y - halfW * sinA + halfH * cosA;

			DrawQuadrangleAA(x1, y1, x2, y2, x3, y3, x4, y4, color, TRUE);
		}
		else
		{
			float s = p.sizeW * 0.6f;
			unsigned int color = GetColor(p.r, p.g, p.b);
			DrawQuadrangleAA(p.x, p.y - s, p.x + s, p.y, p.x, p.y + s, p.x - s, p.y, color, TRUE);
		}

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

void StageClear::SpawnDoorSmoke(float x, float y)
{
	mDoorSmoke.clear();
	mDoorSmoke.reserve(32);

	const float kPi = 3.14159265f;
	// 扉の足元と中央付近から四方八方に広がるモクモク煙
	for (int i = 0; i < 30; i++)
	{
		DoorSmokeParticle s;
		float spawnOffsetX = static_cast<float>(GetRand(60)) - 30.0f; // -30〜+30
		float spawnOffsetY = static_cast<float>(GetRand(90)) - 80.0f; // -80〜+10
		s.x = x + spawnOffsetX;
		s.y = y + spawnOffsetY;

		float angle = static_cast<float>(GetRand(360)) * kPi / 180.0f;
		float speed = 2.0f + static_cast<float>(GetRand(45)) / 10.0f; // 2.0〜6.5
		s.vx = speed * std::cos(angle);
		s.vy = speed * std::sin(angle) - 1.2f;

		s.size = 14.0f + static_cast<float>(GetRand(14)); // 14〜28px
		s.alpha = 230 + GetRand(25);

		int shade = GetRand(25);
		s.r = 235 + shade;
		s.g = 230 + shade;
		s.b = 220 + shade;
		if (s.r > 255) s.r = 255;
		if (s.g > 255) s.g = 255;
		if (s.b > 255) s.b = 255;

		s.life = 35 + GetRand(20); // 35〜55フレーム
		s.timer = 0;
		s.isActive = true;

		mDoorSmoke.push_back(s);
	}
}

void StageClear::UpdateDoorSmoke()
{
	for (auto& s : mDoorSmoke)
	{
		if (!s.isActive) continue;

		s.x += s.vx;
		s.y += s.vy;
		s.vx *= 0.90f; // 空気抵抗で徐々に減速
		s.vy *= 0.90f;
		s.vy -= 0.08f; // ふわふわと上昇
		s.size += 0.55f; // モクモクと膨張
		s.timer++;

		if (s.timer >= s.life)
		{
			s.isActive = false;
		}
	}
}

void StageClear::DrawDoorSmoke() const
{
	const BlockMap* blockMap = mpBlockMap;
	int shakeX = (blockMap != nullptr) ? blockMap->GetShakeOffsetX() : 0;
	int shakeY = (blockMap != nullptr) ? blockMap->GetShakeOffsetY() : 0;
	float scrollX = (blockMap != nullptr) ? static_cast<float>(blockMap->GetScrollX()) : 0.0f;
	float scrollY = (blockMap != nullptr) ? static_cast<float>(blockMap->GetScrollY()) : 0.0f;

	for (const auto& s : mDoorSmoke)
	{
		if (!s.isActive) continue;

		float progress = static_cast<float>(s.timer) / static_cast<float>(s.life);
		int currentAlpha = static_cast<int>(static_cast<float>(s.alpha) * (1.0f - progress));
		if (currentAlpha <= 0) continue;

		float screenX = s.x - scrollX - shakeX;
		float screenY = s.y - scrollY - shakeY;

		// 煙の外周（立体感のある薄い影）
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, currentAlpha / 2);
		DrawCircleAA(screenX, screenY, s.size + 2.0f, 16, GetColor(180, 175, 165), TRUE);

		// 煙本体（やわらかな白・クリーム色）
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, currentAlpha);
		DrawCircleAA(screenX, screenY, s.size, 16, GetColor(s.r, s.g, s.b), TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void StageClear::DrawDoor(const BlockMap* pBlockMap) const
{
	if (mState != State::DoorAppear && mState != State::DoorActive)
	{
		return;
	}

	mbDoorDrawnThisFrame = true;

	if (mDoorCurrentFrame < 0 || mDoorCurrentFrame >= StageClearConstants::DoorFrameCount)
	{
		return;
	}

	// 画像ハンドルのフェイルセーフ再読み込み
	if (mDoorGraphs[mDoorCurrentFrame] == -1)
	{
		LoadDivGraph(
			"Resource/Image/OpenDoor.png",
			StageClearConstants::DoorFrameCount,
			StageClearConstants::DoorFrameCount,
			1,
			StageClearConstants::DoorFrameWidth,
			StageClearConstants::DoorFrameHeight,
			const_cast<int*>(mDoorGraphs)
		);
	}

	int graphHandle = mDoorGraphs[mDoorCurrentFrame];
	if (graphHandle == -1) return;

	const BlockMap* blockMap = (pBlockMap != nullptr) ? pBlockMap : mpBlockMap;
	int shakeX = (blockMap != nullptr) ? blockMap->GetShakeOffsetX() : 0;
	int shakeY = (blockMap != nullptr) ? blockMap->GetShakeOffsetY() : 0;
	float scrollX = (blockMap != nullptr) ? static_cast<float>(blockMap->GetScrollX()) : 0.0f;
	float scrollY = (blockMap != nullptr) ? static_cast<float>(blockMap->GetScrollY()) : 0.0f;

	float drawScreenX = mDoorWorldX - scrollX - shakeX;
	float drawScreenY = mDoorWorldY - scrollY - shakeY;

	// 画像下端が地面(mDoorWorldY)に接するように、中心Yを上にずらす
	float centerY = drawScreenY - (StageClearConstants::DoorFrameHeight * StageClearConstants::DoorScale * 0.5f);
	float scale = StageClearConstants::DoorScale * mDoorPopScale;

	DrawRotaGraph(
		static_cast<int>(drawScreenX),
		static_cast<int>(centerY),
		scale,
		0.0,
		graphHandle,
		TRUE
	);
}

void StageClear::Draw() const
{
	if (mState == State::Inactive)
	{
		return;
	}

	// 1. 紙吹雪パーティクル描画
	DrawConfetti();

	// 2. クラッカー本体描画（DoorAppearWait以降は非表示）
	if (mState != State::PreCrackerDelay && mState < State::DoorAppearWait)
	{
		DrawCrackers();
		DrawMuzzleFlashes();
	}

	// 3. 銀色の円形弾描画
	DrawBullets();

	// 4. タイトルロゴ落下に伴う地面衝撃エフェクト（DoorAppearWait以降は非表示）
	if (mState < State::DoorAppearWait)
	{
		DrawLogoGroundImpact();
		DrawImpactDust();
	}

	// 5. 扉描画（GameScene::Drawで呼ばれなかった場合のフェイルセーフ）
	if (!mbDoorDrawnThisFrame && (mState == State::DoorAppear || mState == State::DoorActive))
	{
		DrawDoor(mpBlockMap);
	}

	// 6. 扉出現の煙エフェクト描画
	if (mState == State::DoorAppear || mState == State::DoorActive)
	{
		DrawDoorSmoke();
	}

	// 6. 「ステージクリア」特大文字描画（点滅・常時表示・フェードアウト）
	if (mState != State::Blink && mState != State::Display && mState != State::ClearCleanUp)
	{
		return;
	}

	if (!Master::mpGameManager) return;
	FontManager* fontMgr = Master::mpGameManager->GetFontManager();
	if (!fontMgr) return;

	if (mState == State::Blink)
	{
		bool isVisible = ((mTimer / StageClearConstants::BlinkIntervalFrames) % 2 == 0);
		if (!isVisible)
		{
			return;
		}
	}

	int textAlpha = 255;
	if (mState == State::ClearCleanUp)
	{
		float t = static_cast<float>(mTimer) / static_cast<float>(StageClearConstants::ClearCleanUpFrames);
		textAlpha = static_cast<int>(255.0f * (1.0f - t));
		if (textAlpha <= 0) return;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, textAlpha);
	}

	const char* clearText = "ステージクリア";
	const int fontSize = StageClearConstants::FontSize;

	int textWidth = fontMgr->GetDotStringWidth(fontSize, clearText);
	int textX = (ScreenSize::ScrrenWidth - textWidth) / 2;
	int textY = StageClearConstants::FixedPosY;

	const int outline = StageClearConstants::OutlineWidth;
	const unsigned int blackColor = GetColor(0, 0, 0);
	for (int ox = -outline; ox <= outline; ox += outline)
	{
		for (int oy = -outline; oy <= outline; oy += outline)
		{
			if (ox == 0 && oy == 0) continue;
			fontMgr->DrawDotString(textX + ox, textY + oy, fontSize, blackColor, clearText);
		}
	}

	const unsigned int goldColor = GetColor(255, 230, 70);
	fontMgr->DrawDotString(textX, textY, fontSize, goldColor, clearText);

	if (mState == State::ClearCleanUp)
	{
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}
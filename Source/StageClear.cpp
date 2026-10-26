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
	, mBurstTimer(0)
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
{
	sInstance = this;
	mCrackerGraph = LoadGraph("Resource/Image/Cracker.png");
	mnLogoHandle = LoadGraph(BackGroundGraphPath::TitleLogoGraphs.c_str());
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
		mBurstTimer = 0;
		mAimedCrackerIndex = GetRand(5); // 0〜5からランダムで1基を狙撃手に選出
		mAimLocked = false;
		InitCrackers();
		mConfetti.clear();
		mBullets.clear();
		mImpactDusts.clear();
		InitLogo();
	}
}

void StageClear::Reset()
{
	mState = State::Inactive;
	mTimer = 0;
	mBurstTimer = 0;
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
}

void StageClear::InitCrackers()
{
	mCrackers.clear();
	mCrackers.resize(6);

	const float kPi = 3.14159265f;

	// 左側 3基
	// 上段: やや浅い角度
	mCrackers[0] = { StageClearConstants::CrackerLeftStartX, StageClearConstants::CrackerPosYTop,
		StageClearConstants::CrackerLeftStartX, StageClearConstants::CrackerLeftTargetX, StageClearConstants::CrackerPosYTop,
		33.0f * kPi / 180.0f, -33.0f * kPi / 180.0f, false, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f };

	// 中段: 基準 45度
	mCrackers[1] = { StageClearConstants::CrackerLeftStartX, StageClearConstants::CrackerPosYMid,
		StageClearConstants::CrackerLeftStartX, StageClearConstants::CrackerLeftTargetX, StageClearConstants::CrackerPosYMid,
		45.0f * kPi / 180.0f, -45.0f * kPi / 180.0f, false, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f };

	// 下段: やや急な角度（空高く打ち上げる）
	mCrackers[2] = { StageClearConstants::CrackerLeftStartX, StageClearConstants::CrackerPosYBottom,
		StageClearConstants::CrackerLeftStartX, StageClearConstants::CrackerLeftTargetX, StageClearConstants::CrackerPosYBottom,
		58.0f * kPi / 180.0f, -58.0f * kPi / 180.0f, false, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f };

	// 右側 3基
	// 上段
	mCrackers[3] = { StageClearConstants::CrackerRightStartX, StageClearConstants::CrackerPosYTop,
		StageClearConstants::CrackerRightStartX, StageClearConstants::CrackerRightTargetX, StageClearConstants::CrackerPosYTop,
		-33.0f * kPi / 180.0f, -147.0f * kPi / 180.0f, true, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f };

	// 中段
	mCrackers[4] = { StageClearConstants::CrackerRightStartX, StageClearConstants::CrackerPosYMid,
		StageClearConstants::CrackerRightStartX, StageClearConstants::CrackerRightTargetX, StageClearConstants::CrackerPosYMid,
		-45.0f * kPi / 180.0f, -135.0f * kPi / 180.0f, true, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f };

	// 下段
	mCrackers[5] = { StageClearConstants::CrackerRightStartX, StageClearConstants::CrackerPosYBottom,
		StageClearConstants::CrackerRightStartX, StageClearConstants::CrackerRightTargetX, StageClearConstants::CrackerPosYBottom,
		-58.0f * kPi / 180.0f, -122.0f * kPi / 180.0f, true, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f };

	for (auto& c : mCrackers)
	{
		c.mouthX = c.x + 68.0f * std::cos(c.shootAngleRad);
		c.mouthY = c.y + 68.0f * std::sin(c.shootAngleRad);
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
		c.angle = targetShootAngle + (kPi * 0.25f);
	}
	else
	{
		c.angle = targetShootAngle + (kPi * 0.75f);
	}

	c.mouthX = c.x + 68.0f * std::cos(c.shootAngleRad);
	c.mouthY = c.y + 68.0f * std::sin(c.shootAngleRad);
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
			c.mouthX = c.x + 68.0f * std::cos(c.shootAngleRad);
			c.mouthY = c.y + 68.0f * std::sin(c.shootAngleRad);
		}
	}
	else
	{
		for (auto& c : mCrackers)
		{
			c.recoilOffset *= 0.80f;
			c.squishScaleX += (1.0f - c.squishScaleX) * 0.20f;
			c.squishScaleY += (1.0f - c.squishScaleY) * 0.20f;

			c.mouthX = c.x + 68.0f * std::cos(c.shootAngleRad);
			c.mouthY = c.y + 68.0f * std::sin(c.shootAngleRad);
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
				player->SetDeathReason(DeathReason::DeathType::EnemyHit);
				player->TakeDamage(player->GetHp());
				b.isActive = false;
				mState = State::Inactive;
				return;
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

	float hw = (611.0f * mLogoScale) * 0.5f;
	float hh = (334.0f * mLogoScale) * 0.5f;
	float sinA = std::sin(StageClearConstants::LogoTiltAngleRad);
	float cosA = std::cos(StageClearConstants::LogoTiltAngleRad);
	float tipRelY = (-hw) * sinA + (hh) * cosA;

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
	for (const auto& d : mImpactDusts)
	{
		if (!d.isActive) continue;

		float progress = static_cast<float>(d.timer) / static_cast<float>(d.life);
		int alpha = static_cast<int>(static_cast<float>(d.alpha) * (1.0f - progress));

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawCircleAA(d.x, d.y, d.size, 16, GetColor(200, 185, 165), TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void StageClear::UpdateLogo(Player* player, BlockMap* pBlockMap)
{
	float hw = (611.0f * mLogoScale) * 0.5f;
	float hh = (334.0f * mLogoScale) * 0.5f;
	float sinA = std::sin(StageClearConstants::LogoTiltAngleRad);
	float cosA = std::cos(StageClearConstants::LogoTiltAngleRad);
	float tipRelY = (-hw) * sinA + (hh) * cosA;

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
		if (player != nullptr && !player->GetIsDead())
		{
			float pScreenX = player->GetPosition().x - scrollX;
			float pScreenY = player->GetPosition().y - scrollY;
			float halfW = player->GetPlayerWidth() * 0.40f;
			float halfH = player->GetPlayerHeight() * 0.45f;

			float logoLeft = ScreenSize::CenterX - 380.0f;
			float logoRight = ScreenSize::CenterX + 380.0f;
			float logoBottom = mLogoY + tipRelY;
			float logoTop = mLogoY - tipRelY;

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

			float tipRelX = (-hw) * cosA - (hh) * sinA;
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
		}
	}
}

void StageClear::DrawLogo() const
{
	if (mnLogoHandle == -1) return;
	if (mLogoY < -550.0f) return;

	float currentY = mLogoY + mLogoReboundOffset;
	DrawRotaGraph(
		ScreenSize::CenterX,
		static_cast<int>(currentY),
		mLogoScale,
		StageClearConstants::LogoTiltAngleRad,
		mnLogoHandle,
		TRUE
	);

	if (mbLogoLanded)
	{
		float hw = (611.0f * mLogoScale) * 0.5f;
		float hh = (334.0f * mLogoScale) * 0.5f;
		float sinA = std::sin(StageClearConstants::LogoTiltAngleRad);
		float cosA = std::cos(StageClearConstants::LogoTiltAngleRad);
		float tipRelX = (-hw) * cosA - (hh) * sinA;

		int tipX = static_cast<int>(ScreenSize::CenterX + tipRelX);
		int groundY = static_cast<int>(StageClearConstants::LogoGroundY);

		DrawLine(tipX - 35, groundY + 4, tipX - 5, groundY + 1, GetColor(35, 30, 25), 3);
		DrawLine(tipX - 5, groundY + 1, tipX + 25, groundY + 5, GetColor(35, 30, 25), 3);
		DrawLine(tipX - 12, groundY + 2, tipX - 20, groundY + 12, GetColor(45, 38, 30), 2);
		DrawLine(tipX + 10, groundY + 3, tipX + 18, groundY + 10, GetColor(45, 38, 30), 2);

		DrawCircle(tipX - 8, groundY + 2, 7, GetColor(80, 70, 55), TRUE);
		DrawCircle(tipX + 6, groundY + 3, 8, GetColor(70, 60, 48), TRUE);
		DrawCircle(tipX - 2, groundY + 1, 6, GetColor(95, 82, 65), TRUE);
		DrawCircle(tipX + 14, groundY + 4, 5, GetColor(60, 50, 40), TRUE);
	}
}

void StageClear::Update(Player* player, BlockMap* pBlockMap)
{
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

		if (mTimer >= StageClearConstants::CrackerBurstWaitFrames)
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
		}
		if (InputManager::CheckDownKey(KEY_INPUT_RETURN) == 1)
		{
			mState = State::Finished;
			if (Master::mpGameManager && Master::mpGameManager->GetSceneManager())
			{
				Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_WINRESULT);
			}
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
			mState = State::Finished;
			if (Master::mpGameManager && Master::mpGameManager->GetSceneManager())
			{
				Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_WINRESULT);
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

		DrawRotaGraph3(
			static_cast<int>(drawX),
			static_cast<int>(drawY),
			c.isRightSide ? 37 : 0,
			17,
			c.squishScaleX * StageClearConstants::CrackerScale,
			c.squishScaleY * StageClearConstants::CrackerScale,
			c.angle,
			mCrackerGraph,
			TRUE
		);
	}
}

void StageClear::DrawMuzzleFlashes() const
{
	if (mBurstTimer >= 6) return;

	float progress = static_cast<float>(mBurstTimer) / 6.0f;
	int alpha = static_cast<int>(255.0f * (1.0f - progress));
	float r = 16.0f + progress * 32.0f;

	SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);

	for (const auto& c : mCrackers)
	{
		float mx = c.mouthX;
		float my = c.mouthY;

		DrawQuadrangleAA(mx, my - r, mx + r * 0.35f, my, mx, my + r, mx - r * 0.35f, my, GetColor(255, 240, 150), TRUE);
		DrawQuadrangleAA(mx - r, my, mx, my - r * 0.35f, mx + r, my, mx, my + r * 0.35f, GetColor(255, 240, 150), TRUE);
		DrawCircleAA(mx, my, r * 0.45f, 16, GetColor(255, 255, 255), TRUE);
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

void StageClear::Draw() const
{
	if (mState == State::Inactive)
	{
		return;
	}

	// 1. 紙吹雪パーティクル描画
	DrawConfetti();

	// 2. クラッカー本体描画（スライドイン中・待機中・破裂後）
	if (mState != State::PreCrackerDelay)
	{
		DrawCrackers();
		DrawMuzzleFlashes();
	}

	// 3. 銀色の円形弾描画
	DrawBullets();

	// 4. タイトルロゴ落下＆地面突き刺さり描画
	DrawLogo();
	DrawImpactDust();

	// 5. 「ステージクリア」特大文字描画（点滅・常時表示）
	if (mState != State::Blink && mState != State::Display)
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
}
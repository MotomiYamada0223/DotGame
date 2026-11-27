#include "Stage3BossMovie.h"
#include "Effekseer2DManager.h"
#include "BlockMap.h"
#include "Player.h"
#include "Stage3BossDragon.h"
#include "GameConstants.h"
#include "Master.h"
#include "GameManager.h"
#include "FontManager.h"
#include <cmath>
#include <string>

Stage3BossMovie::Stage3BossMovie()
	: mPhase(Phase::None)
	, mTimer(0)
	, mbIsAwakeningMovie(false)
	, mSavedPlayerScrollX(0.0f)
	, mSavedPlayerScrollY(0.0f)
	, mTargetBossScrollX(0.0f)
	, mTargetBossScrollY(0.0f)
	, mCurrentScrollX(0.0f)
	, mCurrentScrollY(0.0f)
	, mLetterboxHeight(0.0f)
	, mBossStartX(Stage3BossDragon::SPAWN_X)
	, mBossStartY(Stage3BossDragon::SPAWN_Y)
{
}

Stage3BossMovie::~Stage3BossMovie()
{
}

void Stage3BossMovie::Initialize()
{
	Reset();
}

void Stage3BossMovie::Reset()
{
	mPhase              = Phase::None;
	mTimer              = 0;
	mbIsAwakeningMovie  = false;
	mSavedPlayerScrollX = 0.0f;
	mSavedPlayerScrollY = 0.0f;
	mTargetBossScrollX  = 0.0f;
	mTargetBossScrollY  = 0.0f;
	mCurrentScrollX     = 0.0f;
	mCurrentScrollY     = 0.0f;
	mLetterboxHeight    = 0.0f;
	mBossStartX         = Stage3BossDragon::SPAWN_X;
	mBossStartY         = Stage3BossDragon::SPAWN_Y;
}

float Stage3BossMovie::SmoothStep(float t)
{
	if (t < 0.0f) t = 0.0f;
	if (t > 1.0f) t = 1.0f;
	return t * t * (3.0f - 2.0f * t);
}

void Stage3BossMovie::StartMovie(BlockMap& blockMap, Player* player, Stage3BossDragon* boss)
{
	if (mPhase != Phase::None) return;

	mPhase             = Phase::WaitAfterGate;
	mTimer             = 0;
	mbIsAwakeningMovie = false;

	if (player != nullptr)
	{
		player->SetInputDisabled(true);
	}
	if (boss != nullptr)
	{
		boss->SetCutsceneMode(true);
	}

	blockMap.SetVerticalScrollLocked(true);

	mSavedPlayerScrollX = static_cast<float>(blockMap.GetScrollX());
	mSavedPlayerScrollY = static_cast<float>(blockMap.GetScrollY());
	mCurrentScrollX     = mSavedPlayerScrollX;
	mCurrentScrollY     = mSavedPlayerScrollY;

	float bossX = (boss != nullptr) ? boss->GetPosition().x : Stage3BossDragon::SPAWN_X;
	float bossY = (boss != nullptr) ? boss->GetPosition().y : Stage3BossDragon::SPAWN_Y;

	mTargetBossScrollX = bossX - (ScreenSize::ScrrenWidth * 0.65f);
	mTargetBossScrollY = bossY - (ScreenSize::ScrrenHeight * 0.58f);

	float maxScrollX = static_cast<float>(blockMap.GetCurrentWidth() - ScreenSize::ScrrenWidth);
	float maxScrollY = static_cast<float>(blockMap.GetCurrentHeight() - ScreenSize::ScrrenHeight);
	if (maxScrollX < 0.0f) maxScrollX = 0.0f;
	if (maxScrollY < 0.0f) maxScrollY = 0.0f;

	if (mTargetBossScrollX < 0.0f)       mTargetBossScrollX = 0.0f;
	if (mTargetBossScrollX > maxScrollX) mTargetBossScrollX = maxScrollX;
	if (mTargetBossScrollY < 0.0f)       mTargetBossScrollY = 0.0f;
	if (mTargetBossScrollY > maxScrollY) mTargetBossScrollY = maxScrollY;
}

void Stage3BossMovie::StartAwakeningMovie(BlockMap& blockMap, Player* player, Stage3BossDragon* boss)
{
	mPhase             = Phase::AwakenMoveToCenter;
	mTimer             = 0;
	mbIsAwakeningMovie = true;
	mLetterboxHeight   = 0.0f;

	if (player != nullptr)
	{
		player->SetWindFloatMode(false);
		player->SetInputDisabled(true);
	}

	blockMap.SetVerticalScrollLocked(true);

	float maxScrollX = static_cast<float>(blockMap.GetCurrentWidth() - ScreenSize::ScrrenWidth);
	float maxScrollY = static_cast<float>(blockMap.GetCurrentHeight() - ScreenSize::ScrrenHeight);
	if (maxScrollX < 0.0f) maxScrollX = 0.0f;
	if (maxScrollY < 0.0f) maxScrollY = 0.0f;

	float px = (player != nullptr) ? player->GetPosition().x : 3600.0f;
	mSavedPlayerScrollX = px - (ScreenSize::ScrrenWidth * 0.5f);
	if (mSavedPlayerScrollX < 3324.0f)    mSavedPlayerScrollX = 3324.0f;
	if (mSavedPlayerScrollX > maxScrollX) mSavedPlayerScrollX = maxScrollX;
	// 【Stage3.png 5724x1692拡大版 対応】初期スクロールYを5724x1692マップに合わせて設定
	mSavedPlayerScrollY = 300.0f;
	if (mSavedPlayerScrollY > maxScrollY) mSavedPlayerScrollY = maxScrollY;

	mCurrentScrollX = mSavedPlayerScrollX;
	mCurrentScrollY = mSavedPlayerScrollY;

	mBossStartX = (boss != nullptr) ? boss->GetPosition().x : Stage3BossDragon::SPAWN_X;
	mBossStartY = (boss != nullptr) ? boss->GetPosition().y : Stage3BossDragon::SPAWN_Y;

	// 【Stage3.png 5724x1692拡大版 対応】ボス部屋中央（X=4353, Y=860）が画面ど真ん中に映るスクロール座標
	mTargetBossScrollX = AWAKEN_CENTER_X - (ScreenSize::ScrrenWidth * 0.5f);
	mTargetBossScrollY = AWAKEN_CENTER_Y - (ScreenSize::ScrrenHeight * 0.52f);

	if (mTargetBossScrollX < 0.0f)       mTargetBossScrollX = 0.0f;
	if (mTargetBossScrollX > maxScrollX) mTargetBossScrollX = maxScrollX;
	if (mTargetBossScrollY < 0.0f)       mTargetBossScrollY = 0.0f;
	if (mTargetBossScrollY > maxScrollY) mTargetBossScrollY = maxScrollY;
}

void Stage3BossMovie::Update(BlockMap& blockMap, Player* player, Stage3BossDragon* boss)
{
	if (!IsPlaying()) return;

	if (player != nullptr && player->GetIsDead())
	{
		return;
	}

	if (player != nullptr)
	{
		player->SetInputDisabled(true);
	}
	blockMap.SetVerticalScrollLocked(true);

	mTimer++;

	switch (mPhase)
	{
	case Phase::WaitAfterGate:
		mLetterboxHeight = SmoothStep(static_cast<float>(mTimer) / 25.0f) * 84.0f;
		mCurrentScrollX  = mSavedPlayerScrollX;
		mCurrentScrollY  = mSavedPlayerScrollY;

		if (mTimer >= 25)
		{
			mPhase = Phase::PanToBoss;
			mTimer = 0;
		}
		break;

	case Phase::PanToBoss:
	{
		mLetterboxHeight += (84.0f - mLetterboxHeight) * 0.2f;
		float t    = static_cast<float>(mTimer) / 85.0f;
		float ease = SmoothStep(t);

		mCurrentScrollX = mSavedPlayerScrollX + (mTargetBossScrollX - mSavedPlayerScrollX) * ease;
		mCurrentScrollY = mSavedPlayerScrollY + (mTargetBossScrollY - mSavedPlayerScrollY) * ease;

		if (mTimer >= 85)
		{
			mPhase = Phase::FocusOnBoss;
			mTimer = 0;
			if (boss != nullptr)
			{
				boss->TriggerCutsceneRoar();
			}
		}
		break;
	}

	case Phase::FocusOnBoss:
		mLetterboxHeight = 84.0f;
		mCurrentScrollX  = mTargetBossScrollX;
		mCurrentScrollY  = mTargetBossScrollY;

		if (mTimer >= 110)
		{
			mPhase = Phase::PanBackToPlayer;
			mTimer = 0;
			blockMap.mShakeX = 0;
			blockMap.mShakeY = 0;
		}
		break;

	case Phase::PanBackToPlayer:
	{
		float t    = static_cast<float>(mTimer) / 75.0f;
		float ease = SmoothStep(t);

		mCurrentScrollX = mTargetBossScrollX + (mSavedPlayerScrollX - mTargetBossScrollX) * ease;
		mCurrentScrollY = mTargetBossScrollY + (mSavedPlayerScrollY - mTargetBossScrollY) * ease;

		if (mTimer > 45)
		{
			float closeT = static_cast<float>(mTimer - 45) / 30.0f;
			mLetterboxHeight = 84.0f * (1.0f - SmoothStep(closeT));
		}

		if (mTimer >= 75)
		{
			mCurrentScrollX  = mSavedPlayerScrollX;
			mCurrentScrollY  = mSavedPlayerScrollY;
			mLetterboxHeight = 0.0f;
			mPhase           = Phase::Finished;
			mTimer           = 0;

			blockMap.SetScrollX(static_cast<int>(mSavedPlayerScrollX));
			blockMap.SetScrollY(static_cast<int>(mSavedPlayerScrollY));
			blockMap.SetVerticalScrollLocked(false);

			if (player != nullptr)
			{
				player->SetInputDisabled(false);
			}
			if (boss != nullptr)
			{
				boss->SetCutsceneMode(false);
			}
			return;
		}
		break;
	}

	case Phase::AwakenMoveToCenter:
	{
		const int DURATION = 70;
		float t    = static_cast<float>(mTimer) / static_cast<float>(DURATION);
		float ease = SmoothStep(t);

		mLetterboxHeight = ease * 88.0f;
		mCurrentScrollX  = mSavedPlayerScrollX + (mTargetBossScrollX - mSavedPlayerScrollX) * ease;
		mCurrentScrollY  = mSavedPlayerScrollY + (mTargetBossScrollY - mSavedPlayerScrollY) * ease;

		if (boss != nullptr)
		{
			float bx = mBossStartX + (AWAKEN_CENTER_X - mBossStartX) * ease;
			float by = mBossStartY + (AWAKEN_CENTER_Y - mBossStartY) * ease - std::sin(t * 3.14159265f) * 90.0f;
			boss->UpdateAwakeningCutscenePose(bx, by, -1.0f, false, 0.0f);
		}

		if (mTimer >= DURATION)
		{
			mPhase = Phase::AwakenFocusAndTransform;
			mTimer = 0;
		}
		break;
	}

	case Phase::AwakenFocusAndTransform:
	{
		const int DURATION = 155;
		mLetterboxHeight = 88.0f;
		mCurrentScrollX  = mTargetBossScrollX;
		mCurrentScrollY  = mTargetBossScrollY;

		if (boss != nullptr)
		{
			float bobY = std::sin(mTimer * 0.12f) * 10.0f;
			if (mTimer < 45)
			{
				float chargeRatio = static_cast<float>(mTimer) / 45.0f;
				boss->UpdateAwakeningCutscenePose(AWAKEN_CENTER_X, AWAKEN_CENTER_Y + bobY, -1.0f, false, chargeRatio);
				// 黄金放電スパーク
				if (mTimer == 15 || mTimer == 30)
				{
					float dirRot = (mTimer == 15) ? 0.4f : -2.7f;
					Effekseer2DManager::Play(
						Effect2DId::Stage3_Thunder, AWAKEN_CENTER_X, AWAKEN_CENTER_Y + bobY, 0.90f, 1.5f, dirRot, 255, 230, 80, 235
					);
				}
				blockMap.mShakeX = GetRand(8) - 4;
				blockMap.mShakeY = GetRand(8) - 4;
			}
			else
			{
				if (mTimer == 45)
				{
					boss->TriggerGoldenAwakening();
					// 黄金覚醒の瞬間：2D Effekseer 黄金大バースト＋左右への黄金稲妻放電
					Effekseer2DManager::Play(
						Effect2DId::Stage3_HolyStorm, AWAKEN_CENTER_X, AWAKEN_CENTER_Y + bobY, 2.6f, 1.15f, 0.0f, 255, 215, 55, 255
					);
					Effekseer2DManager::Play(
						Effect2DId::Stage3_Thunder, AWAKEN_CENTER_X - 80.0f, AWAKEN_CENTER_Y + bobY, 1.35f, 1.3f, 3.14159265f, 255, 235, 110, 255
					);
					Effekseer2DManager::Play(
						Effect2DId::Stage3_Thunder, AWAKEN_CENTER_X + 80.0f, AWAKEN_CENTER_Y + bobY, 1.35f, 1.3f, 0.0f, 255, 235, 110, 255
					);
				}
				else if (mTimer == 62 || mTimer == 80)
				{
					// 咆哮に合わせた黄金衝撃バースト
					Effekseer2DManager::Play(
						Effect2DId::Stage3_HolyStorm, AWAKEN_CENTER_X, AWAKEN_CENTER_Y + bobY, 1.85f, 1.35f, 0.0f, 255, 200, 45, 225
					);
				}
				boss->UpdateAwakeningCutscenePose(AWAKEN_CENTER_X, AWAKEN_CENTER_Y + bobY, -1.0f, true, 1.0f);
				if (mTimer < 115)
				{
					blockMap.mShakeX = GetRand(16) - 8;
					blockMap.mShakeY = GetRand(16) - 8;
				}
				else
				{
					blockMap.mShakeX = 0;
					blockMap.mShakeY = 0;
				}
			}
		}

		if (mTimer >= DURATION)
		{
			mPhase           = Phase::AwakenReturnToBattle;
			mTimer           = 0;
			blockMap.mShakeX = 0;
			blockMap.mShakeY = 0;
		}
		break;
	}

	case Phase::AwakenReturnToBattle:
	{
		const int DURATION = 65;
		float t    = static_cast<float>(mTimer) / static_cast<float>(DURATION);
		float ease = SmoothStep(t);

		mCurrentScrollX = mTargetBossScrollX + (mSavedPlayerScrollX - mTargetBossScrollX) * ease;
		mCurrentScrollY = mTargetBossScrollY + (mSavedPlayerScrollY - mTargetBossScrollY) * ease;

		if (mTimer > 35)
		{
			float closeT = static_cast<float>(mTimer - 35) / 30.0f;
			mLetterboxHeight = 88.0f * (1.0f - SmoothStep(closeT));
		}

		if (boss != nullptr)
		{
			float bx = AWAKEN_CENTER_X + (Stage3BossDragon::SPAWN_X - AWAKEN_CENTER_X) * ease;
			float by = AWAKEN_CENTER_Y + (Stage3BossDragon::SPAWN_Y - AWAKEN_CENTER_Y) * ease - std::sin(t * 3.14159265f) * 110.0f;
			boss->UpdateAwakeningCutscenePose(bx, by, -1.0f, false, 1.0f);
		}

		if (mTimer >= DURATION)
		{
			mCurrentScrollX  = mSavedPlayerScrollX;
			mCurrentScrollY  = mSavedPlayerScrollY;
			mLetterboxHeight = 0.0f;
			mPhase           = Phase::Finished;
			mTimer           = 0;

			blockMap.SetScrollX(static_cast<int>(mSavedPlayerScrollX));
			blockMap.SetScrollY(static_cast<int>(mSavedPlayerScrollY));
			blockMap.SetVerticalScrollLocked(false);

			if (player != nullptr)
			{
				player->SetInputDisabled(false);
			}
			if (boss != nullptr)
			{
				boss->StartPhase2Battle();
			}
			return;
		}
		break;
	}

	default:
		break;
	}

	blockMap.SetScrollX(static_cast<int>(mCurrentScrollX));
	blockMap.SetScrollY(static_cast<int>(mCurrentScrollY));
}

void Stage3BossMovie::Draw(const BlockMap& blockMap, Stage3BossDragon* boss) const
{
	if (!IsPlaying()) return;

	// 0. 黄金覚醒時の閃光フラッシュ演出
	if (mPhase == Phase::AwakenFocusAndTransform && mTimer >= 42 && mTimer <= 75)
	{
		float flashRatio = 1.0f - (static_cast<float>(std::abs(mTimer - 46)) / 29.0f);
		if (flashRatio < 0.0f) flashRatio = 0.0f;
		if (flashRatio > 1.0f) flashRatio = 1.0f;
		int alpha = static_cast<int>(flashRatio * 210.0f);
		SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
		DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(255, 225, 90), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// 1. 上下のシネマティック黒帯（レターボックス）描画
	int barH = static_cast<int>(mLetterboxHeight);
	if (barH > 0)
	{
		DrawBox(0, 0, ScreenSize::ScrrenWidth, barH, GetColor(0, 0, 0), TRUE);
		DrawBox(0, ScreenSize::ScrrenHeight - barH, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(0, 0, 0), TRUE);
	}

	// 2. 初登場ムービー（FocusOnBoss）の「ドラゴン」ボス名演出
	if (mPhase == Phase::FocusOnBoss && boss != nullptr)
	{
		std::string dragonName = "ドラゴン";
		int fontSize = 108;
		int charsToDraw = (mTimer * 4) / 36;
		if (charsToDraw > 4) charsToDraw = 4;

		int bossScreenX = static_cast<int>(boss->GetPosition().x - blockMap.GetScrollX());
		int startX = bossScreenX - 420;
		if (startX < 120) startX = 120;
		int startY = 260;
		int now = GetNowCount();

		for (int i = 0; i < charsToDraw; ++i)
		{
			std::string charStr = dragonName.substr(i * 2, 2);
			unsigned int color = (i < 2) ? GetColor(255, 80, 60) : GetColor(100, 200, 255);
			int charX = startX + (i * fontSize);
			int swayY = static_cast<int>(std::sin(now / 150.0f + i * 0.8f) * 12.0f);
			Master::mpGameManager->GetFontManager()->DrawDotString(charX + 4, startY + swayY + 4, fontSize, GetColor(0, 0, 0), "%s", charStr.c_str());
			Master::mpGameManager->GetFontManager()->DrawDotString(charX, startY + swayY, fontSize, color, "%s", charStr.c_str());
		}
	}

	// 3. 第2形態覚醒ムービー（AwakenFocusAndTransform）の「アルティメットドラゴン」ボス名演出
	if (mPhase == Phase::AwakenFocusAndTransform && mTimer >= 46 && boss != nullptr)
	{
		std::string ultimateName = "アルティメットドラゴン"; // 全角11文字 (22バイト)
		const int totalChars = 11;
		int fontSize = 86;
		int appearTimer = mTimer - 46;
		int charsToDraw = (appearTimer * totalChars) / 44;
		if (charsToDraw > totalChars) charsToDraw = totalChars;

		int totalWidth = totalChars * fontSize;
		int startX = ScreenSize::CenterX - (totalWidth / 2);
		int startY = 180;
		int now = GetNowCount();

		// 背後の黄金ネームプレート帯
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
		DrawBox(startX - 40, startY - 18, startX + totalWidth + 40, startY + fontSize + 24, GetColor(25, 18, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_ADD, 180);
		DrawLine(startX - 60, startY - 18, startX + totalWidth + 60, startY - 18, GetColor(255, 215, 50), 3);
		DrawLine(startX - 60, startY + fontSize + 24, startX + totalWidth + 60, startY + fontSize + 24, GetColor(255, 215, 50), 3);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		for (int i = 0; i < charsToDraw; ++i)
		{
			std::string charStr = ultimateName.substr(i * 2, 2);
			unsigned int color = (i < 7) ? GetColor(255, 230, 65) : GetColor(255, 95, 45);
			int charX = startX + (i * fontSize);
			int swayY = static_cast<int>(std::sin(now / 130.0f + i * 0.65f) * 10.0f);
			Master::mpGameManager->GetFontManager()->DrawDotString(charX + 4, startY + swayY + 4, fontSize, GetColor(0, 0, 0), "%s", charStr.c_str());
			Master::mpGameManager->GetFontManager()->DrawDotString(charX, startY + swayY, fontSize, color, "%s", charStr.c_str());
		}
	}
}

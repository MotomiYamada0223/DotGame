#include "DeathController.h"
#include "DxLib.h"
#include "GameConstants.h"
#include "Master.h"
#include "InputManager.h"
#include "TutorialTextManager.h"
#include <cmath>

namespace
{
	const int kPreFlashFrames = 22;
	const int kPreFlashHalfFrames = 6;
	const int kPostFlashFrames = 24;
	const int kPostFlashWhiteOutFrame = 10;
	const int kSheenFrames = 6;
	const int kPostTextSkipFrames = 6;
	const int kPostTextWaitFrames = 85;
	const int kAngelBlinkIntervalFrames = 150;
	const int kAngelBlinkDurationFrames = 6;

	bool IsConfirmDown()
	{
		return InputManager::CheckDownKey(KEY_INPUT_SPACE) || InputManager::CheckDownKey(KEY_INPUT_RETURN);
	}
}

DeathController::DeathController()
	: mPhase(Phase::None)
	, mPreFlashTimer(0)
	, mPostFlashTimer(0)
	, mCutInTimer(0)
	, mPostTextTimer(0)
	, mDeathType(DeathReason::DeathType::None)
	, mMotion(AngelMotion::Normal)
	, mAngelAnimState(1)
	, mAngelTalkTimer(0)
	, mAngelBlinkTimer(0)
	, mbAngelBlinking(false)
	, mAngelTextBoxHandle(-1)
{
	LoadGraphs();
}

DeathController::~DeathController()
{
	ReleaseGraphs();
}

void DeathController::LoadGraphs()
{
	mAngelNormalHandles[0] = LoadGraph("Resource/Image/八頭身天使モーション/ノーマル/angel_fullbody_closed.png");
	mAngelNormalHandles[1] = LoadGraph("Resource/Image/八頭身天使モーション/ノーマル/angel_fullbody_open.png");
	mAngelNormalHandles[2] = LoadGraph("Resource/Image/八頭身天使モーション/ノーマル/angel_fullbody_talk.png");

	mAngelLaughHandles[0] = LoadGraph("Resource/Image/八頭身天使モーション/爆笑/爆笑顔1.png");
	mAngelLaughHandles[1] = LoadGraph("Resource/Image/八頭身天使モーション/爆笑/爆笑終わり顔.png");

	mAngelPointHandles[0] = LoadGraph("Resource/Image/八頭身天使モーション/指さし/指さし顔1.png");
	mAngelPointHandles[1] = LoadGraph("Resource/Image/八頭身天使モーション/指さし/指さし顔2.png");

	mAngelHideHandles[0] = LoadGraph("Resource/Image/八頭身天使モーション/隠れ/隠れ顔1.png");
	mAngelHideHandles[1] = LoadGraph("Resource/Image/八頭身天使モーション/隠れ/隠れ顔2.png");

	mAngelTextBoxHandle = LoadGraph("Resource/Image/AngelTextBox.png");
}

void DeathController::ReleaseGraphs()
{
	for (int& h : mAngelNormalHandles) { if (h != -1) { DeleteGraph(h); h = -1; } }
	for (int& h : mAngelLaughHandles) { if (h != -1) { DeleteGraph(h); h = -1; } }
	for (int& h : mAngelPointHandles) { if (h != -1) { DeleteGraph(h); h = -1; } }
	for (int& h : mAngelHideHandles) { if (h != -1) { DeleteGraph(h); h = -1; } }
	if (mAngelTextBoxHandle != -1) { DeleteGraph(mAngelTextBoxHandle); mAngelTextBoxHandle = -1; }
}

void DeathController::Start(DeathReason::DeathType type, AngelMotion motion)
{
	mPhase = Phase::PreFlash;
	mPreFlashTimer = 0;
	mPostFlashTimer = 0;
	mCutInTimer = 0;
	mPostTextTimer = 0;

	mDeathType = type;
	mMotion = motion;
	mAngelAnimState = (motion == AngelMotion::Normal) ? 1 : 0;
	mAngelTalkTimer = 0;
	mAngelBlinkTimer = 0;
	mbAngelBlinking = false;
}

void DeathController::Reset()
{
	mPhase = Phase::None;
	mPreFlashTimer = 0;
	mPostFlashTimer = 0;
	mCutInTimer = 0;
	mPostTextTimer = 0;
	mMotion = AngelMotion::Normal;
	mAngelAnimState = 1;
	mAngelTalkTimer = 0;
	mAngelBlinkTimer = 0;
	mbAngelBlinking = false;
}

// ---------------------------------------------------------------------------
// 更新
// ---------------------------------------------------------------------------

void DeathController::Update(const FrameContext& ctx)
{
	TutorialTextManager* text = ctx.text;

	switch (mPhase)
	{
	case Phase::None:
	case Phase::Finished:
		return;

	case Phase::PreFlash:
		mPreFlashTimer++;
		if (mPreFlashTimer >= kPreFlashFrames)
		{
			// 死因に対応する死亡テキストを開始
			if (text) text->OnPlayerDead(mDeathType);
			mPhase = Phase::CutIn;
			mCutInTimer = 0;
		}
		return;

	case Phase::CutIn:
	{
		mCutInTimer++;

		const bool textFinished = (text == nullptr) || text->GetIsTextCompleted();
		UpdateAngelAnimation(textFinished);

		if (textFinished)
		{
			mPostTextTimer++;
			const bool skipKey = (mPostTextTimer >= kPostTextSkipFrames) && IsConfirmDown();
			if (mPostTextTimer >= kPostTextWaitFrames || skipKey)
			{
				mPhase = Phase::PostFlash;
				mPostFlashTimer = 1;
			}
		}
		return;
	}

	case Phase::PostFlash:
		mPostFlashTimer++;
		if (mPostFlashTimer >= kPostFlashFrames)
		{
			mPhase = Phase::Finished;
		}
		return;
	}
}

void DeathController::UpdateAngelAnimation(bool textFinished)
{
	if (mMotion == AngelMotion::Normal)
	{
		// まばたき
		if (mbAngelBlinking)
		{
			mAngelBlinkTimer++;
			if (mAngelBlinkTimer >= kAngelBlinkDurationFrames)
			{
				mbAngelBlinking = false;
				mAngelBlinkTimer = 0;
			}
			mAngelAnimState = 0;
		}
		else
		{
			mAngelBlinkTimer++;
			if (mAngelBlinkTimer >= kAngelBlinkIntervalFrames)
			{
				mbAngelBlinking = true;
				mAngelBlinkTimer = 0;
			}
		}

		// 口パク
		if (!mbAngelBlinking)
		{
			if (!textFinished)
			{
				mAngelTalkTimer++;
				if (mAngelTalkTimer >= 10)
				{
					mAngelTalkTimer = 0;
					mAngelAnimState = (mAngelAnimState == 1) ? 2 : 1;
				}
			}
			else
			{
				mAngelAnimState = 1;
			}
		}
	}
	else
	{
		// 爆笑・指さし・隠れ：2枚を交互に切り替える
		if (!textFinished)
		{
			mAngelTalkTimer++;
			const int interval = (mMotion == AngelMotion::Laugh) ? 10 : 14;
			if (mAngelTalkTimer >= interval)
			{
				mAngelTalkTimer = 0;
				mAngelAnimState = (mAngelAnimState == 0) ? 1 : 0;
			}
		}
		else
		{
			mAngelAnimState = 0;
		}
	}
}

// ---------------------------------------------------------------------------
// 描画
// ---------------------------------------------------------------------------

void DeathController::Draw(TutorialTextManager* text) const
{
	const bool textFinished = (text == nullptr) || text->GetIsTextCompleted();

	switch (mPhase)
	{
	case Phase::PreFlash:
		if (mPreFlashTimer < kPreFlashHalfFrames)
		{
			// 前半：白フラッシュが急速に立ち上がる
			const float inRate = static_cast<float>(mPreFlashTimer) / kPreFlashHalfFrames;
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255.0f * (inRate * inRate)));
			DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(255, 255, 255), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(180.0f * inRate));
			DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(255, 255, 255), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		else
		{
			// 後半：漆黒に切り替わり、白フラッシュが引いていく
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(0, 0, 0), TRUE);

			const float outRate = 1.0f - static_cast<float>(mPreFlashTimer - kPreFlashHalfFrames)
				/ static_cast<float>(kPreFlashFrames - kPreFlashHalfFrames);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255.0f * (outRate * outRate)));
			DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(255, 255, 255), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(140.0f * outRate));
			DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(255, 255, 255), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		return;

	case Phase::CutIn:
		DrawCutInScene(textFinished);

		// 天使登場直後の光の余韻
		if (mCutInTimer <= kSheenFrames)
		{
			SetDrawBlendMode(DX_BLENDMODE_ADD, (kSheenFrames - mCutInTimer) * 25);
			DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(255, 255, 255), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		return;

	case Phase::PostFlash:
		if (mPostFlashTimer >= kPostFlashWhiteOutFrame)
		{
			// 後半：ゲーム画面へ戻る白フラッシュ（引いていく）
			const float outProgress = static_cast<float>(mPostFlashTimer - kPostFlashWhiteOutFrame)
				/ static_cast<float>(kPostFlashFrames - kPostFlashWhiteOutFrame);
			const float outStrength = 1.0f - outProgress * outProgress;
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255.0f * outStrength));
			DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(255, 255, 255), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(140.0f * outStrength));
			DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(255, 255, 255), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		else
		{
			// 前半：天使を含む画面の上から白が差し込む
			DrawCutInScene(true);

			const float inProgress = static_cast<float>(mPostFlashTimer) / static_cast<float>(kPostFlashWhiteOutFrame);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255.0f * (inProgress * inProgress)));
			DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(255, 255, 255), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(160.0f * inProgress));
			DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(255, 255, 255), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		return;

	default:
		return;
	}
}

void DeathController::DrawCutInScene(bool textFinished) const
{
	// 黒背景
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(0, 0, 0), TRUE);

	DrawAngel(textFinished);
	DrawTextBox(textFinished);
}

void DeathController::DrawAngel(bool textFinished) const
{
	int angelHandle = -1;
	int anchorX = 331;
	int anchorY = 235;
	double baseScale = 1.42;

	if (mMotion == AngelMotion::Normal)
	{
		const int stateIdx = (mAngelAnimState >= 0 && mAngelAnimState < 3) ? mAngelAnimState : 1;
		angelHandle = mAngelNormalHandles[stateIdx];
	}
	else
	{
		anchorX = 384;
		anchorY = 290;
		baseScale = 1.15;
		const int stateIdx = (mAngelAnimState == 1) ? 1 : 0;

		switch (mMotion)
		{
		case AngelMotion::Laugh: angelHandle = mAngelLaughHandles[stateIdx]; break;
		case AngelMotion::Point: angelHandle = mAngelPointHandles[stateIdx]; break;
		case AngelMotion::Hide:  angelHandle = mAngelHideHandles[stateIdx]; break;
		default: break;
		}
	}

	if (angelHandle == -1) return;

	// 登場時のズームイン
	const float zoomT = (mCutInTimer < 10) ? (mCutInTimer / 10.0f) : 1.0f;
	const float easeOut = 1.0f - (1.0f - zoomT) * (1.0f - zoomT);
	const double scale = (baseScale * 1.30) - ((baseScale * 1.30) - baseScale) * easeOut;
	int angelY = static_cast<int>(435.0f - 45.0f * easeOut);

	if (mMotion == AngelMotion::Laugh && !textFinished)
	{
		angelY += (mAngelAnimState == 1) ? -4 : 4;
	}

	DrawRotaGraph2(ScreenSize::CenterX, angelY, anchorX, anchorY, scale, 0.0, angelHandle, TRUE);
}

void DeathController::DrawTextBox(bool textFinished) const
{
	const int boxLeft = 180;
	const int boxTop = 705;
	const int boxRight = ScreenSize::ScrrenWidth - 180;
	const int boxBottom = 975;

	// 枠の背景（セリフ本文は TutorialTextManager::Draw() が描画する）
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 235);
	DrawBox(boxLeft, boxTop, boxRight, boxBottom, GetColor(16, 18, 30), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	if (mAngelTextBoxHandle != -1)
	{
		DrawExtendGraph(boxLeft, boxTop, boxRight, boxBottom, mAngelTextBoxHandle, TRUE);
	}
	DrawBox(boxLeft - 2, boxTop - 2, boxRight + 2, boxBottom + 2, GetColor(240, 200, 90), FALSE);
	DrawBox(boxLeft + 3, boxTop + 3, boxRight - 3, boxBottom - 3, GetColor(110, 85, 35), FALSE);

	if (textFinished)
	{
		const int pulse = 160 + static_cast<int>(90.0f * sinf(mPostTextTimer * 0.18f));
		Master::mpGameManager->GetFontManager()->DrawDotString(
			boxRight - 270, boxBottom - 46, 28, GetColor(pulse, pulse, 120), "SPACE / ENTER >>");
	}
}
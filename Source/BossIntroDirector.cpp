#include "BossIntroDirector.h"
#include "StageClear.h"
#include "Player.h"
#include "BossEnemySlime.h"
#include "BossEnemyDragon.h"
#include "GameConstants.h"
#include "DxLib.h"
#include "Master.h"
#include "GameManager.h"
#include "FontManager.h"
#include <algorithm>
#include <cmath>

BossIntroDirector::BossIntroDirector()
	: mState(State::Inactive)
	, mFadeTimer(0)
	, mStateTimer(0)
	, mTitleTimer(0)
	, mTitleWaveTimer(0)
	, mCurrentTargetX(0.0f)
	, mWarningX(0.0f)
	, mGroundY(0.0f)
	, mpBlockMap(nullptr)
	, mHasEvaded(false)
	, mEKeyWasDown(false)
	, mpBossSlime(nullptr)
	, mpBossDragon(nullptr)
{
}

BossIntroDirector::~BossIntroDirector()
{
}

void BossIntroDirector::Update(Player* player, BlockMap& blockMap)
{
	if (!player) return;
	mpBlockMap = &blockMap;

	// ステージ2判定（0がステージ1、1がステージ2）
	bool isStage2 = (Master::gCurrentStageIndex == 1);

	switch (mState)
	{
	case State::Inactive:
		// プレイヤーがボスエリアに接触したら演出を開始する
		if (!player->GetIsDead() && blockMap.HasBossArea())
		{
			bool entered = player->IsTouchingBossArea();
			if (entered)
			{
				mState = State::Walking;
				mFadeTimer = 0;
				mStateTimer = 0;
				mHasEvaded = false;
				mEKeyWasDown = false;
				mCurrentTargetX = blockMap.GetBossAreaCenterX();
				player->StartAutoWalk(mCurrentTargetX);
			}
		}
		break;

	case State::Walking:
		// 上下黒帯のフェードインを進める
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		// プレイヤーが中央に到着したら待機ステートへ移行し、カメラを固定する
		if (player->IsAutoWalkArrived())
		{
			mState = State::WaitBeforeWarning;
			mStateTimer = 0;
			blockMap.SetCameraLocked(true);
		}
		break;

	case State::WaitBeforeWarning:
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		// 警告表示前の溜め処理
		mStateTimer++;
		if (mStateTimer >= KingSlimeIntroConstants::WaitFramesBeforeWarning)
		{
			mWarningX = player->GetPosition().x;
			mGroundY = player->GetPosition().y + player->GetPlayerHeight() * 0.5f;
			mHasEvaded = false;
			mEKeyWasDown = false;
			mStateTimer = 0;
			mState = State::Warning;
			player->SetSwordReadyStance(true);
		}
		break;

	case State::Warning:
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		// 警告表示中のQTE（回避アクション）入力を監視する
		{
			bool blinkKeyIsDown = (CheckHitKey(KEY_INPUT_LSHIFT) == 1 || CheckHitKey(KEY_INPUT_RSHIFT) == 1 || CheckHitKey(KEY_INPUT_E) == 1);
			if (blinkKeyIsDown && !mEKeyWasDown && !mHasEvaded)
			{
				player->TriggerCinematicEvasion();
				mHasEvaded = true;
			}
			mEKeyWasDown = blinkKeyIsDown;
		}

		mStateTimer++;
		if (mStateTimer >= KingSlimeIntroConstants::WarningDurationFrames)
		{
			mState = State::WarningFadeOut;
			mStateTimer = KingSlimeIntroConstants::WarningFadeOutFrames;
		}
		break;

	case State::WarningFadeOut:
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		{
			bool blinkKeyIsDown = (CheckHitKey(KEY_INPUT_LSHIFT) == 1 || CheckHitKey(KEY_INPUT_RSHIFT) == 1 || CheckHitKey(KEY_INPUT_E) == 1);
			if (blinkKeyIsDown && !mEKeyWasDown && !mHasEvaded)
			{
				player->TriggerCinematicEvasion();
				mHasEvaded = true;
			}
			mEKeyWasDown = blinkKeyIsDown;
		}

		mStateTimer--;
		if (mStateTimer <= 0)
		{
			mState = State::BossFalling;

			// ステージ1ならスライム、ステージ2ならドラゴンを生成して登場演出開始
			if (isStage2)
			{
				VECTOR dragonSpawnPos = VGet(mWarningX, 980.0f, 0.0f);
				mpBossDragon = new BossEnemyDragon(dragonSpawnPos);
				mpBossDragon->StartEntrance();
			}
			else
			{
				const float startY = mGroundY - 1200.0f;
				mpBossSlime = new BossEnemySlime(VGet(mWarningX, startY, 0.0f), mGroundY, player);
			}
		}
		break;

	case State::BossFalling:
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		// ボスの落下・降下演出の完了を監視して着地ステートへ移行する
		if (isStage2)
		{
			if (mpBossDragon != nullptr && mpBossDragon->GetState() == BossEnemyDragon::State::EntranceWait)
			{
				mState = State::BossLanding;
			}
		}
		else
		{
			if (mpBossSlime != nullptr && mpBossSlime->GetState() == KingSlimeState::SpawnLandingReverse)
			{
				mState = State::BossLanding;
			}
		}
		break;

	case State::BossLanding:
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		// 着地演出の完了を監視してタイトル待機ステートへ移行する
		if (isStage2)
		{
			if (mpBossDragon != nullptr && mpBossDragon->GetState() == BossEnemyDragon::State::Fly)
			{
				mState = State::TitleWait;
				mTitleTimer = 0;
			}
		}
		else
		{
			if (mpBossSlime != nullptr && mpBossSlime->IsSpawnFinished())
			{
				mState = State::TitleWait;
				mTitleTimer = 0;
			}
		}
		break;

	case State::TitleWait:
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		mTitleTimer++;
		if (mTitleTimer >= KingSlimeTitleConstants::WaitFramesBeforeTitle)
		{
			mState = State::TitleDisplay;
			mTitleTimer = 0;
			mTitleWaveTimer = 0;
		}
		break;

	case State::TitleDisplay:
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		mTitleTimer++;
		mTitleWaveTimer++;

		{
			const int totalRevealTime = KingSlimeTitleConstants::CharCount * KingSlimeTitleConstants::CharRevealInterval;
			if (mTitleTimer >= totalRevealTime + KingSlimeTitleConstants::HoldFramesAfterReveal)
			{
				mState = State::TitleFadeOut;
				mTitleTimer = 0;
			}
		}
		break;

	case State::TitleFadeOut:
		mTitleTimer++;
		mTitleWaveTimer++;

		{
			float fadeRatio = 1.0f - (static_cast<float>(mTitleTimer) / static_cast<float>(KingSlimeTitleConstants::FadeOutFrames));
			if (fadeRatio < 0.0f) fadeRatio = 0.0f;
			if (fadeRatio > 1.0f) fadeRatio = 1.0f;
			mFadeTimer = static_cast<int>(static_cast<float>(BossIntroConstants::FadeFrames) * fadeRatio);
		}

		if (mTitleTimer >= KingSlimeTitleConstants::FadeOutFrames)
		{
			mState = State::IntroFinished;
			mFadeTimer = 0;
			if (player != nullptr)
			{
				player->EndBossIntroLock();
			}
			if (mpBossSlime != nullptr)
			{
				mpBossSlime->StartBattle();
			}
		}
		break;

	case State::IntroFinished:
		mFadeTimer = 0;
		if (mpBossSlime != nullptr && mpBossSlime->IsDeleteFlag())
		{
			mpBossSlime = nullptr;
		}
		if (mpBossDragon != nullptr && mpBossDragon->HasRetreated())
		{
			mpBossDragon = nullptr;
		}

		// ステージクリアまたはボス生存中はカメラ固定を維持する
		if (StageClear::IsStageClearActive())
		{
			blockMap.SetCameraLocked(true);
		}
		else if (mpBossSlime == nullptr && mpBossDragon == nullptr)
		{
			blockMap.SetCameraLocked(false);
		}
		break;

	case State::Ended:
		break;
	}
}

void BossIntroDirector::Draw() const
{
	if (mState == State::Inactive)
	{
		return;
	}

	// 1. 上下黒帯の描画
	float progress = static_cast<float>(mFadeTimer) / static_cast<float>(BossIntroConstants::FadeFrames);
	if (progress < 0.0f) progress = 0.0f;
	if (progress > 1.0f) progress = 1.0f;

	float inv = 1.0f - progress;
	float ease = 1.0f - (inv * inv * inv);

	int currentHeight = static_cast<int>(static_cast<float>(BossIntroConstants::LetterboxHeight) * ease);
	int currentAlpha = static_cast<int>(static_cast<float>(BossIntroConstants::LetterboxAlpha) * ease);

	if (currentHeight > 0 && currentAlpha > 0)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, currentAlpha);
		DrawBox(0, 0, ScreenSize::ScrrenWidth, currentHeight, GetColor(0, 0, 0), TRUE);
		DrawBox(0, ScreenSize::ScrrenHeight - currentHeight, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// 2. 警告表示の描画
	if (mState == State::Warning || mState == State::WarningFadeOut)
	{
		float scrollX = (mpBlockMap != nullptr) ? static_cast<float>(mpBlockMap->GetScrollX()) : 0.0f;
		float scrollY = (mpBlockMap != nullptr) ? static_cast<float>(mpBlockMap->GetScrollY()) : 0.0f;

		float screenX = mWarningX - scrollX;
		float screenY = mGroundY - scrollY;

		float fadeRatio = 1.0f;
		if (mState == State::WarningFadeOut)
		{
			fadeRatio = static_cast<float>(mStateTimer) / static_cast<float>(KingSlimeIntroConstants::WarningFadeOutFrames);
			if (fadeRatio < 0.0f) fadeRatio = 0.0f;
			if (fadeRatio > 1.0f) fadeRatio = 1.0f;
		}

		DrawWarning(screenX, screenY, fadeRatio);
	}

	// 3. ボス名タイトルの描画（スライム用。ドラゴンは固有の演出UIを持つため非表示）
	if (mState == State::TitleDisplay || mState == State::TitleFadeOut)
	{
		if (mpBossDragon == nullptr)
		{
			float scrollX = (mpBlockMap != nullptr) ? static_cast<float>(mpBlockMap->GetScrollX()) : 0.0f;
			float scrollY = (mpBlockMap != nullptr) ? static_cast<float>(mpBlockMap->GetScrollY()) : 0.0f;

			float screenX = mWarningX - scrollX;
			float screenY = mGroundY - scrollY;

			DrawTitle(screenX, screenY);
		}
	}

	// 4. ボスHPバー・UIの描画
	if (mpBossSlime != nullptr)
	{
		mpBossSlime->DrawHpBar();
	}
}

void BossIntroDirector::DrawTitle(float screenX, float screenY) const
{
	if (!Master::mpGameManager) return;
	FontManager* fontMgr = Master::mpGameManager->GetFontManager();
	if (!fontMgr) return;

	static const char* const kTitleChars[KingSlimeTitleConstants::CharCount] = {
		"キ", "ン", "グ", "ス", "ラ", "イ", "ミ", "ー"
	};

	int revealedCount = 0;
	if (mState == State::TitleDisplay)
	{
		revealedCount = (mTitleTimer / KingSlimeTitleConstants::CharRevealInterval) + 1;
		if (revealedCount > KingSlimeTitleConstants::CharCount)
		{
			revealedCount = KingSlimeTitleConstants::CharCount;
		}
	}
	else if (mState == State::TitleFadeOut)
	{
		revealedCount = KingSlimeTitleConstants::CharCount;
	}

	if (revealedCount <= 0) return;

	int alpha = 255;
	if (mState == State::TitleFadeOut)
	{
		float fadeRatio = 1.0f - (static_cast<float>(mTitleTimer) / static_cast<float>(KingSlimeTitleConstants::FadeOutFrames));
		if (fadeRatio < 0.0f) fadeRatio = 0.0f;
		if (fadeRatio > 1.0f) fadeRatio = 1.0f;
		alpha = static_cast<int>(255.0f * fadeRatio);
	}
	if (alpha <= 0) return;

	int charWidths[KingSlimeTitleConstants::CharCount] = { 0 };
	int totalWidth = 0;
	for (int i = 0; i < KingSlimeTitleConstants::CharCount; i++)
	{
		charWidths[i] = fontMgr->GetDotStringWidth(KingSlimeTitleConstants::FontSize, kTitleChars[i]);
		totalWidth += charWidths[i];
		if (i < KingSlimeTitleConstants::CharCount - 1)
		{
			totalWidth += KingSlimeTitleConstants::CharSpacing;
		}
	}

	const float topLimit = static_cast<float>(UIGraphController::FramePosY + 190);
	const float bottomLimit = screenY - (84.0f * KingSlimeIntroConstants::Scale);
	const float centerY = (topLimit + bottomLimit) * 0.5f;
	const float baseY = centerY - (static_cast<float>(KingSlimeTitleConstants::FontSize) * 0.5f);

	float currentX = screenX - (static_cast<float>(totalWidth) * 0.5f);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

	for (int i = 0; i < KingSlimeTitleConstants::CharCount; i++)
	{
		if (i < revealedCount)
		{
			float waveOffset = std::sin(static_cast<float>(mTitleWaveTimer) * KingSlimeTitleConstants::WaveSpeed + static_cast<float>(i) * KingSlimeTitleConstants::WavePhaseOffset) * KingSlimeTitleConstants::WaveAmplitude;
			int drawX = static_cast<int>(currentX);
			int drawY = static_cast<int>(baseY + waveOffset);

			fontMgr->DrawDotString(
				drawX + 3,
				drawY + 3,
				KingSlimeTitleConstants::FontSize,
				GetColor(0, 0, 0),
				"%s",
				kTitleChars[i]
			);

			unsigned int charColor = (i < 3)
				? GetColor(KingSlimeTitleConstants::ColorGoldR, KingSlimeTitleConstants::ColorGoldG, KingSlimeTitleConstants::ColorGoldB)
				: GetColor(KingSlimeTitleConstants::ColorCyanR, KingSlimeTitleConstants::ColorCyanG, KingSlimeTitleConstants::ColorCyanB);

			fontMgr->DrawDotString(
				drawX,
				drawY,
				KingSlimeTitleConstants::FontSize,
				charColor,
				"%s",
				kTitleChars[i]
			);
		}

		currentX += static_cast<float>(charWidths[i] + KingSlimeTitleConstants::CharSpacing);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void BossIntroDirector::DrawWarning(float screenX, float screenY, float fadeAlphaRatio) const
{
	if (fadeAlphaRatio <= 0.0f) return;

	const float width = KingSlimeIntroConstants::WarningWidth;
	const float height = KingSlimeIntroConstants::WarningHeight;
	const int left = static_cast<int>(screenX - width * 0.5f);
	const int right = static_cast<int>(screenX + width * 0.5f);
	const int bottom = static_cast<int>(screenY);
	const int top = static_cast<int>(screenY - height);

	const int sliceCount = 15;
	const float sliceHeight = height / static_cast<float>(sliceCount);

	for (int i = 0; i < sliceCount; i++)
	{
		int stripTop = static_cast<int>(top + i * sliceHeight);
		int stripBottom = static_cast<int>(top + (i + 1) * sliceHeight);
		if (i == sliceCount - 1) stripBottom = bottom;

		float v = static_cast<float>(i + 1) / static_cast<float>(sliceCount);
		int alpha = static_cast<int>(140.0f * (v * v) * fadeAlphaRatio);
		if (alpha <= 0) continue;

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawBox(left, stripTop, right, stripBottom, GetColor(255, 30, 30), TRUE);
	}

	const int edgeThickness = 6;
	for (int i = 0; i < sliceCount; i++)
	{
		int stripTop = static_cast<int>(top + i * sliceHeight);
		int stripBottom = static_cast<int>(top + (i + 1) * sliceHeight);
		if (i == sliceCount - 1) stripBottom = bottom;

		float v = static_cast<float>(i + 1) / static_cast<float>(sliceCount);
		int borderAlpha = static_cast<int>((60.0f + 180.0f * v) * fadeAlphaRatio);
		if (borderAlpha <= 0) continue;

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, borderAlpha);
		DrawBox(left, stripTop, left + edgeThickness, stripBottom, GetColor(255, 0, 0), TRUE);
		DrawBox(right - edgeThickness, stripTop, right, stripBottom, GetColor(255, 0, 0), TRUE);
	}

	int groundLineAlpha = static_cast<int>(240.0f * fadeAlphaRatio);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, groundLineAlpha);
	DrawBox(left, bottom - 6, right, bottom, GetColor(255, 60, 60), TRUE);

	int midX = static_cast<int>(screenX);
	int midY = static_cast<int>(screenY - height * 0.45f);

	float pulse = 1.0f + 0.1f * std::sin(static_cast<float>(mStateTimer) * 0.2f);

	int markBodyAlpha = static_cast<int>(255.0f * fadeAlphaRatio);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, markBodyAlpha);
	int barWidth = 6;
	int barTop = midY - static_cast<int>(30.0f * pulse);
	int barBottom = midY + static_cast<int>(8.0f * pulse);
	DrawBox(midX - barWidth, barTop, midX + barWidth + 1, barBottom, GetColor(255, 0, 0), TRUE);
	int dotTop = midY + static_cast<int>(16.0f * pulse);
	int dotBottom = midY + static_cast<int>(28.0f * pulse);
	DrawBox(midX - barWidth, dotTop, midX + barWidth + 1, dotBottom, GetColor(255, 0, 0), TRUE);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void BossIntroDirector::EndIntro()
{
	mState = State::Ended;
}

void BossIntroDirector::Reset()
{
	if (mpBlockMap != nullptr)
	{
		mpBlockMap->SetCameraLocked(false);
	}
	if (mpBossSlime != nullptr)
	{
		mpBossSlime->SetDeleteFlag(true);
		mpBossSlime = nullptr;
	}
	if (mpBossDragon != nullptr)
	{
		delete mpBossDragon;
		mpBossDragon = nullptr;
	}
	mState = State::Inactive;
	mFadeTimer = 0;
	mStateTimer = 0;
	mTitleTimer = 0;
	mTitleWaveTimer = 0;
	mCurrentTargetX = 0.0f;
	mWarningX = 0.0f;
	mGroundY = 0.0f;
	mHasEvaded = false;
	mEKeyWasDown = false;
}
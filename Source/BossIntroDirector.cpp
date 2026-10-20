#include "BossIntroDirector.h"
#include "Player.h"
#include "BossEnemySlime.h"
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
{
}

BossIntroDirector::~BossIntroDirector()
{
}

void BossIntroDirector::Update(Player* player, BlockMap& blockMap)
{
	if (!player) return;
	mpBlockMap = &blockMap;

	switch (mState)
	{
	case State::Inactive:
		// プレイヤーにボスエリア接触（またはボスエリアX範囲到達）したら演出開始
		if (!player->GetIsDead() && blockMap.HasBossArea())
		{
			bool entered = player->IsTouchingBossArea() ||
				(player->GetPosition().x >= static_cast<float>(blockMap.GetBossAreaLeft()));

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
		// 帯フェードタイマー加算
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		// プレイヤー中央到着で待機ステートへ遷移し、カメラスクロール固定
		if (player->IsAutoWalkArrived())
		{
			mState = State::WaitBeforeWarning;
			mStateTimer = 0;
			blockMap.SetCameraLocked(true); // ボスエリア中央到達時にカメラ固定
		}
		break;

	case State::WaitBeforeWarning:
		// 帯は最大値維持
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		// 最初の溜め
		mStateTimer++;
		if (mStateTimer >= KingSlimeIntroConstants::WaitFramesBeforeWarning)
		{
			// 警告：プレイヤーの足元X座標を記録
			mWarningX = player->GetPosition().x;
			mGroundY = player->GetPosition().y + player->GetPlayerHeight() * 0.5f;
			mHasEvaded = false;
			mEKeyWasDown = false;
			mStateTimer = 0;
			mState = State::Warning;
			player->SetSwordReadyStance(true); // 警告表示開始と同時に構え一枚絵へ移行
		}
		break;

	case State::Warning:
		// 帯は最大値維持
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		// 警告表示中のみブリンク操作（Shift / Eキー）を受付
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
		// 帯は最大値維持
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		// フェードアウト終了までブリンク回避可能
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
			// 画面上空から急降下開始（4倍に伴い地面Yの1200px上空から落下開始）
			const float startY = mGroundY - 1200.0f;
			mpBossSlime = new BossEnemySlime(VGet(mWarningX, startY, 0.0f), mGroundY, player);
		}
		break;

	case State::BossFalling:
		// 帯は維持
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		if (mpBossSlime != nullptr && mpBossSlime->GetState() == KingSlimeState::SpawnLandingReverse)
		{
			mState = State::BossLanding;
		}
		break;

	case State::BossLanding:
		// 帯は維持
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		if (mpBossSlime != nullptr && mpBossSlime->IsSpawnFinished())
		{
			mState = State::TitleWait;
			mTitleTimer = 0;
		}
		break;

	case State::TitleWait:
		// 帯は維持
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
		// 帯は維持
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

		// 文字のフェードアウト進行度に合わせて上下黒帯もフェードアウト
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
		// 演出完了：黒帯は非表示
		mFadeTimer = 0;
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

	// イージング
	float inv = 1.0f - progress;
	float ease = 1.0f - (inv * inv * inv);

	int currentHeight = static_cast<int>(static_cast<float>(BossIntroConstants::LetterboxHeight) * ease);
	int currentAlpha = static_cast<int>(static_cast<float>(BossIntroConstants::LetterboxAlpha) * ease);

	if (currentHeight > 0 && currentAlpha > 0)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, currentAlpha);

		// 画面上部
		DrawBox(
			0,
			0,
			ScreenSize::ScrrenWidth,
			currentHeight,
			GetColor(0, 0, 0),
			TRUE
		);

		// 画面下部
		DrawBox(
			0,
			ScreenSize::ScrrenHeight - currentHeight,
			ScreenSize::ScrrenWidth,
			ScreenSize::ScrrenHeight,
			GetColor(0, 0, 0),
			TRUE
		);

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

	// 3. ボス名タイトル「キングスライミー」の描画
	if (mState == State::TitleDisplay || mState == State::TitleFadeOut)
	{
		float scrollX = (mpBlockMap != nullptr) ? static_cast<float>(mpBlockMap->GetScrollX()) : 0.0f;
		float scrollY = (mpBlockMap != nullptr) ? static_cast<float>(mpBlockMap->GetScrollY()) : 0.0f;

		float screenX = mWarningX - scrollX;
		float screenY = mGroundY - scrollY;

		DrawTitle(screenX, screenY);
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

	// 表示文字数の算出
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

	// 透明度（フェードアウト時）
	int alpha = 255;
	if (mState == State::TitleFadeOut)
	{
		float fadeRatio = 1.0f - (static_cast<float>(mTitleTimer) / static_cast<float>(KingSlimeTitleConstants::FadeOutFrames));
		if (fadeRatio < 0.0f) fadeRatio = 0.0f;
		if (fadeRatio > 1.0f) fadeRatio = 1.0f;
		alpha = static_cast<int>(255.0f * fadeRatio);
	}
	if (alpha <= 0) return;

	// 各文字の描画幅を計測し全体の横幅を算出
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

	// 垂直方向の位置計算
	// 上端限界: 天使セリフ枠の下 (FramePosY + 190)
	// 下端限界: キングスライム頭頂部 (screenY - 84 * Scale)
	const float topLimit = static_cast<float>(UIGraphController::FramePosY + 190);
	const float bottomLimit = screenY - (84.0f * KingSlimeIntroConstants::Scale);
	const float centerY = (topLimit + bottomLimit) * 0.5f;
	const float baseY = centerY - (static_cast<float>(KingSlimeTitleConstants::FontSize) * 0.5f);

	// 水平方向の中央揃え開始X
	float currentX = screenX - (static_cast<float>(totalWidth) * 0.5f);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

	for (int i = 0; i < KingSlimeTitleConstants::CharCount; i++)
	{
		if (i < revealedCount)
		{
			// 位相差を持たせた緩やかなサイン波による縦揺れ
			float waveOffset = std::sin(static_cast<float>(mTitleWaveTimer) * KingSlimeTitleConstants::WaveSpeed + static_cast<float>(i) * KingSlimeTitleConstants::WavePhaseOffset) * KingSlimeTitleConstants::WaveAmplitude;
			int drawX = static_cast<int>(currentX);
			int drawY = static_cast<int>(baseY + waveOffset);

			// ドロップシャドウ（黒、3pxオフセット）
			fontMgr->DrawDotString(
				drawX + 3,
				drawY + 3,
				KingSlimeTitleConstants::FontSize,
				GetColor(0, 0, 0),
				"%s",
				kTitleChars[i]
			);

			// 文字色：「キング」（i = 0, 1, 2）は金色、「スライミー」（i = 3..7）は濃い水色
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

	// 1. 縦グラデーション
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

	// 2. 左右のエッジライン
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

	// 接地面ライン
	int groundLineAlpha = static_cast<int>(240.0f * fadeAlphaRatio);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, groundLineAlpha);
	DrawBox(left, bottom - 6, right, bottom, GetColor(255, 60, 60), TRUE);

	// 3. 範囲中央の「！」マーク
	int midX = static_cast<int>(screenX);
	int midY = static_cast<int>(screenY - height * 0.45f);

	float pulse = 1.0f + 0.1f * std::sin(static_cast<float>(mStateTimer) * 0.2f);

	// 「！」マーク本体（赤）
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

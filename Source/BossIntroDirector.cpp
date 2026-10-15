#include "BossIntroDirector.h"
#include "Player.h"
#include "BossEnemySlime.h"
#include "GameConstants.h"
#include "DxLib.h"
#include <algorithm>
#include <cmath>

BossIntroDirector::BossIntroDirector()
	: mState(State::Inactive)
	, mFadeTimer(0)
	, mStateTimer(0)
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
		// プレイヤー生存中にボスエリア接触（またはボスエリアX範囲到達）を検知したら演出開始
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
		// 黒帯フェードタイマー加算
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		// プレイヤーが中央に到着したら待機ステートへ遷移し、カメラスクロールを固定
		if (player->IsAutoWalkArrived())
		{
			mState = State::WaitBeforeWarning;
			mStateTimer = 0;
			blockMap.SetCameraLocked(true); // ボスエリア中央到達時にカメラ固定
		}
		break;

	case State::WaitBeforeWarning:
		// 黒帯は最大値を維持
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		// 到着後の少しのタメ時間
		mStateTimer++;
		if (mStateTimer >= KingSlimeIntroConstants::WaitFramesBeforeWarning)
		{
			// 警告発生：プレイヤーの足元とX座標を記録
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
		// 黒帯は最大値を維持
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		// 警告表示中のみEキーの操作を受け付け（QTE）
		{
			bool eKeyIsDown = (CheckHitKey(KEY_INPUT_E) == 1);
			if (eKeyIsDown && !mEKeyWasDown && !mHasEvaded)
			{
				player->TriggerCinematicEvasion();
				mHasEvaded = true;
			}
			mEKeyWasDown = eKeyIsDown;
		}

		mStateTimer++;
		if (mStateTimer >= KingSlimeIntroConstants::WarningDurationFrames)
		{
			mState = State::WarningFadeOut;
			mStateTimer = KingSlimeIntroConstants::WarningFadeOutFrames;
		}
		break;

	case State::WarningFadeOut:
		// 黒帯は最大値を維持
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		// フェードアウト中もギリギリまでEキー回避可能
		{
			bool eKeyIsDown = (CheckHitKey(KEY_INPUT_E) == 1);
			if (eKeyIsDown && !mEKeyWasDown && !mHasEvaded)
			{
				player->TriggerCinematicEvasion();
				mHasEvaded = true;
			}
			mEKeyWasDown = eKeyIsDown;
		}

		mStateTimer--;
		if (mStateTimer <= 0)
		{
			mState = State::BossFalling;
			// 画面上空から急降下開始（4倍化に伴い地面Yから1200px上空から落下開始）
			const float startY = mGroundY - 1200.0f;
			mpBossSlime = new BossEnemySlime(VGet(mWarningX, startY, 0.0f), mGroundY, player);
		}
		break;

	case State::BossFalling:
		// 黒帯は維持
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
		// 黒帯は維持
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
		}

		if (mpBossSlime != nullptr && mpBossSlime->IsSpawnFinished())
		{
			mState = State::IntroFinished;
		}
		break;

	case State::IntroFinished:
		// 演出完了後も黒帯はアクティブのまま維持
		if (mFadeTimer < BossIntroConstants::FadeFrames)
		{
			mFadeTimer++;
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

	// イージング
	float inv = 1.0f - progress;
	float ease = 1.0f - (inv * inv * inv);

	int currentHeight = static_cast<int>(static_cast<float>(BossIntroConstants::LetterboxHeight) * ease);
	int currentAlpha = static_cast<int>(static_cast<float>(BossIntroConstants::LetterboxAlpha) * ease);

	if (currentHeight > 0 && currentAlpha > 0)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, currentAlpha);

		// 画面上部黒帯
		DrawBox(
			0,
			0,
			ScreenSize::ScrrenWidth,
			currentHeight,
			GetColor(0, 0, 0),
			TRUE
		);

		// 画面下部黒帯
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

	// 1. 縦方向グラデーション（下側が濃く、上側に行くほど透明）
	const int sliceCount = 15;
	const float sliceHeight = height / static_cast<float>(sliceCount);

	for (int i = 0; i < sliceCount; i++)
	{
		int stripTop = static_cast<int>(top + i * sliceHeight);
		int stripBottom = static_cast<int>(top + (i + 1) * sliceHeight);
		if (i == sliceCount - 1) stripBottom = bottom;

		// v: 0.0(上端) 〜 1.0(下端)
		float v = static_cast<float>(i + 1) / static_cast<float>(sliceCount);
		// 下側ほど濃くなる2乗カーブ
		int alpha = static_cast<int>(140.0f * (v * v) * fadeAlphaRatio);
		if (alpha <= 0) continue;

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawBox(left, stripTop, right, stripBottom, GetColor(255, 30, 30), TRUE);
	}

	// 2. 左右端のエッジライン（左右端がより赤い、4倍化に合わせて太さ調整）
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
		// 左端ライン
		DrawBox(left, stripTop, left + edgeThickness, stripBottom, GetColor(255, 0, 0), TRUE);
		// 右端ライン
		DrawBox(right - edgeThickness, stripTop, right, stripBottom, GetColor(255, 0, 0), TRUE);
	}

	// 接地下面ライン
	int groundLineAlpha = static_cast<int>(240.0f * fadeAlphaRatio);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, groundLineAlpha);
	DrawBox(left, bottom - 6, right, bottom, GetColor(255, 60, 60), TRUE);

	// 3. 範囲中央の「！」マーク
	int midX = static_cast<int>(screenX);
	int midY = static_cast<int>(screenY - height * 0.45f);

	// 警告マークの鼓動（パルス）エフェクト
	float pulse = 1.0f + 0.1f * std::sin(static_cast<float>(mStateTimer) * 0.2f);
	int circleRadius = static_cast<int>(36.0f * pulse);

	//// 背景丸（暗めの赤）
	//int markBgAlpha = static_cast<int>(220.0f * fadeAlphaRatio);
	//SetDrawBlendMode(DX_BLENDMODE_ALPHA, markBgAlpha);
	//DrawCircle(midX, midY, circleRadius, GetColor(180, 20, 20), TRUE);
	//
	//// 外枠丸（警告の黄色）
	//int markOutlineAlpha = static_cast<int>(255.0f * fadeAlphaRatio);
	//SetDrawBlendMode(DX_BLENDMODE_ALPHA, markOutlineAlpha);
	//DrawCircle(midX, midY, circleRadius, GetColor(255, 230, 0), FALSE);

	// 「！」マーク本体（赤）
	int markBodyAlpha = static_cast<int>(255.0f * fadeAlphaRatio);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, markBodyAlpha);
	// 上の縦棒
	int barWidth = 6;
	int barTop = midY - static_cast<int>(30.0f * pulse);
	int barBottom = midY + static_cast<int>(8.0f * pulse);
	DrawBox(midX - barWidth, barTop, midX + barWidth + 1, barBottom, GetColor(255, 0, 0), TRUE);
	// 下の点
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
	mCurrentTargetX = 0.0f;
	mWarningX = 0.0f;
	mGroundY = 0.0f;
	mHasEvaded = false;
	mEKeyWasDown = false;
}

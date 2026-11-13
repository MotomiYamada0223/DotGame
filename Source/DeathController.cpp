#include "DeathController.h"
#include "DxLib.h"
#include "GameConstants.h"
#include "Master.h"
#include "InputManager.h"
#include "TutorialTextManager.h"
#include <cmath>

// このファイル内だけで使用する定数・補助関数をまとめる。
namespace
{
	// 死亡演出開始時のフラッシュ全体の長さ（フレーム数）。
	const int kPreFlashFrames = 22;
	// 開始フラッシュのうち、白く強く光る前半部分の長さ。
	const int kPreFlashHalfFrames = 6;
	// 天使のカットイン終了後、通常画面へ戻るフラッシュの長さ。
	const int kPostFlashFrames = 24;
	// 終了フラッシュで白く覆い始めるまでのフレーム数。
	const int kPostFlashWhiteOutFrame = 10;
	// 天使登場直後に重ねる光の余韻の長さ。
	const int kSheenFrames = 6;
	// セリフ表示完了後、キーによるスキップを受け付けるまでの待ち時間。
	const int kPostTextSkipFrames = 6;
	// セリフ表示完了後、自動で次の演出へ進むまでの待ち時間。
	const int kPostTextWaitFrames = 85;
	// 通常モーション時に、まばたきを開始する間隔。
	const int kAngelBlinkIntervalFrames = 150;
	// まばたき状態を維持する長さ。
	const int kAngelBlinkDurationFrames = 6;

	// SPACE または ENTER がこのフレームで押されたかを判定する。
	bool IsConfirmDown()
	{
		return InputManager::CheckDownKey(KEY_INPUT_SPACE) || InputManager::CheckDownKey(KEY_INPUT_RETURN);
	}
}

// メンバー変数を初期化し、天使・テキスト枠の画像を読み込む。
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

// 使用していた画像ハンドルを解放する。
DeathController::~DeathController()
{
	ReleaseGraphs();
}

// 天使の各モーション画像と、セリフ枠画像を読み込む。
void DeathController::LoadGraphs()
{
	mAngelNormalHandles[0] = LoadGraph("Resource/Saint/Idel/angel_fullbody_closed.png");
	mAngelNormalHandles[1] = LoadGraph("Resource/Saint/Idel/angel_fullbody_open.png");
	mAngelNormalHandles[2] = LoadGraph("Resource/Saint/Idel/angel_fullbody_talk.png");

	mAngelLaughHandles[0] = LoadGraph("Resource/Saint/Smile/smile_L.png");
	mAngelLaughHandles[1] = LoadGraph("Resource/Saint/Smile/smile_R.png");

	mAngelPointHandles[0] = LoadGraph("Resource/Saint/Hidden/hidden_1.png");
	mAngelPointHandles[1] = LoadGraph("Resource/Saint/Hidden/hidden_2.png");

	mAngelHideHandles[0] = LoadGraph("Resource/Saint/Pointing/pointing_1.png");
	mAngelHideHandles[1] = LoadGraph("Resource/Saint/Pointing/pointing_2.png");

	mAngelTextBoxHandle = LoadGraph("Resource/Saint/AngelTextBox.png");
}

// 読み込み済みの画像を解放する。-1 は未読み込みとして扱う。
void DeathController::ReleaseGraphs()
{
	for (int& h : mAngelNormalHandles) { if (h != -1) { DeleteGraph(h); h = -1; } }
	for (int& h : mAngelLaughHandles) { if (h != -1) { DeleteGraph(h); h = -1; } }
	for (int& h : mAngelPointHandles) { if (h != -1) { DeleteGraph(h); h = -1; } }
	for (int& h : mAngelHideHandles) { if (h != -1) { DeleteGraph(h); h = -1; } }
	if (mAngelTextBoxHandle != -1) { DeleteGraph(mAngelTextBoxHandle); mAngelTextBoxHandle = -1; }
}

// 死亡演出を開始し、タイマー・表示状態・演出内容を初期化する。
void DeathController::Start(DeathReason::DeathType type, AngelMotion motion)
{
	mPhase = Phase::PreFlash;
	mPreFlashTimer = 0;
	mPostFlashTimer = 0;
	mCutInTimer = 0;
	mPostTextTimer = 0;

	// 死因と、登場する天使のモーションを保存する。
	mDeathType = type;
	mMotion = motion;
	mAngelAnimState = (motion == AngelMotion::Normal) ? 1 : 0;
	mAngelTalkTimer = 0;
	mAngelBlinkTimer = 0;
	mbAngelBlinking = false;
}

// 死亡演出を終了状態から初期状態へ戻す。
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
// 更新：演出フェーズと各アニメーションの時間を進める。
// ---------------------------------------------------------------------------

// 現在のフェーズに応じて死亡演出を更新する。
void DeathController::Update(const FrameContext& ctx)
{
	TutorialTextManager* text = ctx.text;

	switch (mPhase)
	{
	case Phase::None:
	case Phase::Finished:
		return;

		// 画面をフラッシュさせた後、死亡理由のテキストと天使のカットインへ移る。
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

		// 天使と死亡理由のテキストを表示する。テキスト終了後は待機・スキップ判定を行う。
	case Phase::CutIn:
	{
		mCutInTimer++;

		// テキスト管理クラスがない場合も、表示完了として扱う。
		const bool textFinished = (text == nullptr) || text->GetIsTextCompleted();
		UpdateAngelAnimation(textFinished);

		if (textFinished)
		{
			mPostTextTimer++;
			// 少し待ってから SPACE / ENTER でスキップ可能にする。
			const bool skipKey = (mPostTextTimer >= kPostTextSkipFrames) && IsConfirmDown();
			if (mPostTextTimer >= kPostTextWaitFrames || skipKey)
			{
				mPhase = Phase::PostFlash;
				mPostFlashTimer = 1;
			}
		}
		return;
	}

	// 白フラッシュを使って死亡演出から通常画面へ戻す。
	case Phase::PostFlash:
		mPostFlashTimer++;
		if (mPostFlashTimer >= kPostFlashFrames)
		{
			mPhase = Phase::Finished;
		}
		return;
	}
}

// 天使のモーションに応じて、まばたき・口パク・表情切り替えを更新する。
void DeathController::UpdateAngelAnimation(bool textFinished)
{
	if (mMotion == AngelMotion::Normal)
	{
		// 通常モーションでは一定間隔で目を閉じ、一定時間後に開く。
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

		// セリフ表示中だけ口の画像を交互に切り替えて口パクを表現する。
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
		// 爆笑・指さし・隠れは、セリフ表示中に2枚の画像を交互に切り替える。
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
// 描画：現在の演出フェーズに対応する画面・画像を描く。
// ---------------------------------------------------------------------------

// フェーズごとに描画内容を切り替える。
void DeathController::Draw(TutorialTextManager* text) const
{
	const bool textFinished = (text == nullptr) || text->GetIsTextCompleted();

	switch (mPhase)
	{
		// 画面をフラッシュさせた後、死亡理由のテキストと天使のカットインへ移る。
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

		// 天使と死亡理由のテキストを表示する。テキスト終了後は待機・スキップ判定を行う。
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

		// 白フラッシュを使って死亡演出から通常画面へ戻す。
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

// 黒背景の上に天使とセリフ枠を重ねて描画する。
void DeathController::DrawCutInScene(bool textFinished) const
{
	// 黒背景
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(0, 0, 0), TRUE);

	DrawAngel(textFinished);
	DrawTextBox(textFinished);
}

// 選択中のモーション画像を決定し、登場時のズーム演出を付けて描画する。
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

	// 登場直後の10フレームで拡大状態から通常サイズへ滑らかに近づける。
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

// セリフ枠を描画する。セリフ本文そのものは TutorialTextManager 側が描画する。
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

	// セリフ終了後は、キー入力を促す案内文字を点滅させる。
	if (textFinished)
	{
		const int pulse = 160 + static_cast<int>(90.0f * sinf(mPostTextTimer * 0.18f));
		Master::mpGameManager->GetFontManager()->DrawDotString(
			boxRight - 270, boxBottom - 46, 28, GetColor(pulse, pulse, 120), "SPACE / ENTER >>");
	}
}
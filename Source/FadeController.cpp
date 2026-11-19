#include "FadeController.h"
#include "DxLib.h"
#include "GameConstants.h"

namespace
{
	// フェードの最大府透明度
	const int kMaxAlpha = 255;
}

FadeController::FadeController()
	:mState(FadeState::Idle)
	, mAlpha(0)
	, mnTotalFrames(0)
	, mnFrameCount(0)
{

}

FadeController::~FadeController()
{
}

// 画面を暗くする
void FadeController::StartFadeOut(int fadeFrames)
{
	mState = FadeState::FadeOut;
	mnTotalFrames = (fadeFrames > 0) ? fadeFrames : 0;
	mnFrameCount = 0;
	mAlpha = 0;

	if (mnTotalFrames == 0)
	{
		mState = FadeState::Finished;
		mAlpha = kMaxAlpha;
	}
}

void FadeController::StartFadeIn(int fadeFrames)
{
	mState = FadeState::FadeIn;
	mnTotalFrames = (fadeFrames > 0) ? fadeFrames : 0;
	mnFrameCount = 0;
	mAlpha = kMaxAlpha;

	if (mnTotalFrames == 0)
	{
		mState = FadeState::Finished;
		mAlpha = 0;
	}
}

void FadeController::Update()
{
	if (mState != FadeState::FadeOut && mState != FadeState::FadeIn)
	{
		return;
	}

	mnFrameCount += 1;

	// 進行度
	float prograss = static_cast<float>(mnFrameCount) / static_cast<float>(mnTotalFrames);
	if (prograss >= 1.0f)
	{
		prograss = 1.0f;
	}

	if (mState == FadeState::FadeOut)
	{
		mAlpha = static_cast<int>(kMaxAlpha * prograss);
	}
	else
	{
		mAlpha = static_cast<int>(kMaxAlpha * (1.0f - prograss));
	}

	// 指定フレームに到着したら完了
	if (mnFrameCount >= mnTotalFrames)
	{
		mState = FadeState::Finished;
	}
}

void FadeController::Draw() const
{
	if (mState == FadeState::Idle || mAlpha <= 0)
	{
		return;
	}

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, mAlpha);
	DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

bool FadeController::IsFadeOutFinished() const
{
	return mState == FadeState::Finished && mAlpha == kMaxAlpha;
}

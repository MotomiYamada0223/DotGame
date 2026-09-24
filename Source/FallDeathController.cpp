#include "FallDeathController.h"
#include "GameConstants.h"
#include "Master.h"
#include "SceneManager.h"
#include "Scene.h"
#include "InputManager.h"


void FallDeathController::Update(VECTOR playerPos)
{
	// プレイヤーが地点を超えたら
	if (!mActive && playerPos.y >= PlayerConstants::PlayerDeathHeight)
	{
		mActive = true;
		mIsPressEnter = false;
		mReviveTimer = TextTimer::MaxFallDeathTimer;
	}

	if (mActive)
	{
		// Enterが押されたら表示
		if (InputManager::CheckDownKey(KEY_INPUT_RETURN) || mIsPressEnter)
		{
			mIsPressEnter = true;
		}
	}

	// 復活待機中はタイマーを減らす
	if (mIsPressEnter)
	{
		mReviveTimer -= 1.0f;
		if (mReviveTimer <= 0.0f)
		{
			mReviveTimer = 0.0f;
		}
	}
}

void FallDeathController::Draw(int live)
{
	if (!mActive) { return; }

	// 画面を暗くする
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);

	DrawBox(
		0,
		0,
		ScreenSize::ScrrenWidth,
		ScreenSize::ScrrenHeight,
		ColorOption::Black,
		TRUE
	);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// GAME OVER表示
	Master::mpGameManager->GetFontManager()->DrawDotString(
		TextPosition::FallDeathX,
		TextPosition::FallDeathY,
		200,
		ColorOption::White,
		"GAME OVER Enterで復活"
	);

	// Enterが押された後
	if (mIsPressEnter)
	{
		Master::mpGameManager->GetFontManager()->DrawDotString(
			TextPosition::LivesX,
			TextPosition::LivesY,
			100,
			ColorOption::White,
			"残機 × %d",
			live
		);

		Master::mpGameManager->GetFontManager()->DrawDotString(
			TextPosition::LivesX,
			TextPosition::LivesY + 100,
			60,
			ColorOption::White,
			"%.2f",
			mReviveTimer / 60.0f
		);
	}
}


// フラグ関係のリセット関数
void FallDeathController::Reset()
{
	mActive = false;
	mIsPressEnter = false;
	mReviveTimer = TextTimer::MaxFallDeathTimer;
}



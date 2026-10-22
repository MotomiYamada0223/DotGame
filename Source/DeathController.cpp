#include "DeathController.h"
#include "GameConstants.h"
#include "Master.h"
#include "SceneManager.h"
#include "Scene.h"
#include "InputManager.h"


DeathController::DeathController()
	: mActive(false)
	, mIsPressKey(false)
{

}

DeathController::~DeathController()
{

}

void DeathController::Update(
	VECTOR playerPos,
	int playerLive,
	int playerHp)
{
	if (!mActive &&
		(playerPos.y >= PlayerConstants::PlayerDeathHeight ||
		 playerHp <= 0))
	{	
		mActive = true;
		mIsPressKey = false;
	}

	if (!mActive || mIsPressKey)
	{
		return;
	}

	// Retryか選択画面か
	if (InputManager::CheckDownKey(KEY_INPUT_R))
	{
		mIsPressKey = true;
		Master::mpGameManager->GetSceneManager()->SetNextScene(
			SceneManager::SCENE_LOAD
		);
	}
	else if (InputManager::CheckDownKey(KEY_INPUT_Q))
	{
		mIsPressKey = true;
		Master::mpGameManager->GetSceneManager()->SetNextScene(
			SceneManager::SCENE_STAGESELECT
		);
	}
}

void DeathController::Draw(int live) const
{
	if (!mActive)
	{
		return;
	}

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 220);

	DrawBox(
		0,
		0,
		ScreenSize::ScrrenWidth,
		ScreenSize::ScrrenHeight,
		ColorOption::Black,
		TRUE
	);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	int textSize = 80;
	Master::mpGameManager->GetFontManager()->DrawDotString(
		TextPosition::FallDeathX,
		TextPosition::FallDeathY,
		textSize,
		ColorOption::White,
		"Rキー:   Retry(復活)"
	);
	Master::mpGameManager->GetFontManager()->DrawDotString(
		TextPosition::FallDeathX,
		TextPosition::FallDeathY + 110,
		textSize,
		ColorOption::White,
		"Qキー:   Quit(ステージ選択)"
	);
}


// フラグ関係のリセット関数
void DeathController::Reset()
{
	mActive = false;
	mIsPressKey = false;
}



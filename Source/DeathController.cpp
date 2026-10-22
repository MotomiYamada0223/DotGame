#include "DeathController.h"
#include "GameConstants.h"
#include "Master.h"
#include "SceneManager.h"
#include "Scene.h"
#include "InputManager.h"


DeathController::DeathController()
	: mActive(false)
	, mIsPressEnter(false)
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
		 playerHp <= 0) &&
		playerLive > 1)
	{	
		mActive = true;
		mIsPressEnter = false;
	}

	if (!mActive || mIsPressEnter)
	{
		return;
	}

	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		mIsPressEnter = true;
		Master::mpGameManager->GetSceneManager()->SetNextScene(
			SceneManager::SCENE_LOAD
		);
	}
}

void DeathController::Draw(int live) const
{
	if (!mActive)
	{
		return;
	}

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

	Master::mpGameManager->GetFontManager()->DrawDotString(
		TextPosition::FallDeathX,
		TextPosition::FallDeathY,
		200,
		ColorOption::White,
		"GAME OVER Enterで復活"
	);
}


// フラグ関係のリセット関数
void DeathController::Reset()
{
	mActive = false;
	mIsPressEnter = false;
}



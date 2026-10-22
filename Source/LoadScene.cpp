#include "LoadScene.h"
#include <DxLib.h>
#include "Master.h"
#include "GameConstants.h"

LoadScene::LoadScene()
	:Scene()
	, mbFlag(false)
	, mfTimer(MAX_TIMER)
	, mPrevTime(0)
	, mnHandle(-1)
{
	mnHandle = LoadGraph("Resource/Icon/playerIcon.png");
}

LoadScene::~LoadScene()
{
	DeleteGraph(mnHandle);
}

void LoadScene::Initialize()
{
	mPrevTime = GetNowCount();
	mfTimer = MAX_TIMER;
}

void LoadScene::Draw()
{
	DrawRotaGraph(
		ScreenSize::CenterX - 250,
		ScreenSize::CenterY,
		1.0f,
		0.0f,
		mnHandle, TRUE);

	Master::mpGameManager->GetFontManager()->DrawDotString(
		ScreenSize::CenterX,
		ScreenSize::CenterY,
		120,
		ColorOption::White,
		"écã@ Å~ %d",
		Master::gPlayerLive
	);

	Scene::Draw();
}

void LoadScene::Update()
{
	int currentCount = GetNowCount();
	float deltaTime = static_cast<float>(currentCount - mPrevTime) / 1000.0f;

	mPrevTime = currentCount;
	if (!mbFlag)
	{
		mfTimer -= deltaTime;
		if (mfTimer <= 0.0f)
		{
			mbFlag = true;
		}
	}

	if (mbFlag)
	{
		// ÉQÅ[ÉÄâÊñ Ç…Ç¢Ç≠ÇÊÇ§Ç…ÇµÇΩ
		Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_GAME);
	}

	Scene::Update();
}

void LoadScene::Finalize()
{

}
#include "LoseResultScene.h"
#include "InputManager.h"
#include "Master.h"
#include "GameConstants.h"

LoseResultScene::LoseResultScene()
	:Scene()
{

}

LoseResultScene::~LoseResultScene()
{

}

void LoseResultScene::Initialize()
{

}

void LoseResultScene::Update()
{
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_TITLE);
	}

	Scene::Update();
}

void LoseResultScene::Draw()
{
	Master::mpGameManager->GetFontManager()->DrawDotString(
		ScreenSize::CenterX,
		ScreenSize::CenterY,
		100,
		ColorOption::White,
		"”s–k‰æ–Ê\nEnter‚Åƒ^ƒCƒgƒ‹"
	);

	Scene::Draw();
}

void LoseResultScene::Finalize()
{

}
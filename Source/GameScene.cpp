#include "GameScene.h"
#include "Utility.h"
#include "Master.h"
#include "InputManager.h"

GameScene::GameScene()
	:Scene()
{

}

GameScene::~GameScene()
{

}

void GameScene::Initialize()
{

}

void GameScene::Update()
{
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TITLE);
	}

	Scene::Update();
}

void GameScene::Draw()
{
	DrawFormatStringToHandle(500, 500, GetColor(255, 255, 255),
		Master::mpFontManager->GetDotFont(), "Enter‚ÅƒŠƒUƒ‹ƒh‰æ–Ê");
}

void GameScene::Finalize()
{

}
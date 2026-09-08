#include "WinResultScene.h"
#include "InputManager.h"
#include "Master.h"

WinResultScene::WinResultScene()
	:Scene()
{

}

WinResultScene::~WinResultScene()
{

}

void WinResultScene::Initialize()
{

}

void WinResultScene::Update()
{
	// Enterでタイトル画面へ
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TITLE);
	}

	Scene::Update();
}

void WinResultScene::Draw()
{
	DrawFormatStringToHandle(500, 500, GetColor(255, 255, 255),
		Master::mpFontManager->GetDotFont(), "勝利画面\n\nEnterでタイトル");

	Scene::Draw();
}

void WinResultScene::Finalize()
{

}
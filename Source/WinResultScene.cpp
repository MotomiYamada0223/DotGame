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
		Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_TITLE);
	}

	Scene::Update();
}

void WinResultScene::Draw()
{
		Master::mpGameManager->GetFontManager()->DrawDotString(
			500,
			500,
			100,
			GetColor(255, 255, 255),
			"勝利画面\n\nEnterでタイトル"
		);

	Scene::Draw();
}

void WinResultScene::Finalize()
{

}
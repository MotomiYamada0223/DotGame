#include "GameScene.h"
#include "Utility.h"
#include "Master.h"
#include "InputManager.h"

/// <summary>
/// ゲーム中の演出をするクラス
/// </summary>
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
	// Enterで勝利画面
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_WINRESULT);
	}

	// 基底クラスのUpdateの呼び出し
	Scene::Update();
}

void GameScene::Draw()
{
	DrawFormatStringToHandle(500, 500, GetColor(255, 255, 255),
		Master::mpFontManager->GetDotFont(), "ゲーム画面\n\nEnterでリザルド画面");

	// 基底クラスのDrawの呼び出し
	Scene::Draw();
}

void GameScene::Finalize()
{

}
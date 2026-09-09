#include "GameScene.h"
#include "Utility.h"
#include "Master.h"
#include "InputManager.h"
#include "Player.h" 

GameScene::GameScene()
	:Scene()
{

}

GameScene::~GameScene()
{

}

void GameScene::Initialize()
{
	// プレイヤーの生成
	new Player(VGet(Utility::SCREEN_WIDTH / 2.0f, Utility::SCREEN_HEIGHT / 2.0f, 0.0f));
}

void GameScene::Update()
{
	// Enterでリザルト画面へ
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_WINRESULT);
	}

	// 基底クラスの更新処理を呼びだす
	Scene::Update();
}

void GameScene::Draw()
{
	DrawFormatStringToHandle(500, 500, GetColor(255, 255, 255),
		Master::mpFontManager->GetDotFont(), "ゲームシーン\n\n Enterでリザルトシーン");

	// 基底クラスの更新処理を呼びだす
	Scene::Draw();
}

void GameScene::Finalize()
{

}
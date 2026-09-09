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

	// チュートリアルテキストの初期化
	// CSVのファイルを作成する
	mTutorialText.Initialize("Resource/TutorialText/tutorialTextData.csv");
}

void GameScene::Update()
{
	// チュートリアルテキストの更新
	mTutorialText.Update(1.0f / 60.0f);
	

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
	// チュートリアル文字の描画
	mTutorialText.Draw();

	DrawFormatStringToHandle(500, 500, GetColor(255, 255, 255),
		Master::mpFontManager->GetDotFont(), "ゲームシーン\n\n Enterでリザルトシーン");

	// 基底クラスの更新処理を呼びだす
	Scene::Draw();
}

void GameScene::Finalize()
{

}
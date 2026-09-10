#include "GameScene.h"
#include "Utility.h"
#include "Master.h"
#include "InputManager.h"
#include "Player.h" 
#include "Enemy.h" 

GameScene::GameScene()
	:Scene()
{
	spawnTimer = 0;
	mpPlayer = nullptr;
}

GameScene::~GameScene()
{

}

void GameScene::Initialize()
{
	// プレイヤーの生成
	
	//mpPlayer = new Player(VGet(Utility::SCREEN_WIDTH / 2.0f, Utility::SCREEN_HEIGHT / 2.0, 0.0f));
	mpPlayer = new Player(VGet(Utility::SCREEN_WIDTH / 2.0f, 850, 0.0f));

	// 敵の生成

	if (mpPlayer != nullptr)
	{
		// 画面左側 (X=0 付近)、Yはプレイヤーと同じ高さで生成

		new Enemy(VGet(-50.0f, mpPlayer->GetPosition().y, 0.0f));
	}

	// チュートリアルテキストの初期化
	
	// CSVのファイル読み込み
	// チュートリアルテキストとブロックマップのタイル
	mTutorialText.Initialize("Resource/TutorialText/tutorialTextData.csv");
	mBlockMap.Load("Resource/Map/MapFile.csv", "Resource/Map/image_Map.png");

	spawnTimer = 0;
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

	mpPlayer->PlayerMove(mBlockMap);

	// クラスのUpdate呼び出し
	Scene::Update();
}

void GameScene::Draw()
{
	// チュートリアルの描画 
	
	mTutorialText.Draw();

	DrawFormatStringToHandle(500, 500, GetColor(255, 255, 255),
		Master::mpFontManager->GetDotFont(), "ゲームシーン\n\n Enterでリザルトへ");

	mBlockMap.Draw();

	// クラスのDraw呼び出し
	Scene::Draw();
}

void GameScene::Finalize()
{

}
#include "GameScene.h"
#include "Utility.h"
#include "Master.h"
#include "InputManager.h"
#include "Player.h" 
#include "Enemy.h" 
#include "Saint.h"
#include "GameConstants.h"

GameScene::GameScene()
	:Scene()
{
	spawnTimer = 0;
	mpPlayer = nullptr;
	mbIsLoaded = false;
}

GameScene::~GameScene()
{

}

void GameScene::Initialize()
{
	// CSVのファイル読み込み
	// チュートリアルテキストとブロックマップのタイル
	mTutorialText.Initialize(CsvPath::TutorialText);
	mBlockMap.Load(CsvPath::BlockMapFile, BlockMapGraphPath::BlockMap);

	// プレイヤーの生成
	mpPlayer = new Player(VGet(ScreenSize::CenterX - 600, 800, 0.0f));

	// Saint（しゃべるキャラクター）を画面上部に配置
	new Saint(VGet(Utility::SCREEN_WIDTH / 2.0f, 150.0f, 0.0f));

	// 敵の生成

	if (mpPlayer != nullptr)
	{
		// 画面左側 (X=0 付近)、Yはプレイヤーと同じ高さで生成

		new Enemy(VGet(-50.0f, mpPlayer->GetPosition().y, 0.0f));
	}

	spawnTimer = 0;

	mbIsLoaded = true;
}

void GameScene::Update()
{
	// チュートリアルテキストの更新
	mTutorialText.Update(1.0f / 60.0f);
	
	// Enterでリザルト画面へ
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_WINRESULT);
	}	

	mpPlayer->PlayerMove(mBlockMap);

	// クラスのUpdate呼び出し
	Scene::Update();
}

void GameScene::Draw()
{
	if (!mbIsLoaded) { return; }

	DrawFormatStringToHandle(500, 500, GetColor(255, 255, 255),
		Master::mpGameManager->GetFontManager()->GetDotFont(), "ゲームシーン\n\n Enterでリザルトへ");

	// クラスのDraw呼び出し
	Scene::Draw();

	mpPlayer->DebugDraw(); // デバッグ表示するため
	mBlockMap.Draw(); // ブロックマップの描画
	mTutorialText.Draw(); // チュートリアルの描画
}

void GameScene::Finalize()
{

}
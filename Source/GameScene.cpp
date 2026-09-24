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

	// 背景画像と当たり判定画像を読み込む
	mBlockMap.Load(
		BlockMapGraphPath::Background,
		BlockMapGraphPath::Collision
	);


	// プレイヤーの生成
	mpPlayer = new Player(VGet(ScreenSize::CenterX - 600, 600, 0.0f));

	// Saint（しゃべるキャラクター）を画面上部に配置
	new Saint(VGet(Utility::SCREEN_WIDTH / 2.0f, 150.0f, 0.0f));

	// 敵の生成
	if (mpPlayer != nullptr)
	{
		// 画面左側 (X=0 付近)、Yはプレイヤーと同じ高さで生成
		new Enemy(VGet(1000.0f, mpPlayer->GetPosition().y - 100, 0.0f));
	}

	spawnTimer = 0;

	mbIsLoaded = true;
}

void GameScene::Update()
{
	// チュートリアルテキストの更新
	mTutorialText.Update(1.0f / 60.0f);
	
	mpPlayer->PlayerMove(mBlockMap);



	// シーン上に存在するすべての敵をオブジェクトマネージャー経由で一括取得
	/// 個別のコードを追加することなく共通の移動処理を実行するため
	ObjectManager* objManager = GetObjectManager();
	if (objManager != nullptr)
	{
		std::vector<Object2D*> enemyList = objManager->GetObject2DListByTag(Object2D::Enemy2D);
		for (Object2D* obj : enemyList)
		{
			Enemy* enemy = dynamic_cast<Enemy*>(obj);
			if (enemy != nullptr)
			{
				enemy->EnemyMove(mBlockMap); // 敵に追加した移動関数を呼び出す
			}
		}
	}


	// クラスのUpdate呼び出し
	Scene::Update();
}

void GameScene::Draw()
{
	if (!mbIsLoaded) { return; }

	// クラスのDraw呼び出し
	Scene::Draw();

	// デバッグ系
	mpPlayer->DebugDraw(); // ブロックデバッグ
	mBlockMap.DebugDraw(); // ブロックマップデバッグ表示
	mTutorialText.DebugDraw(); // テキスト

	// オブジェクトの描画
	mBlockMap.Draw(); // ブロックマップの描画
	mTutorialText.Draw(); // チュートリアルの描画

	mpPlayer->DrawFallDeath(); // 死亡テキストの表示
}

void GameScene::Finalize()
{

}
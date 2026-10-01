#include "TestScene.h"
#include "NeedleTrap.h"
#include "Utility.h"
#include "Master.h"
#include "InputManager.h"
#include "Player.h" 
#include "Enemy.h"
#include "EnemySlime.h" 
#include "Saint.h"
#include "GameConstants.h"

TestScene::TestScene()
	:Scene()
{
	spawnTimer = 0;
	mpPlayer = nullptr;
	mbIsLoaded = false;
}

TestScene::~TestScene()
{

}

void TestScene::Initialize()
{
	// CSVのファイル読み込み
	// チュートリアルテキストとブロックマップのタイル
	mTutorialText.Initialize(CsvPath::TutorialText);

	// 背景画像と当たり判定画像を読み込む
		mBlockMap.Load(
		BlockMapGraphPath::TestBackground,
		BlockMapGraphPath::TestCollision
	);


	// プレイヤーの生成
	mpPlayer = new Player(VGet(ScreenSize::CenterX - 6000, 00, 0.0f));

	// Saint（しゃべるキャラクター）を画面上部に配置
	new Saint(VGet(Utility::SCREEN_WIDTH / 2.0f, 150.0f, 0.0f));

	// 敵の生成
	if (mpPlayer != nullptr)
	{
		// 画面左側 (X=0 付近)、Yはプレイヤーと同じ高さで生成
				EnemySlime* slime = new EnemySlime(VGet(1000.0f, 300, 0.0f));
		slime->UpdateStatusByProgress(mProgress);

		// トラップのテスト配置 (スライムと同じY座標 300 付近に配置)
		new NeedleTrap(VGet(500.0f, 450.0f, 0.0f), TrapType::PopUp);
		new NeedleTrap(VGet(1500.0f, 400.0f, 0.0f), TrapType::AntiJump);
	}

	spawnTimer = 0;

	mbIsLoaded = true;
}

void TestScene::Update()
{
	// Pキーで進行度切り替え (デバッグ用)
	static bool pKeyWasDown = false;
	bool pKeyIsDown = (CheckHitKey(KEY_INPUT_P) == 1);
	if (pKeyIsDown && !pKeyWasDown)
	{
		if (mProgress == GameProgress::Tutorial1)
		{
			mProgress = GameProgress::Tutorial2; // すり抜けON (無敵)
		}
		else
		{
			mProgress = GameProgress::Tutorial1; // すり抜けOFF (即死)
		}

		if (mpPlayer) mpPlayer->UpdateStatusByProgress(mProgress);

		ObjectManager* objManager = Master::mpGameManager->GetSceneManager()->GetCurrentScene()->GetObjectManager();
		if (objManager)
		{
			std::vector<Object2D*> enemyList = objManager->GetObject2DListByTag(Object2D::Enemy2D);
			for (Object2D* obj : enemyList)
			{
				UnitStatus* enemyStatus = dynamic_cast<UnitStatus*>(obj);
				if (enemyStatus)
				{
					enemyStatus->UpdateStatusByProgress(mProgress);
				}
			}
		}
	}
	pKeyWasDown = pKeyIsDown;
	//残機がないかの判定がtrueなら
	if (mpPlayer->IsOutOfLive())
	{
		Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_LOSERESULT);
	}

	// チュートリアルテキストの更新
	if (!mpPlayer->GetIsDead()) mTutorialText.Update(1.0f / 60.0f);
	
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

void TestScene::Draw()
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

	mpPlayer->DrawFallDeath(); // 死亡時テキスト

	// デバッグ表示: 現在の進行度
	const char* progStr = (mProgress == GameProgress::Tutorial1) ? "Tutorial1 (Death)" : "Tutorial2 (Immune)";
	DrawFormatString(10, 30, GetColor(0, 255, 0), "Progress: %s  [Press P to toggle]", progStr);
}

void TestScene::Finalize()
{

}
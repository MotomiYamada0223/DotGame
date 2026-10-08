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
#include "HiddenBlock.h"
#include "MovingFloor.h"
#include "FloorSpawner.h"
#include "ObjectManager.h"

TestScene::TestScene()
	:Scene()
	, mProgress()
	, mSaint(VGet(Utility::SCREEN_WIDTH / 2.0f, 150.0f, 0.0f)) // 天使の作成

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
	mTutorialText.Initialize(CsvPath::TutorialText, CsvPath::TutorialDeathText);

	// 背景画像と当たり判定画像を読み込む
	if (!mBlockMap.Load(
	BlockMapGraphPath::TestBackground,
	BlockMapGraphPath::TestCollision
	))
	{
		return;
	}
	mBackground.Load(ScrollGraphPath::Stage1);

	// プレイヤーの生成
	mpPlayer = new Player(VGet(ScreenSize::CenterX - 6000, 00, 0.0f));

	// 隠しブロックの生成
	new HiddenBlock(VGet(3100, 100.0f, 0.0f), &mBlockMap);

	
	// 上から下へ無限に降ってくる足場のスポナーを配置
	new FloorSpawner(
		VGet(6300.0f, 1180.0f, 0.0f),
		VGet(0.0f, -3.0f, 0.0f),
		120,
		mpPlayer,
		&mBlockMap,
		FloorFeature::Normal,
		CharacterGraphPath::MoveFloorBig // ここで画像パスを指定
	);

	new FloorSpawner(
		VGet(6600.0f, -100.0f, 0.0f),
		VGet(0.0f, 3.0f, 0.0f),
		120,
		mpPlayer,
		&mBlockMap,
		FloorFeature::SpeedUpOnRide,
		CharacterGraphPath::MoveFloorBig
		);

	new FloorSpawner(
		VGet(6850.0f, 1180.0f, 0.0f),
		VGet(0.0f, -3.0f, 0.0f),
		120,
		mpPlayer,
		&mBlockMap,
		FloorFeature::Normal,
		CharacterGraphPath::MoveFloorBig 
	);


	// 例1: 左右に往復する基本の動く床
	MovingFloor* floor1 = new MovingFloor(
		VGet(900.0f, 200.0f,0.0f), VGet(1800.0f, 200.0f, 0.0f),
		2.0f, MovePattern::Loop, FloorFeature::SpeedUpOnRide, mpPlayer, &mBlockMap
	);	

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

	SetTextUpdate(); // テキスト更新の呼び出し
	mSaint.Update(); // 天使の更新

	if (mpPlayer)
	{
		mBackground.Move(static_cast<int>(mpPlayer->GetCurrentSpeed()), mBlockMap.GetIsScrolling(), mBlockMap.GetScrollDirection());
	}


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

void TestScene::SetTextUpdate()
{
	// チュートリアルテキストの更新
	mpPlayer->PlayerMove(mBlockMap);
	// プレイヤーの死亡状態の確認
	bool isPlayerDead = mpPlayer->GetIsDead();

	// 死亡した瞬間
	if (isPlayerDead && !mbWasPlayerDead)
	{
		mTutorialText.OnPlayerDead();
	}
	// 復活した瞬間
	else if (!isPlayerDead && mbWasPlayerDead)
	{
		mTutorialText.OnPlayerRevive();
	}

	// 今フレームの死亡状態を保存
	mbWasPlayerDead = isPlayerDead;
	mTutorialText.Update(1.0f / 60.0f);
}


void TestScene::Draw()
{
	if (!mbIsLoaded) { return; }

	// オブジェクトの描画
	mBackground.Draw(); // スクロール背景

	Scene::Draw();

	mBlockMap.Draw(); // ブロックマップの描画

	// 一番手前に描画したいもの
	mpPlayer->DrawFallDeath(); // 死亡時テキスト
	mDrawFrame.Draw(); // フレーム描画
	mSaint.Draw(); // 天使の描画

	// デバッグ系
	mpPlayer->DebugDraw(); // ブロックデバッグ
	mBlockMap.DebugDraw(); // ブロックマップデバッグ表示
	mTutorialText.DebugDraw(); // テキスト

	// オブジェクトの描画 (マップを手前に描画)
	mTutorialText.Draw();

	// デバッグ表示：現在の進行度
	const char* progStr = (mProgress == GameProgress::Tutorial1) ? "Tutorial1 (Death)" : "Tutorial2 (Immune)";
	DrawFormatString(10, 30, GetColor(0, 255, 0), "Progress: %s  [Press P to toggle]", progStr);
}

void TestScene::Finalize()
{

}
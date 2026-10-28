#include "GameScene.h"
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
#include "FeintFloor.h"
#include "ObjectManager.h"

GameScene::GameScene()
	:Scene()
	, mSaint(VGet(Utility::SCREEN_WIDTH / 2.0f, 150.0f, 0.0f)) // 天使の作成
	, mbCircleFlag(false)
	, mLoadData(
		mpPlayer,
		mBlockMap,
		mBackground,
		mTutorialText,
		mProgress)
{
	mbIsLoaded = false;
}

GameScene::~GameScene()
{

}

void GameScene::Initialize()
{
	// 天使のセッターとテキストのゲッターの紐づけをする
	mSaint.SetIsTextCompletedChecker([this]()
	{
		return mTutorialText.GetIsTextCompleted();
	});

	// プレイヤーの生成
	mpPlayer = new Player(VGet(0, 400, 0.0f));
	mLoadData.LoadInitialize(Master::gCurrentStageIndex);
	mStageClear.Reset();

	MovingFloor::s_IsReverseAndDashTriggered = false;

	mbIsLoaded = true;
}

void GameScene::Update()
{
	// 円のフェード更新
	mCircleFade.UpdateCircleFade();

	mLeaf.LeafUpdate(mBlockMap, mpPlayer);

	// デバッグ
	if (InputManager::CheckDownKey(KEY_INPUT_9))
	{
		Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_TYPE::SCENE_STAGESELECT);
	}

	// 1キーでプレイヤーの座標を (7200, 700) にワープ (デバッグ用)
	static bool oneKeyWasDown = false;
	bool oneKeyIsDown = (CheckHitKey(KEY_INPUT_1) == 1 || CheckHitKey(KEY_INPUT_NUMPAD1) == 1);
	if (oneKeyIsDown && !oneKeyWasDown)
	{
		if (mpPlayer != nullptr)
		{
			mpPlayer->SetPosition(VGet(6000.0f, 1700.0f, 0.0f));
			mpPlayer->SetVelocityY(0.0f);
			mpPlayer->ResetBossIntroState();
			mBlockMap.SetCameraLocked(false);
			mBossIntro.Reset();
			//mStageClear.Reset();
		}
	}
	oneKeyWasDown = oneKeyIsDown;



	// ステージクリア演出中はカメラ固定を維持し、画面スクロールを抑止
	if (mStageClear.IsActive())
	{
		mBlockMap.SetCameraLocked(true);
	}

	// チュートリアルテキストの更新
	mpPlayer->PlayerMove(mBlockMap);
	SetTextUpdate(); // テキスト更新の呼び出し
	mBossIntro.Update(mpPlayer, mBlockMap);
	mStageClear.Update(mpPlayer, &mBlockMap); // ステージクリア演出の更新
	mSaint.Update(); // 天使の更新
	mLoadData.LoadUpdate();

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

void GameScene::SetTextUpdate()
{
	// プレイヤーの死亡状態の確認
	bool isPlayerDead = mpPlayer->GetIsDead();
	// 死亡した瞬間
	if (isPlayerDead && !mbWasPlayerDead)
	{
		mTutorialText.OnPlayerDead(mpPlayer->GetDeathReason());
	}
	// 復活した瞬間
	else if (!isPlayerDead && mbWasPlayerDead)
	{
		mTutorialText.OnPlayerRevive();
		mTutorialText.Reset();
	}

	// 今フレームの死亡状態を保存
	mbWasPlayerDead = isPlayerDead;
	mTutorialText.Update(1.0f / 60.0f, mpPlayer->GetPosition().x);
	mTutorialText.SetActionState(mpPlayer->GetActionState());
}


void GameScene::Draw()
{
	if (!mbIsLoaded) { return; }

	if (mbCircleFlag)
	{
		// オブジェクトの描画
		mBackground.Draw(mBlockMap.GetShakeOffsetX(), mBlockMap.GetShakeOffsetY()); // スクロール背景

		Scene::Draw();

		mBlockMap.Draw(); // ブロックマップの描画
		mLeaf.LeafDraw(mBlockMap); // 落下してくる葉の描画

		// 一番手前に描画したいもの
		mpPlayer->DrawFallDeath(); // 死亡時テキスト
		mDrawFrame.Draw(); // フレーム描画
		mSaint.Draw(); // 天使の描画


		mpPlayer->DrawLife();



		// ボス演出の黒帯
		mBossIntro.Draw();
		mStageClear.Draw();

		// デバッグ系
		mpPlayer->DebugDraw(); // ブロックデバッグ
		mBlockMap.DebugDraw(); // ブロックマップデバッグ表示
		mTutorialText.DebugDraw(); // テキスト

		// オブジェクトの描画 (マップを手前に描画)
		mTutorialText.Draw();
	}

	if (mCircleFade.GetStateFlag())
	{
		mbCircleFlag = true;
	}
	mCircleFade.DrawCircleFade(mpPlayer, mBlockMap);
	mLoadData.LoadDraw();
}

void GameScene::Finalize()
{

}
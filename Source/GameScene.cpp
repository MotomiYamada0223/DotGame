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
		mProgress,
		mRollingBoulder,
		mCart
		)
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

	// プレイヤーの生成（各ステージに応じた地面接地座標からスタート）
	VECTOR spawnPos = VGet(GameLoadDataConstants::ForestPlayerSpawnX, GameLoadDataConstants::ForestPlayerSpawnY, 0.0f);
	if (Master::gCurrentStageIndex == GameLoadDataConstants::StageIndexCave)
	{
		spawnPos = VGet(GameLoadDataConstants::CavePlayerSpawnX, GameLoadDataConstants::CavePlayerSpawnY, 0.0f);
	}
	mpPlayer = new Player(spawnPos);
	mLoadData.LoadInitialize(Master::gCurrentStageIndex);
	mHotSpring.Initialize();
	mStageClear.Reset();

	MovingFloor::s_IsReverseAndDashTriggered = false;

	mbIsLoaded = true;
}

void GameScene::Update()
{
	// 円のフェード更新
	mCircleFade.UpdateCircleFade();
	mLeaf.LeafUpdate(mBlockMap, mpPlayer);
	mRollingBoulder.Update(mBlockMap, mpPlayer);
	mCart.Update(mBlockMap, mpPlayer);
	mHotSpring.Update(mBlockMap, mpPlayer);


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
	mSaint.Update(); // 天使の更新（生存中 or フェーズ1の時のみ）
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

	// 復活した瞬間（フェーズ0 & 生存状態に戻った）
	if (!isPlayerDead && mbWasPlayerDead)
	{
		mTutorialText.OnPlayerRevive();
		mTutorialText.Reset();
	}

	// 今フレームの死亡状態を保存
	mbWasPlayerDead = isPlayerDead;

	// フェーズ1（天使テキスト表示中）のみテキスト更新
	// フェーズ2以降（GAME OVER）はテキストを止める
	bool isPhase1 = mpPlayer->IsCutInSaint(); // ← ここを新しい関数に置き換え！
	if (!isPlayerDead || isPhase1)
	{
		mTutorialText.Update(1.0f / 60.0f, mpPlayer->GetPosition().x);
	}
	mTutorialText.SetActionState(mpPlayer->GetActionState());
}

void GameScene::Draw()
{
	if (!mbIsLoaded) { return; }

	if (mbCircleFlag)
	{
		// オブジェクトの描画
		mBackground.Draw(mBlockMap.GetShakeOffsetX(), mBlockMap.GetShakeOffsetY()); // スクロール背景

		// ステージクリア扉（プレイヤーの背面、背景の手前に描画）
		mStageClear.DrawDoor(&mBlockMap);

		Scene::Draw();

		// ステージ（ブロックマップ）の背面に突き刺さるタイトルロゴを描画
		mStageClear.DrawBehindStage(&mBlockMap);

		DrawBossOrder(); // ボスとブロックマップの描画をしている

		// Sキー（表側落下時）：プレイヤーをマップの手前側に描画
		if (mpPlayer != nullptr)
		{
			mpPlayer->DrawFront();
		}
		mLeaf.LeafDraw(mBlockMap);
		mRollingBoulder.Draw(mBlockMap.GetScrollX(), mBlockMap.GetScrollY());
		mCart.Draw(mBlockMap.GetScrollX(), mBlockMap.GetScrollY());
		mHotSpring.Draw(mBlockMap.GetScrollX(), mBlockMap.GetScrollY());


		// 一番手前に描画したいもの
		mpPlayer->DrawFallDeath();
		// ボス演出の黒帯
		mBossIntro.Draw();
		mStageClear.Draw();

		//// デバッグ系
		//mpPlayer->DebugDraw(); // ブロックデバッグ
		//mBlockMap.DebugDraw(); // ブロックマップデバッグ表示
		//mTutorialText.DebugDraw(); // テキスト


		// 天使・ハート・テキストの描画
		bool isDeathPhase1 = mpPlayer->IsCutInSaint();
		bool showAngelUI = !mpPlayer->GetIsDead() || isDeathPhase1;
		if (showAngelUI)
		{
			if (mpPlayer->IsCutInSaint())
			{
				mTutorialText.Draw(TutorialTextManager::TEXT_DOWN); // 死亡時の天使場面用位置
			}
			else
			{
				mDrawFrame.Draw(); // フレーム描画
				mSaint.Draw();
				mpPlayer->DrawLife();
				mTutorialText.Draw(TutorialTextManager::TEXT_UP);   // 通常時の位置
			}
		}
	}

	if (mCircleFade.GetStateFlag())
	{
		mbCircleFlag = true;
	}
	mCircleFade.DrawCircleFade(mpPlayer, mBlockMap);
}

void GameScene::Finalize()
{

}

void GameScene::DrawBossOrder()
{
	if (Master::gCurrentStageIndex == 0)
	{
		mBossIntro.DrawBoss(); // ボス画像の描画
	}

	mBlockMap.Draw(); // ブロックマップの描画

	if (Master::gCurrentStageIndex == 1)
	{
		mBossIntro.DrawBoss(); // ボス画像の描画
	}
}
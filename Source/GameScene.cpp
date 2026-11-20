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
#include <algorithm>
#include "ObjectManager.h"


void GameScene::InitBgm()
{
	if (Master::mpGameManager == nullptr) return;
	SoundManager* sound = Master::mpGameManager->GetSoundManager();
	if (sound == nullptr) return;

	if (Master::gCurrentStageIndex == 1)
	{
		sound->PlayBGM(SoundManager::BGM_STAGE2);
	}
	else if (Master::gCurrentStageIndex == 2)
	{
		sound->PlayBGM(SoundManager::BGM_STAGE3);
	}
	else
	{
		sound->PlayBGM(SoundManager::BGM_STAGE1);
	}
}



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

void GameScene::PlayBossBgm()
{
	if (mbBossBgmStarted) return;
	if (Master::mpGameManager == nullptr) return;
	SoundManager* sound = Master::mpGameManager->GetSoundManager();
	if (sound == nullptr) return;

	if (Master::gCurrentStageIndex == 1)
	{
		sound->PlayBGM(SoundManager::BGM_STAGE2_BOSS, true);
	}
	else if (Master::gCurrentStageIndex == 2)
	{
		sound->PlayBGM(SoundManager::BGM_STAGE3_BOSS, true);
	}
	else
	{
		sound->PlayBGM(SoundManager::BGM_STAGE1_BOSS, true);
	}

	mbBossBgmStarted = true;
}

GameScene::~GameScene()
{

}

void GameScene::Initialize()
{
	InitBgm();

	// 天使のセッターとテキストのゲッターの紐づけをする
	mSaint.SetIsTextCompletedChecker([this]()
	{
		if ((mCavePhase == CavePhase::Opening || mCavePhase == CavePhase::TransitionToMain) &&
			(mpPlayer == nullptr || !mpPlayer->IsCutInSaint()))
		{
			return true; // 洞窟オープニング中は喋らず口を閉じる
		}
		return mTutorialText.GetIsTextCompleted();
	});

	const bool isCave = (Master::gCurrentStageIndex == GameLoadDataConstants::StageIndexCave);
	const bool runCaveOpening = (isCave && !Master::gCaveOpeningCompleted);

	// プレイヤーの生成（各ステージに応じた開始座標を設定）
	VECTOR spawnPos = VGet(GameLoadDataConstants::ForestPlayerSpawnX, GameLoadDataConstants::ForestPlayerSpawnY, 0.0f);
	if (runCaveOpening)
	{
		spawnPos = VGet(CaveOpeningConstants::PlayerSpawnX, CaveOpeningConstants::PlayerSpawnY, 0.0f);
	}
	else if (isCave)
	{
		spawnPos = VGet(GameLoadDataConstants::CavePlayerSpawnX, GameLoadDataConstants::CavePlayerSpawnY, 0.0f);
	}

	mpPlayer = new Player(spawnPos);
	if (runCaveOpening)
	{
		mCavePhase = CavePhase::Opening;
		mLoadData.StageCaveOpening();
	}
	else
	{
		mCavePhase = isCave ? CavePhase::MainCave : CavePhase::None;
		mLoadData.LoadInitialize(Master::gCurrentStageIndex);
	}

	mHotSpring.Initialize();
	mStageClear.Reset();

	MovingFloor::s_IsReverseAndDashTriggered = false;
	mbIsLoaded = true;
}

void GameScene::Update()
{
	// デバッグ機能：1キーでボスエリア直前の安全な足場にワープ
	if (InputManager::CheckDownKey(KEY_INPUT_1) || InputManager::CheckDownKey(KEY_INPUT_NUMPAD1))
	{
		DebugWarpToBossArea();
	}

	// 円のフェード・オープニング切替フェード更新
	mCircleFade.UpdateCircleFade();
	mOpeningFade.Update();

	if (mCavePhase == CavePhase::Opening || mCavePhase == CavePhase::TransitionToMain)
	{
		if (mpPlayer != nullptr)
		{
			mpPlayer->SetSuppressFallDeath(mCavePhase == CavePhase::TransitionToMain);
			mpPlayer->PlayerMove(mBlockMap);

			if (mCavePhase == CavePhase::Opening && !mpPlayer->GetIsDead())
			{
				if (mBlockMap.CheckAndTriggerOpeningPit(
					mpPlayer->GetPosition().x,
					mpPlayer->GetPosition().y,
					mpPlayer->GetPlayerWidth(),
					mpPlayer->GetPlayerHeight(),
					mpPlayer->GetIsGrounded(),
					mpPlayer->GetVelocityY()))
				{
					mpPlayer->ForceStartFall();
				}

				if (mBlockMap.IsOpeningPitBroken() &&
					mpPlayer->GetPosition().y >= CaveOpeningConstants::TransitionFadeStartY)
				{
					mCavePhase = CavePhase::TransitionToMain;
					mOpeningFade.StartFadeOut(CaveOpeningConstants::TransitionFadeOutFrames);
				}
			}
		}

		SetTextUpdate();
		if (mpPlayer != nullptr)
		{
			mBackground.Move(static_cast<int>(mpPlayer->GetCurrentSpeed()), mBlockMap.GetIsScrolling(), mBlockMap.GetScrollDirection());
		}
		mSaint.Update();
		Scene::Update();
		mBlockMap.UpdateOpeningPitBreak();

		if (mCavePhase == CavePhase::TransitionToMain && mOpeningFade.IsFadeOutFinished())
		{
			SwitchToMainCave(true);
		}
		return;
	}

	mLeaf.LeafUpdate(mBlockMap, mpPlayer);
	mRollingBoulder.Update(mBlockMap, mpPlayer);
	mCart.Update(mBlockMap, mpPlayer);
	mHotSpring.Update(mBlockMap, mpPlayer);


	// ステージクリア演出中はカメラ固定を維持し、画面スクロールを抑止
	if (mStageClear.IsActive())
	{
		mBlockMap.SetCameraLocked(true);
	}

	if (mBossIntro.IsActive())
	{
		PlayBossBgm();
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

	// 洞窟ボス登場時の天井破砕・落下破片エフェクトの更新
	mBlockMap.UpdateCeilingBreak();
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
	if ((mCavePhase == CavePhase::Opening || mCavePhase == CavePhase::TransitionToMain) && !isPhase1)
	{
		return; // 洞窟オープニング中の通常時はセリフを進めない（無言）
	}

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
		const bool isOpeningPhase = (mCavePhase == CavePhase::Opening || mCavePhase == CavePhase::TransitionToMain);

		// オブジェクトの描画
		mBackground.Draw(mBlockMap.GetShakeOffsetX(), mBlockMap.GetShakeOffsetY()); // スクロール背景

		if (!isOpeningPhase)
		{
			// ステージクリア扉（プレイヤーの背面、背景の手前に描画）
			mStageClear.DrawDoor(&mBlockMap);
		}

		Scene::Draw();

		if (!isOpeningPhase)
		{
			// ステージ（ブロックマップ）の背面に突き刺さるタイトルロゴを描画
			mStageClear.DrawBehindStage(&mBlockMap);
		}

		DrawBossOrder(); // ボスとブロックマップの描画をしている

		// Sキー（表側落下時）：プレイヤーをマップの手前側に描画
		if (mpPlayer != nullptr)
		{
			mpPlayer->DrawFront();
		}

		if (!isOpeningPhase)
		{
			mLeaf.LeafDraw(mBlockMap);
			mRollingBoulder.Draw(mBlockMap.GetScrollX(), mBlockMap.GetScrollY());
			mCart.Draw(mBlockMap.GetScrollX(), mBlockMap.GetScrollY());
			mHotSpring.Draw(mBlockMap.GetScrollX(), mBlockMap.GetScrollY());
		}

		// 一番手前に描画したいもの
		mpPlayer->DrawFallDeath();
		if (!isOpeningPhase)
		{
			// ボス演出の黒帯
			mBossIntro.Draw();
			mStageClear.Draw();
		}

		// デバッグ系
		mpPlayer->DebugDraw(); // ブロックデバッグ
		mBlockMap.DebugDraw(); // ブロックマップデバッグ表示
		mTutorialText.DebugDraw(); // テキスト

		// デバッグ用ワープキーのガイド表示
		DrawFormatString(DebugWarpConstants::GuideTextX, DebugWarpConstants::GuideTextY, GetColor(0, 255, 0), "[1]: Warp to Boss Area");


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
	mOpeningFade.Draw();
}

void GameScene::Finalize()
{

}

void GameScene::DrawBossOrder()
{
	if (mCavePhase == CavePhase::Opening || mCavePhase == CavePhase::TransitionToMain)
	{
		mBlockMap.Draw();
		return;
	}

	if (Master::gCurrentStageIndex == 0)
	{
		mBossIntro.DrawBoss(); // ボス画像の描画
	}

	mBlockMap.Draw(); // ブロックマップの描画

	if (Master::gCurrentStageIndex == 1)
	{
		mBossIntro.DrawBoss(); // ボス画像の描画
		mBlockMap.DrawCeilingForegroundDebris(); // 手前側の天井破砕岩塊・砂煙を描画
	}
}

void GameScene::SwitchToMainCave(bool dropFromAbove)
{
	float carryVy = CaveOpeningConstants::MainCaveDropInitialVy;
	if (mpPlayer != nullptr)
	{
		carryVy = (std::min)(
			CaveOpeningConstants::MainCaveDropMaxCarryVy,
			(std::max)(CaveOpeningConstants::MainCaveDropInitialVy, mpPlayer->GetVelocityY()));
	}

	Master::gCaveOpeningCompleted = true;
	mCavePhase = CavePhase::MainCave;

	mLoadData.StageCave();
	mHotSpring.Initialize();
	mStageClear.Reset();
	mbBossBgmStarted = false;
	mbWasPlayerDead = false;

	if (mpPlayer != nullptr)
	{
		mpPlayer->SetSpawnPos(VGet(GameLoadDataConstants::CavePlayerSpawnX, GameLoadDataConstants::CavePlayerSpawnY, 0.0f));
		if (dropFromAbove)
		{
			mpPlayer->StartAirDrop(
				GameLoadDataConstants::CavePlayerSpawnX,
				CaveOpeningConstants::MainCaveDropStartY,
				carryVy);
		}
		else
		{
			mpPlayer->SetSuppressFallDeath(false);
			mpPlayer->WarpTo(GameLoadDataConstants::CavePlayerSpawnX, GameLoadDataConstants::CavePlayerSpawnY);
		}
		mBlockMap.SnapCameraToPosition(GameLoadDataConstants::CavePlayerSpawnX, GameLoadDataConstants::CavePlayerSpawnY);
	}

	if (dropFromAbove)
	{
		mOpeningFade.StartFadeIn(CaveOpeningConstants::TransitionFadeInFrames);
	}
}

void GameScene::DebugWarpToBossArea()
{
	if (mpPlayer == nullptr) return;

	if (mCavePhase == CavePhase::Opening || mCavePhase == CavePhase::TransitionToMain)
	{
		SwitchToMainCave(false);
	}

	float warpX = DebugWarpConstants::ForestBossPreWarpX;
	float warpY = DebugWarpConstants::ForestBossPreWarpY;

	if (Master::gCurrentStageIndex == 1) // ステージ2: 洞窟
	{
		warpX = DebugWarpConstants::CaveBossPreWarpX;
		warpY = DebugWarpConstants::CaveBossPreWarpY;
	}

	// プレイヤーの座標をワープ先に設定し、各種状態をリセット
	mpPlayer->WarpTo(warpX, warpY);

	// カメラをプレイヤーがスクロール領域内（画面中央800px付近）に収まるようにスナップ
	mBlockMap.SnapCameraToPosition(warpX, warpY);
}

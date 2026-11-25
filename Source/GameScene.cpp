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
		if (IsSilentAngelPhase() && (mpPlayer == nullptr || !mpPlayer->IsCutInSaint()))
		{
			return true; // 無言フェーズ中は口パクを止める
		}
		return mTutorialText.GetIsTextCompleted();
	});

	const bool isCave = (Master::gCurrentStageIndex == GameLoadDataConstants::StageIndexCave);
	const int postBossPhase = isCave ? Master::gCavePostBossPhase : 0;
	const bool runCaveOpening = (isCave && postBossPhase == 0 && !Master::gCaveOpeningCompleted);

	// プレイヤーの生成（各ステージ・進行状態に応じた開始座標を設定）
	VECTOR spawnPos = VGet(GameLoadDataConstants::ForestPlayerSpawnX, GameLoadDataConstants::ForestPlayerSpawnY, 0.0f);
	if (isCave)
	{
		if (postBossPhase == 1)
		{
			spawnPos = VGet(CaveOpeningConstants::PostBossSpawnX, CaveOpeningConstants::PlayerSpawnY, 0.0f);
		}
		else if (runCaveOpening)
		{
			spawnPos = VGet(CaveOpeningConstants::PlayerSpawnX, CaveOpeningConstants::PlayerSpawnY, 0.0f);
		}
		else
		{
			spawnPos = VGet(GameLoadDataConstants::CavePlayerSpawnX, GameLoadDataConstants::CavePlayerSpawnY, 0.0f);
		}
	}

	mpPlayer = new Player(spawnPos);
	if (postBossPhase == 1)
	{
		SetupPostBossSurface(false);
	}
	else if (postBossPhase == 2)
	{
		SetupBossRoomClearDoor(false, 0.0f);
	}
	else if (runCaveOpening)
	{
		mCavePhase = CavePhase::Opening;
		mLoadData.StageCaveOpening();
		mHotSpring.Initialize();
		mStageClear.Reset();
	}
	else
	{
		mCavePhase = isCave ? CavePhase::MainCave : CavePhase::None;
		mLoadData.LoadInitialize(Master::gCurrentStageIndex);
		mHotSpring.Initialize();
		mStageClear.Reset();
	}

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

	if (IsOpeningMapPhase())
	{
		if (mpPlayer != nullptr)
		{
			const bool isTransitioning = (mCavePhase == CavePhase::TransitionToMain || mCavePhase == CavePhase::TransitionToBossRoom);
			mpPlayer->SetSuppressFallDeath(isTransitioning);
			mpPlayer->SetAllowExitRight(mCavePhase == CavePhase::PostBossSurface);
			mpPlayer->PlayerMove(mBlockMap);

			if (!mpPlayer->GetIsDead())
			{
				const VECTOR pPos = mpPlayer->GetPosition();
				if (mCavePhase == CavePhase::Opening)
				{
					if (mBlockMap.CheckAndTriggerOpeningPit(
						pPos.x,
						pPos.y,
						mpPlayer->GetPlayerWidth(),
						mpPlayer->GetPlayerHeight(),
						mpPlayer->GetIsGrounded(),
						mpPlayer->GetVelocityY()))
					{
						mpPlayer->ForceStartFall();
					}

					if (mBlockMap.IsOpeningPitBroken() &&
						pPos.y >= CaveOpeningConstants::TransitionFadeStartY)
					{
						mCavePhase = CavePhase::TransitionToMain;
						mOpeningFade.StartFadeOut(CaveOpeningConstants::TransitionFadeOutFrames);
					}
				}
				else if (mCavePhase == CavePhase::PostBossSurface)
				{
					// 右側の光へ歩み寄るまではマップ右端でカメラを止め、踏み込んでから先の虚空を映す
					if (pPos.x >= CaveOpeningConstants::PostBossScrollUnlockX)
					{
						mBlockMap.SetAllowScrollPastRight(true);
					}

					// 左側の割れた穴へ再度落ちた場合のみボス部屋へ戻す（右端の光へ進んだ場合はそのまま落下死亡）
					const float pitLeft = static_cast<float>(mBlockMap.GetOpeningPitLeft()) - 40.0f;
					const float pitRight = static_cast<float>(mBlockMap.GetOpeningPitRight()) + 40.0f;
					if (pPos.y >= CaveOpeningConstants::TransitionFadeStartY &&
						pPos.x >= pitLeft &&
						pPos.x <= pitRight)
					{
						mCavePhase = CavePhase::TransitionToBossRoom;
						mOpeningFade.StartFadeOut(CaveOpeningConstants::TransitionFadeOutFrames);
					}
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

		if (mOpeningFade.IsFadeOutFinished())
		{
			if (mCavePhase == CavePhase::TransitionToMain)
			{
				SwitchToMainCave(true);
			}
			else if (mCavePhase == CavePhase::TransitionToBossRoom)
			{
				float carryVy = CaveOpeningConstants::MainCaveDropInitialVy;
				if (mpPlayer != nullptr)
				{
					carryVy = (std::min)(
						CaveOpeningConstants::MainCaveDropMaxCarryVy,
						(std::max)(CaveOpeningConstants::MainCaveDropInitialVy, mpPlayer->GetVelocityY()));
				}
				SetupBossRoomClearDoor(true, carryVy);
			}
		}
		return;
	}

	const bool isBossReturnMode = (mCavePhase == CavePhase::MainCave && Master::gCavePostBossPhase == 2);
	mLeaf.LeafUpdate(mBlockMap, mpPlayer);
	if (!isBossReturnMode)
	{
		mRollingBoulder.Update(mBlockMap, mpPlayer);
		mCart.Update(mBlockMap, mpPlayer);
		mHotSpring.Update(mBlockMap, mpPlayer);
	}

	if (isBossReturnMode && mpPlayer != nullptr && !mpPlayer->GetIsDead() && !mStageClear.IsActive() && !mStageClear.IsFinished())
	{
		mStageClear.SpawnImmediateDoor(CaveOpeningConstants::BossReturnDoorX, CaveOpeningConstants::BossReturnDoorGroundY);
	}


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
	if (mStageClear.ConsumeCaveAscentComplete())
	{
		SetupPostBossSurface(true);
		return;
	}
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

	mbWasPlayerDead = isPlayerDead;

	// フェーズ1（天使テキスト表示中）のみテキスト更新

	bool isPhase1 = mpPlayer->IsCutInSaint();
	bool isOpeningPhase = IsOpeningMapPhase();

	// オープニング中は通常チュートリアルを更新する
	if (IsSilentAngelPhase() && !isPhase1 && !isOpeningPhase)
	{
		return;
	}

	if (!isPlayerDead || isPhase1 || isOpeningPhase)
	{
		mTutorialText.Update(
			1.0f / 60.0f,
			mpPlayer->GetPosition().x);
	}

	mTutorialText.SetActionState(mpPlayer->GetActionState());
}



//void GameScene::SetTextUpdate()
//{
//	// プレイヤーの死亡状態の確認
//	bool isPlayerDead = mpPlayer->GetIsDead();
//
//	// 復活した瞬間（フェーズ0 & 生存状態に戻った）
//	if (!isPlayerDead && mbWasPlayerDead)
//	{
//		mTutorialText.OnPlayerRevive();
//		mTutorialText.Reset();
//	}
//
//	// 今フレームの死亡状態を保存
//	mbWasPlayerDead = isPlayerDead;
//
//	// フェーズ1（天使テキスト表示中）のみテキスト更新
//	// フェーズ2以降（GAME OVER）はテキストを止める
//	bool isPhase1 = mpPlayer->IsCutInSaint(); // ← ここを新しい関数に置き換え！
//	if (IsSilentAngelPhase() && !isPhase1)
//	{
//		return; // 洞窟オープニング・帰還後の通常時はセリフを進めない（無言）
//	}
//
//	if (!isPlayerDead || isPhase1)
//	{
//		mTutorialText.Update(1.0f / 60.0f, mpPlayer->GetPosition().x);
//	}
//	mTutorialText.SetActionState(mpPlayer->GetActionState());
//}

void GameScene::Draw()
{
	if (!mbIsLoaded) { return; }

	if (mbCircleFlag)
	{
		const bool isOpeningPhase = IsOpeningMapPhase();
		const bool isBossReturnMode = (mCavePhase == CavePhase::MainCave && Master::gCavePostBossPhase == 2);

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

		if (!isOpeningPhase && !isBossReturnMode)
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

		//// デバッグ系
		//mpPlayer->DebugDraw(); // ブロックデバッグ
		//mBlockMap.DebugDraw(); // ブロックマップデバッグ表示
		//mTutorialText.DebugDraw(); // テキスト

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
	if (IsOpeningMapPhase())
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

bool GameScene::IsOpeningMapPhase() const
{
	return mCavePhase == CavePhase::Opening ||
		mCavePhase == CavePhase::TransitionToMain ||
		mCavePhase == CavePhase::PostBossSurface ||
		mCavePhase == CavePhase::TransitionToBossRoom;
}

bool GameScene::IsSilentAngelPhase() const
{
	// ボス撃破後の帰還モード（MainCave + postBossPhase==2）は無言
	return (mCavePhase == CavePhase::MainCave && Master::gCavePostBossPhase == 2);
}

void GameScene::CleanupNonPlayerObjects()
{
	mBossIntro.Reset();
	if (ObjectManager* objManager = GetObjectManager())
	{
		objManager->DeleteAll2DExcept(mpPlayer);
	}
	mRollingBoulder.Reset();
	mCart.Reset();
	mHotSpring.Initialize();
	mStageClear.Reset();
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
	Master::gCavePostBossPhase = 0;
	mCavePhase = CavePhase::MainCave;

	CleanupNonPlayerObjects();
	mLoadData.StageCave();
	mbBossBgmStarted = false;
	mbWasPlayerDead = false;

	if (mpPlayer != nullptr)
	{
		mpPlayer->SetAllowExitRight(false);
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

void GameScene::SetupPostBossSurface(bool withFadeIn)
{
	Master::gCaveOpeningCompleted = true;
	Master::gCavePostBossPhase = 1;
	mCavePhase = CavePhase::PostBossSurface;

	CleanupNonPlayerObjects();
	mLoadData.StageCaveOpening();
	// ボス撃破後の帰還シーンでは天使は無言なのでフェーズをNoneに上書き
	mTutorialText.SetCaveFallPhase(TutorialTextManager::CaveFallPhase::None);
	mBlockMap.SetOpeningPitAlreadyBroken();
	mBlockMap.SetAllowScrollPastRight(false);
	mbBossBgmStarted = false;
	mbWasPlayerDead = false;

	if (withFadeIn)
	{
		InitBgm();
	}

	if (mpPlayer != nullptr)
	{
		mpPlayer->SetSuppressFallDeath(false);
		mpPlayer->SetAllowExitRight(true);
		mpPlayer->SetSpawnPos(VGet(CaveOpeningConstants::PostBossSpawnX, CaveOpeningConstants::PlayerSpawnY, 0.0f));
		mpPlayer->WarpTo(CaveOpeningConstants::PostBossSpawnX, CaveOpeningConstants::PlayerSpawnY);
		mpPlayer->SetFacingRight(true);
		mBlockMap.SnapCameraToPosition(CaveOpeningConstants::PostBossSpawnX, CaveOpeningConstants::PlayerSpawnY);
	}

	if (withFadeIn)
	{
		mOpeningFade.StartFadeIn(CaveOpeningConstants::TransitionFadeInFrames);
	}
}

void GameScene::SetupBossRoomClearDoor(bool dropFromCeiling, float carryVy)
{
	Master::gCaveOpeningCompleted = true;
	Master::gCavePostBossPhase = 2;
	mCavePhase = CavePhase::MainCave;

	CleanupNonPlayerObjects();
	mLoadData.StageCaveBossClearReturn();
	mBossIntro.EndIntro();
	mbBossBgmStarted = false;
	mbWasPlayerDead = false;

	const float bossCenterX = mBlockMap.HasBossArea()
		? mBlockMap.GetBossAreaCenterX()
		: 8913.0f;
	mBlockMap.InitBrokenCeilingHoleImmediate(bossCenterX);
	mStageClear.SpawnImmediateDoor(CaveOpeningConstants::BossReturnDoorX, CaveOpeningConstants::BossReturnDoorGroundY);

	if (mpPlayer != nullptr)
	{
		const float floorPlayerY = CaveOpeningConstants::BossReturnDoorGroundY - mpPlayer->GetPlayerHeight() * 0.5f;
		mpPlayer->SetAllowExitRight(false);
		mpPlayer->SetSuppressFallDeath(false);
		mpPlayer->SetSpawnPos(VGet(bossCenterX, floorPlayerY, 0.0f));

		if (dropFromCeiling)
		{
			mpPlayer->StartAirDrop(
				bossCenterX,
				CaveOpeningConstants::BossReturnDropStartY,
				(std::max)(CaveOpeningConstants::MainCaveDropInitialVy, carryVy));
			mOpeningFade.StartFadeIn(CaveOpeningConstants::TransitionFadeInFrames);
		}
		else
		{
			mpPlayer->WarpTo(bossCenterX, floorPlayerY);
		}
		mpPlayer->SetFacingRight(true);
		mBlockMap.SnapCameraToPosition(bossCenterX, floorPlayerY);
	}
}

void GameScene::DebugWarpToBossArea()
{
	if (mpPlayer == nullptr) return;
	if (Master::gCurrentStageIndex == GameLoadDataConstants::StageIndexCave && Master::gCavePostBossPhase != 0)
	{
		return;
	}

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

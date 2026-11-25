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


// ステージに応じた通常BGMを再生する
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


// ゲームシーンのコンストラクタ（天使とロードデータの生成）
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

// ステージに応じたボス戦BGMを再生する（一度だけ再生）
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

// デストラクタ（現状は特に後始末なし）
GameScene::~GameScene()
{

}

// シーンの初期化処理（BGM・天使・プレイヤー・ステージデータの設定）
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

	// 洞窟ステージの進行状態を判定する
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

	// 足場の反転・ダッシュ状態を初期化する
	MovingFloor::s_IsReverseAndDashTriggered = false;
	// 読み込み完了フラグを立てる
	mbIsLoaded = true;
}

// ゲームシーンの毎フレーム更新処理
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

	// オープニングマップ中の処理（洞窟の穴落下・フェード切替）
	if (IsOpeningMapPhase())
	{
		if (mpPlayer != nullptr)
		{
			// 落下死亡の抑止や右端移動の許可をフェーズに応じて設定する
			const bool isTransitioning = (mCavePhase == CavePhase::TransitionToMain || mCavePhase == CavePhase::TransitionToBossRoom);
			mpPlayer->SetSuppressFallDeath(isTransitioning);
			mpPlayer->SetAllowExitRight(mCavePhase == CavePhase::PostBossSurface);
			mpPlayer->PlayerMove(mBlockMap);

			// 生存中のみ穴・遷移の判定を行う
			if (!mpPlayer->GetIsDead())
			{
				const VECTOR pPos = mpPlayer->GetPosition();
				if (mCavePhase == CavePhase::Opening)
				{
					// 穴の発動判定（踏んだら落下開始）
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

					// 穴が割れた後、一定の高さまで落下したら本編へ遷移する
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

					// 割れた穴の左右範囲を算出する
					const float pitLeft = static_cast<float>(mBlockMap.GetOpeningPitLeft()) - 40.0f;
					const float pitRight = static_cast<float>(mBlockMap.GetOpeningPitRight()) + 40.0f;

					// 左側の割れた穴へ再度落ちた場合のみボス部屋へ戻す（右端の光へ進んだ場合はそのまま落下死亡）
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

		// フェードアウト完了後に次のフェーズへ切り替える
		if (mOpeningFade.IsFadeOutFinished())
		{
			if (mCavePhase == CavePhase::TransitionToMain)
			{
				SwitchToMainCave(true);
			}
			else if (mCavePhase == CavePhase::TransitionToBossRoom)
			{
				// 落下時の引き継ぎ速度を計算する
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

	// ボス帰還モードかどうかを判定する
	const bool isBossReturnMode = (mCavePhase == CavePhase::MainCave && Master::gCavePostBossPhase == 2);

	// リーフ・転がる岩・カート・温泉の更新
	mLeaf.LeafUpdate(mBlockMap, mpPlayer);
	if (!isBossReturnMode)
	{
		mRollingBoulder.Update(mBlockMap, mpPlayer);
		mCart.Update(mBlockMap, mpPlayer);
		mHotSpring.Update(mBlockMap, mpPlayer);
	}

	// ボス帰還中は扉を即時出現させる
	if (isBossReturnMode && mpPlayer != nullptr && !mpPlayer->GetIsDead() && !mStageClear.IsActive() && !mStageClear.IsFinished())
	{
		mStageClear.SpawnImmediateDoor(CaveOpeningConstants::BossReturnDoorX, CaveOpeningConstants::BossReturnDoorGroundY);
	}


	// ステージクリア演出中はカメラ固定を維持し、画面スクロールを抑止
	if (mStageClear.IsActive())
	{
		mBlockMap.SetCameraLocked(true);
	}

	// ボス登場演出中はボスBGMを再生する
	if (mBossIntro.IsActive())
	{
		PlayBossBgm();
	}

	// チュートリアルテキストの更新
	mpPlayer->PlayerMove(mBlockMap);
	SetTextUpdate(); // テキスト更新の呼び出し
	mBossIntro.Update(mpPlayer, mBlockMap);
	mStageClear.Update(mpPlayer, &mBlockMap); // ステージクリア演出の更新

	// ステージクリア演出後に洞窟の上昇が完了したら、ボス撃破後の地上シーンへ移行する
	if (mStageClear.ConsumeCaveAscentComplete())
	{
		SetupPostBossSurface(true);
		return;
	}
	mSaint.Update(); // 天使の更新（生存中 or フェーズ1の時のみ）

	// 背景のスクロール
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


// チュートリアルテキストの表示・更新を行う
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

	// 次フレームの復活判定のため現在の死亡状態を保存する
	mbWasPlayerDead = isPlayerDead;

	// フェーズ1（天使テキスト表示中）のみテキスト更新

	bool isPhase1 = mpPlayer->IsCutInSaint();
	bool isOpeningPhase = IsOpeningMapPhase();

	// オープニング中は通常チュートリアルを更新する
	if (IsSilentAngelPhase() && !isPhase1 && !isOpeningPhase)
	{
		return;
	}

	// 生存中、天使テキスト中、オープニング中のみ更新する
	if (!isPlayerDead || isPhase1 || isOpeningPhase)
	{
		mTutorialText.Update(
			1.0f / 60.0f,
			mpPlayer->GetPosition().x,
			mpPlayer->GetPosition().y);
	}

	// プレイヤーの行動状態をテキスト判定へ渡す
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

// ゲームシーンの描画処理
void GameScene::Draw()
{
	if (!mbIsLoaded) { return; }

	// 円フェード完了後に描画を開始する
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

		// ボス・ブロックマップの描画
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

		// 落下死亡時の演出・プレイヤー描画
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

	// 円フェードの開始状態を記録する
	if (mCircleFade.GetStateFlag())
	{
		mbCircleFlag = true;
	}

	// 円フェードとオープニングフェードを描画
	mCircleFade.DrawCircleFade(mpPlayer, mBlockMap);
	mOpeningFade.Draw();
}

// 終了処理（現状は特に処理なし）
void GameScene::Finalize()
{

}

// ボス・ブロックマップ・天井破砕などの描画順を制御する
void GameScene::DrawBossOrder()
{
	// オープニング中はブロックマップのみ描画する
	if (IsOpeningMapPhase())
	{
		mBlockMap.Draw();
		return;
	}

	// ステージ1はボスをブロックマップの背面に描画する
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

// オープニング系のフェーズかどうかを判定する
bool GameScene::IsOpeningMapPhase() const
{
	return mCavePhase == CavePhase::Opening ||
		mCavePhase == CavePhase::TransitionToMain ||
		mCavePhase == CavePhase::PostBossSurface ||
		mCavePhase == CavePhase::TransitionToBossRoom;
}

// 天使が無言になるフェーズかどうかを判定する
bool GameScene::IsSilentAngelPhase() const
{
	// ボス撃破後の帰還モード（MainCave + postBossPhase==2）は無言
	return (mCavePhase == CavePhase::MainCave && Master::gCavePostBossPhase == 2);
}

// プレイヤー以外のオブジェクト・演出状態を初期化する
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

// 洞窟本編へ切り替える（落下演出の有無を指定）
void GameScene::SwitchToMainCave(bool dropFromAbove)
{
	// 落下時の引き継ぎ速度を計算する
	float carryVy = CaveOpeningConstants::MainCaveDropInitialVy;
	if (mpPlayer != nullptr)
	{
		carryVy = (std::min)(
			CaveOpeningConstants::MainCaveDropMaxCarryVy,
			(std::max)(CaveOpeningConstants::MainCaveDropInitialVy, mpPlayer->GetVelocityY()));
	}

	// 洞窟の進行状態を本編に設定する
	Master::gCaveOpeningCompleted = true;
	Master::gCavePostBossPhase = 0;
	mCavePhase = CavePhase::MainCave;

	// 前ステージの不要オブジェクトを削除し、本編のマップを読み込む
	CleanupNonPlayerObjects();
	mLoadData.StageCave();
	mbBossBgmStarted = false;
	mbWasPlayerDead = false;

	if (mpPlayer != nullptr)
	{
		// プレイヤーを本編の開始位置へ配置する
		mpPlayer->SetAllowExitRight(false);
		mpPlayer->SetSpawnPos(VGet(GameLoadDataConstants::CavePlayerSpawnX, GameLoadDataConstants::CavePlayerSpawnY, 0.0f));

		// 上空からの落下、または直接ワープで配置する
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

	// 落下時のみフェードインを開始する
	if (dropFromAbove)
	{
		mOpeningFade.StartFadeIn(CaveOpeningConstants::TransitionFadeInFrames);
	}
}

// ボス撃破後の地上（右側の光へ進む場面）をセットアップする
void GameScene::SetupPostBossSurface(bool withFadeIn)
{
	// 洞窟の進行状態を設定する
	Master::gCaveOpeningCompleted = true;
	Master::gCavePostBossPhase = 1;
	mCavePhase = CavePhase::PostBossSurface;

	// 不要オブジェクトを削除し、ボス撃破後の地上マップを読み込む
	CleanupNonPlayerObjects();
	mLoadData.StageCaveOpening();
	// ボス撃破後の帰還シーンでは天使は無言なのでフェーズをNoneに上書き
	mTutorialText.SetCaveFallPhase(TutorialTextManager::CaveFallPhase::None);
	mBlockMap.SetOpeningPitAlreadyBroken();
	mBlockMap.SetAllowScrollPastRight(false);
	mbBossBgmStarted = false;
	mbWasPlayerDead = false;

	// フェードイン時のみBGMを切り替える
	if (withFadeIn)
	{
		InitBgm();
	}

	// プレイヤーを地上の開始位置へ配置する
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

// ボス部屋の帰還用クリア扉をセットアップする（天井からの落下演出の有無を指定）
void GameScene::SetupBossRoomClearDoor(bool dropFromCeiling, float carryVy)
{
	// 洞窟の進行状態を設定する
	Master::gCaveOpeningCompleted = true;
	Master::gCavePostBossPhase = 2;
	mCavePhase = CavePhase::MainCave;

	// 不要オブジェクトを削除し、帰還用マップを読み込む
	CleanupNonPlayerObjects();
	mLoadData.StageCaveBossClearReturn();
	mBossIntro.EndIntro();
	mbBossBgmStarted = false;
	mbWasPlayerDead = false;

	// ボスエリアの中心X座標を取得する
	const float bossCenterX = mBlockMap.HasBossArea()
		? mBlockMap.GetBossAreaCenterX()
		: 8913.0f;

	// 天井の穴を即時に開いた状態にする
	mBlockMap.InitBrokenCeilingHoleImmediate(bossCenterX);

	// 帰還用の扉を即時出現させる
	mStageClear.SpawnImmediateDoor(CaveOpeningConstants::BossReturnDoorX, CaveOpeningConstants::BossReturnDoorGroundY);

	if (mpPlayer != nullptr)
	{
		// プレイヤーの配置位置を計算する
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

// デバッグ用：ボス直前の安全な足場へワープする
void GameScene::DebugWarpToBossArea()
{
	if (mpPlayer == nullptr) return;
	if (Master::gCurrentStageIndex == GameLoadDataConstants::StageIndexCave && Master::gCavePostBossPhase != 0)
	{
		return;
	}

	// 洞窟が本編前の段階ならまず本編へ切り替える
	if (mCavePhase == CavePhase::Opening || mCavePhase == CavePhase::TransitionToMain)
	{
		SwitchToMainCave(false);
	}

	// ワープ先の座標を初期化する
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
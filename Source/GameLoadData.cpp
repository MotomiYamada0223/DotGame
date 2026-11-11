#include "Master.h"
#include "GameConstants.h"
#include "Scene.h"
#include "MovingFloor.h"
#include "GameLoadData.h"
#include "EnemySlime.h" 
#include "FeintFloor.h"
#include "CrumbleFloor.h"
#include "FloorSpawner.h"
#include "HiddenBlock.h"
#include "NeedleTrap.h"
#include "EnemySkeleton.h"

GameLoadData::GameLoadData(
	Player*& player,
	BlockMap& blockMap,
	Background& background,
	TutorialTextManager& tutorialText,

	GameProgress& progress,
	RollingBoulder& rollBoulder,
	Minecart& cart
)
	: mpPlayer(player)
	, mProgress(progress)
	, mBlockMap(blockMap)
	, mBackground(background)
	, mTutorialText(tutorialText)
	, mRollingBoulder(rollBoulder)
	, mCart(cart)
{
}


void GameLoadData::LoadInitialize(int stageNumber)
{
	mnLoadNumber = stageNumber;
	if (stageNumber == GameLoadDataConstants::StageIndexForest)
	{
		StageForest();
	}
	else if (stageNumber == GameLoadDataConstants::StageIndexCave)
	{
		StageCave();
	}
	else
	{
		StageCave();
	}
}

void GameLoadData::LoadDraw()
{
	DebugDraw();
}

void GameLoadData::LoadUpdate()
{
	DebugUpdate();
}





void GameLoadData::StageForest()
{
	// CSVのファイル読み込み
	// チュートリアルテキストとブロックマップのタイル
	mTutorialText.Initialize(CsvPath::TutorialText, CsvPath::TutorialDeathText);

	// 背景画像と当たり判定画像読み込み
	if (!mBlockMap.Load(
		BlockMapGraphPath::ForestMap,
		BlockMapGraphPath::ForestMapCollision
	))
	{
		return;
	}
	mBackground.Load(ScrollGraphPath::StageForest);

	using namespace GameLoadDataConstants;

	// フェイント床の配置
	new FeintFloor(
		VGet(ForestFeintFloorPosX, ForestFeintFloorPosY, 0.0f),
		mpPlayer,
		&mBlockMap
	);

	// 崩れる床の配置
	new CrumbleFloor(
		VGet(ForestCrumbleFloorPosX, ForestCrumbleFloorPosY, 0.0f),
		mpPlayer,
		&mBlockMap
	);

	// 隠しブロックの生成
	new HiddenBlock(VGet(ForestHiddenBlockPosX, ForestHiddenBlockPosY, 0.0f), &mBlockMap);


	// 下から上・上から下へ移動する床のスポナー配置
	new FloorSpawner(
		VGet(ForestSpawner1PosX, ForestSpawner1PosY, 0.0f),
		VGet(0.0f, ForestSpawner1VelY, 0.0f),
		ForestSpawnerInterval,
		mpPlayer,
		&mBlockMap,
		FloorFeature::Normal,
		CharacterGraphPath::MoveFloorBig
	);

	new FloorSpawner(
		VGet(ForestSpawner2PosX, ForestSpawner2PosY, 0.0f),
		VGet(0.0f, ForestSpawner2VelY, 0.0f),
		ForestSpawnerInterval,
		mpPlayer,
		&mBlockMap,
		FloorFeature::SpeedUpOnRide,
		CharacterGraphPath::MoveFloorBig
	);

	new FloorSpawner(
		VGet(ForestSpawner3PosX, ForestSpawner3PosY, 0.0f),
		VGet(0.0f, ForestSpawner3VelY, 0.0f),
		ForestSpawnerInterval,
		mpPlayer,
		&mBlockMap,
		FloorFeature::ReverseAndDash,
		CharacterGraphPath::MoveFloorBig
	);




	// 敵の生成
	if (mpPlayer != nullptr)
	{
		// スライムの配置
		EnemySlime* slime = new EnemySlime(VGet(ForestSlime1PosX, ForestSlime1PosY, 0.0f));

		EnemySlime* slime1 = new EnemySlime(VGet(ForestSlime2PosX, ForestSlime2PosY, 0.0f));
		slime1->UpdateStatusByProgress(mProgress);

		// トラップの配置
		new NeedleTrap(VGet(ForestNeedle1PosX, ForestNeedle1PosY, 0.0f), TrapType::Normal);
		new NeedleTrap(VGet(ForestNeedle2PosX, ForestNeedle2PosY, 0.0f), TrapType::AntiJump);
		new NeedleTrap(VGet(ForestNeedle3PosX, ForestNeedle3PosY, 0.0f), TrapType::Normal);
		new NeedleTrap(VGet(ForestNeedle4PosX, ForestNeedle4PosY, 0.0f), TrapType::PopUp);
		new NeedleTrap(VGet(ForestNeedle5PosX, ForestNeedle5PosY, 0.0f), TrapType::PopUp);
		new NeedleTrap(VGet(ForestNeedle6PosX, ForestNeedle6PosY, 0.0f), TrapType::Normal);
	}
}



void GameLoadData::StageCave()
{
	using namespace GameLoadDataConstants;
	// CSVのファイル読み込み
	// チュートリアルテキストとブロックマップのタイル
	mTutorialText.Initialize(CsvPath::TutorialCave, CsvPath::TutorialDeathText);

	mRollingBoulder.Initialize();

	// 転がり岩の坂下くぼみ巨大針トラップ（せり上がり・上空発射）
	new BoulderNeedleTrap(
		VGet(CaveBoulderSpikePosX, CaveBoulderSpikePosY, 0.0f),
		&mRollingBoulder
	);

	// 背景画像と当たり判定画像読み込み
	if (!mBlockMap.Load(
		BlockMapGraphPath::CaveMap,
		BlockMapGraphPath::CaveMapCollision
	))
	{
		return;
	}
	mBackground.Load(ScrollGraphPath::StageCave);


	// 敵の生成
	if (mpPlayer != nullptr)
	{
		new EnemySkeleton(VGet(CaveSkeleton1PosX, CaveSkeleton1PosY, 0.0f));
		new EnemySkeleton(VGet(CaveSkeleton2PosX, CaveSkeleton2PosY, 0.0f));
	}

	mCart.Initialize();

	// 左右に往復する洞窟足場
	new MovingFloor(
		VGet(CaveMovingFloorStartX, CaveMovingFloorPosY, 0.0f),
		VGet(CaveMovingFloorEndX, CaveMovingFloorPosY, 0.0f),
		CaveMovingFloorSpeed,
		MovePattern::Loop,
		FloorFeature::Normal,
		mpPlayer,
		&mBlockMap,
		CharacterGraphPath::CaveFloor,
		CaveMovingFloorScale
	);

	// 動く足場の下の凹みに血糊針を敷き詰める（一番左はAntiJump、それ以外はNormal）
	for (int i = 0; i < CavePitNeedleCount; ++i)
	{
		float needleCenterX = CavePitStartX + (i + 0.5f) * CavePitNeedleWidth;
		TrapType trapType = (i == 0) ? TrapType::AntiJump : TrapType::Normal;
		new NeedleTrap(
			VGet(needleCenterX, CavePitNeedleCenterY, 0.0f),
			trapType,
			CharacterGraphPath::BloodNeedle,
			CavePitNeedleScale
		);
	}

	// ボスエリア直前・壊せる壁の目の前で上から降ってくる針（先ほど発射された針が落ちてきた設定）
	new NeedleTrap(
		VGet(CaveFallingNeedlePosX, CaveFallingNeedleInitialY, 0.0f),
		TrapType::Falling,
		CharacterGraphPath::BloodNeedle,
		1.0f
	);

}

void GameLoadData::DebugUpdate()
{
	// Pキーで進行度切り替え (デバッグ用)
	static bool pKeyWasDown = false;
	bool pKeyIsDown = (CheckHitKey(KEY_INPUT_P) == 1);
	if (pKeyIsDown && !pKeyWasDown)
	{
		if (mProgress == GameProgress::Tutorial1)
		{
			mProgress = GameProgress::Tutorial2; // 手加減ON (無敵)
		}
		else
		{
			mProgress = GameProgress::Tutorial1; // 手加減OFF (即死)
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
}




void GameLoadData::DebugDraw()
{
	// デバッグ表示：現在の進行度
	const char* progStr = (mProgress == GameProgress::Tutorial1) ? "Tutorial1 (Death)" : "Tutorial2 (Immune)";
	DrawFormatString(GameLoadDataConstants::DebugProgressTextX, GameLoadDataConstants::DebugProgressTextY, GetColor(0, 255, 0), "Progress: %s  [Press P to toggle]", progStr);
	DrawFormatString(GameLoadDataConstants::DebugWarpTextX, GameLoadDataConstants::DebugWarpTextY, GetColor(0, 255, 0), "[1]: Warp to Boss Area (7200, 700)");
}

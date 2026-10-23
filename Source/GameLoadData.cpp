#include "Master.h"
#include "GameConstants.h"
#include "Scene.h"
#include "MovingFloor.h"
#include "GameLoadData.h"
#include "EnemySlime.h" 
#include "FeintFloor.h"
#include "FloorSpawner.h"
#include "HiddenBlock.h"
#include "NeedleTrap.h"

GameLoadData::GameLoadData(
	Player*& player,
	BlockMap& blockMap,
	Background& background,
	TutorialTextManager& tutorialText,
	GameProgress& progress
)
	: mpPlayer(player)
	, mProgress(progress)
	, mBlockMap(blockMap)
	, mBackground(background)
	, mTutorialText(tutorialText)
{
}


void GameLoadData::LoadInitialize(int stageNumber)
{
	if (stageNumber == 0)
	{
		StageForest();
	}
	else if(stageNumber == 1)
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
	DebugUpdate();
}

void GameLoadData::LoadUpdate()
{
	DebugDraw();
}





void GameLoadData::StageForest()
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


	// フェイント床の配置（4つの足場の一番下のところ）
	new FeintFloor(
		VGet(4920.0f, 625.0f, 0.0f),
		mpPlayer,
		&mBlockMap
	);

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
		VGet(6575.0f, -100.0f, 0.0f),
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
		FloorFeature::ReverseAndDash,
		CharacterGraphPath::MoveFloorBig
	);


	// 左右に往復する基本の動く床
	MovingFloor* floor1 = new MovingFloor(
		VGet(900.0f, 200.0f, 0.0f), VGet(1800.0f, 200.0f, 0.0f),
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
}



void GameLoadData::StageCave()
{
	// CSVのファイル読み込み
	// チュートリアルテキストとブロックマップのタイル
	mTutorialText.Initialize(CsvPath::TutorialText, CsvPath::TutorialDeathText);


	// 背景画像と当たり判定画像を読み込む
	if (!mBlockMap.Load(
		BlockMapGraphPath::CaveMap,
		BlockMapGraphPath::CaveMapCollision
	))
	{
		return;
	}
	mBackground.Load(ScrollGraphPath::Stage1);

	// 敵の生成
	if (mpPlayer != nullptr)
	{
		// 画面左側 (X=0 付近)、Yはプレイヤーと同じ高さで生成
		EnemySlime* slime = new EnemySlime(VGet(1000.0f, mpPlayer->GetPosition().y, 0.0f));
		slime->UpdateStatusByProgress(mProgress);
		EnemySlime* slime1 = new EnemySlime(VGet(1700.0f, mpPlayer->GetPosition().y, 0.0f));
		slime1->UpdateStatusByProgress(mProgress);
	}

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
}




void GameLoadData::DebugDraw()
{
	// デバッグ表示：現在の進行度
	const char* progStr = (mProgress == GameProgress::Tutorial1) ? "Tutorial1 (Death)" : "Tutorial2 (Immune)";
	DrawFormatString(10, 30, GetColor(0, 255, 0), "Progress: %s  [Press P to toggle]", progStr);
	DrawFormatString(10, 50, GetColor(0, 255, 0), "[1]: Warp to Boss Area (7200, 700)");
}
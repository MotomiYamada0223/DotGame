#include "BlockAction.h"
#include "GameConstants.h"
#include "Master.h"
#include "UnitStatus.h"

BlockAction::BlockAction()
	: mCurrentCollisionType(BlockMap::CollisionType::None)
{
}

BlockAction::~BlockAction()
{
}

// 接触した地形の情報を内部に保存する
void BlockAction::SetCollisionType(BlockMap::CollisionType collisionType)
{
	mCurrentCollisionType = collisionType;
}

// 保持している状態がデスブロックの場合は確実に死亡させる
void BlockAction::ExecuteDeath(int& inHp)
{
	if (mCurrentCollisionType == BlockMap::CollisionType::Death)
	{
		inHp = 0;
	}
}

// 保持している状態がゴールブロックの場合にゴール処理を実行する
void BlockAction::ExecuteGoal()
{
	if (mCurrentCollisionType == BlockMap::CollisionType::Goal)
	{
		Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_WINRESULT);
	}
}
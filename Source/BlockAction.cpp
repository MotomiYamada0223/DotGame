#include "BlockAction.h"
#include "Damage.h"
#include "GameConstants.h"

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

// 保持している状態がデスブロックの場合にダメージを適用する
void BlockAction::ExecuteDeath(int& inHp)
{
	if (mCurrentCollisionType == BlockMap::CollisionType::Death)
	{
		Damage::ApplyDamage(inHp, SetDamage::SpikeBlock);
	}
}

// 保持している状態がゴールブロックの場合にゴール処理を実行する
void BlockAction::ExecuteGoal()
{
	if (mCurrentCollisionType == BlockMap::CollisionType::Goal)
	{
		// ゴール時の処理をここに記述
	}
}
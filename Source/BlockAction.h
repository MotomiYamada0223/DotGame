#pragma once

#include "BlockMap.h"

// 特殊地形の判定とアクションを管理するクラス
class BlockAction
{
public:
	BlockAction();
	~BlockAction();

	// 接触した地形の情報を設定する
	void SetCollisionType(BlockMap::CollisionType collisionType);

	// 各ブロックと当たった時の処理を書く
	

	// 設定された地形に応じて死亡処理を実行する
	void ExecuteDeath(int& inHp);

	// 設定された地形に応じてゴール処理を実行する
	void ExecuteGoal();

private:
	// 現在接触している地形の状態を保持する
	BlockMap::CollisionType mCurrentCollisionType;
};
#pragma once
#include <DxLib.h>
#include "Player.h"
#include "BlockMap.h"

// ゲーム開始時のフェードの円の描画
class CircleFade
{
public:
	CircleFade();
	~CircleFade();

	void DrawCircleFade(Player* player, BlockMap block);
	void UpdateCircleFade();

	// アクセサ
	bool GetStateFlag() const; // 指定のステートの場合true

private:
	int mnState;
	int mnTimer;
};
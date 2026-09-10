#pragma once
#include "DxLib.h"
// CSVを読み込んでブロックマップを生成するクラス

// 1ブロックのサイズ 64
const int CHIP_SIZZE = 64;

// マップの横縦の最大チップ数
const int MAX_MAP_WIDTH = 40;
const int MAX_MAP_HEIGTHT = 20;

// タイルセットの設定
// 画像にチップが何個並んでいるか
const int TILESET_COLUMNS = 5;

class BlockMap
{
public:
	BlockMap();
	~BlockMap();

	// CSVファイルと画像をセットで読み込む関数
	bool Load(const char* csvPath, const char* texturePath);

	// マップの描画
	void Draw();

	// プレイヤーの指定した四角がマップに当たっているかの処理をする関数
	bool IsCollision();
};




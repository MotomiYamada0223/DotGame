#pragma once
#include "DxLib.h"
// CSVを読み込んでブロックマップを生成するクラス

// 1ブロックのサイズ 64
const int CHIP_SIZE = 64;

// マップの横縦の最大チップ数
const int MAX_MAP_WIDTH = 30;
const int MAX_MAP_HEIGHT = 17;

// タイルセットの設定
// 画像にチップが横方向に何個並んでいるか
const int TILESET_COLUMNS = 8;

class BlockMap
{
public:
	BlockMap();
	~BlockMap();

	// CSVファイルと画像をセットで読み込む関数
	bool Load(const char* csvPath, const char* texturePath);

	// マップの描画
	void Draw();

	// プレイヤーと当たっているブロックを探す処理（位置が不要な場合は省略可能になっている）
	bool CheckCollisionBlock(
		float x,
		float y,
		float width,
		float height,
		int* blockX = nullptr,
		int* blockY = nullptr
	);

private:
	// タイルセットの画像
	int mnTileGraph;
	// マップデータ
	int mnMapData[MAX_MAP_HEIGHT][MAX_MAP_WIDTH];

	// マップを読み込んだか
	bool mbIsLoaded;
};




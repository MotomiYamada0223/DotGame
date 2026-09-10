#include "BlockMap.h"
#include <string>

// ファイルを読み込むために必要なインクルード
#include <fstream>
#include <sstream>
#include <iostream>

BlockMap::BlockMap()
	: mnTileGraph(-1)
	, mnMapData()
{
	// CSV読み込み前の初期値
	// 全部のブロックを0(透過のもの)を指定しておく
	for (int y = 0; y < MAX_MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAX_MAP_WIDTH; x++)
		{
			mnMapData[y][x] = 0;
		}
	}
}

BlockMap::~BlockMap()
{
	if (mnTileGraph != -1)
	{
		DeleteGraph(mnTileGraph);
	}
}


// CSVのタイルセットファイルの読み込み
bool BlockMap::Load(const char* csvPath, const char* texturePath)
{
	std::ifstream file(csvPath);

	if (!file.is_open())
	{
		std::cout
			<< "マップのCSVファイルが開けませんでした。"
			<< std::endl;

		return false;
	}

	std::string line;

	int y = 0;

	while (std::getline(file, line) &&
		y < MAX_MAP_HEIGHT)
	{
		std::stringstream ss(line);

		std::string value;

		int x = 0;

		while (std::getline(ss, value, ',') &&
			x < MAX_MAP_WIDTH)
		{
			mnMapData[y][x] = std::stoi(value);

			x++;
		}

		y++;
	}

	file.close();

	mnTileGraph = LoadGraph(texturePath);

	if (mnTileGraph == -1)
	{
		std::cout
			<< "タイルセット画像が開けませんでした。"
			<< std::endl;

		return false;
	}

	return true;
}


void BlockMap::Draw()
{
	for (int y = 0; y < MAX_MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAX_MAP_WIDTH; x++)
		{
			// 現在のチップID
			int chipID = mnMapData[y][x];

			// ====================================
			// タイルセット内の位置を計算
			//
			// 0 1 2 3 4
			// 5 6 7 8 9
			//
			// chipID = 7 の場合
			// srcX = 2
			// srcY = 1
			// ====================================

			int tileX = chipID % TILESET_COLUMNS;
			int tileY = chipID / TILESET_COLUMNS;


			// タイルセット画像から切り出す座標
			int srcX = tileX * CHIP_SIZE;
			int srcY = tileY * CHIP_SIZE;


			// マップ上に描画
			DrawRectGraph(
				x * CHIP_SIZE,
				y * CHIP_SIZE,
				srcX,
				srcY,
				CHIP_SIZE,
				CHIP_SIZE,
				mnTileGraph,

				TRUE
			);

			if (chipID >= 0 && chipID <= 10)
			{
				DrawBox(
					x * CHIP_SIZE,
					y * CHIP_SIZE,
					(x + 1) * CHIP_SIZE,
					(y + 1) * CHIP_SIZE,
					GetColor(0, 255, 0),
					FALSE
				);
			}
		}
	}
}


// マップとの当たり判定
bool BlockMap::IsCollision(
	float x,
	float y,
	float width,
	float height)
{
	// プレイヤーが重なっている
	// マップチップの範囲を計算
	int left =
		static_cast<int>(x) / CHIP_SIZE;

	int right =
		static_cast<int>(
			x + width - 1
			) / CHIP_SIZE;

	int top =
		static_cast<int>(y) / CHIP_SIZE;

	int bottom =
		static_cast<int>(
			y + height - 1
			) / CHIP_SIZE;


	// プレイヤーが重なっている
	// マップチップを調べる
	for (int mapY = top; mapY <= bottom; mapY++)
	{
		for (int mapX = left; mapX <= right; mapX++)
		{
			int chipID = mnMapData[mapY][mapX];

			// 1から3は当たり判定あり
			if (chipID >= 1 && chipID <= 3)
			{
				return true;
			}
		}
	}
	return false;
}

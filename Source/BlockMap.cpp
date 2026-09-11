#include "BlockMap.h"
#include "Master.h"
#include "Collision.h"
#include <string>
#include <algorithm>



// ファイルを読み込むために必要なインクルード
#include <fstream>
#include <sstream>
#include <iostream>

namespace
{
	// チップIDが当たり判定を持つかどうかを判定する
	bool IsSolidChip(int chipID)
	{
		return (chipID >= 1 && chipID <= 3);
	}
}

BlockMap::BlockMap()
	: mnTileGraph(-1)
	, mnMapData()
	, mbIsLoaded(false)
{
	if (!mbIsLoaded)
	{
		// CSV読み込み前の初期値
		// マップデータを0で一括初期化するため
		// fill... 指定した範囲のすべての要素を特定の値で埋められる
		std::fill(&mnMapData[0][0], &mnMapData[0][0] + MAX_MAP_WIDTH * MAX_MAP_HEIGHT, 0);
	}
}

BlockMap::~BlockMap()
{
}


// CSVのタイルセットファイルの読み込み
bool BlockMap::Load(const char* csvPath, const char* texturePath)
{
	if (mbIsLoaded) { return true; }

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

	// リソースマネージャー経由
	mnTileGraph = Master::mpResourceManager->LoadGraphics(texturePath);

	if (mnTileGraph == -1)
	{
		std::cout << "タイルセット画像が開けませんでした。"<< std::endl;
		return false;
	}
	mbIsLoaded = true;
	return true;
}


void BlockMap::Draw()
{
	// マップが読み込まれていない場合は描画しない
	if (!mbIsLoaded) { return; }

	for (int y = 0; y < MAX_MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAX_MAP_WIDTH; x++)
		{
			// 現在のチップID
			int chipID = mnMapData[y][x];

			// 0番は何も描画しない
			if (chipID == 0) { continue; }
			
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


			// 0から10まではデバッグ表示している
			if (chipID >= 1 && chipID <= 10)
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

// プレイヤーと当たっているブロックを探す
// プレイヤーと当たっているブロックを探す
bool BlockMap::CheckCollisionBlock(
	float x,
	float y,
	float width,
	float height,
	int* blockX,
	int* blockY)
{
	// プレイヤーの矩形
	VECTOR playerPos = VGet(x, y, 0.0f);
	VECTOR playerSize = VGet(width, height, 0.0f);

	// プレイヤーが重なっているブロック範囲を計算
	// ワールド座標をマップの配列インデックスに変換するため
	int left = static_cast<int>(x) / CHIP_SIZE;
	int right = static_cast<int>(x + width - 1) / CHIP_SIZE;
	int top = static_cast<int>(y) / CHIP_SIZE;
	int bottom = static_cast<int>(y + height - 1) / CHIP_SIZE;

	// マップ範囲を調整
	if (left < 0) {left = 0;}
	if (right >= MAX_MAP_WIDTH) {right = MAX_MAP_WIDTH - 1;}
	if (top < 0) {top = 0;}
	if (bottom >= MAX_MAP_HEIGHT) {bottom = MAX_MAP_HEIGHT - 1;}


	// 周囲のブロックを1個ずつ調べる
	// マップ全体を毎フレーム調べると重いので、プレイヤーが実際に重なっている可能性のある近傍のチップだけを効率よく走査するため
	for (int mapY = top; mapY <= bottom; mapY++)
	{
		for (int mapX = left; mapX <= right; mapX++)
		{
			int chipID = mnMapData[mapY][mapX];

			if (!IsSolidChip(chipID)) { continue; }

			// ブロックの矩形
			VECTOR blockPos = VGet(
				static_cast<float>(mapX * CHIP_SIZE),
				static_cast<float>(mapY * CHIP_SIZE),
				0.0f
			);

			VECTOR blockSize = VGet(
				static_cast<float>(CHIP_SIZE),
				static_cast<float>(CHIP_SIZE),
				0.0f
			);

			// プレイヤーとブロックの当たり判定
			if (Collision::CheckRectToRect(
				playerPos,
				playerSize,
				blockPos,
				blockSize))
			{
				// 格納先が指定されている場合のみブロック座標を代入する
				if (blockX != nullptr)
				{
					*blockX = mapX;
				}
				if (blockY != nullptr)
				{
					*blockY = mapY;
				}
				return true;
			}
		}
	}
	return false;
}
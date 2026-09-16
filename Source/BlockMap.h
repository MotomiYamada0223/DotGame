#pragma once

#include "DxLib.h"
#include <string>
#include <vector>

class BlockMap
{
public:

	enum class CollisionType
	{
		None = 0,   // 当たり判定なし
		Block,  // 当たり判定あり
	};


    BlockMap();
    ~BlockMap();

    // 背景画像と当たり判定画像を読み込む
    bool Load(
        const std::string& backgroundPath,
        const std::string& collisionPath
    );

    void Draw();

    // プレイヤーと当たり判定画像が重なっているか
    // blockX : 衝突した赤ピクセルのX座標
    // blockY : 衝突した赤ピクセルのY座標
    bool CheckCollisionBlock(
        float x,
        float y,
        float width,
        float height,
        int* blockX = nullptr,
        int* blockY = nullptr
    );

	// 場所の種類を取得する
	CollisionType GetCollisionType(int x, int y) const;

private:
    int mnBackgroundGraph;   // 背景画像
    int mnCollisionSoftImage; // 当たり判定画像 (描画はしない)

    // 当たり判定画像のサイズ
    int mnCollisionWidth;
    int mnCollisionHeight;

    // 赤い場所ならtrue
    std::vector<CollisionType> mbCollisionData;

    // 読み込み済みかのフラグ
    bool mbIsLoaded;


    // 指定座標が当たり判定か
    bool IsCollisionPixel(int x, int y) const;

    // 配列のインデックスを取得
    // 2次元座標を1次元配列の番号に変換する。
    int GetCollisionIndex(int x, int y) const { return  y * mnCollisionWidth + x; }
};
#pragma once

#include "DxLib.h"
#include <string>
#include <vector>

class BlockMap
{
public:

    BlockMap();
    ~BlockMap();

    // 背景画像と当たり判定画像を読み込む
    bool Load(
        const std::string& backgroundPath,
        const std::string& collisionPath
    );

    // 背景画像を描画する
    void Draw();

    // プレイヤーと当たり判定画像が重なっているか
    //
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

private:

    // 背景画像
    int mnBackgroundGraph;

    // 当たり判定画像のSoftImage
    int mnCollisionSoftImage;

    // 当たり判定画像のサイズ
    int mnCollisionWidth;
    int mnCollisionHeight;

    // 赤い場所ならtrue
    std::vector<bool> mbCollisionData;

    // 読み込み済みか
    bool mbIsLoaded;

private:

    // 指定座標が当たり判定か
    bool IsCollisionPixel(int x, int y) const;

    // 配列のインデックスを取得
    int GetCollisionIndex(int x, int y) const;
};
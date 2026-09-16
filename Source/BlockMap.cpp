#include "BlockMap.h"
#include "Master.h"

#include <iostream>


namespace
{
    // 当たり判定として扱う色
    constexpr int COLLISION_R = 255;
    constexpr int COLLISION_G = 0;
    constexpr int COLLISION_B = 0;
}

BlockMap::BlockMap()
    : mnBackgroundGraph(-1)
    , mnCollisionSoftImage(-1)
    , mnCollisionWidth(0)
    , mnCollisionHeight(0)
    , mbCollisionData()
    , mbIsLoaded(false)
{
}

BlockMap::~BlockMap()
{
}


// マップ読み込み
// 背景画像と当たり判定画像を読み込む。
// 当たり判定画像は、色によって
// CollisionTypeに変換する。
bool BlockMap::Load(
    const std::string& backgroundPath,
    const std::string& collisionPath)
{
    if (mbIsLoaded)
    {
        return true;
    }

    mnBackgroundGraph =
        Master::mpGameManager
        ->GetResourceManager()
        ->LoadGraphics(backgroundPath);


    if (mnBackgroundGraph == -1)
    {
        std::cout
            << "背景画像が開けませんでした。"
            << std::endl;
        return false;
    }


    // 色だけの画像を読み込む ゲーム中には描画しない
    mnCollisionSoftImage =
        LoadSoftImage(collisionPath.c_str());


    if (mnCollisionSoftImage == -1)
    {
        std::cout
            << "当たり判定画像が開けませんでした。"
            << std::endl;

        return false;
    }


    // 当たり判定画像のサイズを取得
    GetSoftImageSize(
        mnCollisionSoftImage,
        &mnCollisionWidth,
        &mnCollisionHeight
    );


    // 当たり判定データを確保
    // 画像の1ピクセルにつき1個のCollisionTypeを持つ。
    // 最初はすべてNoneにする。
    mbCollisionData.resize(
        mnCollisionWidth * mnCollisionHeight,
        CollisionType::None
    );


    // 当たり判定画像を1ピクセルずつ調べる
    for (int y = 0; y < mnCollisionHeight; y++)
    {
        for (int x = 0; x < mnCollisionWidth;  x++)
        {
            // RGBAを取得するための変数
            int r = 0;
            int g = 0;
            int b = 0;
            int a = 0;


            // 現在のピクセルの色を取得
            int result =
                GetPixelSoftImage(
                    mnCollisionSoftImage,
                    x,
                    y,
                    &r,
                    &g,
                    &b,
                    &a
                );

            if (result != 0)
            {
                continue;
            }


            // 透明なら当たり判定なし
            if (a == 0)
            {
                mbCollisionData[GetCollisionIndex(x, y)] = CollisionType::None;
                continue;
            }


            // 赤色ならBlock
            // 「通れない場所」として扱う
            if (r == COLLISION_R &&
                g == COLLISION_G &&
                b == COLLISION_B)
            {
                mbCollisionData[GetCollisionIndex(x, y)] = CollisionType::Block;
                continue;
            }


            // 赤以外はNone
            mbCollisionData[
                GetCollisionIndex(x, y)
            ] = CollisionType::None;
        }
    }

    // CollisionTypeへ変換済みなので削除する。
    DeleteSoftImage(mnCollisionSoftImage);
    mnCollisionSoftImage = -1;


    mbIsLoaded = true;
    return true;
}


// 背景描画
void BlockMap::Draw()
{
    if (!mbIsLoaded)
    {
        return;
    }

    // 背景画像を描画
    DrawGraph(
        0,
        0,
        mnBackgroundGraph,
        TRUE
    );
}


// 指定座標のCollisionTypeを取得
// x, y の位置に何があるかを返す。
BlockMap::CollisionType
BlockMap::GetCollisionType(int x, int y) const
{
    // マップ外の場合
    // 配列の範囲外アクセスを防ぐ。
    if (x < 0 ||
        x >= mnCollisionWidth ||
        y < 0 ||
        y >= mnCollisionHeight)
    {
        return CollisionType::None;
    }

    // 指定座標のCollisionTypeを返す
    return mbCollisionData[GetCollisionIndex(x, y)];
}


// 指定座標がBlockか確認
// この関数では「通れない場所かどうか」だけを確認する。
bool BlockMap::IsCollisionPixel(
    int x,
    int y) const
{
    // 指定座標の種類を取得
    CollisionType type =
        GetCollisionType(x, y);


    // Blockなら当たり判定あり
    if (type == CollisionType::Block)
    {
        return true;
    }

    // Block以外は当たり判定なし
    return false;
}


// プレイヤーとの当たり判定
// プレイヤーを矩形として扱い、
// 「プレイヤーの矩形内にBlockが1ピクセルでもあるか」
// を調べる。
bool BlockMap::CheckCollisionBlock(
    float x,
    float y,
    float width,
    float height,
    int* blockX,
    int* blockY)
{
    if (!mbIsLoaded)
    {
        return false;
    }

    // プレイヤーの矩形範囲を計算
    int left =
        static_cast<int>(x);
    int right =
        static_cast<int>(
            x + width - 1.0f
            );
    int top =
        static_cast<int>(y);

    int bottom =
        static_cast<int>(
            y + height - 1.0f
            );

    // マップの範囲内に制限
    if (left < 0) { left = 0; }
    if (right >= mnCollisionWidth) { right = mnCollisionWidth - 1; }
    if (top < 0) { top = 0; }
    if (bottom >= mnCollisionHeight) { bottom = mnCollisionHeight - 1; }


    // プレイヤーの矩形内を調べる
    for (int pixelY = top;
        pixelY <= bottom;
        pixelY++)
    {
        for (int pixelX = left;
            pixelX <= right;
            pixelX++)
        {
            // 現在のピクセルがBlockか確認
            if (!IsCollisionPixel(
                pixelX,
                pixelY))
            {
                continue;
            }


            // Blockを発見
            if (blockX != nullptr)
            {
                *blockX = pixelX;
            }

            if (blockY != nullptr)
            {
                *blockY = pixelY;
            }


            // 1つでもBlockがあれば衝突
            return true;
        }
    }

    return false;
}
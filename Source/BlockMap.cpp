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


// ============================================================
// コンストラクタ
// ============================================================

BlockMap::BlockMap()
    : mnBackgroundGraph(-1)
    , mnCollisionSoftImage(-1)
    , mnCollisionWidth(0)
    , mnCollisionHeight(0)
    , mbCollisionData()
    , mbIsLoaded(false)
{
}


// ============================================================
// デストラクタ
// ============================================================

BlockMap::~BlockMap()
{
}


// ============================================================
// マップ読み込み
// ============================================================

bool BlockMap::Load(
    const std::string& backgroundPath,
    const std::string& collisionPath)
{
    if (mbIsLoaded)
    {
        return true;
    }


    // ========================================================
    // 背景画像を読み込む
    // ========================================================

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


    // ========================================================
    // 当たり判定画像をSoftImageとして読み込む
    // ========================================================

    mnCollisionSoftImage =
        LoadSoftImage(collisionPath.c_str());

    if (mnCollisionSoftImage == -1)
    {
        std::cout
            << "当たり判定画像が開けませんでした。"
            << std::endl;

        return false;
    }


    // ========================================================
    // 当たり判定画像のサイズを取得
    // ========================================================

    GetSoftImageSize(
        mnCollisionSoftImage,
        &mnCollisionWidth,
        &mnCollisionHeight
    );


    // ========================================================
    // 背景画像とサイズが同じか確認
    // ========================================================

    // ※ 必要なら後で背景画像のサイズチェックも追加できる


    // ========================================================
    // 当たり判定データを確保
    // ========================================================

    mbCollisionData.resize(
        mnCollisionWidth * mnCollisionHeight,
        false
    );


    // ========================================================
    // 当たり判定画像を調べる
    // ========================================================

    for (int y = 0;
        y < mnCollisionHeight;
        y++)
    {
        for (int x = 0;
            x < mnCollisionWidth;
            x++)
        {
            int r = 0;
            int g = 0;
            int b = 0;
            int a = 0;


            // --------------------------------------------
            // ピクセルのRGBAを取得
            // --------------------------------------------

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


            // =================================================
            // 透明なら通れる
            // =================================================

            if (a == 0)
            {
                mbCollisionData[
                    GetCollisionIndex(x, y)
                ] = false;

                continue;
            }


            // =================================================
            // 赤色なら当たり判定あり
            // =================================================

            if (r == COLLISION_R &&
                g == COLLISION_G &&
                b == COLLISION_B)
            {
                mbCollisionData[
                    GetCollisionIndex(x, y)
                ] = true;
            }
        }
    }


    // ========================================================
    // SoftImageはもう必要ない
    // ========================================================

    DeleteSoftImage(mnCollisionSoftImage);

    mnCollisionSoftImage = -1;


    // ========================================================
    // 読み込み完了
    // ========================================================

    mbIsLoaded = true;

    return true;
}


// ============================================================
// 背景描画
// ============================================================

void BlockMap::Draw()
{
    if (!mbIsLoaded)
    {
        return;
    }


    // 背景画像だけ描画
    DrawGraph(
        0,
        0,
        mnBackgroundGraph,
        TRUE
    );
}


// ============================================================
// 配列インデックス取得
// ============================================================

int BlockMap::GetCollisionIndex(
    int x,
    int y) const
{
    return y * mnCollisionWidth + x;
}


// ============================================================
// 指定座標が当たり判定か
// ============================================================

bool BlockMap::IsCollisionPixel(
    int x,
    int y) const
{
    // マップ外
    if (x < 0 ||
        x >= mnCollisionWidth ||
        y < 0 ||
        y >= mnCollisionHeight)
    {
        return false;
    }


    return mbCollisionData[
        GetCollisionIndex(x, y)
    ];
}


// ============================================================
// プレイヤーとの当たり判定
// ============================================================

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


    // ========================================================
    // プレイヤーの矩形
    // ========================================================

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


    // ========================================================
    // マップ範囲内に制限
    // ========================================================

    if (left < 0)
    {
        left = 0;
    }

    if (right >= mnCollisionWidth)
    {
        right = mnCollisionWidth - 1;
    }

    if (top < 0)
    {
        top = 0;
    }

    if (bottom >= mnCollisionHeight)
    {
        bottom = mnCollisionHeight - 1;
    }


    // ========================================================
    // プレイヤー矩形内のピクセルを調べる
    // ========================================================

    for (int pixelY = top;
        pixelY <= bottom;
        pixelY++)
    {
        for (int pixelX = left;
            pixelX <= right;
            pixelX++)
        {
            // 赤い当たり判定ではない
            if (!IsCollisionPixel(
                pixelX,
                pixelY))
            {
                continue;
            }


            // =================================================
            // 赤いピクセルを発見
            // =================================================

            if (blockX != nullptr)
            {
                *blockX = pixelX;
            }

            if (blockY != nullptr)
            {
                *blockY = pixelY;
            }


            return true;
        }
    }


    return false;
}
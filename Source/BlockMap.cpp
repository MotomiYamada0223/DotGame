#include "BlockMap.h"
#include "Master.h"

#include <iostream>


namespace
{
    // 当たり判定画像で使用する色

    // 赤 → Block
    constexpr int BLOCK_R = 255;
    constexpr int BLOCK_G = 0;
    constexpr int BLOCK_B = 0;


    // 青 → Death
    constexpr int DEATH_R = 0;
    constexpr int DEATH_G = 0;
    constexpr int DEATH_B = 255;


    // 緑 → Goal
    constexpr int GOAL_R = 0;
    constexpr int GOAL_G = 255;
    constexpr int GOAL_B = 0;
}


BlockMap::BlockMap()
    : mnBackgroundGraph(-1)
    , mnCollisionSoftImage(-1)
    , mnCollisionWidth(0)
    , mnCollisionHeight(0)
    , mCollisionData()
    , mbIsLoaded(false)
{
}

BlockMap::~BlockMap()
{
}


// マップ読み込み
bool BlockMap::Load(
    const std::string& backgroundPath,
    const std::string& collisionPath)
{
    if (mbIsLoaded)
    {
        return true;
    }


    // 背景画像の読み込み
    mnBackgroundGraph =
        Master::mpGameManager
        ->GetResourceManager()
        ->LoadGraphics(backgroundPath);

    if (mnBackgroundGraph == -1)
    {
        std::cout<< "背景画像が開けませんでした。" << std::endl;
        return false;
    }

    // 当たり判定画像を読み込む
    mnCollisionSoftImage =
        LoadSoftImage(
            collisionPath.c_str()
        );

    if (mnCollisionSoftImage == -1)
    {
        std::cout<< "当たり判定画像が開けませんでした。" << std::endl;
        return false;
    }


    // 当たり判定画像のサイズを取得
    GetSoftImageSize(
        mnCollisionSoftImage,
        &mnCollisionWidth,
        &mnCollisionHeight
    );


    // CollisionTypeデータを確保
    mCollisionData.resize(
        mnCollisionWidth * mnCollisionHeight,
        CollisionType::None
    );


    // 当たり判定画像を1ピクセルずつ調べる
    for (int y = 0; y < mnCollisionHeight; y++)
    {
        for (int x = 0; x < mnCollisionWidth; x++)
        {
            // RGBAを受け取る変数
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


            // 透明ならNone
            if (a == 0)
            {
                mCollisionData[
                    GetCollisionIndex(x, y)
                ] = CollisionType::None;

                continue;
            }


            // 赤ならBlock
            if (r == BLOCK_R &&
                g == BLOCK_G &&
                b == BLOCK_B)
            {
                mCollisionData[
                    GetCollisionIndex(x, y)
                ] = CollisionType::Block;

                continue;
            }

            // 青ならDeath
            if (r == DEATH_R &&
                g == DEATH_G &&
                b == DEATH_B)
            {
                mCollisionData[
                    GetCollisionIndex(x, y)
                ] = CollisionType::Death;

                continue;
            }


            // 緑ならGoal
            if (r == GOAL_R &&
                g == GOAL_G &&
                b == GOAL_B)
            {
                mCollisionData[
                    GetCollisionIndex(x, y)
                ] = CollisionType::Goal;

                continue;
            }

            // それ以外はNone
            mCollisionData[
                GetCollisionIndex(x, y)
            ] = CollisionType::None;
        }
    }


    // 元の画像データはもう必要ない
    DeleteSoftImage(
        mnCollisionSoftImage
    );

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


// CollisionType取得
BlockMap::CollisionType
BlockMap::GetCollisionType(
    int x,
    int y) const
{
    // マップ外
    if (x < 0 ||
        x >= mnCollisionWidth ||
        y < 0 ||
        y >= mnCollisionHeight)
    {
        // マップ外は何もない扱い
        return CollisionType::None;
    }


    // 指定座標のCollisionTypeを返す
    return mCollisionData[
        GetCollisionIndex(x, y)
    ];
}
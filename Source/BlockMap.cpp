#include "BlockMap.h"
#include "Master.h"
#include "GameConstants.h"
#include <iostream>
#include "GameConstants.h"


BlockMap::BlockMap()
    : mnBackgroundGraph(-1)
    , mnCollisionSoftImage(-1)
    , mnCollisionWidth(0)
    , mnCollisionHeight(0)
    , mCollisionData()
    , mbIsLoaded(false)
    , mBackgroundWidth(0)
    , mBackgroundHeight(0)
    , mScrollX(0)
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
    if (mbIsLoaded) { return true; }

    // 背景画像の読み込み
    mnBackgroundGraph =
        Master::mpGameManager
        ->GetResourceManager()
        ->LoadGraphics(backgroundPath);

    if (mnBackgroundGraph == -1)
    {
        std::cout<< "ブロックの背景画像が開けませんでした。" << std::endl;
        return false;
    }

    // 背景画像のサイズを取得
    GetGraphSize(
        mnBackgroundGraph,
        &mBackgroundWidth,
        &mBackgroundHeight
    );



    // 当たり判定画像を読み込む
    mnCollisionSoftImage =
        LoadSoftImage(
            collisionPath.c_str()
        );

    if (mnCollisionSoftImage == -1)
    {
        std::cout<< "ブロックの当たり判定画像が開けませんでした。" << std::endl;
        return false;
    }


    // 当たり判定画像のサイズを取得
    GetSoftImageSize(
        mnCollisionSoftImage,
        &mnCollisionWidth,
        &mnCollisionHeight
    );

    // CollisionTypeデータを確保 全てNONEにしている
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
            if (result != 0) { continue; }

            // 透明ならNone
            if (a == 0)
            {
                mCollisionData[
                    GetCollisionIndex(x, y)
                ] = CollisionType::None;
                continue;
            }


            // 赤ならBlock
            if (r == BlockCollisionColor::BLOCK_R &&
                g == BlockCollisionColor::BLOCK_G &&
                b == BlockCollisionColor::BLOCK_B)
            {
                mCollisionData[
                    GetCollisionIndex(x, y)
                ] = CollisionType::Block;

                continue;
            }

            // 青ならDeath
            if (r == BlockCollisionColor::DEATH_R &&
                g == BlockCollisionColor::DEATH_G &&
                b == BlockCollisionColor::DEATH_B)
            {
                mCollisionData[
                    GetCollisionIndex(x, y)
                ] = CollisionType::Death;
                continue;
            }


            // 緑ならGoal
            if (r == BlockCollisionColor::GOAL_R &&
                g == BlockCollisionColor::GOAL_G &&
                b == BlockCollisionColor::GOAL_B)
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
    if (!mbIsLoaded) { return; }

    // 背景画像を描画
    DrawRectGraph(
        0,                  // 画面上のX
        0,                  // 画面上のY
        mScrollX,           // 元画像から切り出すX
        0,                  // 元画像から切り出すY
        ScreenSize::ScrrenWidth,       // 切り出す幅
        ScreenSize::ScrrenHeight,      // 切り出す高さ
        mnBackgroundGraph,
        TRUE
    );
}

// マップのスクロール処理
void BlockMap::Move()
{
    if (!mbIsLoaded) { return; }

    if (CheckHitKey(KEY_INPUT_1))
    {
        mScrollX -= 5;
    }

    if (CheckHitKey(KEY_INPUT_2))
    {
        mScrollX += 5;
    }

    // 左端
    if (mScrollX < 0)
    {
        mScrollX = 0;
    }

    // 右端
    int maxScrollX =
        mBackgroundWidth -
        ScreenSize::ScrrenWidth;

    if (maxScrollX < 0)
    {
        maxScrollX = 0;
    }

    if (mScrollX > maxScrollX)
    {
        mScrollX = maxScrollX;
    }
}


// CollisionType取得
BlockMap::CollisionType BlockMap::GetCollisionType(
    int x,
    int y) const
{
    // マップ外
    if (x < 0 || x >= mnCollisionWidth ||
        y < 0 || y >= mnCollisionHeight)
    {
        // マップ外は何もない扱い
        return CollisionType::None;
    }

    return mCollisionData[
        GetCollisionIndex(x, y)
    ];
}


// スクリーン座標をマップ座標に変換
int BlockMap::ScreenToMapX(int screenX) const
{
    return screenX + mScrollX;
}

int BlockMap::ScreenToMapY(int screenY) const
{
    return screenY;
}
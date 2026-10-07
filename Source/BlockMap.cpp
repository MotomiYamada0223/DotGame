#include "BlockMap.h"
#include "Master.h"
#include "GameConstants.h"
#include <iostream>
#include "Camera.h"


BlockMap::BlockMap()
    : mnBackgroundGraph(-1)
    , mnCollisionSoftImage(-1)
    , mnCollisionWidth(0)
    , mnCollisionHeight(0)
    , mCollisionData()
    , mbIsLoaded(false)
    , mnBackgroundWidth(0)
    , mnBackgroundHeight(0)
    , mfMoveDirection(0.0f)
    , mbIsScrolling(false)
{
    //mSpawnPos = initPos;
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
        &mnBackgroundWidth,
        &mnBackgroundHeight
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
        mCamera.GetScrollX(),           // 元画像から切り出すX
        mCamera.GetScrollY(),                  // 元画像から切り出すY
        ScreenSize::ScrrenWidth,       // 切り出す幅
        ScreenSize::ScrrenHeight,      // 切り出す高さ
        mnBackgroundGraph,
        TRUE
    );
}

// デバッグ描画まとめ
void BlockMap::DebugDraw()
{
    // デバッグ表示
    // 左スクロール開始位置
    DrawLine(
        MapScrollConstants::ScrollStartLeftX,
        0,
        MapScrollConstants::ScrollStartLeftX,
        ScreenSize::ScrrenHeight,
        GetColor(0, 255, 0)
    );

    // 右スクロール開始位置
    DrawLine(
        MapScrollConstants::ScrollStartRightX,
        0,
        MapScrollConstants::ScrollStartRightX,
        ScreenSize::ScrrenHeight,
        GetColor(255, 0, 0)
    );

    // 上スクロール
    DrawLine(
        0,
        MapScrollConstants::ScrollStartUpY,
        ScreenSize::ScrrenWidth,
        MapScrollConstants::ScrollStartUpY,
        GetColor(255, 0, 0)
    );

    // 下スクロール
    DrawLine(
        0,
        MapScrollConstants::ScrollStartDownY,
        ScreenSize::ScrrenWidth,
        MapScrollConstants::ScrollStartDownY,
        GetColor(255, 0, 0)
    );



    DrawFormatString(
        20,
        200,
        GetColor(255, 255, 255),
        "ScrollX: %d",
        mCamera.GetScrollX()
    );
}

// マップのスクロール処理
void BlockMap::Move(int playerScreenX, int playerScreenY, float moveDirection, float currentSpeed)
{
    if (!mbIsLoaded) { return; }
    mfMoveDirection = moveDirection;

    mCamera.Update(
        playerScreenX,
        playerScreenY,
        moveDirection,
        currentSpeed,
        mnBackgroundWidth,
        ScreenSize::ScrrenWidth
    );
}



// CollisionType取得 xとyはワールド座標
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
    return screenX + mCamera.GetScrollX();
}

int BlockMap::ScreenToMapY(int screenY) const
{
    return screenY;
}

void BlockMap::SetCollisionBlock(int startX, int startY, int width, int height, CollisionType type)
{
    for (int y = startY; y < startY + height; ++y)
    {
        for (int x = startX; x < startX + width; ++x)
        {
            if (x >= 0 && x < mnCollisionWidth && y >= 0 && y < mnCollisionHeight)
            {
                mCollisionData[GetCollisionIndex(x, y)] = type;
            }
        }
    }
}

void BlockMap::ResetScroll()
{
    mCamera.SetScrollX(0);
}
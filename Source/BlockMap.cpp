#include "BlockMap.h"
#include "Master.h"
#include "GameConstants.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include "Camera.h"


BlockMap::BlockMap()
    : mnBackgroundGraph(-1)
    , mnCollisionSoftImage(-1)
    , mnCollisionWidth(0)
    , mnCollisionHeight(0)
    , mCollisionData()
    , mbIsLoaded(false)
    , mbHasBossArea(false)
    , mnBossAreaLeft(0)
    , mnBossAreaRight(0)
    , mnBackgroundWidth(0)
    , mnBackgroundHeight(0)
    , mfMoveDirection(0.0f)
    , mbIsScrolling(false)
{
    //mSpawnPos = initPos;
}

BlockMap::~BlockMap()
{
    ResetCeilingBreak();
}

// マップ読み込み
bool BlockMap::Load(
    const std::string& backgroundPath,
    const std::string& collisionPath)
{
    if (mbIsLoaded) { return true; }
    ResetCeilingBreak();

    // 破壊可能壁の読み込み（洞窟ステージ専用）
    mWalls.clear();
    if (backgroundPath.find("Stage1-2") != std::string::npos || backgroundPath.find("Cave") != std::string::npos)
    {
        FILE* fp = nullptr;
        fopen_s(&fp, "Resource/Map/Cave_WallConfig.txt", "r");
        if (fp)
        {
            int count = 0;
            if (fscanf_s(fp, "%d", &count) == 1)
            {
                for (int i = 0; i < count; ++i)
                {
                    int minX = 0, minY = 0, maxX = 0, maxY = 0;
                    char pathBuf[256] = { 0 };
                    if (fscanf_s(fp, "%d %d %d %d %255s",
                        &minX, &minY, &maxX, &maxY,
                        pathBuf, (unsigned int)sizeof(pathBuf)) == 5)
                    {
                        auto wall = std::make_shared<DestructibleWall>();
                        if (wall->Load(minX, minY, maxX, maxY, pathBuf))
                        {
                            mWalls.push_back(wall);
                        }
                    }
                }
            }
            fclose(fp);
        }
    }

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

    mbHasBossArea = false;
    mnBossAreaLeft = 0;
    mnBossAreaRight = 0;

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

            // オレンジ色ならBreakableBlock
            if (r == BlockCollisionColor::BREAKABLE_R &&
                g == BlockCollisionColor::BREAKABLE_G &&
                b == BlockCollisionColor::BREAKABLE_B)
            {
                mCollisionData[
                    GetCollisionIndex(x, y)
                ] = CollisionType::BreakableBlock;

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

            // 黄色ならBossArea
            if (r == BlockCollisionColor::BOSSAREA_R &&
                g == BlockCollisionColor::BOSSAREA_G &&
                b == BlockCollisionColor::BOSSAREA_B)
            {
                mCollisionData[
                    GetCollisionIndex(x, y)
                ] = CollisionType::BossArea;

                if (!mbHasBossArea)
                {
                    mbHasBossArea = true;
                    mnBossAreaLeft = x;
                    mnBossAreaRight = x;
                }
                else
                {
                    if (x < mnBossAreaLeft) { mnBossAreaLeft = x; }
                    if (x > mnBossAreaRight) { mnBossAreaRight = x; }
                }
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

    mCamera.ResetScrollInit();

    mnCollisionSoftImage = -1;
    mbIsLoaded = true;
    return true;
}


// 背景描画
void BlockMap::Draw()
{
    if (!mbIsLoaded) { return; }

    const int sx = mCamera.GetDrawScrollX();
    const int sy = mCamera.GetDrawScrollY();
    const int sw = ScreenSize::ScrrenWidth;
    const int sh = ScreenSize::ScrrenHeight;

    bool drawSplitForCeilingHole = false;
    int cx1 = 0, cx2 = 0, cy1 = 0, cy2 = 0;

    if (mbCeilingBreakActive)
    {
        cx1 = (std::max)(mCeilingHoleLeft, sx);
        cx2 = (std::min)(mCeilingHoleRight, sx + sw);
        cy1 = (std::max)(mCeilingHoleTop, sy);
        cy2 = (std::min)(mCeilingHoleBottom, sy + sh);
        if (cx1 < cx2 && cy1 < cy2)
        {
            drawSplitForCeilingHole = true;
        }
    }

    if (!drawSplitForCeilingHole)
    {
        // 通常時：背景画像を1回で描画
        DrawRectGraph(0, 0, sx, sy, sw, sh, mnBackgroundGraph, TRUE);
    }
    else
    {
        // 天井破砕領域を除いた周囲4帯を切り出して描画（穴の奥に背景が透けて見える）
        if (cx1 > sx)
        {
            DrawRectGraph(0, 0, sx, sy, cx1 - sx, sh, mnBackgroundGraph, TRUE);
        }
        if (sx + sw > cx2)
        {
            DrawRectGraph(cx2 - sx, 0, cx2, sy, (sx + sw) - cx2, sh, mnBackgroundGraph, TRUE);
        }
        if (cy1 > sy)
        {
            DrawRectGraph(cx1 - sx, 0, cx1, sy, cx2 - cx1, cy1 - sy, mnBackgroundGraph, TRUE);
        }
        if (sy + sh > cy2)
        {
            DrawRectGraph(cx1 - sx, cy2 - sy, cx1, cy2, cx2 - cx1, (sy + sh) - cy2, mnBackgroundGraph, TRUE);
        }
    }

    if (mbCeilingBreakActive)
    {
        // 未破壊の天井チャンク＋奥側へ落下中の天井破片を描画
        for (const auto& chunk : mCeilingChunks)
        {
            if (!chunk.isBroken)
            {
                if (chunk.origY + chunk.size < sy || chunk.origY > sy + sh) continue;
                if (chunk.origX + chunk.size < sx || chunk.origX > sx + sw) continue;

                int shakeX = 0;
                int shakeY = 0;
                if (chunk.isTarget &&
                    mLatestDragonBottomY + DragonCeilingBreakConstants::BreakFrontOffsetY + DragonCeilingBreakConstants::ShakePreviewDistY >= chunk.origY)
                {
                    shakeX = GetRand(6) - 3;
                    shakeY = GetRand(6) - 3;
                }
                // 未破壊チャンクは分割テクスチャ対応の DrawRectGraph で元マップから直接描画し、欠落を防ぐ
                DrawRectGraph(static_cast<int>(chunk.origX) - sx + shakeX,
                              static_cast<int>(chunk.origY) - sy + shakeY,
                              static_cast<int>(chunk.origX),
                              static_cast<int>(chunk.origY),
                              chunk.size,
                              chunk.size,
                              mnBackgroundGraph,
                              TRUE);
            }
            else if (!chunk.isFinished && !chunk.isForeground)
            {
                DrawBrokenCeilingChunk(chunk, sx, sy);
            }
        }
    }

    // 破壊可能壁の描画
    for (auto& wall : mWalls)
    {
        wall->Draw(sx, sy);
    }
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

    DrawFormatString(
        20,
        200,
        GetColor(255, 255, 255),
        "ScrollX: %d",
        mCamera.GetScrollX()
    );
}

// マップのスクロール処理
void BlockMap::Move(int playerScreenX, float playerMapY, float moveDirection, float currentSpeed)
{
    if (!mbIsLoaded) { return; }
    mfMoveDirection = moveDirection;

    mCamera.Update(
        playerScreenX,
        playerMapY,
        moveDirection,
        currentSpeed,
        mnBackgroundWidth,
        ScreenSize::ScrrenWidth,
        mnBackgroundHeight,
        ScreenSize::ScrrenHeight
    );

    for (auto wall : mWalls)
    {
        wall->Update();
    }
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
    return screenY + mCamera.GetScrollY();
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
    mCamera.SetScrollY(0);
    mCamera.SetLocked(false);
    mCamera.ResetScrollInit();
    ResetCeilingBreak();
}

void BlockMap::SnapCameraToPosition(float targetWorldX, float targetWorldY)
{
    // プレイヤーがカメラスクロール領域（ScrollStartLeftX〜ScrollStartRightX）の中央（800px）に収まるようにscrollXを算出
    const int targetScreenX = (MapScrollConstants::ScrollStartLeftX + MapScrollConstants::ScrollStartRightX) / 2;
    int scrollX = static_cast<int>(targetWorldX) - targetScreenX;
    int maxScrollX = (std::max)(0, mnBackgroundWidth - ScreenSize::ScrrenWidth);
    if (scrollX < 0) scrollX = 0;
    if (scrollX > maxScrollX) scrollX = maxScrollX;

    // 縦スクロール (targetWorldY - 720)
    int maxScrollY = (std::max)(0, mnBackgroundHeight - ScreenSize::ScrrenHeight);
    int scrollY = static_cast<int>(targetWorldY) - 720;
    if (scrollY < 0) scrollY = 0;
    if (scrollY > maxScrollY) scrollY = maxScrollY;

    mCamera.SetLocked(false);
    mCamera.SetScrollX(scrollX);
    mCamera.SetScrollY(scrollY);
    mCamera.ResetScrollInit();
}

bool BlockMap::BreakWallAtArea(float left, float top, float right, float bottom)
{
    bool hitAny = false;

    // 破壊可能壁の座標だけを削る条件として渡す
    auto isBreakable = [this](int x, int y) {
        return GetCollisionType(x, y) == CollisionType::BreakableBlock;
        };

    for (auto& wall : mWalls)
    {
        auto broken = wall->BreakArea(left, top, right, bottom, isBreakable);

        // 削れた座標は通り抜け可能にする
        for (const auto& p : broken)
        {
            mCollisionData[GetCollisionIndex(p.x, p.y)] = CollisionType::None;
        }

        if (!broken.empty())
        {
            hitAny = true;
        }
    }

    return hitAny;
}

void BlockMap::ResetCeilingBreak()
{
    for (auto& chunk : mCeilingChunks)
    {
        // 開通させた天井の当たり判定ブロックを元に戻す（リトライ時用）
        if (chunk.hadCollision && !mCollisionData.empty())
        {
            const int startX = static_cast<int>(chunk.origX);
            const int startY = static_cast<int>(chunk.origY);
            for (int py = startY; py < startY + chunk.size; ++py)
            {
                if (py < 0 || py >= mnCollisionHeight) continue;
                for (int px = startX; px < startX + chunk.size; ++px)
                {
                    if (px < 0 || px >= mnCollisionWidth) continue;
                    mCollisionData[GetCollisionIndex(px, py)] = CollisionType::Block;
                }
            }
        }

        if (chunk.graphHandle != -1)
        {
            DeleteGraph(chunk.graphHandle);
            chunk.graphHandle = -1;
        }
    }
    mCeilingChunks.clear();
    if (mCeilingRegionGraph != -1)
    {
        DeleteGraph(mCeilingRegionGraph);
        mCeilingRegionGraph = -1;
    }
    mCeilingDusts.clear();
    mbCeilingBreakActive = false;
    mCeilingHoleLeft = 0;
    mCeilingHoleTop = 0;
    mCeilingHoleRight = 0;
    mCeilingHoleBottom = 0;
    mLatestDragonBottomY = -9999.0f;
}

void BlockMap::InitCeilingBreak(float centerWorldX)
{
    ResetCeilingBreak();
    if (!mbIsLoaded || mnBackgroundGraph == -1) return;

    const int chunkSize = DragonCeilingBreakConstants::ChunkSize;
    const int cols      = DragonCeilingBreakConstants::Cols;
    const int rows      = DragonCeilingBreakConstants::Rows;
    const int regionW   = cols * chunkSize;
    const int regionH   = rows * chunkSize;

    mCeilingBreakCenterX = centerWorldX;
    mCeilingHoleLeft     = static_cast<int>(centerWorldX) - regionW / 2;
    if (mCeilingHoleLeft < 0) mCeilingHoleLeft = 0;
    if (mCeilingHoleLeft + regionW > mnBackgroundWidth)
    {
        mCeilingHoleLeft = (std::max)(0, mnBackgroundWidth - regionW);
    }
    mCeilingHoleRight  = mCeilingHoleLeft + regionW;
    mCeilingHoleTop    = DragonCeilingBreakConstants::CeilingTopY;
    mCeilingHoleBottom = DragonCeilingBreakConstants::CeilingBottomY;
    mLatestDragonBottomY = -9999.0f;

    // 巨大マップ画像は DxLib で分割テクスチャ扱いとなり直接の DerivationGraph が -1 を返すため、
    // 破砕対象領域だけを単一オフスクリーン画像へ転写してから各ブロック片を切り出す
    mCeilingRegionGraph = MakeScreen(regionW, regionH, FALSE);
    if (mCeilingRegionGraph != -1)
    {
        const int prevScreen = GetDrawScreen();
        SetDrawScreen(mCeilingRegionGraph);
        ClearDrawScreen();
        DrawRectGraph(0, 0,
                      mCeilingHoleLeft, mCeilingHoleTop,
                      regionW, regionH,
                      mnBackgroundGraph, FALSE);
        SetDrawScreen(prevScreen);
    }

    mCeilingChunks.reserve(static_cast<size_t>(cols * rows));

    for (int r = 0; r < rows; ++r)
    {
        // 左右外枠列 (0, cols-1) は残し、その内側をランダムにギザギザさせる
        // 壊れる幅: 10〜14列 (240px〜336px) ≒ ドラゴンの体一つ分
        const int leftStartCol = 1 + GetRand(2);
        const int rightEndCol  = (cols - 2) - GetRand(2);

        for (int c = 0; c < cols; ++c)
        {
            CeilingChunk chunk;
            const int origX = mCeilingHoleLeft + c * chunkSize;
            const int origY = mCeilingHoleTop + r * chunkSize;

            chunk.origX = static_cast<float>(origX);
            chunk.origY = static_cast<float>(origY);
            chunk.size  = chunkSize;
            chunk.isTarget = (c >= leftStartCol && c <= rightEndCol);
            chunk.isBroken = false;
            chunk.isFinished = false;
            chunk.isForeground = ((r + c) % 2 == 0);
            chunk.x = chunk.origX + static_cast<float>(chunkSize) * 0.5f;
            chunk.y = chunk.origY + static_cast<float>(chunkSize) * 0.5f;
            chunk.alpha = 255.0f;

            // 中央が先に突き抜けられ、外側はわずかに遅れて崩れる波形
            float distRatio = std::abs(chunk.x - centerWorldX) / (static_cast<float>(regionW) * 0.5f);
            chunk.triggerDelayY = distRatio * 36.0f + static_cast<float>(GetRand(24));

            // 単一オフスクリーン画像から石レンガテクスチャを切り出す
            chunk.graphHandle = (mCeilingRegionGraph != -1)
                ? DerivationGraph(c * chunkSize, r * chunkSize, chunkSize, chunkSize, mCeilingRegionGraph)
                : -1;

            // 破壊対象チャンクは物理当たり判定（天井ブロック）も消去して穴を開通させる
            chunk.hadCollision = false;
            if (chunk.isTarget && !mCollisionData.empty())
            {
                for (int py = origY; py < origY + chunkSize; ++py)
                {
                    if (py < 0 || py >= mnCollisionHeight) continue;
                    for (int px = origX; px < origX + chunkSize; ++px)
                    {
                        if (px < 0 || px >= mnCollisionWidth) continue;
                        const int idx = GetCollisionIndex(px, py);
                        if (mCollisionData[idx] == CollisionType::Block)
                        {
                            chunk.hadCollision = true;
                            mCollisionData[idx] = CollisionType::None;
                        }
                    }
                }
            }

            mCeilingChunks.push_back(chunk);
        }
    }

    mbCeilingBreakActive = true;
}

void BlockMap::ProgressCeilingBreak(float dragonBottomWorldY, float dragonCenterWorldX)
{
    if (!mbCeilingBreakActive) return;

    mLatestDragonBottomY = dragonBottomWorldY;

    for (auto& chunk : mCeilingChunks)
    {
        if (!chunk.isTarget || chunk.isBroken) continue;

        if (dragonBottomWorldY + DragonCeilingBreakConstants::BreakFrontOffsetY - chunk.triggerDelayY >= chunk.origY)
        {
            chunk.isBroken = true;

            const float dirFromCenter = (chunk.x >= dragonCenterWorldX) ? 1.0f : -1.0f;
            const float spreadRatio = std::abs(chunk.x - dragonCenterWorldX) / 180.0f;
            chunk.vx = dirFromCenter * (2.0f + spreadRatio * 4.5f) + (static_cast<float>(GetRand(40) - 20) * 0.12f);
            chunk.vy = 12.0f + static_cast<float>(GetRand(16));
            chunk.rotSpeed = static_cast<float>(GetRand(32) - 16) * 0.01f;

            // 砕けた瞬間の砂煙＆ドラゴンの前方・周囲に散る砕石パーティクルを生成
            for (int d = 0; d < 2; ++d)
            {
                CeilingDust dust;
                dust.x = chunk.x + static_cast<float>(GetRand(16) - 8);
                dust.y = chunk.y + static_cast<float>(GetRand(16) - 8);
                dust.vx = chunk.vx * 0.65f + static_cast<float>(GetRand(30) - 15) * 0.16f;
                dust.vy = chunk.vy * 0.55f + static_cast<float>(GetRand(24) - 10) * 0.2f;
                dust.radius = static_cast<float>(GetRand(6) + 5);
                dust.alpha = 185.0f;
                const int shade = 90 + GetRand(40);
                dust.color = GetColor(shade + 10, shade + 5, shade - 10);
                mCeilingDusts.push_back(dust);
            }

            CeilingDust splinter;
            splinter.x = chunk.x + static_cast<float>(GetRand(12) - 6);
            splinter.y = chunk.y + static_cast<float>(GetRand(12) - 6);
            splinter.vx = chunk.vx * 0.85f + static_cast<float>(GetRand(24) - 12) * 0.15f;
            splinter.vy = chunk.vy * 0.8f + static_cast<float>(GetRand(25)) * 0.18f;
            splinter.radius = static_cast<float>(GetRand(2) + 3);
            splinter.alpha = 225.0f;
            const int rockShade = 65 + GetRand(35);
            splinter.color = GetColor(rockShade + 8, rockShade + 4, rockShade);
            mCeilingDusts.push_back(splinter);
        }
    }
}

void BlockMap::UpdateCeilingBreak()
{
    if (!mbCeilingBreakActive) return;

    const float halfChunk = static_cast<float>(DragonCeilingBreakConstants::ChunkSize) * 0.5f;

    for (auto& chunk : mCeilingChunks)
    {
        if (!chunk.isBroken || chunk.isFinished) continue;

        chunk.x += chunk.vx;
        chunk.vy += DragonCeilingBreakConstants::ChunkGravity;
        chunk.y += chunk.vy;
        chunk.angle += chunk.rotSpeed;

        // ボス部屋床（Y=1903）との衝突・バウンド処理
        if (chunk.y + halfChunk >= DragonCeilingBreakConstants::FloorY)
        {
            chunk.y = DragonCeilingBreakConstants::FloorY - halfChunk;
            if (chunk.bounceCount == 0 && chunk.vy > 4.0f)
            {
                chunk.vy = -chunk.vy * (0.22f + static_cast<float>(GetRand(15)) * 0.01f);
                chunk.vx *= 0.65f;
                chunk.rotSpeed *= 0.5f;
                chunk.bounceCount++;

                CeilingDust dust;
                dust.x = chunk.x;
                dust.y = DragonCeilingBreakConstants::FloorY - 4.0f;
                dust.vx = static_cast<float>(GetRand(40) - 20) * 0.1f;
                dust.vy = -static_cast<float>(GetRand(20) + 10) * 0.15f;
                dust.radius = static_cast<float>(GetRand(5) + 6);
                dust.alpha = 160.0f;
                const int shade = 100 + GetRand(35);
                dust.color = GetColor(shade + 10, shade + 5, shade - 10);
                mCeilingDusts.push_back(dust);
            }
            else
            {
                chunk.vy = 0.0f;
                chunk.vx *= 0.85f;
                chunk.rotSpeed = 0.0f;
                chunk.alpha -= 12.0f;
                if (chunk.alpha <= 0.0f)
                {
                    chunk.alpha = 0.0f;
                    chunk.isFinished = true;
                }
            }
        }
    }

    // 砂煙・岩粉パーティクルの更新
    for (auto it = mCeilingDusts.begin(); it != mCeilingDusts.end(); )
    {
        it->x += it->vx;
        it->y += it->vy;
        it->vy += 0.18f;
        it->vx *= 0.96f;
        it->radius += 0.25f;
        it->alpha -= 6.5f;

        if (it->alpha <= 0.0f)
        {
            it = mCeilingDusts.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void BlockMap::DrawBrokenCeilingChunk(const CeilingChunk& chunk, int sx, int sy) const
{
    if (chunk.alpha < 255.0f)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(chunk.alpha));
    }

    if (chunk.graphHandle != -1)
    {
        DrawRotaGraph(static_cast<int>(chunk.x) - sx,
                      static_cast<int>(chunk.y) - sy,
                      1.0,
                      static_cast<double>(chunk.angle),
                      chunk.graphHandle,
                      FALSE);
    }
    else if (mnBackgroundGraph != -1)
    {
        // 万一オフスクリーン派生に失敗した場合も元マップから直接矩形描画で破片を表示する
        const int halfSize = chunk.size / 2;
        DrawRectGraph(static_cast<int>(chunk.x) - halfSize - sx,
                      static_cast<int>(chunk.y) - halfSize - sy,
                      static_cast<int>(chunk.origX),
                      static_cast<int>(chunk.origY),
                      chunk.size,
                      chunk.size,
                      mnBackgroundGraph,
                      TRUE);
    }

    if (chunk.alpha < 255.0f)
    {
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

void BlockMap::DrawCeilingForegroundDebris() const
{
    if (!mbCeilingBreakActive) return;

    const int sx = mCamera.GetDrawScrollX();
    const int sy = mCamera.GetDrawScrollY();

    // 1. ドラゴンの手前に描く回転落下チャンク
    for (const auto& chunk : mCeilingChunks)
    {
        if (!chunk.isBroken || chunk.isFinished || !chunk.isForeground) continue;
        DrawBrokenCeilingChunk(chunk, sx, sy);
    }

    // 2. 砂煙・岩粉パーティクル
    for (const auto& d : mCeilingDusts)
    {
        if (d.alpha <= 0.0f) continue;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(d.alpha));
        DrawCircle(static_cast<int>(d.x) - sx, static_cast<int>(d.y) - sy, static_cast<int>(d.radius), d.color, TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

#include "BlockMap.h"
#include "Master.h"
#include "GameConstants.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include "Camera.h"

void BlockMap::UpdateDustParticles(std::vector<CeilingDust>& dusts, float gravity, float fadeSpeed)
{
    for (auto it = dusts.begin(); it != dusts.end(); )
    {
        it->x += it->vx;
        it->y += it->vy;
        it->vy += gravity;
        it->vx *= 0.96f;
        it->radius += 0.25f;
        it->alpha -= fadeSpeed;

        if (it->alpha <= 0.0f)
        {
            it = dusts.erase(it);
        }
        else
        {
            ++it;
        }
    }
}


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
    , mbHasBreakableBounds(false)
    , mnBreakableMinX(0)
    , mnBreakableMinY(0)
    , mnBreakableMaxX(0)
    , mnBreakableMaxY(0)
    , mnBackgroundWidth(0)
    , mnBackgroundHeight(0)
    , mfMoveDirection(0.0f)
    , mbIsScrolling(false)
{
    //mSpawnPos = initPos;
}

BlockMap::~BlockMap()
{
    Unload();
}

void BlockMap::Unload()
{
    ResetCeilingBreak();
    ResetOpeningPitBreak();

    if (mnCollisionSoftImage != -1)
    {
        DeleteSoftImage(mnCollisionSoftImage);
        mnCollisionSoftImage = -1;
    }

    mCollisionData.clear();
    mWalls.clear();

    mnBackgroundGraph = -1;
    mnBackgroundWidth = 0;
    mnBackgroundHeight = 0;
    mnCollisionWidth = 0;
    mnCollisionHeight = 0;
    mbIsLoaded = false;
    mbHasBossArea = false;
    mnBossAreaLeft = 0;
    mnBossAreaRight = 0;
    mbHasBreakableBounds = false;
    mnBreakableMinX = 0;
    mnBreakableMinY = 0;
    mnBreakableMaxX = 0;
    mnBreakableMaxY = 0;
    mbHasBreakableBounds = false;
    mnBreakableMinX = 0;
    mnBreakableMinY = 0;
    mnBreakableMaxX = 0;
    mnBreakableMaxY = 0;
    mbIsScrolling = false;
    mfMoveDirection = 0.0f;

    mCamera.SetScrollX(0);
    mCamera.SetScrollY(0);
    mCamera.SetLocked(false);
    mCamera.SetVerticalLocked(false);
    mCamera.SetRightScrollExtra(0);
    mCamera.ResetScrollInit();
    mDebris.clear();
    mShakeX = 0;
    mShakeY = 0;
}

void BlockMap::SetAllowScrollPastRight(bool allow)
{
    mCamera.SetRightScrollExtra(allow ? CaveOpeningConstants::PostBossRightScrollExtra : 0);
}

// マップ読み込み
bool BlockMap::Load(
    const std::string& backgroundPath,
    const std::string& collisionPath)
{
    if (mbIsLoaded) { return true; }
    ResetCeilingBreak();
    ResetOpeningPitBreak();

    // 破壊可能壁の読み込み（洞窟本編 CaveMap 専用：Cave_Opening には読み込まない）
    mWalls.clear();
    if (backgroundPath.find("Stage1-2") != std::string::npos || backgroundPath.find("CaveMap") != std::string::npos)
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
    mbHasBreakableBounds = false;
    mnBreakableMinX = 0;
    mnBreakableMinY = 0;
    mnBreakableMaxX = 0;
    mnBreakableMaxY = 0;

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

                if (!mbHasBreakableBounds)
                {
                    mbHasBreakableBounds = true;
                    mnBreakableMinX = x;
                    mnBreakableMaxX = x;
                    mnBreakableMinY = y;
                    mnBreakableMaxY = y;
                }
                else
                {
                    if (x < mnBreakableMinX) { mnBreakableMinX = x; }
                    if (x > mnBreakableMaxX) { mnBreakableMaxX = x; }
                    if (y < mnBreakableMinY) { mnBreakableMinY = y; }
                    if (y > mnBreakableMaxY) { mnBreakableMaxY = y; }
                }

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


void BlockMap::DrawBackgroundWithCutout(
    int sx, int sy, int sw, int sh,
    int holeLeft, int holeTop, int holeRight, int holeBottom) const
{
    const int cx1 = (std::max)(holeLeft, sx);
    const int cx2 = (std::min)(holeRight, sx + sw);
    const int cy1 = (std::max)(holeTop, sy);
    const int cy2 = (std::min)(holeBottom, sy + sh);

    if (cx1 >= cx2 || cy1 >= cy2)
    {
        DrawRectGraph(0, 0, sx, sy, sw, sh, mnBackgroundGraph, TRUE);
        return;
    }

    // くり抜き領域を除いた周囲4帯を切り出して描画（穴の奥に背景が透けて見える）
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

// 背景描画
void BlockMap::Draw()
{
    if (!mbIsLoaded) { return; }

    const int sx = GetDrawScrollX();
    const int sy = GetDrawScrollY();
    int sw = ScreenSize::ScrrenWidth;
    int sh = ScreenSize::ScrrenHeight;
    const int curScreen = GetDrawScreen();
    if (curScreen > 0)
    {
        GetGraphSize(curScreen, &sw, &sh);
    }
    const int drawSw = (std::max)(0, (std::min)(sw, mnBackgroundWidth - sx));

    if (drawSw > 0)
    {
        if (mbCeilingBreakActive)
        {
            DrawBackgroundWithCutout(sx, sy, drawSw, sh, mCeilingHoleLeft, mCeilingHoleTop, mCeilingHoleRight, mCeilingHoleBottom);
        }
        else if (mbOpeningMapMode && mbOpeningPitBroken)
        {
            DrawBackgroundWithCutout(sx, sy, drawSw, sh, mOpeningPitLeft, mOpeningPitTop, mOpeningPitRight + 1, mOpeningPitBottom + 1);
        }
        else
        {
            // 通常時：背景画像を1回で描画
            DrawRectGraph(0, 0, sx, sy, drawSw, sh, mnBackgroundGraph, TRUE);
        }
    }

    if (mbOpeningMapMode)
    {
        DrawOpeningExitLight(sx, sy);

        if (mbOpeningPitBroken)
        {
            for (const auto& chunk : mOpeningPitChunks)
            {
                if (!chunk.isFinished)
                {
                    DrawBrokenCeilingChunk(chunk, sx, sy);
                }
            }
            for (const auto& d : mOpeningPitDusts)
            {
                if (d.alpha <= 0.0f) continue;
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(d.alpha));
                DrawCircle(static_cast<int>(d.x) - sx, static_cast<int>(d.y) - sy, static_cast<int>(d.radius), d.color, TRUE);
            }
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
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

    // ギミック破片パーティクルの描画
    if (!mDebris.empty())
    {
        for (const auto& d : mDebris)
        {
            if (d.life <= 0) continue;
            int alpha = (d.maxLife > 0) ? (255 * d.life) / d.maxLife : 255;
            if (alpha < 0) alpha = 0;
            if (alpha > 255) alpha = 255;
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            int dx = static_cast<int>(d.x) - sx;
            int dy = static_cast<int>(d.y) - sy;
            int half = d.size / 2;
            DrawBox(dx - half, dy - half, dx + half, dy + half, d.color, TRUE);
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
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

    for (auto it = mDebris.begin(); it != mDebris.end(); )
    {
        it->x += it->vx;
        it->y += it->vy;
        it->vy += 0.35f;
        it->life--;
        if (it->life <= 0)
        {
            it = mDebris.erase(it);
        }
        else
        {
            ++it;
        }
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

void BlockMap::ClearCollisionArea(int minX, int minY, int maxX, int maxY)
{
    if (maxX >= minX && maxY >= minY)
    {
        SetCollisionBlock(minX, minY, maxX - minX + 1, maxY - minY + 1, CollisionType::None);
    }
}

void BlockMap::FillCollisionArea(int minX, int minY, int maxX, int maxY, CollisionType type)
{
    if (maxX >= minX && maxY >= minY)
    {
        SetCollisionBlock(minX, minY, maxX - minX + 1, maxY - minY + 1, type);
    }
}

void BlockMap::ResetScroll()
{
    mCamera.SetScrollX(0);
    mCamera.SetScrollY(0);
    mCamera.SetLocked(false);
    mCamera.SetVerticalLocked(false);
    mCamera.ResetScrollInit();
    mDebris.clear();
    mShakeX = 0;
    mShakeY = 0;
    ResetCeilingBreak();
}

void BlockMap::SnapCameraToPosition(float targetWorldX, float targetWorldY)
{
    // プレイヤーがカメラスクロール領域（ScrollStartLeftX〜ScrollStartRightX）の中央（800px）に収まるようにscrollXを算出
    const int targetScreenX = (MapScrollConstants::ScrollStartLeftX + MapScrollConstants::ScrollStartRightX) / 2;
    int scrollX = static_cast<int>(targetWorldX) - targetScreenX;
    int maxScrollX = (std::max)(0, (mnBackgroundWidth + mCamera.GetRightScrollExtra()) - ScreenSize::ScrrenWidth);
    if (scrollX < 0) scrollX = 0;
    if (scrollX > maxScrollX) scrollX = maxScrollX;

    // 縦スクロール (targetWorldY - 720)
    int maxScrollY = (std::max)(0, mnBackgroundHeight - ScreenSize::ScrrenHeight);
    int scrollY = static_cast<int>(targetWorldY) - 720;
    if (scrollY < 0) scrollY = 0;
    if (scrollY > maxScrollY) scrollY = maxScrollY;

    mCamera.SetLocked(false);
    mCamera.SetMaxScroll(maxScrollX, maxScrollY);
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
            chunk.width = chunkSize;
            chunk.height = chunkSize;
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

void BlockMap::InitBrokenCeilingHoleImmediate(float centerWorldX)
{
    InitCeilingBreak(centerWorldX);
    if (!mbCeilingBreakActive) return;

    mLatestDragonBottomY = -9999.0f;
    for (auto& chunk : mCeilingChunks)
    {
        if (chunk.isTarget)
        {
            chunk.isBroken = true;
            chunk.isFinished = true;
            chunk.alpha = 0.0f;
        }
    }
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
    UpdateDustParticles(mCeilingDusts, 0.18f, 6.5f);
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
                      TRUE);
    }
    else if (mnBackgroundGraph != -1)
    {
        // 万一オフスクリーン派生に失敗した場合も元マップから直接矩形描画で破片を表示する
        const int halfW = chunk.width / 2;
        const int halfH = chunk.height / 2;
        DrawRectGraph(static_cast<int>(chunk.x) - halfW - sx,
                      static_cast<int>(chunk.y) - halfH - sy,
                      static_cast<int>(chunk.origX),
                      static_cast<int>(chunk.origY),
                      chunk.width,
                      chunk.height,
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

void BlockMap::ResetOpeningPitBreak()
{
    for (auto& chunk : mOpeningPitChunks)
    {
        if (chunk.graphHandle != -1)
        {
            DeleteGraph(chunk.graphHandle);
            chunk.graphHandle = -1;
        }
    }
    mOpeningPitChunks.clear();

    if (mOpeningPitRegionGraph != -1)
    {
        DeleteGraph(mOpeningPitRegionGraph);
        mOpeningPitRegionGraph = -1;
    }

    mOpeningPitDusts.clear();
    mOpeningLightDusts.clear();
    mbOpeningMapMode = false;
    mbOpeningPitBroken = false;
    mOpeningPitLeft = 0;
    mOpeningPitTop = 0;
    mOpeningPitRight = 0;
    mOpeningPitBottom = 0;
    mOpeningLightTimer = 0;
}

void BlockMap::InitOpeningPitBreak()
{
    ResetOpeningPitBreak();
    if (!mbIsLoaded || mnBackgroundGraph == -1) return;

    mbOpeningMapMode = true;
    mbOpeningPitBroken = false;
    mOpeningLightTimer = 0;

    if (mbHasBreakableBounds)
    {
        mOpeningPitLeft = mnBreakableMinX;
        mOpeningPitTop = mnBreakableMinY;
        mOpeningPitRight = mnBreakableMaxX;
        mOpeningPitBottom = mnBreakableMaxY;
    }
    else
    {
        mOpeningPitLeft = CaveOpeningConstants::PitLeftX;
        mOpeningPitTop = CaveOpeningConstants::PitTopY;
        mOpeningPitRight = CaveOpeningConstants::PitRightX;
        mOpeningPitBottom = CaveOpeningConstants::PitBottomY;
    }

    // 右端の出口風ライト内に漂う塵パーティクルを初期化
    const float lightStartX = (std::max)(0.0f, static_cast<float>(mnBackgroundWidth) - CaveOpeningConstants::ExitLightWidth + 40.0f);
    const float lightWidth = (std::max)(60.0f, static_cast<float>(mnBackgroundWidth) - lightStartX - 10.0f);
    const float minY = static_cast<float>(CaveOpeningConstants::CorridorTopY + 18);
    const float heightY = static_cast<float>((CaveOpeningConstants::CorridorBottomY - 18) - (CaveOpeningConstants::CorridorTopY + 18));

    mOpeningLightDusts.reserve(CaveOpeningConstants::ExitLightDustCount);
    for (int i = 0; i < CaveOpeningConstants::ExitLightDustCount; ++i)
    {
        OpeningLightDust d;
        d.x = lightStartX + (static_cast<float>(GetRand(1000)) / 1000.0f) * lightWidth;
        d.y = minY + (static_cast<float>(GetRand(1000)) / 1000.0f) * heightY;
        d.vx = -0.18f - (static_cast<float>(GetRand(25)) * 0.01f);
        d.vy = (static_cast<float>(GetRand(20) - 10) * 0.015f);
        d.phase = static_cast<float>(GetRand(628)) * 0.01f;
        d.size = (GetRand(2) == 0) ? 4 : 3;
        mOpeningLightDusts.push_back(d);
    }
}

void BlockMap::SetOpeningPitAlreadyBroken()
{
    InitOpeningPitBreak();
    if (!mbOpeningMapMode) return;

    mbOpeningPitBroken = true;
    const int pitW = (mOpeningPitRight - mOpeningPitLeft) + 1;
    const int pitH = (mOpeningPitBottom - mOpeningPitTop) + 1;
    SetCollisionBlock(mOpeningPitLeft, mOpeningPitTop, pitW, pitH, CollisionType::None);
}

bool BlockMap::CheckAndTriggerOpeningPit(
    float playerX, float playerY,
    float playerWidth, float playerHeight,
    bool isGrounded, float velocityY)
{
    if (!mbOpeningMapMode || mbOpeningPitBroken) return false;

    const float halfW = playerWidth * 0.5f;
    const float halfH = playerHeight * 0.5f;
    const float footY = playerY + halfH;
    const float pitTopF = static_cast<float>(mOpeningPitTop);

    // 左端の赤ブロック(<=PitLeft-1)へ足を引っ掛けず確実に落下させるため、足幅が完全に穴範囲へ乗った瞬間に即時崩壊させる
    const bool horizontallyOnPit =
        (playerX >= static_cast<float>(mOpeningPitLeft) + halfW) &&
        (playerX <= static_cast<float>(mOpeningPitRight) - halfW);

    const bool steppingOnSurface =
        (isGrounded || velocityY >= 0.0f) &&
        (footY >= pitTopF - 4.0f) &&
        (footY <= pitTopF + 20.0f);

    if (horizontallyOnPit && steppingOnSurface)
    {
        TriggerOpeningPitBreak(playerX);
        return true;
    }
    return false;
}

void BlockMap::TriggerOpeningPitBreak(float playerWorldX)
{
    if (!mbOpeningMapMode || mbOpeningPitBroken) return;
    if (!mbIsLoaded || mnBackgroundGraph == -1) return;

    mbOpeningPitBroken = true;

    // 対象領域の物理当たり判定（BreakableBlock）を即時すべて削除する
    const int pitW = (mOpeningPitRight - mOpeningPitLeft) + 1;
    const int pitH = (mOpeningPitBottom - mOpeningPitTop) + 1;
    SetCollisionBlock(mOpeningPitLeft, mOpeningPitTop, pitW, pitH, CollisionType::None);

    // 背景マップ画像から床領域をオフスクリーン転写し、タイル状の落下破片を切り出す
    mOpeningPitRegionGraph = MakeScreen(pitW, pitH, TRUE);
    if (mOpeningPitRegionGraph != -1)
    {
        const int prevScreen = GetDrawScreen();
        SetDrawScreen(mOpeningPitRegionGraph);
        ClearDrawScreen();
        DrawRectGraph(0, 0,
                      mOpeningPitLeft, mOpeningPitTop,
                      pitW, pitH,
                      mnBackgroundGraph, TRUE);
        SetDrawScreen(prevScreen);
    }

    const int chunkSize = CaveOpeningConstants::PitChunkSize;
    int row = 0;
    for (int py = mOpeningPitTop; py <= mOpeningPitBottom; py += chunkSize, ++row)
    {
        const int tileH = (std::min)(chunkSize, (mOpeningPitBottom + 1) - py);
        for (int px = mOpeningPitLeft; px <= mOpeningPitRight; px += chunkSize)
        {
            const int tileW = (std::min)(chunkSize, (mOpeningPitRight + 1) - px);
            CeilingChunk chunk;
            chunk.origX = static_cast<float>(px);
            chunk.origY = static_cast<float>(py);
            chunk.size = chunkSize;
            chunk.width = tileW;
            chunk.height = tileH;
            chunk.isTarget = true;
            chunk.isBroken = true;
            chunk.isFinished = false;
            chunk.x = chunk.origX + static_cast<float>(tileW) * 0.5f;
            chunk.y = chunk.origY + static_cast<float>(tileH) * 0.5f;
            chunk.alpha = 255.0f;

            const float dirFromPlayer = (chunk.x >= playerWorldX) ? 1.0f : -1.0f;
            const float spread = static_cast<float>(GetRand(100)) * 0.01f;

            if (row <= 1)
            {
                // 表面の破片は左右へ弾けながら崩落（CrumbleFloor 風の手応え）
                chunk.vx = dirFromPlayer * (1.2f + spread * 3.0f);
                chunk.vy = -2.2f + static_cast<float>(row) * 1.8f + (static_cast<float>(GetRand(20) - 10) * 0.1f);
                chunk.rotSpeed = dirFromPlayer * (0.03f + spread * 0.06f);
            }
            else
            {
                chunk.vx = dirFromPlayer * (0.5f + spread * 1.8f);
                chunk.vy = 2.0f + static_cast<float>(row) * 0.85f;
                chunk.rotSpeed = dirFromPlayer * (0.015f + spread * 0.04f);
            }

            chunk.graphHandle = (mOpeningPitRegionGraph != -1)
                ? DerivationGraph(px - mOpeningPitLeft, py - mOpeningPitTop, tileW, tileH, mOpeningPitRegionGraph)
                : -1;

            mOpeningPitChunks.push_back(chunk);
        }
    }

    // 表面の砂煙パーティクルを生成
    for (int i = 0; i < 28; ++i)
    {
        CeilingDust dust;
        dust.x = static_cast<float>(mOpeningPitLeft + GetRand(pitW));
        dust.y = static_cast<float>(mOpeningPitTop + GetRand(36) - 6);
        dust.vx = static_cast<float>(GetRand(40) - 20) * 0.14f;
        dust.vy = -static_cast<float>(GetRand(22) + 4) * 0.16f;
        dust.radius = static_cast<float>(GetRand(7) + 6);
        dust.alpha = 185.0f;
        const int shade = 95 + GetRand(35);
        dust.color = GetColor(shade + 10, shade + 6, shade - 8);
        mOpeningPitDusts.push_back(dust);
    }

    if (Master::mpGameManager && Master::mpGameManager->GetSoundManager())
    {
        Master::mpGameManager->GetSoundManager()->PlaySE(SoundManager::SE_STAGE2_BREAKABLE_WALL);
    }
    StartCameraShake(CaveOpeningConstants::PitBreakShakePower, 0.7f, 1.0f, CaveOpeningConstants::PitBreakShakeFrames);
}

void BlockMap::UpdateOpeningPitBreak()
{
    if (!mbOpeningMapMode) return;

    mOpeningLightTimer++;

    const float lightLeft = (std::max)(0.0f, static_cast<float>(mnBackgroundWidth) - CaveOpeningConstants::ExitLightWidth + 20.0f);
    const float lightRight = static_cast<float>(mnBackgroundWidth) - 8.0f;
    const float topBound = static_cast<float>(CaveOpeningConstants::CorridorTopY + 14);
    const float botBound = static_cast<float>(CaveOpeningConstants::CorridorBottomY - 14);

    for (auto& d : mOpeningLightDusts)
    {
        d.x += d.vx;
        d.y += d.vy + std::sinf(static_cast<float>(mOpeningLightTimer) * 0.03f + d.phase) * 0.12f;
        if (d.x < lightLeft)
        {
            d.x = lightRight;
            d.y = topBound + (static_cast<float>(GetRand(1000)) / 1000.0f) * (botBound - topBound);
        }
        if (d.y < topBound) d.y = botBound;
        if (d.y > botBound) d.y = topBound;
    }

    if (!mbOpeningPitBroken) return;

    for (auto& chunk : mOpeningPitChunks)
    {
        if (chunk.isFinished) continue;

        chunk.vx *= 0.99f;
        chunk.vy += CaveOpeningConstants::PitChunkGravity;
        if (chunk.vy > 28.0f) chunk.vy = 28.0f;

        chunk.x += chunk.vx;
        chunk.y += chunk.vy;
        chunk.angle += chunk.rotSpeed;

        if (chunk.y > static_cast<float>(mnBackgroundHeight) + 60.0f)
        {
            chunk.alpha -= 20.0f;
            if (chunk.alpha <= 0.0f)
            {
                chunk.alpha = 0.0f;
                chunk.isFinished = true;
            }
        }
    }

    UpdateDustParticles(mOpeningPitDusts, 0.14f, 5.5f);
}

void BlockMap::DrawOpeningExitLight(int sx, int sy) const
{
    if (!mbOpeningMapMode || mnBackgroundWidth <= 0) return;

    const float exitWorldX = static_cast<float>(mnBackgroundWidth);
    const float lightStartWorldX = (std::max)(0.0f, exitWorldX - CaveOpeningConstants::ExitLightWidth);
    if (exitWorldX < static_cast<float>(sx) || lightStartWorldX > static_cast<float>(sx + ScreenSize::ScrrenWidth))
    {
        return;
    }

    const int topY = CaveOpeningConstants::CorridorTopY - sy;
    const int bottomY = (CaveOpeningConstants::CorridorBottomY + 1) - sy;
    const float pulse = 0.92f + 0.08f * std::sinf(static_cast<float>(mOpeningLightTimer) * 0.04f);

    // 1. 洞窟出口から通路内へ広がる柔らかい外光グラデーション
    const int hazeStrips = 24;
    const float hazeWidth = exitWorldX - lightStartWorldX;
    for (int i = 0; i < hazeStrips; ++i)
    {
        const float t0 = static_cast<float>(i) / static_cast<float>(hazeStrips);
        const float t1 = static_cast<float>(i + 1) / static_cast<float>(hazeStrips);
        const float tMid = (t0 + t1) * 0.5f;

        const int x0 = static_cast<int>(lightStartWorldX + hazeWidth * t0) - sx;
        const int x1 = static_cast<int>(lightStartWorldX + hazeWidth * t1) - sx;
        const int alpha = static_cast<int>(tMid * tMid * 95.0f * pulse);
        if (alpha <= 0 || x1 <= x0) continue;

        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
        DrawBox(x0, topY, x1, bottomY, GetColor(255, 244, 198), TRUE);
    }

    // 2. 斜めに差し込む光条（ゴッドレイ）
    const int rayColor = GetColor(255, 242, 185);
    const int exitScreenX = static_cast<int>(exitWorldX) - sx;

    const int ray1Alpha = static_cast<int>((22.0f + 6.0f * std::sinf(static_cast<float>(mOpeningLightTimer) * 0.03f)) * pulse);
    SetDrawBlendMode(DX_BLENDMODE_ADD, ray1Alpha);
    DrawQuadrangle(
        static_cast<int>(exitWorldX - 240.0f) - sx, topY,
        static_cast<int>(exitWorldX - 110.0f) - sx, topY,
        static_cast<int>(exitWorldX - 490.0f) - sx, bottomY,
        static_cast<int>(exitWorldX - 650.0f) - sx, bottomY,
        rayColor, TRUE);

    const int ray2Alpha = static_cast<int>((30.0f + 7.0f * std::sinf(static_cast<float>(mOpeningLightTimer) * 0.045f + 1.4f)) * pulse);
    SetDrawBlendMode(DX_BLENDMODE_ADD, ray2Alpha);
    DrawQuadrangle(
        static_cast<int>(exitWorldX - 115.0f) - sx, topY,
        exitScreenX, topY,
        static_cast<int>(exitWorldX - 315.0f) - sx, bottomY,
        static_cast<int>(exitWorldX - 470.0f) - sx, bottomY,
        rayColor, TRUE);

    const int ray3Alpha = static_cast<int>((34.0f + 6.0f * std::sinf(static_cast<float>(mOpeningLightTimer) * 0.05f + 2.8f)) * pulse);
    SetDrawBlendMode(DX_BLENDMODE_ADD, ray3Alpha);
    DrawQuadrangle(
        exitScreenX, topY + 40,
        exitScreenX, topY + 210,
        static_cast<int>(exitWorldX - 105.0f) - sx, bottomY,
        static_cast<int>(exitWorldX - 285.0f) - sx, bottomY,
        rayColor, TRUE);

    // 3. 右端出口境界の強い輝きコア
    const int coreStrips = 10;
    const float coreStartX = (std::max)(0.0f, exitWorldX - CaveOpeningConstants::ExitLightCoreWidth);
    const float coreW = exitWorldX - coreStartX;
    for (int i = 0; i < coreStrips; ++i)
    {
        const float t0 = static_cast<float>(i) / static_cast<float>(coreStrips);
        const float t1 = static_cast<float>(i + 1) / static_cast<float>(coreStrips);
        const float tMid = (t0 + t1) * 0.5f;

        const int x0 = static_cast<int>(coreStartX + coreW * t0) - sx;
        const int x1 = static_cast<int>(coreStartX + coreW * t1) - sx;
        const int alpha = static_cast<int>(tMid * tMid * 155.0f * pulse);
        if (alpha <= 0 || x1 <= x0) continue;

        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
        DrawBox(x0, topY, x1, bottomY, GetColor(255, 252, 232), TRUE);
    }

    // 4. 光の中に浮かぶ塵パーティクル
    for (const auto& d : mOpeningLightDusts)
    {
        const float distRatio = (std::max)(0.0f, (std::min)(1.0f, (d.x - lightStartWorldX) / hazeWidth));
        const float twinkle = 0.55f + 0.45f * std::sinf(static_cast<float>(mOpeningLightTimer) * 0.06f + d.phase);
        const int alpha = static_cast<int>(150.0f * distRatio * twinkle);
        if (alpha <= 0) continue;

        const int dx = static_cast<int>(d.x) - sx;
        const int dy = static_cast<int>(d.y) - sy;
        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
        DrawBox(dx, dy, dx + d.size, dy + d.size, GetColor(255, 248, 200), TRUE);
    }

    // 5. 右端（X >= 4176）より外側の空間へ続くような明かり（ボスクリア後のフェイク出口空間）
    if (mCamera.GetRightScrollExtra() > 0 && exitScreenX < ScreenSize::ScrrenWidth)
    {
        const int outsideLeft = (std::max)(0, exitScreenX);
        const int outsideW = ScreenSize::ScrrenWidth - outsideLeft;
        const int outsideStrips = 12;
        for (int i = 0; i < outsideStrips; ++i)
        {
            const float t = 1.0f - (static_cast<float>(i) / static_cast<float>(outsideStrips));
            const int x0 = outsideLeft + (outsideW * i) / outsideStrips;
            const int x1 = outsideLeft + (outsideW * (i + 1)) / outsideStrips;
            const int alpha = static_cast<int>((45.0f + 135.0f * t * t) * pulse);
            if (x1 <= x0 || alpha <= 0) continue;

            SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
            DrawBox(x0, 0, x1, ScreenSize::ScrrenHeight, GetColor(255, 250, 224), TRUE);
        }
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

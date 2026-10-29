#include "DestructibleWall.h"
#include <algorithm>
#include <cmath>
#include <utility>

DestructibleWall::~DestructibleWall()
{
    Release();
}

void DestructibleWall::Release()
{
    if (mSoftImage != -1)
    {
        DeleteSoftImage(mSoftImage);
        mSoftImage = -1;
    }
    if (mGraphHandle != -1)
    {
        DeleteGraph(mGraphHandle);
        mGraphHandle = -1;
    }
    mBlockEffect.Clear();
    mIsDirty = false;
}

bool DestructibleWall::Load(int minX, int minY, int maxX, int maxY, const std::string& imagePath)
{
    Release();

    mMinX = minX;
    mMinY = minY;
    mMaxX = maxX;
    mMaxY = maxY;

    mSoftImage = LoadSoftImage(imagePath.c_str());
    if (mSoftImage == -1) return false;

    GetSoftImageSize(mSoftImage, &mImageWidth, &mImageHeight);

    mGraphHandle = CreateGraphFromSoftImage(mSoftImage);
    if (mGraphHandle == -1)
    {
        Release();
        return false;
    }
    return true;
}

std::vector<DestructibleWall::Point> DestructibleWall::BreakArea(
    float left, float top, float right, float bottom,
    const std::function<bool(int, int)>& isBreakable)
{
    std::vector<Point> broken;
    if (mSoftImage == -1) return broken;

    // 攻撃範囲と壁の範囲の重なりを求める
    const int startX = (std::max)(static_cast<int>(std::floor(left)), mMinX);
    const int endX = (std::min)(static_cast<int>(std::floor(right)), mMaxX);
    const int startY = (std::max)(static_cast<int>(std::floor(top)), mMinY);
    const int endY = (std::min)(static_cast<int>(std::floor(bottom)), mMaxY);
    if (startX > endX || startY > endY) return broken;

    for (int y = startY; y <= endY; ++y)
    {
        for (int x = startX; x <= endX; ++x)
        {
            const int lx = x - mMinX;
            const int ly = y - mMinY;
            if (lx < 0 || ly < 0 || lx >= mImageWidth || ly >= mImageHeight) continue;

            // 当たり判定が破壊可能壁でない場所は削らない
            if (!isBreakable(x, y)) continue;

            int r = 0, g = 0, b = 0, a = 0;
            if (GetPixelSoftImage(mSoftImage, lx, ly, &r, &g, &b, &a) != 0) continue;
            if (a == 0) continue; // すでに削れている

            DrawPixelSoftImage(mSoftImage, lx, ly, 0, 0, 0, 0); // 完全透明にする
            broken.push_back({ x, y });
        }
    }

    if (!broken.empty())
    {
        mIsDirty = true;

        // 破片の生成を BlockEffect に任せる
        mBlockEffect.Spawn(
            startX,
            startY,
            endX - startX + 1,
            endY - startY + 1,
            static_cast<int>(broken.size())
        );
    }

    return broken;
}

void DestructibleWall::Update()
{
    // 1フレームに何度削られても、テクスチャ再転送は1回だけ行う
    if (mIsDirty && mSoftImage != -1 && mGraphHandle != -1)
    {
        ReCreateGraphFromSoftImage(mSoftImage, mGraphHandle);
        mIsDirty = false;
    }

    // 破片エフェクトの更新を BlockEffect に任せる
    mBlockEffect.Update();
}

void DestructibleWall::Draw(int scrollX, int scrollY) const
{
    if (mGraphHandle != -1)
    {
        DrawGraph(mMinX - scrollX, mMinY - scrollY, mGraphHandle, TRUE);
    }

    // 破片エフェクトの描画を BlockEffect に任せる
    mBlockEffect.Draw(scrollX, scrollY);
}
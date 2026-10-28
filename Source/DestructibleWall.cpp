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
    mDebris.clear();
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
        SpawnDebris(startX, startY,
            endX - startX + 1, endY - startY + 1,
            static_cast<int>(broken.size()));
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

    for (auto it = mDebris.begin(); it != mDebris.end(); )
    {
        it->x += it->vx;
        it->y += it->vy;
        it->vy += 0.4f;   // 重力
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

void DestructibleWall::Draw(int scrollX, int scrollY) const
{
    if (mGraphHandle != -1)
    {
        DrawGraph(mMinX - scrollX, mMinY - scrollY, mGraphHandle, TRUE);
    }

    for (const auto& d : mDebris)
    {
        const int dx = static_cast<int>(d.x) - scrollX;
        const int dy = static_cast<int>(d.y) - scrollY;
        DrawBox(dx, dy, dx + d.size, dy + d.size, d.color, TRUE);
    }
}

void DestructibleWall::SpawnDebris(int startX, int startY, int w, int h, int brokenCount)
{
    const int numDebris = (std::min)(brokenCount / 20 + 8, 40);
    for (int i = 0; i < numDebris; ++i)
    {
        Debris d;
        d.x = static_cast<float>(startX + GetRand(w));
        d.y = static_cast<float>(startY + GetRand(h));
        d.vx = (GetRand(20) - 10) * 1.0f;
        d.vy = (GetRand(18) - 14) * 1.0f;
        d.size = GetRand(8) + 4;
        d.life = GetRand(30) + 20;
        const int c = GetRand(40) + 60;
        d.color = GetColor(c + 15, c, c - 15);
        mDebris.push_back(d);
    }
}
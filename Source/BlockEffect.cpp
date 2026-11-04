#include "BlockEffect.h"
#include <algorithm>

void BlockEffect::Spawn(
    int startX,
    int startY,
    int width,
    int height,
    int brokenCount)
{
    if (width <= 0 || height <= 0 || brokenCount <= 0)
    {
        return;
    }

    const int numDebris =
        (std::min)(brokenCount / 20 + 8, 40);

    for (int i = 0; i < numDebris; ++i)
    {
        Debris d;

        d.x = static_cast<float>(startX + GetRand(width));
        d.y = static_cast<float>(startY + GetRand(height));

        d.vx = (GetRand(20) - 10) * 1.0f;
        d.vy = (GetRand(18) - 14) * 1.0f;

        d.size = GetRand(8) + 4;
        d.life = GetRand(30) + 20;

        const int c = GetRand(40) + 60;
        d.color = GetColor(c + 15, c, c - 15);

        mDebris.push_back(d);
    }
}

void BlockEffect::Update()
{
    for (auto it = mDebris.begin(); it != mDebris.end();)
    {
        it->x += it->vx;
        it->y += it->vy;
        it->vy += gravity;
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

void BlockEffect::Draw(int scrollX, int scrollY) const
{
    for (const auto& d : mDebris)
    {
        const int dx = static_cast<int>(d.x) - scrollX;
        const int dy = static_cast<int>(d.y) - scrollY;

        DrawBox(
            dx,
            dy,
            dx + d.size,
            dy + d.size,
            d.color,
            TRUE
        );
    }
}

void BlockEffect::Clear()
{
    mDebris.clear();
}
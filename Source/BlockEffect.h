#pragma once

#include "DxLib.h"
#include <vector>

class BlockEffect
{
public:
    BlockEffect() = default;
    ~BlockEffect() = default;

    // 破片エフェクトを生成
    void Spawn(int startX, int startY, int width, int height, int brokenCount);

    // 破片の移動・重力・寿命を更新
    void Update();

    // 破片を描画
    void Draw(int scrollX, int scrollY) const;

    // エフェクトをクリア
    void Clear();

private:
    struct Debris
    {
        float x = 0.0f;
        float y = 0.0f;
        float vx = 0.0f;
        float vy = 0.0f;

        int size = 4;
        int life = 0;
        int color = 0;
    };

    std::vector<Debris> mDebris;

    // 破片専用の重力
    const float gravity = 0.4f;
};
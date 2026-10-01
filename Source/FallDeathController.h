#pragma once
#include <vector>
#include "DxLib.h"

class GameConstanes;

/// <summary>
/// ゲームオーバーしたときの表示とタイマーを管理するコントローラークラス
/// </summary>
class FallDeathController
{
public:

    FallDeathController();
    ~FallDeathController();
    void Update(VECTOR playerPos, int playerLive);
    void Draw(int live) const;

    bool IsActive() const { return mActive; };
    bool IsReviveReady() const { return mIsPressEnter; }
    bool IsReviveFinished() const
    {
        return mIsPressEnter && mReviveTimer <= 0.0f;
    }

    void Reset();

private:
    bool mActive;
    bool mIsPressEnter;
    float mReviveTimer;
};
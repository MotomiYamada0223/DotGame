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

    void Update(VECTOR playerPos, int playerLive);
    void Draw(int live);

    bool IsActive() const { return mActive; };
    bool IsReviveReady() const { return mIsPressEnter; }
    bool IsReviveFinished() const
    {
        return mIsPressEnter && mReviveTimer <= 0.0f;
    }

    void Reset();

private:
    bool mActive = false;
    bool mIsPressEnter = false;
    float mReviveTimer;
};
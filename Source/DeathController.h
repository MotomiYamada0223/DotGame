#pragma once
#include <vector>
#include "DxLib.h"

class GameConstanes;

/// <summary>
/// ゲームオーバーしたときの表示とタイマーを管理するコントローラークラス
/// </summary>
class DeathController
{
public:

    DeathController();
    ~DeathController();
    void Update(VECTOR playerPos, int playerLive, int playerHp);
    void Draw(int live) const;

    bool IsActive() const { return mActive; };
    bool IsReviveReady() const { return mIsPressEnter; }

    void Reset();

private:
    bool mActive;
    bool mIsPressEnter;
};
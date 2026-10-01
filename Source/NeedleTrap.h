#pragma once
#include "Enemy.h"
#include "UnitStatus.h"

enum class TrapType {
    PopUp,
    AntiJump
};

enum class TrapState {
    Waiting,
    Active,
    Finished
};

class NeedleTrap : public Enemy, public UnitStatus
{
public:
    NeedleTrap(VECTOR initPos, TrapType type);
    virtual ~NeedleTrap();

    virtual void Update() override;
    virtual void Draw() override;
    virtual void EnemyMove(BlockMap& blockMap) override;
    virtual void UpdateStatusByProgress(GameProgress progress) override;

    // Player側の当たり判定処理（GetSize() / 6.0f）に合わせるため
    // 本来の表示サイズ（32 * 3 = 96）になるように逆算して返す
    virtual int GetSizeX() { return 32 * 3 * 6; }
    virtual int GetSizeY() { return 32 * 3 * 6; }


private:
    TrapType mType;
    TrapState mState;
    float mBaseY;
    float mTargetY;
    float mTrapSpeed;
    int mGraphHandle;
};

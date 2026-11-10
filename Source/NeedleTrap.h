#pragma once
#include "Enemy.h"
#include "UnitStatus.h"
#include <string>

class RollingBoulder;

enum class TrapType {
    Normal,
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
    NeedleTrap(
        VECTOR initPos,
        TrapType type = TrapType::Normal,
        const std::string& graphPath = "",
        float scale = 1.0f
    );
    virtual ~NeedleTrap();

    virtual void Update() override;
    virtual void Draw() override;
    virtual void EnemyMove(BlockMap& blockMap) override;
    virtual void UpdateStatusByProgress(GameProgress progress) override;

    // Player側の当たり判定処理（GetSize() / 6.0f）に合わせるため
    // 本来の表示サイズになるように逆算して返す
    virtual int GetSizeX() override { return static_cast<int>(mfEnemyWidth * 6.0f); }
    virtual int GetSizeY() override { return static_cast<int>(mfEnemyHeight * 6.0f); }

protected:
    TrapType mType;
    TrapState mState;
    float mBaseY;
    float mTargetY;
    float mTrapSpeed;
    float mScale;
    int mGraphHandle;
};

// 洞窟ステージの転がり岩坂下くぼみ用巨大針トラップ
class BoulderNeedleTrap : public NeedleTrap
{
public:
    BoulderNeedleTrap(VECTOR initPos, const RollingBoulder* rollingBoulder);
    virtual ~BoulderNeedleTrap() override;

    virtual void Update() override;
    virtual void Draw() override;
    virtual void EnemyMove(BlockMap& blockMap) override;

private:
    enum class SpikeState {
        Hidden,     // くぼみ底面で潜伏
        Emerging,   // 地面からせり上がり中
        Ready,      // くぼみに完全に突き出して待機
        Launched,   // 上空へロケット発射中
        Finished    // 画面外へ飛び去り消滅
    };

    SpikeState mSpikeState;
    const RollingBoulder* mpRollingBoulder;
    float mSpikeHeight;
    float mSpikeMaxHeight;
    float mSpikeVy;
    float mBaseX;
    float mBaseY;
};

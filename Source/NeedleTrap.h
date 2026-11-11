#pragma once
#include "Enemy.h"
#include "UnitStatus.h"
#include <string>

class RollingBoulder;

enum class TrapType {
    Normal,
    PopUp,
    AntiJump,
    Falling,
    ShakingDummy // カモフラージュ用ダミー震え針
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

    virtual float GetCollisionWidth() const override
    {
        if (mType == TrapType::Falling && mState != TrapState::Active) return 0.0f;
        return mfEnemyWidth;
    }
    virtual float GetCollisionHeight() const override
    {
        if (mType == TrapType::Falling && mState != TrapState::Active) return 0.0f;
        return mfEnemyHeight;
    }

protected:
    TrapType mType;
    TrapState mState;
    float mBaseY;
    float mTargetY;
    float mTrapSpeed;
    float mScale;
    int mGraphHandle;

    bool mbIsShaking = false;
    int mShakeOffsetX = 0;
    int mShakeOffsetY = 0;
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

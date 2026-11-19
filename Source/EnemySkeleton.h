#pragma once
#include "Enemy.h"
#include "UnitStatus.h"
#include <vector>

class EnemySkeleton : public Enemy, public UnitStatus
{
public:
    EnemySkeleton(VECTOR initPos);
    virtual ~EnemySkeleton();

    virtual void UpdateStatusByProgress(GameProgress progress) override;
    virtual void Update() override;
    virtual void Draw() override;
    virtual void EnemyMove(BlockMap& blockMap) override;

    virtual bool IsAttacking() const override;
    virtual void GetAttackHitbox(float& left, float& top, float& right, float& bottom) const override;
    virtual bool HasCustomDeath() const override { return true; }
    virtual bool IsActiveTarget() const override { return mDeathState == DeathState::Alive; }

public:
    enum class State { Idle, Move, Attack };
    enum class DeathState
    {
        Alive,
        PlayingAnim, // skeleton_death.png 全8コマを順番に再生中
        FadingOut    // 最終コマ後、6フレーム目(index 5)に切り替えてフェードアウト中
    };
    State GetState() const { return mState; }
    DeathState GetDeathState() const { return mDeathState; }

    void SetDrawScale(float scale) { mDrawScale = scale; }
    float GetDrawScale() const { return mDrawScale; }

    struct Arrow {
        float x;
        float y;
        float vx;
        float vy;
        float facing;
        bool active;
        int life;
    };

private:
    std::vector<int> mIdleAnim;
    std::vector<int> mMoveAnim;
    std::vector<int> mAttackAnim;
    std::vector<int> mDeathAnim;

    int mArrowHandle;
    std::vector<Arrow> mArrows;
    bool mbArrowShotInThisAnim;

    int mAnimFrame;
    int mAnimTimer;

    float mMoveDirection;   // -1.0f or 1.0f
    float mFacingDirection; // -1.0f (left) or 1.0f (right)
    int mActionTimer;
    int mAttackCooldown;

    float mDrawScale;
    int mIdleSETimer;

    State mState;
    DeathState mDeathState;
    int mDeathTimer;
};

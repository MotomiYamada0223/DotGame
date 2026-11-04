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

public:
    enum class State { Idle, Move, Attack };
    State GetState() const { return mState; }

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

    State mState;
};

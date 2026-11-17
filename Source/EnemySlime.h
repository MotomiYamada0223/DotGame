#pragma once
#include "Enemy.h"
#include "UnitStatus.h"
#include "TextureAnimation.h"

class EnemySlime : public Enemy, public UnitStatus
{
public:
    EnemySlime(VECTOR initPos);
    virtual ~EnemySlime();

    virtual void Update() override;
    virtual void Draw() override;
    virtual void EnemyMove(BlockMap& blockMap) override;
    virtual void UpdateStatusByProgress(GameProgress progress) override;

	bool IsPlayerDead(); // プレイヤーが死亡しているかの判定

    // カスタム死亡演出を持つか
    virtual bool HasCustomDeath() const override { return true; }
    // プレイヤーとの本体接触判定・被攻撃判定が有効か
    virtual bool IsActiveTarget() const override { return mDeathState == DeathState::Alive; }

    enum class DeathState {
        Alive,
        PlayingAnim,
        FadingOut
    };

    DeathState GetDeathState() const { return mDeathState; }

private:
    TextureAnimation* mpAnimIdle;
    TextureAnimation* mpAnimMove;
    TextureAnimation* mpAnimAttack;
    TextureAnimation* mpAnimDeath;
    DeathState mDeathState;
    int mDeathTimer;
    float mCurrentMoveDirection;
    int mActionTimer;
    bool mIsFacingRight;
    bool mIsChasing;
    float mVisionLength;
    float mVisionBaseHeight;
    float mVisionAngle;

    bool mIsAttacking;
    int mAttackCooldownTimer;
    VECTOR mTargetPlayerPos;
    bool mHasJumped;
    float mJumpSpeedX;

    // 体当たり攻撃の起こり（溜め演出）関連
    bool mIsCharging;
    bool mHasChargedThisAttack;
    int mChargeTimer;

protected:
    float mMoveSpeed;
};
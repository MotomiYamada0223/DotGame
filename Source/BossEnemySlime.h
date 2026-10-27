#pragma once
#include "Enemy.h"
#include "UnitStatus.h"
#include "TextureAnimation.h"
#include <vector>

class Player;

enum class KingSlimeState
{
    SpawnFalling,        // 出現：空からの急降下
    SpawnLandingReverse, // 出現：着地時のジャンプ逆再生
    Idle,                // 待機モーション
    Move,                // 移動
    JumpAttack,          // ジャンプ攻撃
    BeamAttack,          // ビーム攻撃
    Tackle,              // 体当たり
    Death                // 死亡
};

class BossEnemySlime : public Enemy, public UnitStatus
{
public:
    enum class BossPositionSlot
    {
        LeftEdge = 0,
        Center = 1,
        RightEdge = 2
    };

    enum class JumpPhase
    {
        Prepare,        // モーション再生
        Ascending,      
        Warning,        // 警告表示
        WarningFadeOut, 
        Falling,        
        LandingReverse  
    };

    enum class DeathPhase
    {
        PlayingAnimation, 
        PostAnimPause,    
        Shaking,          
        PreExplodePause,  
        Exploded,         
        WaitDroplets,     
        Finished          
    };
    enum class BeamPhase
    {
        OpenToCharge,   // 1〜3枚目まで再生
        Charging,       // 3枚目で一時停止
        Converging,     // 4〜9枚目を再生
        Firing,         // 10〜11枚目を再生し11枚目で停止
        Recovery        // 12〜13枚目を再生
    };

    struct ChargeParticle
    {
        float angle;
        float distance;
        float maxDistance;
        float speed;
        float radius;
    };

    struct SlimeDroplet
    {
        VECTOR pos;
        VECTOR vel;
        float radius;
        int lifeTime;
        bool isActive;
        bool isSplashing;
        int splashTimer;
    };

    BossEnemySlime(VECTOR initPos, float groundY = 0.0f, Player* targetPlayer = nullptr);
    virtual ~BossEnemySlime();

    virtual void Update() override;
    virtual void Draw() override;
    virtual void EnemyMove(BlockMap& blockMap) override;
    virtual void UpdateStatusByProgress(GameProgress progress) override;
    virtual bool CheckAttackCollision(const VECTOR& targetPos, const VECTOR& targetSize) const override;
    virtual float GetCollisionOffsetY() const override { return mCollisionOffsetY; }
    virtual void TakeDamage(int damage) override;
    virtual bool HasCustomDeath() const override { return true; }
    virtual bool IsActiveTarget() const override { return (mState != KingSlimeState::Death); }
    void DrawHpBar() const;


    KingSlimeState GetState() const { return mState; }
    bool IsSpawnFinished() const { return mState != KingSlimeState::SpawnFalling && mState != KingSlimeState::SpawnLandingReverse; }

    void StartBattle();
    bool IsBattleActive() const { return mIsBattleActive; }

private:
    void StartJumpAttack();
    void SelectNextJumpTarget();
    float GetSlotWorldX(BossPositionSlot slot) const;
    float FindGroundY(const BlockMap& blockMap, float x) const;
    float GetLandingCenterY(float groundY) const;
    void HandleLanding(BlockMap& blockMap);

    void StartBeamAttack();
    void InitChargeParticles();
    void UpdateChargeParticles(bool isConverging);

    bool CanTriggerTackle() const;
    void StartTackleAttack();

    void DrawJumpWarning(float screenX, float screenY, float fadeAlphaRatio) const;
    void DrawBeamEffect(float mouthScreenX, float mouthScreenY) const;
    void SpawnLandingDroplets(const BlockMap& blockMap);
    void SpawnDeathExplosionDroplets();
    void StartDeathSequence();
    void UpdateDroplets();
    void DrawDroplets() const;

private:
    KingSlimeState mState;
    JumpPhase mJumpPhase;
    BeamPhase mBeamPhase;
    BossPositionSlot mCurrentSlot;
    BossPositionSlot mTargetSlot;

    TextureAnimation* mpAnimIdle;
    TextureAnimation* mpAnimMove;
    TextureAnimation* mpAnimJump;
    TextureAnimation* mpAnimAttack;
    TextureAnimation* mpAnimLongRange;
    TextureAnimation* mpAnimDeath;
    DeathPhase mDeathPhase;

    float mGroundY;
    float mBossAreaCenterX;
    float mTargetX;
    Player* mpPlayer;
    bool mIsFacingRight;
    float mFallSpeed;
    bool mCrushProcessed;
    bool mIsBattleActive;

    int mIdleTimer;
    int mPhaseTimer;
    ChargeParticle mChargeParticles[24];
    float mCollisionOffsetY;

    // 体当たり関連メンバ変数
    int mTackleCooldownTimer;
    bool mHasTackleLaunched;
    bool mIsTackleCharging;
    bool mHasChargedThisTackle;
    int mTackleChargeTimer;
    float mJumpSpeedX;
    float mTackleVelocityY;
    float mTargetPlayerX;

    // HPバー関連メンバ変数（ダメージ黄色バー制御）
    float mYellowHp;
    int mYellowDelayTimer;
    mutable bool mHpBarDrawnThisFrame;

    int mPrevHp;

    // 着地時スライム液体弾（ドロップレット）
    mutable std::vector<SlimeDroplet> mSlimeDroplets;
};


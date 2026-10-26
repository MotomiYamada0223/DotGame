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
    Move,                // 移動（今後実装）
    JumpAttack,          // ジャンプ攻撃
    BeamAttack,          // ビーム攻撃
    Tackle,              // 体当たり（今後実装）
    Death                // 死亡（今後実装）
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
        Prepare,        // 地上でのジャンプモーション再生（1〜6枚目）
        Ascending,      // 上空へ飛び上がる
        Warning,        // 着地予測の赤警告表示
        WarningFadeOut, // 赤警告のフェードアウト
        Falling,        // 急降下
        LandingReverse  // 着地時のジャンプ逆再生（6〜1枚目）
    };

    enum class DeathPhase
    {
        PlayingAnimation, // 1. kingSlime_death.png アニメーション再生（1〜8枚目）
        PostAnimPause,    // 2. 最後のコマ（8枚目）で少しの間ストップ
        Shaking,          // 3. 体を左右に小刻みに震わせる
        PreExplodePause,  // 4. 震えが止まり、爆発直前の一瞬の静寂
        Exploded,         // 5. スライムが爆発！全方位に液体弾射出＆画面揺れ＆衝撃波
        WaitDroplets,     // 6. 液体弾が飛翔・スプラッシュ中（全消滅待ち）
        Finished          // 7. 演出完了（削除）
    };
    enum class BeamPhase
    {
        OpenToCharge,   // 1〜3枚目まで再生して口を開ける
        Charging,       // 3枚目で一時停止し、水色の玉に小玉を吸引＋予告ライン表示
        Converging,     // 4〜9枚目を再生しながら玉を一点へ収束させる攻撃直前演出
        Firing,         // 10〜11枚目を再生し11枚目で停止、端まで貫通するレーザー照射
        Recovery        // 12〜13枚目を再生して待機へ戻る
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
    void DrawTackleDebug() const;
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


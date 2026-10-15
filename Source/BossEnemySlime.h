#pragma once
#include "Enemy.h"
#include "UnitStatus.h"
#include "TextureAnimation.h"

class Player;

enum class KingSlimeState
{
    SpawnFalling,        // 出現：空からの急降下
    SpawnLandingReverse, // 出現：着地時のジャンプ逆再生
    Idle,                // 待機モーション
    Move,                // 移動（今後実装）
    JumpAttack,          // ジャンプ攻撃（今後実装）
    BeamAttack,          // ビーム攻撃（今後実装）
    Tackle,              // 体当たり（今後実装）
    Death                // 死亡（今後実装）
};

class BossEnemySlime : public Enemy, public UnitStatus
{
public:
    BossEnemySlime(VECTOR initPos, float groundY = 0.0f, Player* targetPlayer = nullptr);
    virtual ~BossEnemySlime();

    virtual void Update() override;
    virtual void Draw() override;
    virtual void EnemyMove(BlockMap& blockMap) override;
    virtual void UpdateStatusByProgress(GameProgress progress) override;

    KingSlimeState GetState() const { return mState; }
    bool IsSpawnFinished() const { return mState != KingSlimeState::SpawnFalling && mState != KingSlimeState::SpawnLandingReverse; }

private:
    KingSlimeState mState;

    TextureAnimation* mpAnimIdle;
    TextureAnimation* mpAnimMove;
    TextureAnimation* mpAnimJump;
    TextureAnimation* mpAnimAttack;

    float mGroundY;
    Player* mpPlayer;
    bool mIsFacingRight;
    float mFallSpeed;
    bool mCrushProcessed;
};

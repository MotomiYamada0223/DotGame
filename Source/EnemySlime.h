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

private:
    TextureAnimation* mpAnimIdle;
    TextureAnimation* mpAnimMove;
    TextureAnimation* mpAnimAttack;
    float mCurrentMoveDirection;
    int mActionTimer;
    bool mIsFacingRight;

protected:
    float mMoveSpeed;
};
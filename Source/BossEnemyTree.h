#pragma once
#include "Enemy.h"
#include "UnitStatus.h"

class BossEnemyTree : public Enemy, public UnitStatus
{
public:
    BossEnemyTree(VECTOR initPos);
    virtual ~BossEnemyTree() {}

    virtual void UpdateStatusByProgress(GameProgress progress) override;
};

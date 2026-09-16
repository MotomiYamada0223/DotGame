#pragma once
#include "Enemy.h"
#include "UnitStatus.h"

class BossEnemyDragon : public Enemy, public UnitStatus
{
public:
    BossEnemyDragon(VECTOR initPos);
    virtual ~BossEnemyDragon() {}

    virtual void UpdateStatusByProgress(GameProgress progress) override;
};

#pragma once
#include "Enemy.h"
#include "UnitStatus.h"

class BossEnemySnake : public Enemy, public UnitStatus
{
public:
    BossEnemySnake(VECTOR initPos);
    virtual ~BossEnemySnake() {}

    virtual void UpdateStatusByProgress(GameProgress progress) override;
};

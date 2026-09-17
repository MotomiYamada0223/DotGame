#pragma once
#include "Enemy.h"
#include "UnitStatus.h"

class EnemyWolf : public Enemy, public UnitStatus
{
public:
    EnemyWolf(VECTOR initPos);
    virtual ~EnemyWolf() {}

    virtual void UpdateStatusByProgress(GameProgress progress) override;
};

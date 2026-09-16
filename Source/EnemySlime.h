#pragma once
#include "Enemy.h"
#include "UnitStatus.h"

class EnemySlime : public Enemy, public UnitStatus
{
public:
    EnemySlime(VECTOR initPos);
    virtual ~EnemySlime() {}

    virtual void UpdateStatusByProgress(GameProgress progress) override;
};

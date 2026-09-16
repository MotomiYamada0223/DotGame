#pragma once
#include "Enemy.h"
#include "UnitStatus.h"

class EnemySkeleton : public Enemy, public UnitStatus
{
public:
    EnemySkeleton(VECTOR initPos);
    virtual ~EnemySkeleton() {}

    virtual void UpdateStatusByProgress(GameProgress progress) override;
};

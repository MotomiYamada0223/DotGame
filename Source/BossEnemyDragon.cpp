#include "BossEnemyDragon.h"

BossEnemyDragon::BossEnemyDragon(VECTOR initPos)
    : Enemy(initPos)
{
    UpdateStatusByProgress(GameProgress::BossDragon);
}

void BossEnemyDragon::UpdateStatusByProgress(GameProgress progress)
{
    mMaxHp = 450; mHp = 450;
    mAttack = 75; 
    mHasInstantKillAttack = true; // 火炎即死
}

#include "BossEnemyTree.h"

BossEnemyTree::BossEnemyTree(VECTOR initPos)
    : Enemy(initPos)
{
    UpdateStatusByProgress(GameProgress::BossTree);
}

void BossEnemyTree::UpdateStatusByProgress(GameProgress progress)
{
    mMaxHp = 50; mHp = 50;
    mAttack = 5; // または即死
    mHasInstantKillAttack = true; // 一部即死
}

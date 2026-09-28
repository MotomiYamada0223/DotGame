#include "GameConstants.h"
#include "BossEnemyTree.h"

BossEnemyTree::BossEnemyTree(VECTOR initPos)
    : Enemy(CharacterGraphPath::Dragon, initPos)
{
    UpdateStatusByProgress(GameProgress::BossTree);
}

void BossEnemyTree::UpdateStatusByProgress(GameProgress progress)
{
    mMaxHp = 50; mHp = 50;
    mAttack = 5; // ‚Ü‚½‚Í‘¦Ž€
    mHasInstantKillAttack = true; // ˆê•”‘¦Ž€
}

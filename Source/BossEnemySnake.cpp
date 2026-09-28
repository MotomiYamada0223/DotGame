#include "GameConstants.h"
#include "BossEnemySnake.h"

BossEnemySnake::BossEnemySnake(VECTOR initPos)
    : Enemy(CharacterGraphPath::Dragon, initPos)
{
    UpdateStatusByProgress(GameProgress::BossSnake);
}

void BossEnemySnake::UpdateStatusByProgress(GameProgress progress)
{
    mMaxHp = 150; mHp = 150;
    mAttack = 15; // “Å‚Í•Ê“rˆ—
    mHasInstantKillAttack = true; // Î‰»‘¦€
}

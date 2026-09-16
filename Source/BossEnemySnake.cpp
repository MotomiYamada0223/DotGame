#include "BossEnemySnake.h"

BossEnemySnake::BossEnemySnake(VECTOR initPos)
    : Enemy(initPos)
{
    UpdateStatusByProgress(GameProgress::BossSnake);
}

void BossEnemySnake::UpdateStatusByProgress(GameProgress progress)
{
    mMaxHp = 150; mHp = 150;
    mAttack = 15; // 毒は別途処理
    mHasInstantKillAttack = true; // 石化即死
}

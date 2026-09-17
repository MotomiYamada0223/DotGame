#include "EnemySlime.h"

EnemySlime::EnemySlime(VECTOR initPos)
    : Enemy(initPos)
{
    UpdateStatusByProgress(GameProgress::Tutorial1);
}

void EnemySlime::UpdateStatusByProgress(GameProgress progress)
{
    switch (progress)
    {
    case GameProgress::Tutorial1:
        mMaxHp = 15; mHp = 15;
        mAttack = 9999; 
        mHasInstantKillAttack = true;
        break;
    default: 
        mMaxHp = 15; mHp = 15;
        mAttack = 5;
        mHasInstantKillAttack = false;
        break;
    }
}

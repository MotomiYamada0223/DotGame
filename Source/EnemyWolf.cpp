#include "EnemyWolf.h"

EnemyWolf::EnemyWolf(VECTOR initPos)
    : Enemy(initPos)
{
    UpdateStatusByProgress(GameProgress::Tutorial3);
}

void EnemyWolf::UpdateStatusByProgress(GameProgress progress)
{
    switch (progress)
    {
    case GameProgress::Tutorial1:
    case GameProgress::PostTutorial1:
    case GameProgress::Tutorial2:
    case GameProgress::PostTutorial2:
    case GameProgress::Tutorial3:
        mMaxHp = 135; mHp = 135;
        mAttack = 9999;
        mHasInstantKillAttack = true;
        mUseCounterTeleport = true;
        break;
    default:
        mMaxHp = 135; mHp = 135;
        mAttack = 75;
        mHasInstantKillAttack = false;
        mUseCounterTeleport = false;
        break;
    }
}

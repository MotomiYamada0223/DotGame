#include "EnemySkeleton.h"

EnemySkeleton::EnemySkeleton(VECTOR initPos)
    : Enemy(initPos)
{
    UpdateStatusByProgress(GameProgress::Tutorial2);
}

void EnemySkeleton::UpdateStatusByProgress(GameProgress progress)
{
    switch (progress)
    {
    case GameProgress::Tutorial1:
    case GameProgress::PostTutorial1:
    case GameProgress::Tutorial2:
        mMaxHp = 45; mHp = 45;
        mAttack = 9999;
        mHasInstantKillAttack = true;
        mIsGiantInvincible = true;
        break;
    default:
        mMaxHp = 45; mHp = 45;
        mAttack = 15;
        mHasInstantKillAttack = false;
        mIsGiantInvincible = false;
        break;
    }
}

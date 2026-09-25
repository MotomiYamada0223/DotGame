#include "UnitStatus.h"

UnitStatus::UnitStatus()
    : mMaxHp(0), mHp(0), mAttack(0), mDefense(0), mMagic(0)
    , mIsBlinkWallDeathImmune(false), mAttackReachLevel(0), mIsAttackFlashy(false)
    , mBlinkCooldownLevel(0), mIsPoisonImmune(false), mIsPetrificationImmune(false)
    , mIsFireImmune(false)
    , mHasInstantKillAttack(false), mIsGiantInvincible(false), mUseCounterTeleport(false)
{
}

UnitStatus::~UnitStatus()
{
}

void UnitStatus::UpdateStatusByProgress(GameProgress progress)
{
    // 基底クラスでは何もしない。派生クラスでオーバーライドする
}

// 敵との判定で使用するダメージ処理
void UnitStatus::TakeDamage(int damage)
{
    mHp -= damage;
    if (mHp < 0) mHp = 0;
}

// 残機がない判定
bool UnitStatus::IsOutOfLive()
{
    if (mlives > 0)
    {
        return false;
    }
    return true;
}
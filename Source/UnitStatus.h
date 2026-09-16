#pragma once
#include <string>

enum class GameProgress
{
    Tutorial1,
    PostTutorial1,
    Tutorial2,
    PostTutorial2,
    Tutorial3,
    PostTutorial3,
    BossTree,
    BossSnake,
    BossDragon
};

class UnitStatus
{
public:
    UnitStatus();
    virtual ~UnitStatus();

    // 基本パラメータ
    int mMaxHp;
    int mHp;
    int mAttack;
    int mDefense;
    int mMagic;

    // プレイヤー用フラグ
    bool mIsBlinkWallDeathImmune;
    int mAttackReachLevel;
    bool mIsAttackFlashy;
    int mBlinkCooldownLevel;
    bool mIsPoisonImmune;
    bool mIsPetrificationImmune;
    bool mIsFireImmune;

    // 敵用フラグ
    bool mHasInstantKillAttack;
    bool mIsGiantInvincible;
    bool mUseCounterTeleport;

    // 進行度に応じたステータス更新関数
    virtual void UpdateStatusByProgress(GameProgress progress);
    
    // ダメージ処理
    virtual void TakeDamage(int damage);
};

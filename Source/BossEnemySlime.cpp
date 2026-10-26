#include "BossEnemySlime.h"
#include "StageClear.h"
#include "GameConstants.h"
#include "Player.h"
#include "BlockMap.h"
#include "Collision.h"
#include "Master.h"
#include "GameManager.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "FontManager.h"
#include <cmath>
#include <algorithm>

BossEnemySlime::BossEnemySlime(VECTOR initPos, float groundY, Player* targetPlayer)
    : Enemy(CharacterGraphPath::KingSlimeIdle, initPos)
    , mState(KingSlimeState::SpawnFalling)
    , mJumpPhase(JumpPhase::Prepare)
    , mBeamPhase(BeamPhase::OpenToCharge)
    , mCurrentSlot(BossPositionSlot::Center)
    , mTargetSlot(BossPositionSlot::Center)
    , mGroundY(groundY)
    , mBossAreaCenterX(initPos.x)
    , mTargetX(initPos.x)
    , mpPlayer(targetPlayer)
    , mIsFacingRight(false)
    , mFallSpeed(KingSlimeAIConstants::JumpFallSpeed)
    , mCrushProcessed(false)
    , mIsBattleActive(false)
    , mIdleTimer(0)
    , mPhaseTimer(0)
    , mCollisionOffsetY(0.0f)
    , mTackleCooldownTimer(60)
    , mHasTackleLaunched(false)
    , mIsTackleCharging(false)
    , mHasChargedThisTackle(false)
    , mTackleChargeTimer(0)
    , mJumpSpeedX(0.0f)
    , mpAnimDeath(nullptr)
    , mDeathPhase(DeathPhase::PlayingAnimation)
    , mTackleVelocityY(0.0f)
    , mTargetPlayerX(initPos.x)
    , mYellowHp(100.0f)
    , mYellowDelayTimer(0)
    , mHpBarDrawnThisFrame(false)
    , mPrevHp(100)
{
    // キングスライムの当たり判定サイズ
    const float baseHeight = 75.0f * KingSlimeIntroConstants::HitBoxScale;
    mfEnemyWidth = 90.0f * KingSlimeIntroConstants::HitBoxScale;
    mfEnemyHeight = baseHeight + KingSlimeAIConstants::HitBoxBottomExtend;
    mCollisionOffsetY = KingSlimeAIConstants::HitBoxBottomExtend * 0.5f;

    mpAnimIdle = new TextureAnimation(initPos, CharacterGraphPath::KingSlimeIdle, 6, 6, 1, 8);
    mpAnimMove = new TextureAnimation(initPos, CharacterGraphPath::KingSlimeMove, 7, 7, 1, 8);
    mpAnimJump = new TextureAnimation(initPos, CharacterGraphPath::KingSlimeJump, 6, 6, 1, 5);
    mpAnimAttack = new TextureAnimation(initPos, CharacterGraphPath::KingSlimeAttack, 7, 7, 1, 8);
    mpAnimLongRange = new TextureAnimation(initPos, CharacterGraphPath::KingSlimeLongRangeAttack, 13, 13, 1, KingSlimeAIConstants::BeamMouthOpenInterval);

    // アニメーションのスケール設定 (4倍)
    mpAnimIdle->SetScale(KingSlimeIntroConstants::Scale);
    mpAnimMove->SetScale(KingSlimeIntroConstants::Scale);
    mpAnimJump->SetScale(KingSlimeIntroConstants::Scale);
    mpAnimAttack->SetScale(KingSlimeIntroConstants::Scale);
    mpAnimLongRange->SetScale(KingSlimeIntroConstants::Scale);
    mpAnimLongRange->SetLoop(false);

    mpAnimDeath = new TextureAnimation(initPos, CharacterGraphPath::KingSlimeDeath, 8, 8, 1, KingSlimeDeathConstants::AnimInterval);
    mpAnimDeath->SetScale(KingSlimeIntroConstants::Scale);
    mpAnimDeath->SetLoop(false);

    // 出現時はジャンプモーションの6枚目（インデックス5）で固定
    if (mpAnimJump)
    {
        mpAnimJump->SetCurrentFrame(5);
        mpAnimJump->SetReverse(true);
        mpAnimJump->SetLoop(false);
    }

    InitChargeParticles();
    UpdateStatusByProgress(GameProgress::Tutorial1);
}

BossEnemySlime::~BossEnemySlime()
{
    if (mpAnimIdle) delete mpAnimIdle;
    if (mpAnimMove) delete mpAnimMove;
    if (mpAnimJump) delete mpAnimJump;
    if (mpAnimAttack) delete mpAnimAttack;
    if (mpAnimLongRange) delete mpAnimLongRange;
    if (mpAnimDeath) delete mpAnimDeath;
}

void BossEnemySlime::StartBattle()
{
    mIsBattleActive = true;
    // 登場演出終了と同時に行動を開始できるよう待機タイマーを満たす
    mIdleTimer = KingSlimeAIConstants::IdleWaitFrames;
    mYellowHp = static_cast<float>(mHp);
    mYellowDelayTimer = 0;
}

void BossEnemySlime::TakeDamage(int damage)
{
    if (mState == KingSlimeState::Death) return;

    // ダメージ前のHPを記憶（黄色バーが既に減少中なら現在のmYellowHpを維持してそこから引く）
    if (mYellowHp < static_cast<float>(mHp))
    {
        mYellowHp = static_cast<float>(mHp);
    }

    UnitStatus::TakeDamage(damage);

    // ダメージを受けた分のバーが黄色く変化し、少ししたら一定速度で減少し始める
    mYellowDelayTimer = KingSlimeHpBarConstants::YellowDelayFrames;

    if (mHp <= 0)
    {
        mHp = 0;
        StartDeathSequence();
    }
}

void BossEnemySlime::StartDeathSequence()
{
    mState = KingSlimeState::Death;
    mDeathPhase = DeathPhase::PlayingAnimation;
    mPhaseTimer = 0;

    // 空中にいる場合は着地させる
    if (mpBlockMap != nullptr)
    {
        mGroundY = FindGroundY(*mpBlockMap, mvPosition.x);
        mvPosition.y = GetLandingCenterY(mGroundY);
    }

    mJumpSpeedX = 0.0f;
    mTackleVelocityY = 0.0f;

    if (mpAnimDeath)
    {
        mpAnimDeath->ResetAnimation();
        mpAnimDeath->SetCurrentFrame(0);
        mpAnimDeath->SetLoop(false);
        mpAnimDeath->SetReverse(false);
        mpAnimDeath->SetInterval(KingSlimeDeathConstants::AnimInterval);
    }
}

float BossEnemySlime::GetSlotWorldX(BossPositionSlot slot) const
{
    switch (slot)
    {
    case BossPositionSlot::LeftEdge:
        return mBossAreaCenterX - KingSlimeAIConstants::EdgeOffsetX;
    case BossPositionSlot::RightEdge:
        return mBossAreaCenterX + KingSlimeAIConstants::EdgeOffsetX;
    case BossPositionSlot::Center:
    default:
        return mBossAreaCenterX;
    }
}

float BossEnemySlime::FindGroundY(const BlockMap& blockMap, float x) const
{
    int testX = static_cast<int>(x);
    int startY = static_cast<int>(mGroundY) - 150;
    int endY = static_cast<int>(mGroundY) + 150;
    for (int y = startY; y <= endY; ++y)
    {
        if (blockMap.GetCollisionType(testX, y) == BlockMap::CollisionType::Block)
        {
            return static_cast<float>(y);
        }
    }
    return mGroundY;
}

float BossEnemySlime::GetLandingCenterY(float groundY) const
{
    // 拡張された当たり判定の下端がマップの床（groundY）と正確に接触する中心Y座標
    return groundY - (mfEnemyHeight * 0.5f + mCollisionOffsetY);
}

void BossEnemySlime::SelectNextJumpTarget()
{
    BossPositionSlot nextSlot = BossPositionSlot::Center;

    // 20%の確率でロジックを無視し、現在いる場所ではないどこかへランダムジャンプ
    int roll = GetRand(99);
    if (roll < KingSlimeAIConstants::RandomTargetChancePercent)
    {
        BossPositionSlot candidates[2];
        int count = 0;
        for (int i = 0; i < 3; i++)
        {
            BossPositionSlot s = static_cast<BossPositionSlot>(i);
            if (s != mCurrentSlot)
            {
                candidates[count++] = s;
            }
        }
        nextSlot = candidates[GetRand(count - 1)];
    }
    else
    {
        // 80%はプレイヤーの位置に応じたロジック選択
        if (mpPlayer != nullptr)
        {
            float playerX = mpPlayer->GetPosition().x;
            float diffX = playerX - mBossAreaCenterX;

            if (std::abs(diffX) <= KingSlimeAIConstants::CenterZoneHalfWidth)
            {
                // プレイヤーが中央付近にいるなら中央へジャンプ
                nextSlot = BossPositionSlot::Center;
            }
            else if (diffX < 0.0f)
            {
                // プレイヤーが左側にいるなら右端へジャンプしてビーム攻撃へ
                nextSlot = BossPositionSlot::RightEdge;
            }
            else
            {
                // プレイヤーが右側にいるなら左端へジャンプしてビーム攻撃へ
                nextSlot = BossPositionSlot::LeftEdge;
            }
        }
        else
        {
            nextSlot = BossPositionSlot::Center;
        }
    }

    mTargetSlot = nextSlot;
    mTargetX = GetSlotWorldX(mTargetSlot);
}

void BossEnemySlime::StartJumpAttack()
{
    SelectNextJumpTarget();

    mState = KingSlimeState::JumpAttack;
    mJumpPhase = JumpPhase::Prepare;
    mPhaseTimer = 0;

    // ジャンプ先（同じ場所ならプレイヤー方向）を向く
    if (std::abs(mTargetX - mvPosition.x) > 1.0f)
    {
        mIsFacingRight = (mTargetX > mvPosition.x);
    }
    else if (mpPlayer != nullptr)
    {
        mIsFacingRight = (mpPlayer->GetPosition().x > mvPosition.x);
    }

    // 発動時はジャンプモーション（1〜6枚目、インデックス0〜5）を順再生
    if (mpAnimJump)
    {
        mpAnimJump->ResetAnimation();
        mpAnimJump->SetReverse(false);
        mpAnimJump->SetLoop(false);
        mpAnimJump->SetInterval(KingSlimeAIConstants::JumpPrepareAnimInterval);
    }
}

void BossEnemySlime::HandleLanding(BlockMap& blockMap)
{
    mGroundY = FindGroundY(blockMap, mvPosition.x);
    mvPosition.y = GetLandingCenterY(mGroundY);

    // 登場演出と共通の画面揺れ処理
    blockMap.StartCameraShake(
        KingSlimeShakeConstants::ShakePower,
        KingSlimeShakeConstants::RatioX,
        KingSlimeShakeConstants::RatioY,
        KingSlimeShakeConstants::DurationFrames
    );

    // 登場演出と共通の着地ジャンプ逆再生（6枚目→1枚目、インデックス5→0）
    if (mpAnimJump)
    {
        mpAnimJump->SetCurrentFrame(5);
        mpAnimJump->SetReverse(true);
        mpAnimJump->SetLoop(false);
        mpAnimJump->SetInterval(5);
    }
}

void BossEnemySlime::StartBeamAttack()
{
    mState = KingSlimeState::BeamAttack;
    mBeamPhase = BeamPhase::OpenToCharge;
    mPhaseTimer = 0;

    // ボスエリア中央へ向けて放つため、中央方向を向く
    mIsFacingRight = (mvPosition.x < mBossAreaCenterX);

    if (mpAnimLongRange)
    {
        mpAnimLongRange->ResetAnimation();
        mpAnimLongRange->SetReverse(false);
        mpAnimLongRange->SetLoop(false);
        mpAnimLongRange->SetCurrentFrame(0);
    }

    InitChargeParticles();
}

bool BossEnemySlime::CanTriggerTackle() const
{
    // クールタイム中またはプレイヤー死亡時は発動しない
    if (mTackleCooldownTimer > 0) return false;
    if (mpPlayer == nullptr || mpPlayer->GetIsDead()) return false;

    // プレイヤーとの水平距離が特定範囲内か判定
    float playerX = mpPlayer->GetPosition().x;
    float dist = std::abs(playerX - mvPosition.x);
    if (dist > KingSlimeAIConstants::TackleRangeMax) return false;

    // 40%の確率判定
    int roll = GetRand(99);
    return (roll < KingSlimeAIConstants::TackleChancePercent);
}

void BossEnemySlime::StartTackleAttack()
{
    mState = KingSlimeState::Tackle;
    mPhaseTimer = 0;
    mHasTackleLaunched = false;
    mIsTackleCharging = false;
    mHasChargedThisTackle = false;
    mTackleChargeTimer = 0;
    mJumpSpeedX = 0.0f;
    mTackleVelocityY = 0.0f;
    mTargetPlayerX = (mpPlayer != nullptr) ? mpPlayer->GetPosition().x : mvPosition.x;

    // プレイヤーの方向を向く
    if (mpPlayer != nullptr)
    {
        mIsFacingRight = (mpPlayer->GetPosition().x > mvPosition.x);
    }

    if (mpAnimAttack)
    {
        mpAnimAttack->ResetAnimation();
        mpAnimAttack->SetReverse(false);
        mpAnimAttack->SetLoop(false);
        mpAnimAttack->SetInterval(KingSlimeAIConstants::TackleAnimInterval);
    }
}

void BossEnemySlime::InitChargeParticles()
{
    for (int i = 0; i < KingSlimeAIConstants::BeamParticleCount; i++)
    {
        float angleDeg = static_cast<float>(GetRand(359));
        mChargeParticles[i].angle = angleDeg * 0.01745329f;
        mChargeParticles[i].maxDistance = 70.0f + static_cast<float>(GetRand(120));
        mChargeParticles[i].distance = static_cast<float>(GetRand(static_cast<int>(mChargeParticles[i].maxDistance)));
        mChargeParticles[i].speed = 2.5f + static_cast<float>(GetRand(25)) * 0.1f;
        mChargeParticles[i].radius = 4.0f + static_cast<float>(GetRand(6));
    }
}

void BossEnemySlime::UpdateChargeParticles(bool isConverging)
{
    float speedMultiplier = isConverging ? 2.4f : 1.0f;
    for (int i = 0; i < KingSlimeAIConstants::BeamParticleCount; i++)
    {
        mChargeParticles[i].distance -= mChargeParticles[i].speed * speedMultiplier;
        if (mChargeParticles[i].distance <= 4.0f)
        {
            if (isConverging)
            {
                // 収束フェーズでは中心へ吸い込まれたままにする
                mChargeParticles[i].distance = 0.0f;
            }
            else
            {
                // チャージ中は外周から繰り返し吸い込まれる
                float angleDeg = static_cast<float>(GetRand(359));
                mChargeParticles[i].angle = angleDeg * 0.01745329f;
                mChargeParticles[i].maxDistance = 70.0f + static_cast<float>(GetRand(120));
                mChargeParticles[i].distance = mChargeParticles[i].maxDistance;
                mChargeParticles[i].speed = 2.5f + static_cast<float>(GetRand(25)) * 0.1f;
                mChargeParticles[i].radius = 4.0f + static_cast<float>(GetRand(6));
            }
        }
    }
}

void BossEnemySlime::Update()
{
    Enemy::Update();

    // 着地時スライム液体弾の更新
    UpdateDroplets();

    mHpBarDrawnThisFrame = false;

    // HP減少の検知（TakeDamage以外で直接mHpが減った場合のセーフティ）
    if (mHp < mPrevHp)
    {
        if (mYellowHp < static_cast<float>(mPrevHp))
        {
            mYellowHp = static_cast<float>(mPrevHp);
        }
        mYellowDelayTimer = KingSlimeHpBarConstants::YellowDelayFrames;
    }
    mPrevHp = mHp;

    if (mHp <= 0 && mState != KingSlimeState::Death)
    {
        mHp = 0;
        StartDeathSequence();
    }

    // 黄色バーの処理：待機時間後に一定速度でmHpに向けて減少
    if (mYellowDelayTimer > 0)
    {
        mYellowDelayTimer--;
    }
    else if (mYellowHp > static_cast<float>(mHp))
    {
        mYellowHp -= KingSlimeHpBarConstants::YellowDrainSpeed;
        if (mYellowHp < static_cast<float>(mHp))
        {
            mYellowHp = static_cast<float>(mHp);
        }
    }
    else if (mYellowHp < static_cast<float>(mHp))
    {
        mYellowHp = static_cast<float>(mHp);
    }

    if (mTackleCooldownTimer > 0)
    {
        mTackleCooldownTimer--;
    }

    if (mState == KingSlimeState::Death)
    {
        if (mDeathPhase == DeathPhase::PlayingAnimation)
        {
            if (mpAnimDeath)
            {
                mpAnimDeath->Update();
                if (mpAnimDeath->IsFinished() || mpAnimDeath->GetCurrentFrame() >= 7)
                {
                    mpAnimDeath->SetCurrentFrame(7);
                    mDeathPhase = DeathPhase::PostAnimPause;
                    mPhaseTimer = 0;
                }
            }
        }
        else if (mDeathPhase == DeathPhase::PostAnimPause)
        {
            mPhaseTimer++;
            if (mPhaseTimer >= KingSlimeDeathConstants::PostAnimPauseFrames)
            {
                mDeathPhase = DeathPhase::Shaking;
                mPhaseTimer = 0;
            }
        }
        else if (mDeathPhase == DeathPhase::Shaking)
        {
            mPhaseTimer++;
            if (mPhaseTimer >= KingSlimeDeathConstants::ShakeDurationFrames)
            {
                mDeathPhase = DeathPhase::PreExplodePause;
                mPhaseTimer = 0;
            }
        }
        else if (mDeathPhase == DeathPhase::PreExplodePause)
        {
            mPhaseTimer++;
            if (mPhaseTimer >= KingSlimeDeathConstants::PreExplodePauseFrames)
            {
                mDeathPhase = DeathPhase::Exploded;
                mPhaseTimer = 0;

                // 爆発の瞬間！画面揺れ＆全方位に液体弾を散布
                if (mpBlockMap != nullptr)
                {
                    mpBlockMap->StartCameraShake(
                        KingSlimeShakeConstants::ShakePower * 2.0f,
                        KingSlimeShakeConstants::RatioX,
                        KingSlimeShakeConstants::RatioY,
                        35
                    );
                }
                SpawnDeathExplosionDroplets();
            }
        }
        else if (mDeathPhase == DeathPhase::Exploded)
        {
            mPhaseTimer++;
            // 爆発エフェクト進行後、液体弾の消滅監視フェーズへ
            if (mPhaseTimer >= 30)
            {
                mDeathPhase = DeathPhase::WaitDroplets;
                mPhaseTimer = 0;
            }
        }
        else if (mDeathPhase == DeathPhase::WaitDroplets)
        {
            // 全ての液体弾が着地・消滅したか判定
            bool anyDropletActive = false;
            for (const auto& droplet : mSlimeDroplets)
            {
                if (droplet.isActive)
                {
                    anyDropletActive = true;
                    break;
                }
            }
            if (!anyDropletActive)
            {
                // 全ての液体弾が消滅！ステージクリア演出をトリガーし、ボスの死亡完了
                StageClear::Trigger();
                mDeathPhase = DeathPhase::Finished;
                SetDeleteFlag(true);
            }
        }
        return;
    }

    if (mState == KingSlimeState::SpawnFalling)
    {
        // 落下中は6枚目で固定（アニメーション更新は行わない）
        if (mpAnimJump)
        {
            mpAnimJump->SetCurrentFrame(5);
        }
    }
    else if (mState == KingSlimeState::SpawnLandingReverse)
    {
        // 着地後のジャンプ逆再生
        if (mpAnimJump)
        {
            mpAnimJump->Update();
            if (mpAnimJump->IsFinished())
            {
                // 逆再生終了 -> Idleモーションへ移行
                mState = KingSlimeState::Idle;
                mIdleTimer = 0;
                if (mpAnimIdle)
                {
                    mpAnimIdle->ResetAnimation();
                    mpAnimIdle->SetInterval(8);
                }
            }
        }
    }
    else if (mState == KingSlimeState::Idle)
    {
        if (mpAnimIdle)
        {
            mpAnimIdle->Update();
        }
    }
    else if (mState == KingSlimeState::JumpAttack)
    {
        if (mJumpPhase == JumpPhase::Prepare)
        {
            if (mpAnimJump)
            {
                mpAnimJump->Update();
                if (mpAnimJump->IsFinished())
                {
                    // 6枚目まで再生完了したら上空へ飛び上がる
                    mpAnimJump->SetCurrentFrame(5);
                    mJumpPhase = JumpPhase::Ascending;
                }
            }
        }
        else if (mJumpPhase == JumpPhase::Ascending ||
                 mJumpPhase == JumpPhase::Warning ||
                 mJumpPhase == JumpPhase::WarningFadeOut ||
                 mJumpPhase == JumpPhase::Falling)
        {
            // 滞空・警告・落下中は6枚目（インデックス5）で固定
            if (mpAnimJump)
            {
                mpAnimJump->SetCurrentFrame(5);
            }
        }
        else if (mJumpPhase == JumpPhase::LandingReverse)
        {
            if (mpAnimJump)
            {
                mpAnimJump->Update();
                if (mpAnimJump->IsFinished())
                {
                    // 左右の端へ着地した場合はビーム攻撃へ派生
                    if (mCurrentSlot == BossPositionSlot::LeftEdge ||
                        mCurrentSlot == BossPositionSlot::RightEdge)
                    {
                        StartBeamAttack();
                    }
                    else
                    {
                        // 中央へ着地後：条件を満たしていれば40%の確率で体当たりへ割り込み
                        if (CanTriggerTackle())
                        {
                            StartTackleAttack();
                        }
                        else
                        {
                            mState = KingSlimeState::Idle;
                            mIdleTimer = 0;
                            if (mpAnimIdle)
                            {
                                mpAnimIdle->ResetAnimation();
                                mpAnimIdle->SetInterval(8);
                            }
                        }
                    }
                }
            }
        }
    }
    else if (mState == KingSlimeState::BeamAttack)
    {
        if (mBeamPhase == BeamPhase::OpenToCharge)
        {
            mPhaseTimer++;
            int frame = mPhaseTimer / KingSlimeAIConstants::BeamMouthOpenInterval;
            if (frame >= 2)
            {
                // 3枚目（インデックス2）で一時停止し、タメ演出へ移行
                if (mpAnimLongRange) mpAnimLongRange->SetCurrentFrame(2);
                mBeamPhase = BeamPhase::Charging;
                mPhaseTimer = 0;
                InitChargeParticles();
            }
            else
            {
                if (mpAnimLongRange) mpAnimLongRange->SetCurrentFrame(frame);
            }
        }
        else if (mBeamPhase == BeamPhase::Charging)
        {
            // 3枚目（インデックス2）で一時停止維持
            if (mpAnimLongRange) mpAnimLongRange->SetCurrentFrame(2);
            UpdateChargeParticles(false);

            mPhaseTimer++;
            if (mPhaseTimer >= KingSlimeAIConstants::BeamChargeFrames)
            {
                // タメ終了 -> 攻撃直前演出（4〜9枚目再生＋エネルギー収束）へ
                mBeamPhase = BeamPhase::Converging;
                mPhaseTimer = 0;
            }
        }
        else if (mBeamPhase == BeamPhase::Converging)
        {
            // 攻撃直前演出中に4〜9枚目（インデックス3〜8、計6枚）を再生
            int frameOffset = (mPhaseTimer * 6) / KingSlimeAIConstants::BeamConvergeFrames;
            if (frameOffset < 0) frameOffset = 0;
            if (frameOffset > 5) frameOffset = 5;
            if (mpAnimLongRange) mpAnimLongRange->SetCurrentFrame(3 + frameOffset);

            UpdateChargeParticles(true);

            mPhaseTimer++;
            if (mPhaseTimer >= KingSlimeAIConstants::BeamConvergeFrames)
            {
                // 収束完了 -> ビーム照射（10, 11枚目再生）へ
                mBeamPhase = BeamPhase::Firing;
                mPhaseTimer = 0;
            }
        }
        else if (mBeamPhase == BeamPhase::Firing)
        {
            // ビーム放出時に10枚目（インデックス9）→11枚目（インデックス10）を再生し、持続中は11枚目でストップ
            if (mPhaseTimer < KingSlimeAIConstants::BeamFireStartInterval)
            {
                if (mpAnimLongRange) mpAnimLongRange->SetCurrentFrame(9);
            }
            else
            {
                if (mpAnimLongRange) mpAnimLongRange->SetCurrentFrame(10);
            }

            mPhaseTimer++;
            if (mPhaseTimer >= KingSlimeAIConstants::BeamFireHoldFrames)
            {
                // ビーム終了 -> 12枚目以降（インデックス11〜12）の再生へ
                mBeamPhase = BeamPhase::Recovery;
                mPhaseTimer = 0;
            }
        }
        else if (mBeamPhase == BeamPhase::Recovery)
        {
            // 12枚目（インデックス11）→13枚目（インデックス12）を再生
            if (mPhaseTimer < KingSlimeAIConstants::BeamRecoveryInterval)
            {
                if (mpAnimLongRange) mpAnimLongRange->SetCurrentFrame(11);
            }
            else
            {
                if (mpAnimLongRange) mpAnimLongRange->SetCurrentFrame(12);
            }

            mPhaseTimer++;
            if (mPhaseTimer >= KingSlimeAIConstants::BeamRecoveryInterval * 2)
            {
                // ビーム終了後：条件を満たしていれば40%の確率で体当たりへ割り込み
                if (CanTriggerTackle())
                {
                    StartTackleAttack();
                }
                else
                {
                    mState = KingSlimeState::Idle;
                    mIdleTimer = 0;
                    if (mpAnimIdle)
                    {
                        mpAnimIdle->ResetAnimation();
                        mpAnimIdle->SetInterval(8);
                    }
                }
            }
        }
    }
    else if (mState == KingSlimeState::Tackle)
    {
        if (mpAnimAttack)
        {
            // 3枚目（インデックス2）での溜め処理（アニメーションを止めて震えながら力を溜める）
            if (mIsTackleCharging)
            {
                mTackleChargeTimer--;

                // 溜め中もプレイヤーの最新座標・向きを追跡して狙いを定める
                if (mpPlayer != nullptr)
                {
                    float px = mpPlayer->GetPosition().x;
                    float dir = (px > mvPosition.x) ? 1.0f : -1.0f;
                    mTargetPlayerX = px + dir * KingSlimeAIConstants::TackleOvershootX;

                    float minX = mBossAreaCenterX - KingSlimeAIConstants::EdgeOffsetX - 50.0f;
                    float maxX = mBossAreaCenterX + KingSlimeAIConstants::EdgeOffsetX + 50.0f;
                    if (mTargetPlayerX < minX) mTargetPlayerX = minX;
                    else if (mTargetPlayerX > maxX) mTargetPlayerX = maxX;

                    mIsFacingRight = (dir > 0.0f);
                }

                if (mTackleChargeTimer <= 0)
                {
                    mIsTackleCharging = false;

                    // 溜め完了！ 4枚目（インデックス3）に進めて即座に小ジャンプ突進開始
                    mpAnimAttack->SetCurrentFrame(3);
                    mHasTackleLaunched = true;
                    // 残り4コマ（3, 4, 5, 6） × interval
                    float jumpFrames = 4.0f * static_cast<float>(KingSlimeAIConstants::TackleAnimInterval);
                    mJumpSpeedX = (mTargetPlayerX - mvPosition.x) / jumpFrames;
                    mTackleVelocityY = KingSlimeAIConstants::TackleJumpSpeedY;
                }
            }
            else
            {
                // 通常のアニメーション進行
                mpAnimAttack->Update();
                int currentFrame = mpAnimAttack->GetCurrentFrame();

                // 2枚目（インデックス1）の時にプレイヤー座標を初期記録
                if (currentFrame == 1 && mpPlayer != nullptr)
                {
                    float px = mpPlayer->GetPosition().x;
                    float dir = (px > mvPosition.x) ? 1.0f : -1.0f;
                    mTargetPlayerX = px + dir * KingSlimeAIConstants::TackleOvershootX;

                    float minX = mBossAreaCenterX - KingSlimeAIConstants::EdgeOffsetX - 50.0f;
                    float maxX = mBossAreaCenterX + KingSlimeAIConstants::EdgeOffsetX + 50.0f;
                    if (mTargetPlayerX < minX) mTargetPlayerX = minX;
                    else if (mTargetPlayerX > maxX) mTargetPlayerX = maxX;

                    mIsFacingRight = (dir > 0.0f);
                }

                // 3枚目（インデックス2）になったらアニメーションを止めて溜め（震え）を開始
                if (currentFrame == 2 && !mHasTackleLaunched && !mHasChargedThisTackle)
                {
                    mIsTackleCharging = true;
                    mHasChargedThisTackle = true;
                    mTackleChargeTimer = KingSlimeAIConstants::TackleChargeFrames;
                }

                // 溜め完了後の4枚目（インデックス3）以降での小ジャンプ突進フォールバック
                if (currentFrame >= 3 && !mHasTackleLaunched)
                {
                    mHasTackleLaunched = true;
                    float jumpFrames = 4.0f * static_cast<float>(KingSlimeAIConstants::TackleAnimInterval);
                    mJumpSpeedX = (mTargetPlayerX - mvPosition.x) / jumpFrames;
                    mTackleVelocityY = KingSlimeAIConstants::TackleJumpSpeedY;
                }
            }

            // モーション完了（最後まで再生）で待機へ移行
            if (mpAnimAttack->IsFinished())
            {
                // 体当たり着地の小画面揺れ
                if (mpBlockMap != nullptr)
                {
                    mGroundY = FindGroundY(*mpBlockMap, mvPosition.x);
                    mvPosition.y = GetLandingCenterY(mGroundY);
                    mpBlockMap->StartCameraShake(
                        KingSlimeShakeConstants::ShakePower * 0.5f,
                        KingSlimeShakeConstants::RatioX,
                        KingSlimeShakeConstants::RatioY,
                        15
                    );
                }
                mJumpSpeedX = 0.0f;
                mTackleVelocityY = 0.0f;

                // クールタイム設定
                mTackleCooldownTimer = KingSlimeAIConstants::TackleCooldownFrames;

                // 着地後の位置に応じてスロットを更新
                float diffCenter = std::abs(mvPosition.x - mBossAreaCenterX);
                if (diffCenter <= KingSlimeAIConstants::CenterZoneHalfWidth)
                {
                    mCurrentSlot = BossPositionSlot::Center;
                }
                else if (mvPosition.x < mBossAreaCenterX)
                {
                    mCurrentSlot = BossPositionSlot::LeftEdge;
                }
                else
                {
                    mCurrentSlot = BossPositionSlot::RightEdge;
                }

                mState = KingSlimeState::Idle;
                mIdleTimer = 0;
                if (mpAnimIdle)
                {
                    mpAnimIdle->ResetAnimation();
                    mpAnimIdle->SetInterval(8);
                }
            }
        }
    }
}

void BossEnemySlime::EnemyMove(BlockMap& blockMap)
{
    mpBlockMap = &blockMap;
    if (blockMap.HasBossArea())
    {
        mBossAreaCenterX = blockMap.GetBossAreaCenterX();
    }

    if (mState == KingSlimeState::Death)
    {
        return;
    }

    if (mState == KingSlimeState::SpawnFalling)
    {
        mvPosition.y += mFallSpeed;

        // キングスライムの足元（拡張当たり判定下端）がマップ床に達したか判定
        mGroundY = FindGroundY(blockMap, mvPosition.x);
        const float targetCenterY = GetLandingCenterY(mGroundY);
        if (mvPosition.y >= targetCenterY)
        {
            // 着地処理（画面揺れ＋ジャンプ逆再生設定）
            HandleLanding(blockMap);
            mState = KingSlimeState::SpawnLandingReverse;
            mCurrentSlot = BossPositionSlot::Center;

            // 押しつぶし判定（登場演出で回避を行っていなければプレイヤー即死）
            if (!mCrushProcessed && mpPlayer != nullptr)
            {
                mCrushProcessed = true;
                if (!mpPlayer->HasEvadedBossSpawn())
                {
                    // 押しつぶし圧死
                    mpPlayer->SetDeathReason(DeathReason::DeathType::EnemyHit);
                    mpPlayer->TakeDamage(mpPlayer->GetHp());
                }

                // プレイヤーの方向を向く
                mIsFacingRight = (mpPlayer->GetPosition().x > mvPosition.x);
            }
        }
    }
    else if (mState == KingSlimeState::Idle)
    {
        if (mpPlayer != nullptr)
        {
            mIsFacingRight = (mpPlayer->GetPosition().x > mvPosition.x);
        }

        // 登場演出完了後、プレイヤーが生存していれば待機後に次の行動を開始
        if (mIsBattleActive && (mpPlayer == nullptr || !mpPlayer->GetIsDead()))
        {
            mIdleTimer++;
            if (mIdleTimer >= KingSlimeAIConstants::IdleWaitFrames)
            {
                mIdleTimer = 0;
                StartJumpAttack();
            }
        }
    }
    else if (mState == KingSlimeState::JumpAttack)
    {
        if (mJumpPhase == JumpPhase::Ascending)
        {
            mvPosition.y -= KingSlimeAIConstants::JumpAscendSpeed;
            const float topY = mGroundY - KingSlimeAIConstants::JumpStartHeightOffset;
            if (mvPosition.y <= topY)
            {
                mvPosition.y = topY;
                mvPosition.x = mTargetX;
                mGroundY = FindGroundY(blockMap, mTargetX);
                mPhaseTimer = 0;
                mJumpPhase = JumpPhase::Warning;
            }
        }
        else if (mJumpPhase == JumpPhase::Warning)
        {
            mPhaseTimer++;
            if (mPhaseTimer >= KingSlimeAIConstants::JumpWarningDurationFrames)
            {
                mJumpPhase = JumpPhase::WarningFadeOut;
                mPhaseTimer = KingSlimeAIConstants::JumpWarningFadeOutFrames;
            }
        }
        else if (mJumpPhase == JumpPhase::WarningFadeOut)
        {
            mPhaseTimer--;
            if (mPhaseTimer <= 0)
            {
                mJumpPhase = JumpPhase::Falling;
            }
        }
        else if (mJumpPhase == JumpPhase::Falling)
        {
            mvPosition.y += mFallSpeed;

            mGroundY = FindGroundY(blockMap, mvPosition.x);
            const float targetCenterY = GetLandingCenterY(mGroundY);
            if (mvPosition.y >= targetCenterY)
            {
                // 既存の着地処理を共用（座標補正・画面揺れ・ジャンプ逆再生）
                HandleLanding(blockMap);
                mCurrentSlot = mTargetSlot;
                mJumpPhase = JumpPhase::LandingReverse;

                // 体力が半分以下になった後、すべてのジャンプ攻撃着地時に周囲へ円形液体弾を一斉散布
                if (mHp <= static_cast<int>(static_cast<float>(mMaxHp) * KingSlimeAIConstants::DropletHpThresholdRatio))
                {
                    SpawnLandingDroplets(blockMap);
                }

                // 左右端に降りた場合は中央方向、中央に降りた場合はプレイヤー方向を向く
                if (mCurrentSlot == BossPositionSlot::LeftEdge)
                {
                    mIsFacingRight = true;
                }
                else if (mCurrentSlot == BossPositionSlot::RightEdge)
                {
                    mIsFacingRight = false;
                }
                else if (mpPlayer != nullptr)
                {
                    mIsFacingRight = (mpPlayer->GetPosition().x > mvPosition.x);
                }
            }
        }
    }
    else if (mState == KingSlimeState::Tackle)
    {
        if (mHasTackleLaunched)
        {
            mvPosition.x += mJumpSpeedX;
            mTackleVelocityY += KingSlimeAIConstants::TackleGravity;
            mvPosition.y += mTackleVelocityY;

            mGroundY = FindGroundY(blockMap, mvPosition.x);
            const float targetCenterY = GetLandingCenterY(mGroundY);
            if (mvPosition.y >= targetCenterY)
            {
                mvPosition.y = targetCenterY;
                mTackleVelocityY = 0.0f;
            }

            // ボスエリア外に出ないようにクランプ
            float minX = mBossAreaCenterX - KingSlimeAIConstants::EdgeOffsetX - 50.0f;
            float maxX = mBossAreaCenterX + KingSlimeAIConstants::EdgeOffsetX + 50.0f;
            if (mvPosition.x < minX) mvPosition.x = minX;
            else if (mvPosition.x > maxX) mvPosition.x = maxX;
        }
    }
}

bool BossEnemySlime::CheckAttackCollision(const VECTOR& targetPos, const VECTOR& targetSize) const
{
    // 1. ビーム照射中の当たり判定
    if (mState == KingSlimeState::BeamAttack && mBeamPhase == BeamPhase::Firing)
    {
        const float dir = mIsFacingRight ? 1.0f : -1.0f;
        const float mouthWorldX = mvPosition.x + dir * KingSlimeAIConstants::BeamMouthOffsetX;
        const float mouthWorldY = mvPosition.y + KingSlimeAIConstants::BeamMouthOffsetY;

        const float beamLeft = mIsFacingRight ? mouthWorldX : (mouthWorldX - KingSlimeAIConstants::BeamLength);
        const float beamTop = mouthWorldY - KingSlimeAIConstants::BeamHalfHeight;

        VECTOR beamPos = VGet(beamLeft, beamTop, 0.0f);
        VECTOR beamSize = VGet(KingSlimeAIConstants::BeamLength, KingSlimeAIConstants::BeamHalfHeight * 2.0f, 0.0f);

        if (Collision::CheckRectToRect(targetPos, targetSize, beamPos, beamSize))
        {
            return true;
        }
    }

    // 2. 着地時に飛び散るスライム液体弾（ドロップレット）の当たり判定
    bool hasHitDroplet = false;
    for (auto& droplet : mSlimeDroplets)
    {
        if (droplet.isActive && !droplet.isSplashing)
        {
            float closestX = (droplet.pos.x < targetPos.x) ? targetPos.x :
                ((droplet.pos.x > targetPos.x + targetSize.x) ? (targetPos.x + targetSize.x) : droplet.pos.x);
            float closestY = (droplet.pos.y < targetPos.y) ? targetPos.y :
                ((droplet.pos.y > targetPos.y + targetSize.y) ? (targetPos.y + targetSize.y) : droplet.pos.y);

            float dx = droplet.pos.x - closestX;
            float dy = droplet.pos.y - closestY;

            if ((dx * dx + dy * dy) <= (droplet.radius * droplet.radius))
            {
                droplet.isSplashing = true;
                droplet.splashTimer = KingSlimeAIConstants::DropletSplashFrames;
                hasHitDroplet = true;
            }
        }
    }
    if (hasHitDroplet)
    {
        return true;
    }

    return false;
}

void BossEnemySlime::Draw()
{
    if (mpBlockMap != nullptr)
    {
        // 1. ジャンプ攻撃の着地予測赤警告の描画
        if (mState == KingSlimeState::JumpAttack &&
            (mJumpPhase == JumpPhase::Warning || mJumpPhase == JumpPhase::WarningFadeOut))
        {
            const float warnScreenX = ConvertToScreenX(mTargetX, mpBlockMap);
            const float warnScreenY = ConvertToScreenY(mGroundY, mpBlockMap);
            float fadeRatio = 1.0f;
            if (mJumpPhase == JumpPhase::WarningFadeOut)
            {
                fadeRatio = static_cast<float>(mPhaseTimer) / static_cast<float>(KingSlimeAIConstants::JumpWarningFadeOutFrames);
                if (fadeRatio < 0.0f) fadeRatio = 0.0f;
                if (fadeRatio > 1.0f) fadeRatio = 1.0f;
            }
            DrawJumpWarning(warnScreenX, warnScreenY, fadeRatio);
        }

        // 2. キングスライム本体の描画
        const float screenX = ConvertToScreenX(mvPosition.x, mpBlockMap);
        // テクスチャの原点オフセット補正 (テクスチャ200x200の中心Y=100に対してスライム見た目中心Y=84のため、差分16px * Scaleを下方に補正)
        const float textureCenterY = mvPosition.y + (16.0f * KingSlimeIntroConstants::Scale);
        const float screenY = ConvertToScreenY(textureCenterY, mpBlockMap);

        if (isDamaged) { SetDrawBright(255, 100, 100); }

        TextureAnimation* currentAnim = mpAnimIdle;
        if (mState == KingSlimeState::Death)
        {
            currentAnim = mpAnimDeath;
        }
        else if (mState == KingSlimeState::SpawnFalling ||
            mState == KingSlimeState::SpawnLandingReverse ||
            mState == KingSlimeState::JumpAttack)
        {
            currentAnim = mpAnimJump;
        }
        else if (mState == KingSlimeState::BeamAttack)
        {
            currentAnim = mpAnimLongRange;
        }
        else if (mState == KingSlimeState::Tackle)
        {
            currentAnim = mpAnimAttack;
        }

        if (mState == KingSlimeState::Death)
        {
            if (mDeathPhase < DeathPhase::Exploded && currentAnim != nullptr)
            {
                float drawScreenX = screenX;
                float drawScreenY = screenY;

                // 左右の小刻みな震え（約3フレーム間隔で左右反転）
                if (mDeathPhase == DeathPhase::Shaking)
                {
                    const int interval = (KingSlimeDeathConstants::ShakeInterval > 0) ? KingSlimeDeathConstants::ShakeInterval : 3;
                    float dir = ((mPhaseTimer / interval) % 2 == 0) ? 1.0f : -1.0f;
                    float shakeX = dir * KingSlimeDeathConstants::ShakePower;
                    drawScreenX += shakeX;
                }

                currentAnim->SetPosition(VGet(drawScreenX, drawScreenY, 0.0f));
                currentAnim->Draw(mIsFacingRight);
            }
            else if (mDeathPhase == DeathPhase::Exploded)
            {
                // 爆発フラッシュと衝撃波リングの描画
                if (mPhaseTimer <= 30)
                {
                    float progress = static_cast<float>(mPhaseTimer) / 30.0f;
                    float radius = 40.0f + progress * 280.0f;
                    int alpha = static_cast<int>(240.0f * (1.0f - progress));

                    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
                    // 外側衝撃波リング
                    DrawCircle(static_cast<int>(screenX), static_cast<int>(screenY), static_cast<int>(radius), GetColor(80, 200, 255), FALSE);
                    DrawCircle(static_cast<int>(screenX), static_cast<int>(screenY), static_cast<int>(radius - 2.0f), GetColor(180, 240, 255), FALSE);
                    DrawCircle(static_cast<int>(screenX), static_cast<int>(screenY), static_cast<int>(radius - 4.0f), GetColor(255, 255, 255), FALSE);

                    // 内側閃光コア（最初の半分）
                    if (progress < 0.4f)
                    {
                        int coreAlpha = static_cast<int>(255.0f * (1.0f - progress * 2.5f));
                        SetDrawBlendMode(DX_BLENDMODE_ALPHA, coreAlpha);
                        DrawCircle(static_cast<int>(screenX), static_cast<int>(screenY), static_cast<int>(radius * 0.5f), GetColor(255, 255, 255), TRUE);
                        DrawCircle(static_cast<int>(screenX), static_cast<int>(screenY), static_cast<int>(radius * 0.7f), GetColor(140, 220, 255), TRUE);
                    }
                    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
                }
            }
        }
        else if (currentAnim != nullptr)
        {
            float drawScreenX = screenX;
            float drawScreenY = screenY;

            // 体当たり攻撃の3枚目溜め時：スライムを激しく震わせる（力を溜める演出）
            if (mState == KingSlimeState::Tackle && mIsTackleCharging)
            {
                const float shakePower = KingSlimeAIConstants::TackleShakePower;
                float shakeOffsetX = (static_cast<float>(GetRand(static_cast<int>(shakePower * 20.0f))) * 0.1f) - shakePower;
                float shakeOffsetY = (static_cast<float>(GetRand(static_cast<int>(shakePower * 10.0f))) * 0.1f) - (shakePower * 0.5f);
                drawScreenX += shakeOffsetX;
                drawScreenY += shakeOffsetY;
            }

            currentAnim->SetPosition(VGet(drawScreenX, drawScreenY, 0.0f));
            currentAnim->Draw(mIsFacingRight);
        }

        if (isDamaged) { SetDrawBright(255, 255, 255); }

        // 3. ビーム攻撃のチャージ球・吸引エフェクト・予告ライン・レーザー本体の描画
        if (mState == KingSlimeState::BeamAttack)
        {
            const float dir = mIsFacingRight ? 1.0f : -1.0f;
            const float mouthScreenX = ConvertToScreenX(mvPosition.x + dir * KingSlimeAIConstants::BeamMouthOffsetX, mpBlockMap);
            const float mouthScreenY = ConvertToScreenY(mvPosition.y + KingSlimeAIConstants::BeamMouthOffsetY, mpBlockMap);
            DrawBeamEffect(mouthScreenX, mouthScreenY);
        }

        // 4. 着地時スライム液体弾（ドロップレット）の描画
        DrawDroplets();

        // 5. 体当たり判定距離のデバッグ視覚化
        DrawTackleDebug();

        // 5. HPバーの描画（最前面UIとしてBossIntroDirectorで未描画の場合のフォールバック）
        if (!mHpBarDrawnThisFrame && mIsBattleActive)
        {
            DrawHpBar();
        }

        
    }
    else
    {
        Object2D::Draw();
    }
}

void BossEnemySlime::DrawJumpWarning(float screenX, float screenY, float fadeAlphaRatio) const
{
    if (fadeAlphaRatio <= 0.0f) return;

    const float width = KingSlimeIntroConstants::WarningWidth;
    const float height = KingSlimeIntroConstants::WarningHeight;
    const int left = static_cast<int>(screenX - width * 0.5f);
    const int right = static_cast<int>(screenX + width * 0.5f);
    const int bottom = static_cast<int>(screenY);
    const int top = static_cast<int>(screenY - height);

    // 1. 縦グラデーション
    const int sliceCount = 15;
    const float sliceHeight = height / static_cast<float>(sliceCount);

    for (int i = 0; i < sliceCount; i++)
    {
        int stripTop = static_cast<int>(top + i * sliceHeight);
        int stripBottom = static_cast<int>(top + (i + 1) * sliceHeight);
        if (i == sliceCount - 1) stripBottom = bottom;

        float v = static_cast<float>(i + 1) / static_cast<float>(sliceCount);
        int alpha = static_cast<int>(140.0f * (v * v) * fadeAlphaRatio);
        if (alpha <= 0) continue;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DrawBox(left, stripTop, right, stripBottom, GetColor(255, 30, 30), TRUE);
    }

    // 2. 左右のエッジライン
    const int edgeThickness = 6;
    for (int i = 0; i < sliceCount; i++)
    {
        int stripTop = static_cast<int>(top + i * sliceHeight);
        int stripBottom = static_cast<int>(top + (i + 1) * sliceHeight);
        if (i == sliceCount - 1) stripBottom = bottom;

        float v = static_cast<float>(i + 1) / static_cast<float>(sliceCount);
        int borderAlpha = static_cast<int>((60.0f + 180.0f * v) * fadeAlphaRatio);
        if (borderAlpha <= 0) continue;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, borderAlpha);
        DrawBox(left, stripTop, left + edgeThickness, stripBottom, GetColor(255, 0, 0), TRUE);
        DrawBox(right - edgeThickness, stripTop, right, stripBottom, GetColor(255, 0, 0), TRUE);
    }

    // 3. 接地面ライン
    int groundLineAlpha = static_cast<int>(240.0f * fadeAlphaRatio);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, groundLineAlpha);
    DrawBox(left, bottom - 6, right, bottom, GetColor(255, 60, 60), TRUE);

    // 4. 範囲中央の「！」マーク
    int midX = static_cast<int>(screenX);
    int midY = static_cast<int>(screenY - height * 0.45f);
    float pulse = 1.0f + 0.1f * std::sin(static_cast<float>(mPhaseTimer) * 0.25f);

    int markBodyAlpha = static_cast<int>(255.0f * fadeAlphaRatio);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, markBodyAlpha);
    int barWidth = 6;
    int barTop = midY - static_cast<int>(30.0f * pulse);
    int barBottom = midY + static_cast<int>(8.0f * pulse);
    DrawBox(midX - barWidth, barTop, midX + barWidth + 1, barBottom, GetColor(255, 0, 0), TRUE);
    int dotTop = midY + static_cast<int>(16.0f * pulse);
    int dotBottom = midY + static_cast<int>(28.0f * pulse);
    DrawBox(midX - barWidth, dotTop, midX + barWidth + 1, dotBottom, GetColor(255, 0, 0), TRUE);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void BossEnemySlime::DrawBeamEffect(float mouthScreenX, float mouthScreenY) const
{
    const int dot = KingSlimeAIConstants::BeamDotPixelSize; // 4pxドット単位
    auto snapDot = [dot](float v) -> int {
        int iv = static_cast<int>(std::floor(v));
        return (iv / dot) * dot;
    };

    // ドット絵調の段階的な円（魔力球）を描画するラムダ
    auto drawDotOrb = [dot](int centerX, int centerY, int radius, unsigned int color) {
        int r = (radius / dot) * dot;
        if (r <= 0) return;
        if (r == dot)
        {
            DrawBox(centerX - dot, centerY - dot, centerX + dot, centerY + dot, color, TRUE);
            return;
        }
        for (int dy = -r; dy < r; dy += dot)
        {
            float sampleY = static_cast<float>(dy) + static_cast<float>(dot) * 0.5f;
            float sq = static_cast<float>(r * r) - sampleY * sampleY;
            if (sq <= 0.0f) continue;
            int dx = (static_cast<int>(std::sqrt(sq)) / dot) * dot;
            if (dx < dot) dx = dot;
            DrawBox(centerX - dx, centerY + dy, centerX + dx, centerY + dy + dot, color, TRUE);
        }
    };

    const int cx = snapDot(mouthScreenX);
    const int cy = snapDot(mouthScreenY);
    const int beamEndX = mIsFacingRight
        ? snapDot(mouthScreenX + KingSlimeAIConstants::BeamLength)
        : snapDot(mouthScreenX - KingSlimeAIConstants::BeamLength);
    const int boxLeft = (cx < beamEndX) ? cx : beamEndX;
    const int boxRight = (cx < beamEndX) ? beamEndX : cx;

    if (mBeamPhase == BeamPhase::Charging || mBeamPhase == BeamPhase::Converging)
    {
        const bool isConverging = (mBeamPhase == BeamPhase::Converging);
        float progress = 0.0f;
        float mainRadius = 12.0f;

        if (!isConverging)
        {
            progress = static_cast<float>(mPhaseTimer) / static_cast<float>(KingSlimeAIConstants::BeamChargeFrames);
            if (progress > 1.0f) progress = 1.0f;
            // ドット段階に合わせて緩やかに脈動しながら大きくなる
            float pulse = 1.0f + 0.07f * std::sin(static_cast<float>(mPhaseTimer) * 0.3f);
            mainRadius = (12.0f + (KingSlimeAIConstants::BeamOrbMaxRadius - 12.0f) * progress) * pulse;
        }
        else
        {
            // 攻撃直前演出：水色の魔力球が段階的に小さくなり、一点に魔力が凝縮していく
            progress = static_cast<float>(mPhaseTimer) / static_cast<float>(KingSlimeAIConstants::BeamConvergeFrames);
            if (progress > 1.0f) progress = 1.0f;
            mainRadius = KingSlimeAIConstants::BeamOrbMaxRadius * (1.0f - progress)
                       + KingSlimeAIConstants::BeamOrbMinRadius * progress;
        }

        // 1. 赤い領域の点滅による予告エリア（端まで貫通）
        const int halfBand = snapDot(KingSlimeAIConstants::BeamHalfHeight);

        // タメ中から攻撃直前まで同じ点滅描画を一定リズムで継続
        int totalChargeTimer = isConverging ? (KingSlimeAIConstants::BeamChargeFrames + mPhaseTimer) : mPhaseTimer;
        float blink = 0.5f + 0.5f * std::sin(static_cast<float>(totalChargeTimer) * 0.25f);

        int fillAlpha = static_cast<int>(40.0f + 50.0f * blink);
        int edgeAlpha = static_cast<int>(100.0f + 100.0f * blink);

        // 赤い警告領域の半透明塗りつぶし
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, fillAlpha);
        DrawBox(boxLeft, cy - halfBand, boxRight, cy + halfBand, GetColor(255, 30, 30), TRUE);

        // 上下の境界エッジライン
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, edgeAlpha);
        DrawBox(boxLeft, cy - halfBand, boxRight, cy - halfBand + dot, GetColor(255, 60, 60), TRUE);
        DrawBox(boxLeft, cy + halfBand - dot, boxRight, cy + halfBand, GetColor(255, 60, 60), TRUE);

        // 2. 吸い込まれる小さな水色の魔力玉（ドット球＋ドット尾）
        for (int i = 0; i < KingSlimeAIConstants::BeamParticleCount; i++)
        {
            const ChargeParticle& p = mChargeParticles[i];
            if (p.distance <= 2.0f) continue;

            int px = snapDot(mouthScreenX + std::cos(p.angle) * p.distance);
            int py = snapDot(mouthScreenY + std::sin(p.angle) * p.distance);
            float distRatio = p.distance / p.maxDistance;
            int pAlpha = static_cast<int>(235.0f * (1.0f - distRatio * 0.45f));
            int pRad = snapDot(p.radius * (0.5f + 0.6f * distRatio));
            if (pRad < dot) pRad = dot;

            // 外側から吸い込まれる軌跡の1ドット残像
            int tailX = snapDot(mouthScreenX + std::cos(p.angle) * (p.distance + dot * 2.5f));
            int tailY = snapDot(mouthScreenY + std::sin(p.angle) * (p.distance + dot * 2.5f));
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, pAlpha / 2);
            DrawBox(tailX - dot, tailY - dot, tailX + dot, tailY + dot, GetColor(40, 165, 245), TRUE);

            // 小さな魔力玉本体（水色外枠＋白水色コアのドット球）
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, pAlpha);
            drawDotOrb(px, py, pRad + dot, GetColor(45, 185, 250));
            drawDotOrb(px, py, pRad, GetColor(205, 250, 255));
        }

        // 3. 口元の水色ベースの魔力球本体（4段階のドット球でファンタジー魔法らしく表現）
        int rAura = snapDot(mainRadius * 1.45f);
        int rOuter = snapDot(mainRadius * 1.15f);
        int rMid = snapDot(mainRadius * 0.85f);
        int rCore = snapDot(mainRadius * 0.45f);
        if (rOuter < dot * 2) rOuter = dot * 2;
        if (rMid < dot) rMid = dot;
        if (rCore < dot) rCore = dot;

        // 外層の深い水魔法オーラ
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 115);
        drawDotOrb(cx, cy, rAura, GetColor(20, 105, 220));

        // 濃い水色の魔力外殻
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 215);
        drawDotOrb(cx, cy, rOuter, GetColor(40, 175, 245));

        // 明るい水色の魔力内層
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 245);
        drawDotOrb(cx, cy, rMid, GetColor(115, 230, 255));

        // 中心の白水色コア
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
        drawDotOrb(cx, cy, rCore, GetColor(230, 255, 255));

        // ドット絵らしい左上のハイライト粒
        if (rOuter >= dot * 4)
        {
            int hlX = cx - snapDot(static_cast<float>(rOuter) * 0.45f);
            int hlY = cy - snapDot(static_cast<float>(rOuter) * 0.45f);
            DrawBox(hlX, hlY, hlX + dot * 2, hlY + dot * 2, GetColor(245, 255, 255), TRUE);
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
    else if (mBeamPhase == BeamPhase::Firing || mBeamPhase == BeamPhase::Recovery)
    {
        float heightScale = 1.0f;
        int masterAlpha = 255;

        if (mBeamPhase == BeamPhase::Recovery)
        {
            float recProgress = static_cast<float>(mPhaseTimer) / static_cast<float>(KingSlimeAIConstants::BeamRecoveryInterval * 2);
            if (recProgress > 1.0f) recProgress = 1.0f;
            heightScale = 1.0f - recProgress;
            masterAlpha = static_cast<int>(230.0f * (1.0f - recProgress));
        }

        if (heightScale <= 0.05f || masterAlpha <= 0)
        {
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            return;
        }

        const int baseHalfOuter = snapDot(KingSlimeAIConstants::BeamHalfHeight * 1.35f * heightScale);
        const int baseHalfMain = snapDot(KingSlimeAIConstants::BeamHalfHeight * 1.0f * heightScale);
        const int baseHalfInner = snapDot(KingSlimeAIConstants::BeamHalfHeight * 0.65f * heightScale);
        const int baseHalfCore = snapDot(KingSlimeAIConstants::BeamHalfHeight * 0.35f * heightScale);

        // 1. 外層：深い魔法オーラ
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (masterAlpha * 135) / 255);
        DrawBox(boxLeft, cy - baseHalfOuter, boxRight, cy + baseHalfOuter, GetColor(20, 105, 220), TRUE);

        // 2. 中層：濃い水色の魔力流
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (masterAlpha * 225) / 255);
        DrawBox(boxLeft, cy - baseHalfMain, boxRight, cy + baseHalfMain, GetColor(45, 180, 250), TRUE);

        // 3. 内層：明るい水色の奔流
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (masterAlpha * 245) / 255);
        DrawBox(boxLeft, cy - baseHalfInner, boxRight, cy + baseHalfInner, GetColor(120, 235, 255), TRUE);

        // 4. 中心核：白水色の高密度魔力コア
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, masterAlpha);
        DrawBox(boxLeft, cy - baseHalfCore, boxRight, cy + baseHalfCore, GetColor(230, 255, 255), TRUE);

        // 5. 口元の魔力放出ドット球（マズルオーブ）
        int muzzleOuter = snapDot(static_cast<float>(baseHalfMain) * 1.45f);
        int muzzleMid = snapDot(static_cast<float>(baseHalfMain) * 1.1f);
        int muzzleCore = snapDot(static_cast<float>(baseHalfMain) * 0.65f);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (masterAlpha * 160) / 255);
        drawDotOrb(cx, cy, muzzleOuter + dot * 2, GetColor(20, 110, 225));

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (masterAlpha * 235) / 255);
        drawDotOrb(cx, cy, muzzleMid, GetColor(55, 190, 250));

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, masterAlpha);
        drawDotOrb(cx, cy, muzzleCore, GetColor(230, 255, 255));

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

void BossEnemySlime::UpdateStatusByProgress(GameProgress progress)
{
    switch (progress)
    {
    case GameProgress::Tutorial1:
        mMaxHp = 100; mHp = 100;
        mAttack = 15;
        mHasInstantKillAttack = false;
        break;
    default:
        mMaxHp = 100; mHp = 100;
        mAttack = 15;
        mHasInstantKillAttack = false;
        break;
    }
    mYellowHp = static_cast<float>(mHp);
    mPrevHp = mHp;
}

void BossEnemySlime::DrawTackleDebug() const
{
    if (mpBlockMap == nullptr) return;
    if (mState == KingSlimeState::SpawnFalling || mState == KingSlimeState::Death) return;

    const float rangeMax = KingSlimeAIConstants::TackleRangeMax;
    const float groundScreenY = ConvertToScreenY(mGroundY, mpBlockMap);
    const float bossScreenX = ConvertToScreenX(mvPosition.x, mpBlockMap);
    const float leftScreenX = ConvertToScreenX(mvPosition.x - rangeMax, mpBlockMap);
    const float rightScreenX = ConvertToScreenX(mvPosition.x + rangeMax, mpBlockMap);

    const int groundY = static_cast<int>(groundScreenY);
    const int bX = static_cast<int>(bossScreenX);
    const int lX = static_cast<int>(leftScreenX);
    const int rX = static_cast<int>(rightScreenX);

    float dist = 0.0f;
    bool inRange = false;
    int pX = bX;
    if (mpPlayer != nullptr)
    {
        float px = mpPlayer->GetPosition().x;
        dist = std::abs(px - mvPosition.x);
        inRange = (dist <= rangeMax);
        pX = static_cast<int>(ConvertToScreenX(px, mpBlockMap));
    }

    const bool onCooldown = (mTackleCooldownTimer > 0);

    // 状態に応じたカラー決定
    int areaColor;
    int borderColor;
    int areaAlpha;
    const char* statusText;

    if (mState == KingSlimeState::Tackle)
    {
        if (mIsTackleCharging)
        {
            areaColor = GetColor(255, 120, 40);
            borderColor = GetColor(255, 140, 50);
            areaAlpha = 80;
            statusText = "CHARGING TACKLE (SHAKING)";
        }
        else
        {
            areaColor = GetColor(255, 30, 30);
            borderColor = GetColor(255, 50, 50);
            areaAlpha = 80;
            statusText = "TACKLE ATTACKING (LEAPING)";
        }
    }
    else if (inRange)
    {
        if (!onCooldown)
        {
            // 範囲内 ＆ クールタイム完了（発動可能：赤）
            areaColor = GetColor(255, 50, 50);
            borderColor = GetColor(255, 60, 60);
            areaAlpha = 70;
            statusText = "IN RANGE (TRIGGER READY: 40%)";
        }
        else
        {
            // 範囲内だがクールタイム中（黄橙）
            areaColor = GetColor(255, 170, 30);
            borderColor = GetColor(255, 190, 40);
            areaAlpha = 55;
            statusText = "IN RANGE (COOLDOWN)";
        }
    }
    else
    {
        // 範囲外（青）
        areaColor = GetColor(30, 150, 240);
        borderColor = GetColor(60, 190, 255);
        areaAlpha = 35;
        statusText = "OUT OF RANGE";
    }

    // 1. 地面の判定エリア帯（左右650px = 計1300px）
    const int bandTop = groundY - 28;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, areaAlpha);
    DrawBox(lX, bandTop, rX, groundY, areaColor, TRUE);

    // 2. 判定エリアの境界線
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    // 上面ライン
    DrawBox(lX, bandTop, rX, bandTop + 2, borderColor, TRUE);
    // 左右の境界垂直ライン
    DrawBox(lX - 1, groundY - 45, lX + 2, groundY, borderColor, TRUE);
    DrawBox(rX - 1, groundY - 45, rX + 2, groundY, borderColor, TRUE);

    // 境界ラインの距離マーカーラベル
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    DrawFormatString(lX - 28, groundY - 60, borderColor, "-%d", static_cast<int>(rangeMax));
    DrawFormatString(rX - 8, groundY - 60, borderColor, "+%d", static_cast<int>(rangeMax));

    // 3. ボスとプレイヤーを結ぶ距離ライン
    if (mpPlayer != nullptr)
    {
        int lineY = groundY - 14;
        int minPX = (bX < pX) ? bX : pX;
        int maxPX = (bX > pX) ? bX : pX;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
        DrawBox(minPX, lineY - 1, maxPX, lineY + 2, borderColor, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // プレイヤー位置のピン
        DrawBox(pX - 2, groundY - 22, pX + 3, groundY - 6, GetColor(255, 255, 255), TRUE);
        // ボス中心ピン
        DrawBox(bX - 2, groundY - 22, bX + 3, groundY - 6, GetColor(255, 255, 0), TRUE);

        // ライン中央付近に現在距離を表示
        int textMidX = (bX + pX) / 2 - 25;
        DrawFormatString(textMidX, lineY - 18, GetColor(255, 255, 255), "%.0fpx", dist);
    }

    // 4. 頭上のデバッグステータスウィンドウ
    const float textureCenterY = mvPosition.y + (16.0f * KingSlimeIntroConstants::Scale);
    const int bossHeadY = static_cast<int>(ConvertToScreenY(textureCenterY, mpBlockMap)) - 145;
    const int infoWidth = 260;
    const int infoHeight = 54;
    const int infoLeft = bX - infoWidth / 2;
    const int infoTop = (bossHeadY > 60) ? (bossHeadY - infoHeight) : (groundY - 120);

    // 背景黒ボックス
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    DrawBox(infoLeft, infoTop, infoLeft + infoWidth, infoTop + infoHeight, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 枠線
    DrawBox(infoLeft, infoTop, infoLeft + infoWidth, infoTop + infoHeight, borderColor, FALSE);

    // テキスト情報
    DrawFormatString(infoLeft + 8, infoTop + 5, borderColor, "[Tackle Check] %s", statusText);
    DrawFormatString(infoLeft + 8, infoTop + 21, GetColor(240, 240, 240), "Dist: %5.1f / Max: %5.1f", dist, rangeMax);
    if (onCooldown)
    {
        DrawFormatString(infoLeft + 8, infoTop + 37, GetColor(255, 180, 50), "CD: %3dF (WAITING)", mTackleCooldownTimer);
    }
    else
    {
        DrawFormatString(infoLeft + 8, infoTop + 37, GetColor(80, 255, 120), "CD: READY (40%% CHANCE)");
    }
}
void BossEnemySlime::DrawHpBar() const
{
    mHpBarDrawnThisFrame = true;

    // 登場演出の黒帯が消えて戦闘開始するまでは非表示
    if (!mIsBattleActive) return;
    if (mState == KingSlimeState::Death && (mDeathPhase == DeathPhase::Finished)) return;

    // バーの配置座標とサイズ（フェードやスムージングなしでそのまま表示）
    const int totalWidth = KingSlimeHpBarConstants::Width;
    const int totalHeight = KingSlimeHpBarConstants::Height;
    const int borderThick = KingSlimeHpBarConstants::BorderThickness;
    const int padding = KingSlimeHpBarConstants::Padding;

    const int barLeft = (ScreenSize::ScrrenWidth - totalWidth) / 2;
    const int barTop = KingSlimeHpBarConstants::PosY;
    const int barRight = barLeft + totalWidth;
    const int barBottom = barTop + totalHeight;

    // ボス名テキスト「キングスライミー」
    DrawFormatString(barLeft + 2, barTop - 22, GetColor(255, 255, 255), "キングスライミー");

    // バーの内側背景（黒〜暗色のベーススロット）
    DrawBox(barLeft, barTop, barRight, barBottom, GetColor(15, 15, 20), TRUE);

    // バーの内側領域
    const int innerLeft = barLeft + borderThick + padding;
    const int innerRight = barRight - borderThick - padding;
    const int innerTop = barTop + borderThick + padding;
    const int innerBottom = barBottom - borderThick - padding;

    if (innerRight > innerLeft)
    {
        const int maxInnerWidth = innerRight - innerLeft;

        // 実際のHP比率（赤いバー）
        float redRatio = (mMaxHp > 0) ? (static_cast<float>(mHp) / static_cast<float>(mMaxHp)) : 0.0f;
        if (redRatio < 0.0f) redRatio = 0.0f;
        if (redRatio > 1.0f) redRatio = 1.0f;

        // ダメージを受けた分の黄色バー比率
        float yellowRatio = (mMaxHp > 0) ? (mYellowHp / static_cast<float>(mMaxHp)) : 0.0f;
        if (yellowRatio < 0.0f) yellowRatio = 0.0f;
        if (yellowRatio > 1.0f) yellowRatio = 1.0f;

        // 1. 黄色バー（受けたダメージ分のバー：少ししたら一定速度で減少）
        int yellowWidth = static_cast<int>(static_cast<float>(maxInnerWidth) * yellowRatio);
        int yellowRight = innerLeft + yellowWidth;
        if (yellowRight > innerRight) yellowRight = innerRight;

        if (yellowRight > innerLeft)
        {
            // 鮮やかな黄色
            DrawBox(innerLeft, innerTop, yellowRight, innerBottom, GetColor(255, 205, 30), TRUE);
            // 上部ハイライト
            DrawBox(innerLeft, innerTop, yellowRight, innerTop + 2, GetColor(255, 240, 120), TRUE);
        }

        // 2. 赤いバー（現在の実HP：即座に減少位置へ）
        int redWidth = static_cast<int>(static_cast<float>(maxInnerWidth) * redRatio);
        int redRight = innerLeft + redWidth;
        if (redRight > innerRight) redRight = innerRight;

        if (redRight > innerLeft)
        {
            // 鮮やかな赤
            DrawBox(innerLeft, innerTop, redRight, innerBottom, GetColor(230, 40, 40), TRUE);
            // 上部ハイライト
            DrawBox(innerLeft, innerTop, redRight, innerTop + 2, GetColor(255, 130, 130), TRUE);
        }
    }

    // 白枠の描画（指定の「白枠」）
    const int whiteColor = GetColor(255, 255, 255);
    // 上枠
    DrawBox(barLeft, barTop, barRight, barTop + borderThick, whiteColor, TRUE);
    // 下枠
    DrawBox(barLeft, barBottom - borderThick, barRight, barBottom, whiteColor, TRUE);
    // 左枠
    DrawBox(barLeft, barTop, barLeft + borderThick, barBottom, whiteColor, TRUE);
    // 右枠
    DrawBox(barRight - borderThick, barTop, barRight, barBottom, whiteColor, TRUE);
}

void BossEnemySlime::SpawnLandingDroplets(const BlockMap& blockMap)
{
    // 非アクティブな弾を整理
    mSlimeDroplets.erase(
        std::remove_if(mSlimeDroplets.begin(), mSlimeDroplets.end(), [](const SlimeDroplet& d) { return !d.isActive; }),
        mSlimeDroplets.end()
    );

    const int count = KingSlimeAIConstants::DropletCount;
    const float minSpeed = KingSlimeAIConstants::DropletSpeedMin;
    const float maxSpeed = KingSlimeAIConstants::DropletSpeedMax;
    const float minRadius = KingSlimeAIConstants::DropletRadiusMin;
    const float maxRadius = KingSlimeAIConstants::DropletRadiusMax;

    const float angleRangeDeg = KingSlimeAIConstants::DropletAngleRangeDeg;
    const float spreadX = KingSlimeAIConstants::DropletSpawnSpreadX;
    const float baseOffsetY = KingSlimeAIConstants::DropletSpawnOffsetY;

    for (int i = 0; i < count; i++)
    {
        SlimeDroplet drop;

        // キングスライム上部付近に絞って出現（左右±spreadX、上部オフセット±8px）
        float offsetX = -spreadX + static_cast<float>(GetRand(static_cast<int>(spreadX * 20.0f))) * 0.1f;
        float offsetY = baseOffsetY + static_cast<float>(GetRand(16) - 8);
        drop.pos = VGet(mvPosition.x + offsetX, mvPosition.y + offsetY, 0.0f);

        // 真上を0度として、-45度〜+45度（計90度）の範囲でランダムな角度
        float angleDeg = -angleRangeDeg + static_cast<float>(GetRand(static_cast<int>(angleRangeDeg * 20.0f))) * 0.1f;
        float angleRad = angleDeg * (3.14159265f / 180.0f);

        // ランダムな初速
        float speed = minSpeed + static_cast<float>(GetRand(static_cast<int>((maxSpeed - minSpeed) * 10.0f))) * 0.1f;
        drop.vel.x = speed * std::sin(angleRad);
        drop.vel.y = -speed * std::cos(angleRad);
        drop.vel.z = 0.0f;

        // ランダムな半径
        drop.radius = minRadius + static_cast<float>(GetRand(static_cast<int>((maxRadius - minRadius) * 10.0f))) * 0.1f;
        drop.lifeTime = KingSlimeAIConstants::DropletLifeFrames;
        drop.isActive = true;
        drop.isSplashing = false;
        drop.splashTimer = 0;

        mSlimeDroplets.push_back(drop);
    }
}


void BossEnemySlime::SpawnDeathExplosionDroplets()
{
    // 非アクティブな弾を整理
    mSlimeDroplets.erase(
        std::remove_if(mSlimeDroplets.begin(), mSlimeDroplets.end(), [](const SlimeDroplet& d) { return !d.isActive; }),
        mSlimeDroplets.end()
    );

    const int count = KingSlimeDeathConstants::DeathDropletCount; // 24個
    const float minSpeed = KingSlimeDeathConstants::DeathDropletSpeedMin;
    const float maxSpeed = KingSlimeDeathConstants::DeathDropletSpeedMax;
    const float minRadius = KingSlimeAIConstants::DropletRadiusMin;
    const float maxRadius = KingSlimeAIConstants::DropletRadiusMax;

    // スライムの中心（mvPosition）から全方位（0度〜360度）に一斉射出
    for (int i = 0; i < count; i++)
    {
        SlimeDroplet drop;
        drop.pos = VGet(mvPosition.x, mvPosition.y, 0.0f);

        // 360度均等配分 ＋ ランダムジッター（±5度）
        float baseAngleDeg = static_cast<float>(i) * (360.0f / static_cast<float>(count));
        float jitterDeg = static_cast<float>(GetRand(10) - 5);
        float angleDeg = baseAngleDeg + jitterDeg;
        float angleRad = angleDeg * (3.14159265f / 180.0f);

        float speed = minSpeed + static_cast<float>(GetRand(static_cast<int>((maxSpeed - minSpeed) * 10.0f))) * 0.1f;
        drop.vel.x = speed * std::cos(angleRad);
        drop.vel.y = speed * std::sin(angleRad);
        drop.vel.z = 0.0f;

        drop.radius = minRadius + static_cast<float>(GetRand(static_cast<int>((maxRadius - minRadius) * 10.0f))) * 0.1f;
        drop.lifeTime = KingSlimeAIConstants::DropletLifeFrames;
        drop.isActive = true;
        drop.isSplashing = false;
        drop.splashTimer = 0;

        mSlimeDroplets.push_back(drop);
    }
}
void BossEnemySlime::UpdateDroplets()
{
    for (auto& droplet : mSlimeDroplets)
    {
        if (!droplet.isActive) continue;

        if (droplet.isSplashing)
        {
            droplet.splashTimer--;
            if (droplet.splashTimer <= 0)
            {
                droplet.isActive = false;
            }
            continue;
        }

        // 速度と重力適用
        droplet.pos.x += droplet.vel.x;
        droplet.vel.y += KingSlimeAIConstants::DropletGravity;
        droplet.pos.y += droplet.vel.y;

        droplet.lifeTime--;
        if (droplet.lifeTime <= 0)
        {
            droplet.isActive = false;
            continue;
        }

        // 床（地面）との着地判定（下方向に落下中のみ）
        if (mpBlockMap != nullptr && droplet.vel.y > 0.0f)
        {
            float groundY = FindGroundY(*mpBlockMap, droplet.pos.x);
            if (droplet.pos.y >= groundY - droplet.radius * 0.3f)
            {
                droplet.pos.y = groundY - droplet.radius * 0.3f;
                droplet.isSplashing = true;
                droplet.splashTimer = KingSlimeAIConstants::DropletSplashFrames;
            }
        }
    }
}

void BossEnemySlime::DrawDroplets() const
{
    if (mpBlockMap == nullptr) return;

    for (const auto& droplet : mSlimeDroplets)
    {
        if (!droplet.isActive) continue;

        float screenX = ConvertToScreenX(droplet.pos.x, mpBlockMap);
        float screenY = ConvertToScreenY(droplet.pos.y, mpBlockMap);
        float r = droplet.radius;

        // 画面外ならスキップ
        if (screenX < -50.0f || screenX > ScreenSize::ScrrenWidth + 50.0f ||
            screenY < -50.0f || screenY > ScreenSize::ScrrenHeight + 50.0f)
        {
            continue;
        }

        int scX = static_cast<int>(screenX);
        int scY = static_cast<int>(screenY);

        if (droplet.isSplashing)
        {
            // 着地スプラッシュ（横に広がってフェードアウト）
            float splashProgress = 1.0f - static_cast<float>(droplet.splashTimer) / static_cast<float>(KingSlimeAIConstants::DropletSplashFrames);
            int alpha = static_cast<int>(220.0f * (1.0f - splashProgress));
            if (alpha <= 0) continue;

            int ovalRx = static_cast<int>(r * (1.0f + splashProgress * 0.8f));
            int ovalRy = static_cast<int>(r * (0.45f * (1.0f - splashProgress * 0.5f)));
            if (ovalRy < 2) ovalRy = 2;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawOval(scX, scY, ovalRx, ovalRy, GetColor(45, 175, 250), TRUE);
            DrawOval(scX, scY, static_cast<int>(ovalRx * 0.65f), static_cast<int>(ovalRy * 0.65f), GetColor(140, 230, 255), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }
        else
        {
            // 飛翔中の円形液体弾
            // 1. 外枠／濃い青の縁取り
            DrawCircle(scX, scY, static_cast<int>(r + 2.0f), GetColor(20, 80, 200), TRUE);

            // 2. メインの液体スライム色（鮮やかなスカイブルー）
            DrawCircle(scX, scY, static_cast<int>(r), GetColor(45, 175, 250), TRUE);

            // 3. 内側の液体コア（明るい水色）
            DrawCircle(scX, static_cast<int>(scY - r * 0.15f), static_cast<int>(r * 0.65f), GetColor(130, 230, 255), TRUE);

            // 4. ハイライト光沢（白色のドット光沢）
            DrawCircle(static_cast<int>(scX - r * 0.3f), static_cast<int>(scY - r * 0.35f), static_cast<int>(r * 0.25f), GetColor(255, 255, 255), TRUE);
        }
    }
}

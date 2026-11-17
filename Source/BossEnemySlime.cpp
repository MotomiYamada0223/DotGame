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
    Master::mpGameManager->GetSoundManager()->PlaySE(SoundManager::SE_STAGE1_SLIME_LAND);
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

    // 着地時地面衝撃波の更新
    if (mpBlockMap != nullptr)
    {
        UpdateShockwaves(*mpBlockMap);
    }

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

                Master::mpGameManager->GetSoundManager()->PlaySE(SoundManager::SE_STAGE1_BEAM_FIRE);
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
                    mpPlayer->SetDeathReason(DeathReason::DeathType::EnemySlime);
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

                // 中央に着地した時は、常に地面を這う衝撃波（大ジャンプ回避）を左右に発生！
                if (mCurrentSlot == BossPositionSlot::Center)
                {
                    SpawnCenterShockwaves(blockMap);
                }

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

    // 3. 地面を這う衝撃波の当たり判定（指定座標でクリップされた有効領域のみ判定）
    for (const auto& sw : mShockwaves)
    {
        VECTOR swPos, swSize;
        if (GetShockwaveClippedHitbox(sw, swPos, swSize))
        {
            if (Collision::CheckRectToRect(targetPos, targetSize, swPos, swSize))
            {
                return true;
            }
        }
    }

    return false;
}

void BossEnemySlime::DrawSlime()
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

        // 5. 地面を這う衝撃波の描画
        DrawShockwaves();



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
    float pulse = 1.0f + 0.12f * std::sin(static_cast<float>(mPhaseTimer) * 0.25f);

    int markBodyAlpha = static_cast<int>(255.0f * fadeAlphaRatio);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, markBodyAlpha);
    int barWidth = 6;
    int barTop = midY - static_cast<int>(30.0f * pulse);
    int barBottom = midY + static_cast<int>(8.0f * pulse);
    int dotTop = midY + static_cast<int>(16.0f * pulse);
    int dotBottom = midY + static_cast<int>(28.0f * pulse);

    DrawBox(midX - barWidth, barTop, midX + barWidth + 1, barBottom, GetColor(255, 0, 0), TRUE);
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

void BossEnemySlime::DrawHpBar() const
{
    mHpBarDrawnThisFrame = true;

    // 登場演出の黒帯が消えて戦闘開始するまでは非表示
    if (!mIsBattleActive) return;
    if (mpPlayer != nullptr && mpPlayer->GetIsDead()) return;

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

void BossEnemySlime::SpawnCenterShockwaves(const BlockMap& blockMap)
{
    // 非アクティブな衝撃波を整理
    mShockwaves.erase(
        std::remove_if(mShockwaves.begin(), mShockwaves.end(),
            [](const Shockwave& sw) { return !sw.isActive; }),
        mShockwaves.end()
    );

    const float h = KingSlimeShockwaveConstants::Height;
    const float w = KingSlimeShockwaveConstants::Width;
    const float spd = KingSlimeShockwaveConstants::Speed;

    // 左向き(-1)と右向き(+1)の2本の衝撃波を生成
    for (int dir : { -1, 1 })
    {
        Shockwave sw;
        sw.pos = VGet(mvPosition.x + static_cast<float>(dir) * 60.0f, mGroundY, 0.0f);
        sw.speedX = static_cast<float>(dir) * spd;
        sw.height = h;
        sw.width = w;
        sw.lifeTimer = KingSlimeShockwaveConstants::LifeFrames;
        sw.isActive = true;
        mShockwaves.push_back(sw);
    }
}

void BossEnemySlime::UpdateShockwaves(const BlockMap& blockMap)
{
    for (auto& sw : mShockwaves)
    {
        if (!sw.isActive) continue;

        const float dir = (sw.speedX > 0.0f) ? 1.0f : -1.0f;
        const float vanishX = mBossAreaCenterX + dir * KingSlimeShockwaveConstants::WallLimitX;

        // 指定座標に到達した後も同じ速度で前進を続け、通り過ぎた前側から順々に消滅させる
        sw.pos.x += sw.speedX;
        sw.lifeTimer--;

        // 地形追従（指定座標より先の壁等でY座標が乱れないようクランプ）
        float sampleX = sw.pos.x;
        if (dir > 0.0f && sampleX > vanishX) sampleX = vanishX;
        if (dir < 0.0f && sampleX < vanishX) sampleX = vanishX;
        sw.pos.y = FindGroundY(blockMap, sampleX);

        // 最後尾（尻尾の先端）まで指定座標に到達して通過したら完全に消滅
        float frontX = sw.pos.x + dir * (sw.width * 0.5f);
        float tailX  = frontX - dir * KingSlimeShockwaveConstants::VisualLength;
        bool isFullyVanished = (dir > 0.0f) ? (tailX >= vanishX) : (tailX <= vanishX);
        if (isFullyVanished || sw.lifeTimer <= 0)
        {
            sw.isActive = false;
        }
    }
}

bool BossEnemySlime::GetShockwaveClippedHitbox(const Shockwave& sw, VECTOR& outPos, VECTOR& outSize) const
{
    if (!sw.isActive) return false;

    const bool facingRight = (sw.speedX > 0.0f);
    const float dir = facingRight ? 1.0f : -1.0f;
    const float vanishWorldX = mBossAreaCenterX + dir * KingSlimeShockwaveConstants::WallLimitX;

    float rawLeft  = sw.pos.x - sw.width * 0.5f;
    float rawRight = sw.pos.x + sw.width * 0.5f;

    float hitLeft  = rawLeft;
    float hitRight = rawRight;

    // 指定座標を越えて消滅した前側部分をクリップ
    if (facingRight)
    {
        if (hitRight > vanishWorldX) hitRight = vanishWorldX;
    }
    else
    {
        if (hitLeft < vanishWorldX) hitLeft = vanishWorldX;
    }

    float clippedWidth = hitRight - hitLeft;
    if (clippedWidth <= 1.0f) return false;

    outPos  = VGet(hitLeft, sw.pos.y - sw.height, 0.0f);
    outSize = VGet(clippedWidth, sw.height, 0.0f);
    return true;
}

void BossEnemySlime::DrawShockwaves() const
{
    if (mpBlockMap == nullptr) return;

    struct Pt2D
    {
        float x;
        float y;
    };

    // 半平面クリッピング付き三角形描画（指定座標を越えた部分を正確に切り落とす）
    auto drawClippedTriangle = [](const Pt2D& a, const Pt2D& b, const Pt2D& c, float clipX, bool facingRight, unsigned int color)
    {
        auto isInside = [facingRight, clipX](float x) {
            return facingRight ? (x <= clipX) : (x >= clipX);
        };
        Pt2D in[3] = { a, b, c };
        Pt2D out[4];
        int outCount = 0;

        for (int i = 0; i < 3; i++)
        {
            const Pt2D& cur = in[i];
            const Pt2D& prev = in[(i + 2) % 3];
            bool curIn = isInside(cur.x);
            bool prevIn = isInside(prev.x);

            if (curIn)
            {
                if (!prevIn)
                {
                    float dx = cur.x - prev.x;
                    float t = (std::abs(dx) > 0.0001f) ? (clipX - prev.x) / dx : 0.0f;
                    out[outCount++] = { clipX, prev.y + t * (cur.y - prev.y) };
                }
                out[outCount++] = cur;
            }
            else if (prevIn)
            {
                float dx = cur.x - prev.x;
                float t = (std::abs(dx) > 0.0001f) ? (clipX - prev.x) / dx : 0.0f;
                out[outCount++] = { clipX, prev.y + t * (cur.y - prev.y) };
            }
        }

        if (outCount == 3)
        {
            DrawTriangle(static_cast<int>(out[0].x), static_cast<int>(out[0].y),
                         static_cast<int>(out[1].x), static_cast<int>(out[1].y),
                         static_cast<int>(out[2].x), static_cast<int>(out[2].y), color, TRUE);
        }
        else if (outCount == 4)
        {
            DrawTriangle(static_cast<int>(out[0].x), static_cast<int>(out[0].y),
                         static_cast<int>(out[1].x), static_cast<int>(out[1].y),
                         static_cast<int>(out[2].x), static_cast<int>(out[2].y), color, TRUE);
            DrawTriangle(static_cast<int>(out[0].x), static_cast<int>(out[0].y),
                         static_cast<int>(out[2].x), static_cast<int>(out[2].y),
                         static_cast<int>(out[3].x), static_cast<int>(out[3].y), color, TRUE);
        }
    };

    // 半平面クリッピング付き線分描画
    auto drawClippedLine = [](const Pt2D& a, const Pt2D& b, float clipX, bool facingRight, unsigned int color, int thick)
    {
        bool aIn = facingRight ? (a.x <= clipX) : (a.x >= clipX);
        bool bIn = facingRight ? (b.x <= clipX) : (b.x >= clipX);
        if (!aIn && !bIn) return;

        Pt2D p1 = a;
        Pt2D p2 = b;
        if (!aIn)
        {
            float dx = b.x - a.x;
            float t = (std::abs(dx) > 0.0001f) ? (clipX - a.x) / dx : 0.0f;
            p1 = { clipX, a.y + t * (b.y - a.y) };
        }
        else if (!bIn)
        {
            float dx = b.x - a.x;
            float t = (std::abs(dx) > 0.0001f) ? (clipX - a.x) / dx : 0.0f;
            p2 = { clipX, a.y + t * (b.y - a.y) };
        }
        DrawLine(static_cast<int>(p1.x), static_cast<int>(p1.y),
                 static_cast<int>(p2.x), static_cast<int>(p2.y), color, thick);
    };

    for (const auto& sw : mShockwaves)
    {
        if (!sw.isActive) continue;

        const float screenX = ConvertToScreenX(sw.pos.x, mpBlockMap);
        const float screenY = ConvertToScreenY(sw.pos.y, mpBlockMap);

        // 画面外判定（余裕を持たせる）
        if (screenX < -250.0f || screenX > ScreenSize::ScrrenWidth + 250.0f ||
            screenY < -300.0f || screenY > ScreenSize::ScrrenHeight + 100.0f)
        {
            continue;
        }

        const bool facingRight = (sw.speedX > 0.0f);
        const float dir = facingRight ? 1.0f : -1.0f;

        // 指定座標（消滅境界）のワールド座標およびスクリーン座標
        const float vanishWorldX  = mBossAreaCenterX + dir * KingSlimeShockwaveConstants::WallLimitX;
        const float vanishScreenX = ConvertToScreenX(vanishWorldX, mpBlockMap);

        const float frontWorldX = sw.pos.x + dir * (sw.width * 0.5f);
        const float overshoot   = facingRight ? (frontWorldX - vanishWorldX) : (vanishWorldX - frontWorldX);
        const bool  isVanishing = (overshoot > 0.0f);

        // --- 1. 当たり判定（Hitbox）の可視化 ---
        // GetShockwaveClippedHitbox を使い、指定座標でクリップされた有効な当たり判定をそのまま可視化
        VECTOR hitPos, hitSize;
        if (GetShockwaveClippedHitbox(sw, hitPos, hitSize))
        {
            int hitLeft   = static_cast<int>(ConvertToScreenX(hitPos.x, mpBlockMap));
            int hitTop    = static_cast<int>(ConvertToScreenY(hitPos.y, mpBlockMap));
            int hitRight  = static_cast<int>(ConvertToScreenX(hitPos.x + hitSize.x, mpBlockMap));
            int hitBottom = static_cast<int>(ConvertToScreenY(hitPos.y + hitSize.y, mpBlockMap));

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 70);
            DrawBox(hitLeft, hitTop, hitRight, hitBottom, GetColor(255, 40, 40), TRUE);

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 230);
            DrawBox(hitLeft, hitTop, hitRight, hitBottom, GetColor(255, 30, 30), FALSE);
            DrawBox(hitLeft + 1, hitTop + 1, hitRight - 1, hitBottom - 1, GetColor(255, 90, 90), FALSE);

            DrawBox(hitLeft, hitTop - 3, hitRight, hitTop, GetColor(255, 20, 20), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // --- 2. 衝撃波本体の描画（反比例減衰＋上面ギザギザ鋸歯状波） ---

        float elapsed    = static_cast<float>(KingSlimeShockwaveConstants::LifeFrames - sw.lifeTimer);
        float pulseScale = 1.0f + 0.05f * std::sin(elapsed * 0.45f);
        float waveH      = sw.height * pulseScale;
        float visualLen  = KingSlimeShockwaveConstants::VisualLength;

        int baseY = static_cast<int>(screenY);
        float basePosY = static_cast<float>(baseY);

        // 衝撃波自身の本来の先端足元と後端足元（止まらず同じ速度で前進し続ける）
        float frontFootX = screenX + dir * (sw.width * 0.5f);
        float tailFootX  = frontFootX - dir * visualLen;

        // 接地点の衝撃波楕円リング（先端が指定座標に入る前、または通過直後まで表示）
        if (!isVanishing || overshoot < sw.width * 0.6f)
        {
            float ringAlphaRatio = isVanishing ? (1.0f - overshoot / (sw.width * 0.6f)) : 1.0f;
            if (ringAlphaRatio > 0.0f)
            {
                SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(170.0f * ringAlphaRatio));
                int ringCenterX = static_cast<int>(isVanishing ? vanishScreenX : frontFootX);
                DrawOval(ringCenterX, baseY - 3, static_cast<int>(sw.width * 0.7f * ringAlphaRatio), 10, GetColor(0, 140, 255), TRUE);
                DrawOval(ringCenterX, baseY - 3, static_cast<int>(sw.width * 0.45f * ringAlphaRatio), 6, GetColor(100, 220, 255), TRUE);
                DrawOval(ringCenterX, baseY - 3, static_cast<int>(sw.width * 0.22f * ringAlphaRatio), 3, GetColor(255, 255, 255), TRUE);
            }
        }

        // 上面点列（後端 -> スパイク群 -> メインピーク）の構築
        std::vector<Pt2D> topNodes;
        topNodes.push_back({ tailFootX, basePosY });

        const struct SpikeDef {
            float u;
            float valleyU;
            float valleyRatio;
        } spikes[] = {
            { 0.12f, 0.18f, 0.65f },
            { 0.26f, 0.33f, 0.65f },
            { 0.42f, 0.50f, 0.68f },
            { 0.60f, 0.70f, 0.70f },
            { 0.80f, 0.90f, 0.72f },
        };

        for (int i = 0; i < 5; i++)
        {
            const auto& sp = spikes[i];
            float u = sp.u;
            float invDecay = 1.0f / (1.0f + 3.2f * (1.0f - u));
            float spikePulse = 1.0f + 0.06f * std::sin(elapsed * 0.4f + static_cast<float>(i) * 1.3f);
            float h = waveH * invDecay * spikePulse;
            topNodes.push_back({ tailFootX + dir * (visualLen * u), basePosY - h });

            float vu = sp.valleyU;
            float vInvDecay = 1.0f / (1.0f + 3.2f * (1.0f - vu));
            float vh = waveH * vInvDecay * sp.valleyRatio;
            topNodes.push_back({ tailFootX + dir * (visualLen * vu), basePosY - vh });
        }

        // メインピーク（最先端手前でそびえ立つ最大高さ195px）
        Pt2D peakPt    = { frontFootX - dir * 4.0f, basePosY - waveH };
        topNodes.push_back(peakPt);

        // 前面エッジの張り出し牙と先端足元
        Pt2D frontMid  = { frontFootX + dir * 8.0f, basePosY - waveH * 0.45f };
        Pt2D frontFoot = { frontFootX, basePosY };

        // 4重加算ブレンドレイヤー
        struct WaveLayer
        {
            float scaleY;
            float scaleX;
            unsigned int color;
            int alpha;
            int lineThick;
        };

        const WaveLayer layers[] = {
            { 1.06f, 1.05f, GetColor(0, 90, 255),    110, 4 }, // 外側ダークブルーオーラ
            { 1.00f, 1.00f, GetColor(0, 180, 255),   180, 3 }, // 外側ブライトシアン
            { 0.74f, 0.85f, GetColor(100, 235, 255), 220, 2 }, // 中層ライトシアン
            { 0.42f, 0.65f, GetColor(255, 255, 255), 255, 2 }, // 内側純白コア
        };

        for (const auto& layer : layers)
        {
            SetDrawBlendMode(DX_BLENDMODE_ADD, layer.alpha);

            auto scalePt = [&](const Pt2D& pt) -> Pt2D {
                float dx = pt.x - frontFootX;
                float dy = pt.y - basePosY;
                return { frontFootX + dx * layer.scaleX, basePosY + dy * layer.scaleY };
            };

            // 台形ストリップを2三角形ずつクリップ描画（重複ゼロ）
            for (size_t i = 0; i + 1 < topNodes.size(); i++)
            {
                Pt2D t0 = scalePt(topNodes[i]);
                Pt2D t1 = scalePt(topNodes[i + 1]);
                Pt2D b0 = { t0.x, basePosY };
                Pt2D b1 = { t1.x, basePosY };

                drawClippedTriangle(t0, t1, b1, vanishScreenX, facingRight, layer.color);
                drawClippedTriangle(t0, b1, b0, vanishScreenX, facingRight, layer.color);
                drawClippedLine(t0, t1, vanishScreenX, facingRight, layer.color, layer.lineThick);
            }

            // 前面エッジ部分を2三角形でクリップ描画（重複ゼロ）
            Pt2D sPeak       = scalePt(peakPt);
            Pt2D sPeakGround = { sPeak.x, basePosY };
            Pt2D sFrontMid   = scalePt(frontMid);
            Pt2D sFrontFoot  = scalePt(frontFoot);

            drawClippedTriangle(sPeak, sFrontMid, sFrontFoot, vanishScreenX, facingRight, layer.color);
            drawClippedTriangle(sPeak, sFrontFoot, sPeakGround, vanishScreenX, facingRight, layer.color);
            drawClippedLine(sPeak, sFrontMid, vanishScreenX, facingRight, layer.color, layer.lineThick);
            drawClippedLine(sFrontMid, sFrontFoot, vanishScreenX, facingRight, layer.color, layer.lineThick);
        }

        // --- 3. 舞い散るエネルギースパーク粒子（指定座標以内のみ描画） ---
        SetDrawBlendMode(DX_BLENDMODE_ADD, 210);
        for (int p = 0; p < 6; p++)
        {
            float seed = elapsed * 0.2f + static_cast<float>(p) * 1.05f;
            float sparkU = std::fmod(elapsed * 0.04f + static_cast<float>(p) * 0.17f, 1.0f);
            float sparkDistX = visualLen * sparkU;
            float sparkX = frontFootX - dir * sparkDistX + dir * (std::sin(seed) * 8.0f);

            // 指定座標を通過した粒子は描画しない
            if (facingRight && sparkX > vanishScreenX) continue;
            if (!facingRight && sparkX < vanishScreenX) continue;

            float baseH = waveH / (1.0f + 3.2f * sparkU);
            float sparkY = basePosY - baseH - std::fmod(elapsed * 2.5f + static_cast<float>(p) * 15.0f, 35.0f);
            int sparkSize = (p % 2 == 0) ? 3 : 2;
            DrawCircle(static_cast<int>(sparkX), static_cast<int>(sparkY), sparkSize, GetColor(220, 245, 255), TRUE);
        }

        // 接地面の電撃・光の筋（指定座標でクリップ）
        SetDrawBlendMode(DX_BLENDMODE_ADD, 150);
        for (int t = 0; t < 3; t++)
        {
            float offset = std::fmod(elapsed * 5.0f + static_cast<float>(t) * 40.0f, visualLen);
            Pt2D sPt = { frontFootX - dir * offset, basePosY - 2.0f };
            Pt2D ePt = { frontFootX - dir * (offset + 25.0f), basePosY - 2.0f };
            drawClippedLine(sPt, ePt, vanishScreenX, facingRight, GetColor(120, 210, 255), 2);
        }

        // --- 4. 指定座標に到達して順々に消えている瞬間の切断面エネルギー分解エフェクト ---
        if (isVanishing && overshoot < visualLen)
        {
            float uAtVanish = 1.0f - (overshoot / visualLen);
            float cutH = waveH / (1.0f + 3.2f * (1.0f - uAtVanish));
            int vx = static_cast<int>(vanishScreenX);
            int topY = static_cast<int>(basePosY - cutH);

            // 指定座標（切断面）に沿った縦の光柱ラインと散乱スパーク
            SetDrawBlendMode(DX_BLENDMODE_ADD, 230);
            DrawLine(vx, baseY, vx, topY, GetColor(255, 255, 255), 3);
            DrawLine(vx, baseY, vx, topY - 12, GetColor(100, 220, 255), 1);

            for (int k = 0; k < 4; k++)
            {
                float ratio = static_cast<float>(k + 1) / 5.0f;
                int sy = static_cast<int>(basePosY - cutH * ratio);
                float angleSeed = elapsed * 0.7f + static_cast<float>(k) * 1.9f;
                int sx = vx - static_cast<int>(dir * (6.0f + 10.0f * std::abs(std::sin(angleSeed))));
                int ey = sy - static_cast<int>(8.0f + 12.0f * std::abs(std::cos(angleSeed)));
                DrawLine(vx, sy, sx, ey, GetColor(200, 245, 255), 2);
                DrawCircle(sx, ey, 2, GetColor(255, 255, 255), TRUE);
            }
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

#include "BossEnemySlime.h"
#include "GameConstants.h"
#include "Player.h"
#include "BlockMap.h"
#include "Collision.h"
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
    , mJumpSpeedX(0.0f)
    , mTackleVelocityY(0.0f)
    , mTargetPlayerX(initPos.x)
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
}

void BossEnemySlime::StartBattle()
{
    mIsBattleActive = true;
    // 登場演出終了と同時に行動を開始できるよう待機タイマーを満たす
    mIdleTimer = KingSlimeAIConstants::IdleWaitFrames;
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

    if (mTackleCooldownTimer > 0)
    {
        mTackleCooldownTimer--;
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
            mpAnimAttack->Update();
            int currentFrame = mpAnimAttack->GetCurrentFrame();

            // 2枚目（インデックス1）の時にプレイヤー座標を記録
            if (currentFrame == 1 && mpPlayer != nullptr)
            {
                float px = mpPlayer->GetPosition().x;
                float dir = (px > mvPosition.x) ? 1.0f : -1.0f;
                mTargetPlayerX = px + dir * KingSlimeAIConstants::TackleOvershootX;

                // ボスエリア外に出ないようにクランプ
                float minX = mBossAreaCenterX - KingSlimeAIConstants::EdgeOffsetX - 50.0f;
                float maxX = mBossAreaCenterX + KingSlimeAIConstants::EdgeOffsetX + 50.0f;
                if (mTargetPlayerX < minX) mTargetPlayerX = minX;
                else if (mTargetPlayerX > maxX) mTargetPlayerX = maxX;
                mIsFacingRight = (dir > 0.0f);
            }

            // 4枚目（インデックス3）になったら小ジャンプ突進開始
            if (currentFrame >= 3 && !mHasTackleLaunched)
            {
                mHasTackleLaunched = true;
                // 残り4コマ（3, 4, 5, 6） × interval
                float jumpFrames = 4.0f * static_cast<float>(KingSlimeAIConstants::TackleAnimInterval);
                mJumpSpeedX = (mTargetPlayerX - mvPosition.x) / jumpFrames;
                mTackleVelocityY = KingSlimeAIConstants::TackleJumpSpeedY;
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
    if (mState == KingSlimeState::BeamAttack && mBeamPhase == BeamPhase::Firing)
    {
        const float dir = mIsFacingRight ? 1.0f : -1.0f;
        const float mouthWorldX = mvPosition.x + dir * KingSlimeAIConstants::BeamMouthOffsetX;
        const float mouthWorldY = mvPosition.y + KingSlimeAIConstants::BeamMouthOffsetY;

        const float beamLeft = mIsFacingRight ? mouthWorldX : (mouthWorldX - KingSlimeAIConstants::BeamLength);
        const float beamTop = mouthWorldY - KingSlimeAIConstants::BeamHalfHeight;

        VECTOR beamPos = VGet(beamLeft, beamTop, 0.0f);
        VECTOR beamSize = VGet(KingSlimeAIConstants::BeamLength, KingSlimeAIConstants::BeamHalfHeight * 2.0f, 0.0f);

        return Collision::CheckRectToRect(targetPos, targetSize, beamPos, beamSize);
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
        if (mState == KingSlimeState::SpawnFalling ||
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

        if (currentAnim != nullptr)
        {
            currentAnim->SetPosition(VGet(screenX, screenY, 0.0f));
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
}


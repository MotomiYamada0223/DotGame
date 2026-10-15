#include "BossEnemySlime.h"
#include "GameConstants.h"
#include "Player.h"
#include "BlockMap.h"

BossEnemySlime::BossEnemySlime(VECTOR initPos, float groundY, Player* targetPlayer)
    : Enemy(CharacterGraphPath::KingSlimeIdle, initPos)
    , mState(KingSlimeState::SpawnFalling)
    , mGroundY(groundY)
    , mpPlayer(targetPlayer)
    , mIsFacingRight(false)
    , mFallSpeed(26.0f)
    , mCrushProcessed(false)
{
    // キングスライムの当たり判定サイズ (4倍)
    mfEnemyWidth = 90.0f * KingSlimeIntroConstants::HitBoxScale;
    mfEnemyHeight = 75.0f * KingSlimeIntroConstants::HitBoxScale;

    mpAnimIdle = new TextureAnimation(initPos, CharacterGraphPath::KingSlimeIdle, 6, 6, 1, 8);
    mpAnimMove = new TextureAnimation(initPos, CharacterGraphPath::KingSlimeMove, 7, 7, 1, 8);
    mpAnimJump = new TextureAnimation(initPos, CharacterGraphPath::KingSlimeJump, 6, 6, 1, 5);
    mpAnimAttack = new TextureAnimation(initPos, CharacterGraphPath::KingSlimeAttack, 7, 7, 1, 8);

    // アニメーションのスケール設定 (4倍)
    mpAnimIdle->SetScale(KingSlimeIntroConstants::Scale);
    mpAnimMove->SetScale(KingSlimeIntroConstants::Scale);
    mpAnimJump->SetScale(KingSlimeIntroConstants::Scale);
    mpAnimAttack->SetScale(KingSlimeIntroConstants::Scale);

    // 出現時はジャンプモーションの6枚目（インデックス5）で固定
    if (mpAnimJump)
    {
        mpAnimJump->SetCurrentFrame(5);
        mpAnimJump->SetReverse(true);
        mpAnimJump->SetLoop(false);
    }

    UpdateStatusByProgress(GameProgress::Tutorial1);
}

BossEnemySlime::~BossEnemySlime()
{
    if (mpAnimIdle) delete mpAnimIdle;
    if (mpAnimMove) delete mpAnimMove;
    if (mpAnimJump) delete mpAnimJump;
    if (mpAnimAttack) delete mpAnimAttack;
}

void BossEnemySlime::Update()
{
    Enemy::Update();

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
}

void BossEnemySlime::EnemyMove(BlockMap& blockMap)
{
    mpBlockMap = &blockMap;

    if (mState == KingSlimeState::SpawnFalling)
    {
        mvPosition.y += mFallSpeed;

        // キングスライムの足元が地面Yに達したか判定 (中心座標基準)
        const float targetCenterY = mGroundY - (mfEnemyHeight * 0.5f);
        if (mvPosition.y >= targetCenterY)
        {
            mvPosition.y = targetCenterY;

            // 着地！
            mState = KingSlimeState::SpawnLandingReverse;

            // 着地時の画面揺れ発生（縦8割・横2割、約0.4秒）
            blockMap.StartCameraShake(
                KingSlimeShakeConstants::ShakePower,
                KingSlimeShakeConstants::RatioX,
                KingSlimeShakeConstants::RatioY,
                KingSlimeShakeConstants::DurationFrames
            );

            if (mpAnimJump)
            {
                mpAnimJump->SetCurrentFrame(5);
                mpAnimJump->SetReverse(true);
                mpAnimJump->SetLoop(false);
                mpAnimJump->SetInterval(5);
            }

            // 押しつぶし判定（Eキー回避を行っていなければプレイヤー即死）
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
    }
}

void BossEnemySlime::Draw()
{
    if (mpBlockMap != nullptr)
    {
        const float screenX = ConvertToScreenX(mvPosition.x, mpBlockMap);
        // テクスチャの原点オフセット補正 (テクスチャ200x200の中心Y=100に対してスライム見た目中心Y=84のため、差分16px * Scaleを下方に補正)
        const float textureCenterY = mvPosition.y + (16.0f * KingSlimeIntroConstants::Scale);
        const float screenY = ConvertToScreenY(textureCenterY, mpBlockMap);

        if (isDamaged) { SetDrawBright(255, 100, 100); }

        TextureAnimation* currentAnim = mpAnimIdle;
        if (mState == KingSlimeState::SpawnFalling || mState == KingSlimeState::SpawnLandingReverse)
        {
            currentAnim = mpAnimJump;
        }

        if (currentAnim != nullptr)
        {
            currentAnim->SetPosition(VGet(screenX, screenY, 0.0f));
            currentAnim->Draw(mIsFacingRight);
        }

        if (isDamaged) { SetDrawBright(255, 255, 255); }
    }
    else
    {
        Object2D::Draw();
    }
}

void BossEnemySlime::UpdateStatusByProgress(GameProgress progress)
{
    switch (progress)
    {
    case GameProgress::Tutorial1:
        mMaxHp = 100; mHp = 100;
        mAttack = 10;
        mHasInstantKillAttack = false;
        break;
    default:
        mMaxHp = 100; mHp = 100;
        mAttack = 10;
        mHasInstantKillAttack = false;
        break;
    }
}

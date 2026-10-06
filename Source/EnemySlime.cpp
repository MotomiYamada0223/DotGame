#include "EnemySlime.h"
#include "Master.h"
#include "ObjectManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "GameManager.h"
#include <cmath>
#include "DxLib.h"
#include "Texture.h"
#include "GameConstants.h"
#include "BlockMap.h"
#include "Player.h"

EnemySlime::EnemySlime(VECTOR initPos)
    : Enemy("Resource/Image/SampleSlime.png", initPos)
{
    mMoveSpeed = 1.0f; // スライムの移動速度

    // 敵のサイズ設定
    mfEnemyWidth = 64.0f;
    mfEnemyHeight = 64.0f;

    mpAnimIdle = new TextureAnimation(initPos, "Resource/Image/slime_idle.png", 6, 6, 1, 8);
    mpAnimMove = new TextureAnimation(initPos, "Resource/Image/slime_move.png", 6, 6, 1, 8);
    mpAnimAttack = new TextureAnimation(initPos, "Resource/Image/slime_attack.png", 7, 7, 1, 8);

    mCurrentMoveDirection = 1.0f;
    mActionTimer = 48;
    mIsFacingRight = true;
    mIsChasing = false;

    // 視界の設定
    mVisionLength = 600.0f;
    mVisionBaseHeight = 50.0f;
    mVisionAngle = 15.0f;

    // 攻撃関連の初期化
    mIsAttacking = false;
    mAttackCooldownTimer = 0;
    mTargetPlayerPos = VGet(0, 0, 0);
    mHasJumped = false;
    mJumpSpeedX = 0.0f;

    UpdateStatusByProgress(GameProgress::Tutorial1);
}

EnemySlime::~EnemySlime()
{
    if (mpAnimIdle) delete mpAnimIdle;
    if (mpAnimMove) delete mpAnimMove;
    if (mpAnimAttack) delete mpAnimAttack;
}

void EnemySlime::Update()
{
    Enemy::Update();

    // クールタイムの減少
    if (mAttackCooldownTimer > 0)
    {
        mAttackCooldownTimer--;
    }

    if (mpAnimIdle) mpAnimIdle->Update();
    if (mpAnimMove) mpAnimMove->Update();
    if (mpAnimAttack) mpAnimAttack->Update();
}

void EnemySlime::Draw()
{
    if (mpBlockMap != nullptr)
    {
        const float screenX = ConvertToScreenX(mvPosition.x, mpBlockMap);
        const float screenY = mvPosition.y;

        // --- 視界の扇状(台形)デバッグ表示 ---
        float baseHalfY = mVisionBaseHeight / 2.0f;
        float farHalfY = baseHalfY + mVisionLength * std::tan(mVisionAngle * 0.01745329f);

        int x1, y1, x2, y2, x3, y3, x4, y4;

        if (mIsFacingRight)
        {
            x1 = static_cast<int>(screenX);                   y1 = static_cast<int>(screenY - baseHalfY);
            x2 = static_cast<int>(screenX + mVisionLength);   y2 = static_cast<int>(screenY - farHalfY);
            x3 = static_cast<int>(screenX + mVisionLength);   y3 = static_cast<int>(screenY + farHalfY);
            x4 = static_cast<int>(screenX);                   y4 = static_cast<int>(screenY + baseHalfY);
        }
        else
        {
            x1 = static_cast<int>(screenX);                   y1 = static_cast<int>(screenY - baseHalfY);
            x2 = static_cast<int>(screenX);                   y2 = static_cast<int>(screenY + baseHalfY);
            x3 = static_cast<int>(screenX - mVisionLength);   y3 = static_cast<int>(screenY + farHalfY);
            x4 = static_cast<int>(screenX - mVisionLength);   y4 = static_cast<int>(screenY - farHalfY);
        }

        int color = mIsChasing ? GetColor(255, 0, 0) : GetColor(255, 255, 0);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawQuadrangle(x1, y1, x2, y2, x3, y3, x4, y4, color, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        // ------------------------

        if (isDamaged) { SetDrawBright(255, 100, 100); }

        // 現在のモーション
        TextureAnimation* currentAnim = mpAnimIdle;

        if (mIsAttacking)
        {
            currentAnim = mpAnimAttack;
        }
        else if (mCurrentMoveDirection != 0.0f)
        {
            currentAnim = mpAnimMove;
        }

        if (currentAnim != nullptr)
        {
            currentAnim->SetPosition(VGet(screenX, screenY, 0.0f));
            bool turnFlag = mIsFacingRight;
            currentAnim->Draw(turnFlag);
        }

        if (isDamaged) { SetDrawBright(255, 255, 255); }
    }
    else
    {
        Object2D::Draw();
    }
}

void EnemySlime::EnemyMove(BlockMap& blockMap)
{
    mpBlockMap = &blockMap;

    ObjectManager* objManager = Master::mpGameManager->GetSceneManager()->GetCurrentScene()->GetObjectManager();
    Object2D* playerObj = objManager->GetObject2DByTag(Object2D::Tag::Player2D);

    if (playerObj != nullptr)
    {
        float playerX = playerObj->GetPosition().x;
        float playerY = playerObj->GetPosition().y;
        float enemyX = mvPosition.x;
        float enemyY = mvPosition.y;
        float dx = std::abs(playerX - enemyX);
        float dy = std::abs(playerY - enemyY);
        float dist = std::sqrt(dx * dx + dy * dy);



        if (!IsPlayerDead())
        {
            // 攻撃中の処理
            if (mIsAttacking)
            {
                int currentFrame = mpAnimAttack ? mpAnimAttack->GetCurrentFrame() : 0;

                // 2枚目(インデックス1)の時にプレイヤー座標を記録
                if (currentFrame == 1 && mTargetPlayerPos.x == 0.0f && mTargetPlayerPos.y == 0.0f)
                {
                    mTargetPlayerPos = playerObj->GetPosition();
                    // プレイヤーの少し奥(100.0f)を目標にする
                    if (mTargetPlayerPos.x > enemyX) {
                        mTargetPlayerPos.x += 100.0f;
                    }
                    else {
                        mTargetPlayerPos.x -= 100.0f;
                    }
                }

                // 4枚目(インデックス3)になったらジャンプ突進開始
                if (currentFrame == 3 && !mHasJumped)
                {
                    mbIsJumping = true;
                    mHasJumped = true;

                    // アニメーションは全7枚、インデックス3(4枚目)から飛びつき開始
                    // SetInterval(6)の場合、残り4枚(3,4,5,6) × 6フレーム = 約24フレーム
                    float jumpFrames = 24.0f;

                    // 攻撃モーション終了時に目標のX座標へ必ず到達するように速度を算出
                    mJumpSpeedX = (mTargetPlayerPos.x - enemyX) / jumpFrames;

                    // 攻撃モーション終了時に目標のY座標（あるいは着地）に必ず到達するように初速を算出
                    // 等加速度直線運動: Δy = v0 * t + 0.5 * g * t^2
                    // v0 = (Δy / t) - 0.5 * g * t
                    float deltaY = mTargetPlayerPos.y - enemyY;
                    velocityY = (deltaY / jumpFrames) - (0.5f * gravity * jumpFrames);
                }

                // ジャンプ中(空中にいる)の移動処理
                if (mHasJumped && !isGrounded)
                {
                    mMoveSpeed = std::abs(mJumpSpeedX);
                    mCurrentMoveDirection = (mJumpSpeedX > 0) ? 1.0f : -1.0f;
                }
                else
                {
                    // 地面にいる時（溜め中、または着地後）は移動しない
                    mCurrentMoveDirection = 0.0f;
                }

                // 攻撃終了判定 (アニメーションが最後までいったか、着地後)
                if (currentFrame >= 6 || (mHasJumped && isGrounded && velocityY >= 0))
                {
                    mIsAttacking = false;
                    mHasJumped = false;
                    mTargetPlayerPos = VGet(0, 0, 0);
                    // クールタイム3〜4秒
                    mAttackCooldownTimer = 180 + GetRand(60);
                }
            }
            // 非攻撃中の処理
            else
            {
                if (mIsChasing)
                {
                    // --- 追跡中の処理 ---

                    // 攻撃範囲内(距離400以下)でクールタイムが明けていれば攻撃開始
                    if (dist <= 400.0f && mAttackCooldownTimer <= 0)
                    {
                        mIsAttacking = true;
                        mCurrentMoveDirection = 0.0f;
                        mHasJumped = false;
                        mTargetPlayerPos = VGet(0, 0, 0);
                        if (mpAnimAttack) mpAnimAttack->ResetAnimation();

                        // 攻撃時は確実にプレイヤーの方を向く
                        mIsFacingRight = (playerX > enemyX);
                    }
                    else
                    {
                        // 追跡移動処理
                        mActionTimer--;
                        if (mActionTimer <= 0)
                        {
                            if (mCurrentMoveDirection == 0.0f)
                            {
                                // Idle終わり -> 追跡開始 (3〜4秒)
                                mActionTimer = 180 + GetRand(60);
                                mCurrentMoveDirection = mIsFacingRight ? 1.0f : -1.0f;
                            }
                            else
                            {
                                // 追跡終わり -> Idle開始 (1〜2ループ)
                                int idleLoops = 1 + GetRand(1);
                                mActionTimer = idleLoops * 48;
                                mCurrentMoveDirection = 0.0f;
                            }
                        }

                        // 常にプレイヤーの方向に視界(自分の向き)を合わせる
                        if (playerX > enemyX) {
                            mIsFacingRight = true;
                            if (mCurrentMoveDirection != 0.0f) mCurrentMoveDirection = 1.0f;
                        }
                        else {
                            mIsFacingRight = false;
                            if (mCurrentMoveDirection != 0.0f) mCurrentMoveDirection = -1.0f;
                        }

                        if (dist >= mVisionLength)
                        {
                            mIsChasing = false;
                        }
                    }
                }
                else
                {
                    // --- 未発見（徘徊中）の処理 ---

                    float dxVision = mIsFacingRight ? (playerX - enemyX) : (enemyX - playerX);

                    if (dxVision >= 0.0f && dxVision <= mVisionLength)
                    {
                        float maxDy = (mVisionBaseHeight / 2.0f) + dxVision * std::tan(mVisionAngle * 0.01745329f);

                        if (dy <= maxDy)
                        {
                            mIsChasing = true;
                            mActionTimer = 180 + GetRand(60);
                            mCurrentMoveDirection = mIsFacingRight ? 1.0f : -1.0f;
                        }
                    }
                }
            }
        }
    }

    // スピードの設定
    if (mIsAttacking)
    {
        // 攻撃時のスピードはジャンプ時に計算した mJumpSpeedX を適用済み
        if (mpAnimAttack) mpAnimAttack->SetInterval(7); // 攻撃は少し早めに再生
    }
    else if (mIsChasing)
    {
        mMoveSpeed = 2.0f;
        if (mpAnimMove) mpAnimMove->SetInterval(7);
    }
    else
    {
        mMoveSpeed = 1.0f;
        if (mpAnimMove) mpAnimMove->SetInterval(8);
    }

    // 追跡中でない場合(かつ攻撃中でもない場合)はランダム徘徊
    if (!mIsChasing && !mIsAttacking)
    {
        mActionTimer--;
        if (mActionTimer <= 0)
        {
            int randAction = GetRand(2); // 0, 1, 2
            if (randAction == 0) mCurrentMoveDirection = 1.0f;
            else if (randAction == 1) mCurrentMoveDirection = -1.0f;
            else mCurrentMoveDirection = 0.0f;

            int loopCount = 1 + GetRand(2);
            mActionTimer = loopCount * 48;
        }

        if (mCurrentMoveDirection > 0.0f) mIsFacingRight = true;
        else if (mCurrentMoveDirection < 0.0f) mIsFacingRight = false;
    }
    else if (!mIsAttacking)
    {
        // 追跡中も向きを更新
        if (mCurrentMoveDirection > 0.0f) mIsFacingRight = true;
        else if (mCurrentMoveDirection < 0.0f) mIsFacingRight = false;
    }

    mCharacterPhysics.UpdateMoveAndCollision(
        mvPosition,
        velocityY,
        isGrounded,
        mbIsJumping,
        blockMap,
        mfEnemyWidth,
        mfEnemyHeight,
        gravity,
        mMoveSpeed,
        mCurrentMoveDirection
    );
}

void EnemySlime::UpdateStatusByProgress(GameProgress progress)
{
    switch (progress)
    {
    case GameProgress::Tutorial1:
        mMaxHp = 15; mHp = 15;
        mAttack = 9999;
        mHasInstantKillAttack = true; // Tutorial1時のみ即死攻撃
        break;
    default:
        mMaxHp = 15; mHp = 15;
        mAttack = 5;
        mHasInstantKillAttack = false; // それ以外は通常の攻撃力(5)
        break;
    }
}

bool EnemySlime::IsPlayerDead()
{
    ObjectManager* objManager = Master::mpGameManager->GetSceneManager()->GetCurrentScene()->GetObjectManager();
    Object2D* playerObj = objManager->GetObject2DByTag(Object2D::Tag::Player2D);

    if (playerObj != nullptr)
    {
        Player* player = dynamic_cast<Player*>(playerObj);
        if (player != nullptr && player->GetIsDead())
        {
            return true;
        }
    }
    return false;
}
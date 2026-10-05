import sys

content = """#include "EnemySlime.h"
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

EnemySlime::EnemySlime(VECTOR initPos)
    : Enemy("Resource/Image/SampleSlime.png", initPos)
{
    mMoveSpeed = 1.0f; // スライムの移動速度

    // 敵のサイズ設定(画像のサイズのまま使うか)
    // ひとまわり画像が400x400で大きめなので、縮小するか、
    // 見た目通り 1/6 サイズ程度にするか、今のままの比率で設定
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
            // 時計回りに頂点を指定
            x1 = static_cast<int>(screenX);                   y1 = static_cast<int>(screenY - baseHalfY); // 左上
            x2 = static_cast<int>(screenX + mVisionLength);   y2 = static_cast<int>(screenY - farHalfY);  // 右上
            x3 = static_cast<int>(screenX + mVisionLength);   y3 = static_cast<int>(screenY + farHalfY);  // 右下
            x4 = static_cast<int>(screenX);                   y4 = static_cast<int>(screenY + baseHalfY); // 左下
        }
        else
        {
            // 時計回りに指定 (DXライブラリではDrawQuadrangleは時計回りまたは反時計回りでねじれないように)
            x1 = static_cast<int>(screenX);                   y1 = static_cast<int>(screenY - baseHalfY); // 右上
            x2 = static_cast<int>(screenX);                   y2 = static_cast<int>(screenY + baseHalfY); // 右下
            x3 = static_cast<int>(screenX - mVisionLength);   y3 = static_cast<int>(screenY + farHalfY);  // 左下
            x4 = static_cast<int>(screenX - mVisionLength);   y4 = static_cast<int>(screenY - farHalfY);  // 左上
        }
        
        // 追跡中は赤、待機中は黄色
        int color = mIsChasing ? GetColor(255, 0, 0) : GetColor(255, 255, 0);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawQuadrangle(x1, y1, x2, y2, x3, y3, x4, y4, color, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        // ------------------------

        if (isDamaged) { SetDrawBright(255, 100, 100); }
        
        // 現在のモーション
        TextureAnimation* currentAnim = mpAnimIdle; // 基本Idle
        
        if (mCurrentMoveDirection != 0.0f)
        {
            currentAnim = mpAnimMove; // 移動ならMove
        }
        
        if (currentAnim != nullptr)
        {
            currentAnim->SetPosition(VGet(screenX, screenY, 0.0f));
            
            // 右向きならfalse、左向きならtrueで反転させる
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
        
        if (mIsChasing)
        {
            // --- 追跡中の処理 ---
            // 常にプレイヤーの方向に視界(自分の向き)と移動方向を合わせる（振り向く）
            if (playerX > enemyX) {
                mIsFacingRight = true;
                mCurrentMoveDirection = 1.0f;
            } else {
                mIsFacingRight = false;
                mCurrentMoveDirection = -1.0f;
            }
            
            // 逃げ切られたかの判定（直線距離が600以上になったら見失う）
            float dx = std::abs(playerX - enemyX);
            float dy = std::abs(playerY - enemyY);
            float dist = std::sqrt(dx * dx + dy * dy);
            
            if (dist >= mVisionLength)
            {
                mIsChasing = false;
            }
        }
        else
        {
            // --- 未発見（徘徊中）の処理 ---
            // 扇状の視界判定
            float dx = mIsFacingRight ? (playerX - enemyX) : (enemyX - playerX);
            float dy = std::abs(playerY - enemyY);
            
            // 向きの方向にいて、長さ以内か
            if (dx >= 0.0f && dx <= mVisionLength)
            {
                float maxDy = (mVisionBaseHeight / 2.0f) + dx * std::tan(mVisionAngle * 0.01745329f);
                
                if (dy <= maxDy)
                {
                    mIsChasing = true;
                    // 発見した瞬間に向きを更新
                    mCurrentMoveDirection = mIsFacingRight ? 1.0f : -1.0f;
                }
            }
        }
    }
    
    if (mIsChasing)
    {
        mMoveSpeed = 2.0f;
        if (mpAnimMove) mpAnimMove->SetInterval(7);
    }
    else
    {
        mMoveSpeed = 1.0f;
        if (mpAnimMove) mpAnimMove->SetInterval(8);
    }

    // 追跡中でない場合はランダム徘徊
    if (!mIsChasing)
    {
        mActionTimer--;
        if (mActionTimer <= 0)
        {
            int randAction = GetRand(2); // 0, 1, 2
            if (randAction == 0) mCurrentMoveDirection = 1.0f;
            else if (randAction == 1) mCurrentMoveDirection = -1.0f;
            else mCurrentMoveDirection = 0.0f; // 停止
            
            int loopCount = 1 + GetRand(2); // 1, 2, 3
            mActionTimer = loopCount * 48;
        }
        
        // 向きの保存
        if (mCurrentMoveDirection > 0.0f) mIsFacingRight = true;
        else if (mCurrentMoveDirection < 0.0f) mIsFacingRight = false;
    }
    else
    {
        // 追跡中も向きを更新（プレイヤーを追い越した時のため）
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
        mHasInstantKillAttack = true;
        break;
    default: 
        mMaxHp = 15; mHp = 15;
        mAttack = 5;
        mHasInstantKillAttack = false;
        break;
    }
}
"""

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)


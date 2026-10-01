#include "EnemySlime.h"
#include "DxLib.h"
#include "Texture.h"
#include "GameConstants.h"
#include "BlockMap.h"

EnemySlime::EnemySlime(VECTOR initPos)
    : Enemy("Resource/Image/SampleSlime.png", initPos)
{
    mMoveSpeed = 2.0f; // スライム専用の移動速度

    // 当たり判定のサイズ設定（画像のサイズをそのまま使うか調整するか）
    // ひとまず画像が400x400と大きめなので、少し縮小するか検討しつつ、
    // 既存の通り 1/6 サイズ程度にするか、一旦そのままの比率で設定
    mfEnemyWidth = 64.0f;
    mfEnemyHeight = 64.0f;

    mpAnimIdle = new TextureAnimation(initPos, "Resource/Image/slime_idle.png", 6, 6, 1, 8);
    mpAnimMove = new TextureAnimation(initPos, "Resource/Image/slime_move.png", 6, 6, 1, 8);
    mpAnimAttack = new TextureAnimation(initPos, "Resource/Image/slime_attack.png", 7, 7, 1, 8);

    mCurrentMoveDirection = 1.0f;
    mActionTimer = 48;
    mIsFacingRight = true;

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
        if (isDamaged) { SetDrawBright(255, 100, 100); }

        const float screenX = ConvertToScreenX(mvPosition.x, mpBlockMap);
        const float screenY = mvPosition.y;
        
        // 現在のモーションを決定
        TextureAnimation* currentAnim = mpAnimIdle; // 基本はIdle
        
        if (mCurrentMoveDirection != 0.0f)
        {
            currentAnim = mpAnimMove; // 移動中ならMove
        }
        
        // 攻撃判定などがあれば mpAnimAttack にするが、今回は移動と待機のみ
        
        if (currentAnim != nullptr)
        {
            currentAnim->SetPosition(VGet(screenX, screenY, 0.0f));
            
            // 右向きならfalse、左向きならtrueで反転させる (画像がデフォで右向きを想定)
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
    
    // ランダム行動タイマー
    mActionTimer--;
    if (mActionTimer <= 0)
    {
        int randAction = GetRand(2); // 0, 1, 2
        if (randAction == 0) mCurrentMoveDirection = 1.0f;
        else if (randAction == 1) mCurrentMoveDirection = -1.0f;
        else mCurrentMoveDirection = 0.0f; // 停止
        
        // 1秒から3秒で次の行動へ
                // 1枚8フレーム × 6枚 = 1ループ48フレーム
        // 1〜3回モーション(ループ)を行ったら次の行動へ移るようにする
        int loopCount = 1 + GetRand(2); // 1, 2, 3
        mActionTimer = loopCount * 48;
    }
    
    // 向きの保存
    if (mCurrentMoveDirection > 0.0f) mIsFacingRight = true;
    else if (mCurrentMoveDirection < 0.0f) mIsFacingRight = false;

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
#include "NeedleTrap.h"
#include "GameConstants.h"
#include "DxLib.h"
#include "Master.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Player.h"
#include "RollingBoulder.h"
#include <cmath>

NeedleTrap::NeedleTrap(VECTOR initPos, TrapType type, const std::string& graphPath, float scale)
    : Enemy(graphPath.empty() ? CharacterGraphPath::Needle : graphPath, initPos)
    , UnitStatus()
    , mTargetY(0)
    , mScale(scale)
{
    mType = type;
    mState = TrapState::Waiting;
    mBaseY = initPos.y;
    mTrapSpeed = 30.0f;
    
    std::string path = graphPath.empty() ? CharacterGraphPath::Needle : graphPath;
    mGraphHandle = LoadGraph(path.c_str());

    if (mType == TrapType::Normal)
    {
        mvPosition.y = mBaseY;
        mTargetY = mBaseY;
        mState = TrapState::Finished;
    }
    else if (mType == TrapType::PopUp)
    {
        mvPosition.y = mBaseY + 64.0f;
        mTargetY = mBaseY - 32.0f;
    }
    else if (mType == TrapType::AntiJump)
    {
        mvPosition.y = mBaseY;
        mTargetY = mBaseY - 128.0f;
    }
    else if (mType == TrapType::Falling)
    {
        mvPosition.y = mBaseY;
        mTargetY = mBaseY;
    }

    if (path == CharacterGraphPath::Needle)
    {
        mScale = 3.0f;
        mfEnemyWidth = 96.0f;
        mfEnemyHeight = 96.0f;
    }
    else
    {
        int w = 0, h = 0;
        GetGraphSize(mGraphHandle, &w, &h);
        mfEnemyWidth = static_cast<float>(w) * mScale;
        mfEnemyHeight = static_cast<float>(h) * mScale;
    }

    mHasInstantKillAttack = true;
    mIgnoresInvincibility = true;
    mHp = 9999999;
}

NeedleTrap::~NeedleTrap()
{
    if (mGraphHandle != -1)
    {
        DeleteGraph(mGraphHandle);
    }
}

void NeedleTrap::UpdateStatusByProgress(GameProgress progress)
{
    mMaxHp = 9999999;
    mHp = 9999999;
    mAttack = 9999;
    mHasInstantKillAttack = true;
    mIgnoresInvincibility = true;
}

void NeedleTrap::Update()
{
    Enemy::Update();
}

void NeedleTrap::Draw()
{
    if (IsDeleteFlag()) return;
    if (mState == TrapState::Finished && (mType == TrapType::AntiJump || mType == TrapType::Falling)) return;
    if (mState == TrapState::Waiting && mType == TrapType::Falling) return;

    if (mGraphHandle != -1 && mpBlockMap != nullptr)
    {
        int drawX = static_cast<int>(ConvertToScreenX(mvPosition.x, mpBlockMap));
        int drawY = static_cast<int>(ConvertToScreenY(mvPosition.y, mpBlockMap));

        double angle = (mType == TrapType::Falling) ? DX_PI : 0.0;
        DrawRotaGraph(drawX, drawY, static_cast<double>(mScale), angle, mGraphHandle, TRUE);
    }
}

void NeedleTrap::EnemyMove(BlockMap& blockMap)
{
    mpBlockMap = &blockMap;
    if (IsDeleteFlag() || mState == TrapState::Finished || mType == TrapType::Normal) return;

    Scene* currentScene = Master::mpGameManager->GetSceneManager()->GetCurrentScene();
    if (!currentScene) return;
    
    ObjectManager* objManager = currentScene->GetObjectManager();
    if (!objManager) return;

    std::vector<Object2D*> players = objManager->GetObject2DListByTag(Object2D::Player2D);
    if (players.empty()) return;

    Player* player = dynamic_cast<Player*>(players[0]);
    if (!player) return;

    float playerX = player->GetPosition().x;
    float playerY = player->GetPosition().y;

    float distX = std::abs(playerX - mvPosition.x);

    if (mState == TrapState::Waiting)
    {
        if (mType == TrapType::PopUp)
        {
            // 横方向の感知距離を狭め、高い空中ジャンプ中の暴発を防ぐため地面付近（低空・接地時）のみ感知
            if (distX < 85.0f && playerY >= mBaseY - 80.0f)
            {
                mState = TrapState::Active;
            }
        }
        else if (mType == TrapType::AntiJump)
        {
            // x座標の感知範囲を狭める (150 -> 60)
            if (distX < 60.0f && playerY < mvPosition.y - 100.0f)
            {
                mState = TrapState::Active;
            }
        }
        else if (mType == TrapType::Falling)
        {
            // 壊せる壁の手前にプレイヤーが到達したとき
            if (distX < GameLoadDataConstants::CaveFallingNeedleTriggerDistX && playerY >= 1400.0f)
            {
                mState = TrapState::Active;
                // 画面の上端のすぐ外側から降らせる
                float scrollY = (mpBlockMap != nullptr) ? static_cast<float>(mpBlockMap->GetDrawScrollY()) : 0.0f;
                mvPosition.y = scrollY - mfEnemyHeight * 0.5f - 10.0f;
            }
        }
    }
    else if (mState == TrapState::Active)
    {
        if (mType == TrapType::Falling)
        {
            mvPosition.y += mTrapSpeed;

            // 地面に突き刺さるか、画面外（下）へ抜けたときに消滅
            float screenTopY = ConvertToScreenY(mvPosition.y - mfEnemyHeight * 0.5f, mpBlockMap);
            if (mvPosition.y >= GameLoadDataConstants::CaveFallingNeedleFloorY || screenTopY > ScreenSize::ScrrenHeight)
            {
                mState = TrapState::Finished;
                SetDeleteFlag(true);
            }
        }
        else
        {
            mvPosition.y -= mTrapSpeed;
            
            if (mType == TrapType::AntiJump)
            {
                // 針の最下端（中心 + 高さ半分）が画面上端（0）より上に出たら消滅
                float screenBottomY = ConvertToScreenY(mvPosition.y + mfEnemyHeight * 0.5f, mpBlockMap);
                if (screenBottomY < 0.0f)
                {
                    mState = TrapState::Finished;
                    SetDeleteFlag(true);
                }
            }
            else
            {
                if (mvPosition.y <= mTargetY)
                {
                    mvPosition.y = mTargetY;
                    mState = TrapState::Finished;
                }
            }
        }
    }
}

// BoulderNeedleTrap 実装
BoulderNeedleTrap::BoulderNeedleTrap(VECTOR initPos, const RollingBoulder* rollingBoulder)
    : NeedleTrap(initPos, TrapType::Normal, CharacterGraphPath::BloodNeedle, 1.0f)
    , mpRollingBoulder(rollingBoulder)
    , mSpikeState(SpikeState::Hidden)
    , mSpikeHeight(0.0f)
    , mSpikeMaxHeight(115.0f)
    , mSpikeVy(0.0f)
    , mBaseX(initPos.x)
    , mBaseY(initPos.y)
{
    mfEnemyWidth = 72.0f;
    mfEnemyHeight = 115.0f;
}

BoulderNeedleTrap::~BoulderNeedleTrap()
{
}

void BoulderNeedleTrap::Update()
{
    Enemy::Update();
}

void BoulderNeedleTrap::EnemyMove(BlockMap& blockMap)
{
    mpBlockMap = &blockMap;
    if (IsDeleteFlag() || mSpikeState == SpikeState::Finished) return;

    Scene* currentScene = Master::mpGameManager->GetSceneManager()->GetCurrentScene();
    if (!currentScene) return;

    ObjectManager* objManager = currentScene->GetObjectManager();
    if (!objManager) return;

    std::vector<Object2D*> players = objManager->GetObject2DListByTag(Object2D::Player2D);
    if (players.empty()) return;

    Player* player = dynamic_cast<Player*>(players[0]);
    if (!player) return;

    VECTOR pPos = player->GetPosition();

    // 1. Hidden -> Emerging 判定（プレイヤー進入または岩の接近）
    if (mSpikeState == SpikeState::Hidden)
    {
        bool playerInRecess = false;
        if (!player->GetIsDead())
        {
            if (pPos.x >= 4270.0f && pPos.x <= 4360.0f && pPos.y >= 1910.0f)
            {
                playerInRecess = true;
            }
        }

        bool boulderAtRecess = (mpRollingBoulder != nullptr && mpRollingBoulder->HasPassedRecess());

        if (playerInRecess || boulderAtRecess)
        {
            mSpikeState = SpikeState::Emerging;
        }
    }

    // 2. Emerging せり上がりアニメーション
    if (mSpikeState == SpikeState::Emerging)
    {
        mSpikeHeight += 18.0f;
        if (mSpikeHeight >= mSpikeMaxHeight)
        {
            mSpikeHeight = mSpikeMaxHeight;
            mSpikeState = SpikeState::Ready;
        }
    }

    // 3. Emerging または Ready のプレイヤー判定
    if ((mSpikeState == SpikeState::Emerging || mSpikeState == SpikeState::Ready) && !player->GetIsDead())
    {
        // ① プレイヤーがくぼみに入った場合：針に刺さって死亡！（針は飛ばない）
        if (pPos.x >= 4270.0f && pPos.x <= 4360.0f && pPos.y >= 1910.0f)
        {
            mSpikeHeight = mSpikeMaxHeight;
            player->TakeDamage(9999);
            player->SetDeathReason(DeathReason::DeathType::NeedleTrapHit);
        }
        // ② プレイヤーがくぼみに入る以外で針が出てきたあと、くぼみの場所を通ろうとすると発射！
        else if (pPos.x >= 4255.0f && pPos.x <= 4375.0f)
        {
            mSpikeState = SpikeState::Launched;
            mSpikeHeight = mSpikeMaxHeight;
            mvPosition.y = mBaseY;
            mSpikeVy = -24.0f; // 上に向かって発射！
        }
    }

    // 4. Launched 発射移動と空中当たり判定
    if (mSpikeState == SpikeState::Launched)
    {
        mvPosition.y += mSpikeVy;

        if (!player->GetIsDead())
        {
            float pW = 44.0f;
            float pH = 100.0f;
            float pLeft = pPos.x - pW * 0.5f;
            float pRight = pPos.x + pW * 0.5f;
            float pTop = pPos.y - pH * 0.5f;
            float pBottom = pPos.y + pH * 0.5f;

            float spLeft = mBaseX - mfEnemyWidth * 0.5f;
            float spRight = mBaseX + mfEnemyWidth * 0.5f;
            float spTop = mvPosition.y - mSpikeHeight;
            float spBottom = mvPosition.y;

            if (pRight >= spLeft && pLeft <= spRight && pBottom >= spTop && pTop <= spBottom)
            {
                player->TakeDamage(9999);
                player->SetDeathReason(DeathReason::DeathType::NeedleTrapHit);
            }
        }

        // 画面外判定：針の底面（mvPosition.y）が画面上端より上に出たとき消滅
        float screenBaseY = ConvertToScreenY(mvPosition.y, mpBlockMap);
        if (screenBaseY < 0.0f || mvPosition.y < -300.0f)
        {
            mSpikeState = SpikeState::Finished;
            SetDeleteFlag(true);
        }
    }
}

void BoulderNeedleTrap::Draw()
{
    if (IsDeleteFlag()) return;
    if (mSpikeState != SpikeState::Hidden && mSpikeState != SpikeState::Finished && mSpikeHeight > 0.0f)
    {
        if (mGraphHandle != -1 && mpBlockMap != nullptr)
        {
            float leftX = mBaseX - mfEnemyWidth * 0.5f;
            float tipY = mvPosition.y - mSpikeHeight;

            int sx1 = static_cast<int>(ConvertToScreenX(leftX, mpBlockMap));
            int stY = static_cast<int>(ConvertToScreenY(tipY, mpBlockMap));
            int drawW = static_cast<int>(mfEnemyWidth);
            int drawH = static_cast<int>(mSpikeHeight);
            if (drawH > static_cast<int>(mSpikeMaxHeight)) drawH = static_cast<int>(mSpikeMaxHeight);

            DrawRectGraph(sx1, stY, 0, 0, drawW, drawH, mGraphHandle, TRUE);
        }
    }
}

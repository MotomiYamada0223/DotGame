#include "NeedleTrap.h"
#include "GameConstants.h"
#include "DxLib.h"
#include "Master.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Player.h"
#include <cmath>

NeedleTrap::NeedleTrap(VECTOR initPos, TrapType type)
    : Enemy(CharacterGraphPath::Needle, initPos),
      UnitStatus()
{
    mType = type;
    mState = TrapState::Waiting;
    mBaseY = initPos.y;
    mTrapSpeed = 30.0f;
    
    mGraphHandle = LoadGraph(CharacterGraphPath::Needle.c_str());

    if (mType == TrapType::PopUp)
    {
        mvPosition.y = mBaseY + 64.0f;
        mTargetY = mBaseY - 32.0f;
    }
    else if (mType == TrapType::AntiJump)
    {
        mvPosition.y = mBaseY + 32.0f;
        mTargetY = mBaseY - 128.0f;
    }

    mfEnemyWidth = 96.0f;
    mfEnemyHeight = 96.0f;

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
    if (mGraphHandle != -1 && mpBlockMap != nullptr)
    {
        int drawX = static_cast<int>(ConvertToScreenX(mvPosition.x, mpBlockMap));
        int drawY = static_cast<int>(mvPosition.y);

        DrawRotaGraph(drawX, drawY, 3.0, 0.0, mGraphHandle, TRUE);
    }
}

void NeedleTrap::EnemyMove(BlockMap& blockMap)
{
    mpBlockMap = &blockMap;
    if (mState == TrapState::Finished) return;

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
            if (distX < 150.0f)
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
                // 感知した瞬間のプレイヤーのY座標を基準とし、さらに100ピクセル上（マイナス方向）を目標にする
                mTargetY = playerY - 50.0f;
            }
        }
    }
    else if (mState == TrapState::Active)
    {
        mvPosition.y -= mTrapSpeed;
        
        if (mvPosition.y <= mTargetY)
        {
            mvPosition.y = mTargetY;
            mState = TrapState::Finished;
        }
    }
}

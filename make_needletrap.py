import sys

h_content = """#pragma once
#include "Enemy.h"
#include "UnitStatus.h"

enum class TrapType {
    PopUp,
    AntiJump
};

enum class TrapState {
    Waiting,
    Active,
    Finished
};

class NeedleTrap : public Enemy, public UnitStatus
{
public:
    NeedleTrap(VECTOR initPos, TrapType type);
    virtual ~NeedleTrap();

    virtual void Update() override;
    virtual void Draw() override;
    virtual void EnemyMove(BlockMap& blockMap) override;
    virtual void UpdateStatusByProgress(GameProgress progress) override;

private:
    TrapType mType;
    TrapState mState;
    float mBaseY;
    float mTargetY;
    float mTrapSpeed;
    int mGraphHandle;
};
"""

with open('Source/NeedleTrap.h', 'w', encoding='utf-8-sig') as f:
    f.write(h_content)


cpp_content = """#include "NeedleTrap.h"
#include "GameConstants.h"
#include "DxLib.h"
#include "Master.h"
#include "SceneManager.h"
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

    mfEnemyWidth = 64.0f;
    mfEnemyHeight = 64.0f;

    mHasInstantKillAttack = true;
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
}

void NeedleTrap::Update()
{
    // EnemyMoveでトラップ専用の動きを行う
    if (mpBlockMap != nullptr)
    {
        EnemyMove(*mpBlockMap);
    }

    // 更新された座標で当たり判定を更新
    UpdateRect();
}

void NeedleTrap::Draw()
{
    if (mGraphHandle != -1 && mpBlockMap != nullptr)
    {
        int drawX = static_cast<int>(mpBlockMap->ConvertToScreenX(mvPosition.x, mpBlockMap));
        int drawY = static_cast<int>(mvPosition.y);

        DrawRotaGraph(drawX, drawY, 1.0, 0.0, mGraphHandle, TRUE);
    }
}

void NeedleTrap::EnemyMove(BlockMap& blockMap)
{
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
            // プレイヤーが針より高く飛んでいる時に反応
            if (distX < 150.0f && playerY < mvPosition.y)
            {
                mState = TrapState::Active;
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
"""

with open('Source/NeedleTrap.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(cpp_content)


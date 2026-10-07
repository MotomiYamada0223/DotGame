#include "HiddenBlock.h"
#include "Master.h"
#include "ObjectManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Player.h"
#include "GameConstants.h"
#include <math.h>
#include "Texture.h"

HiddenBlock::HiddenBlock(VECTOR initPos, BlockMap* blockMap)
    : Object2D(CharacterGraphPath::HiddenBlock, initPos) // 適切なブロック画像
    , mIsRevealed(false)
    , mIsBouncing(false)
    , mBounceFrame(0)
    , mpBlockMap(blockMap)
{
    mWidth = 64.0f;
    mHeight = 64.0f;
}

HiddenBlock::~HiddenBlock()
{
}

void HiddenBlock::Update()
{
    Object2D::Update();

    if (mIsBouncing)
    {
        mBounceFrame++;
        if (mBounceFrame >= mMaxBounceFrame)
        {
            mIsBouncing = false;
        }
    }

    if (mIsRevealed) return; // 出現済みなら当たりの判定はしない

    ObjectManager* objManager = Master::mpGameManager->GetSceneManager()->GetCurrentScene()->GetObjectManager();
    if (objManager == nullptr) return;

    Object2D* playerObj = objManager->GetObject2DByTag(Object2D::Tag::Player2D);
    if (playerObj == nullptr) return;

    Player* player = dynamic_cast<Player*>(playerObj);
    if (player == nullptr) return;

    // プレイヤーの情報を取得
    VECTOR playerPos = player->GetPosition();
    float playerVy = player->GetVelocityY();

    // プレイヤーが上昇中(ジャンプ中)の時のみ判定
    if (playerVy < 0.0f)
    {
        // プレイヤーはmvPositionが中心座標
        float pWidth = PlayerConstants::PlayerCollisionWidth;
        float pHeight = PlayerConstants::PlayerCollisionHeight;
        float pLeft = playerPos.x - pWidth / 2.0f;
        float pRight = playerPos.x + pWidth / 2.0f;
        float pTop = playerPos.y - pHeight / 2.0f;
        float pBottom = playerPos.y + pHeight / 2.0f;

        // ブロックはmvPositionを左上として扱う
        float bLeft = mvPosition.x;
        float bRight = mvPosition.x + mWidth;
        float bTop = mvPosition.y;
        float bBottom = mvPosition.y + mHeight;

        // X方向の重なり
        bool isIntersectX = (pLeft < bRight) && (pRight > bLeft);
        
        // 下から突き上げる判定: プレイヤーの頭(pTop)がブロックの底(bBottom)より上に行き、
        // かつプレイヤーの底(pBottom)がブロックの底(bBottom)より下にある状態（下からめり込んでいる状態）
        bool isIntersectY = (pTop < bBottom) && (pBottom > bBottom);

        if (isIntersectX && isIntersectY)
        {
            mIsRevealed = true;
            mIsBouncing = true;
            mBounceFrame = 0;

            if (mpBlockMap != nullptr)
            {
                int mapX = static_cast<int>(mvPosition.x);
                int mapY = static_cast<int>(mvPosition.y);
                mpBlockMap->SetCollisionBlock(mapX, mapY, static_cast<int>(mWidth), static_cast<int>(mHeight), BlockMap::CollisionType::Block);
            }
            
            // プレイヤーの頭をぶつけさせて落下させる
            player->SetVelocityY(0.0f);
        }
    }
}

void HiddenBlock::Draw()
{
    if (mpBlockMap != nullptr)
    {
        float screenX = ConvertToScreenX(mvPosition.x, mpBlockMap);
        float screenY = ConvertToScreenY(mvPosition.y, mpBlockMap);

        if (!mIsRevealed)
        {
            // 透明時（未出現時）のデバッグ表示：黄色の枠線を描画する
            int debugColor = GetColor(255, 255, 0);
            DrawBox(static_cast<int>(screenX), static_cast<int>(screenY), 
                    static_cast<int>(screenX + mWidth), static_cast<int>(screenY + mHeight), 
                    debugColor, FALSE);
        }
        else if (mpTexture != nullptr)
        {
            float drawY = screenY;
            float drawWidth = mWidth;
            float drawHeight = mHeight;

            if (mIsBouncing)
            {
                // 0.0 ~ 1.0 の進行度
                float progress = static_cast<float>(mBounceFrame) / static_cast<float>(mMaxBounceFrame);
                // サイン波で 0 -> 1 -> 0 (0 ~ PI) になるようにする
                float bounceRatio = sinf(progress * 3.14159265f);

                // 跳ねる高さ (16ピクセル上に移動)
                float bounceHeight = 16.0f;
                drawY -= bounceHeight * bounceRatio;

                // サイズ拡大 (最大1.2倍)
                float scale = 1.0f + (0.2f * bounceRatio);
                drawWidth = mWidth * scale;
                drawHeight = mHeight * scale;
            }

            // 中心座標を維持して拡大させるための補正（サイズが大きくなった分、左上の座標を少し左上にずらす）
            float offsetX = (drawWidth - mWidth) / 2.0f;
            float offsetY = (drawHeight - mHeight) / 2.0f;

            VECTOR drawPos = VGet(screenX - offsetX, drawY - offsetY, 0.0f);
            VECTOR drawSize = VGet(drawWidth, drawHeight, 0.0f);
            mpTexture->DrawSize(drawPos, drawSize);
        }
    }
}

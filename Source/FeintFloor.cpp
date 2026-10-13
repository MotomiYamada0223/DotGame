#include "FeintFloor.h"
#include "GameConstants.h"
#include "Player.h"
#include "BlockMap.h"
#include <cmath>
#include <algorithm>

FeintFloor::FeintFloor(VECTOR startPos, Player* player, BlockMap* blockMap, std::string imagePath)
    : Object2D(startPos)
    , mpPlayer(player)
    , mpBlockMap(blockMap)
    , mStartPosition(startPos)
    , mIsEvading(false)
    , mMoveDirection(0.0f)
    , mCurrentMovedDistance(0.0f)
    , mIsPlayerRiding(false)
{
    // 画像の読み込み
    mImageHandle = LoadGraph(CharacterGraphPath::FeintFloor.c_str());
    int w, h;
    GetGraphSize(mImageHandle, &w, &h);
    mWidth = static_cast<float>(w);
    mHeight = static_cast<float>(h);
}

FeintFloor::~FeintFloor()
{
}

void FeintFloor::Update()
{
    if (IsDeleteFlag()) return;

    if (!mIsEvading) {
        // 待機中：プレイヤーが近づいてきたかチェック
        if (mpPlayer != nullptr) {
            VECTOR pPos = mpPlayer->GetPosition();
            float pVelY = mpPlayer->GetVelocityY();
            
            // プレイヤーが落下中で、かつ自分より上にいる
            if (pVelY > 0.0f && pPos.y < mvPosition.y) {
                float diffX = std::abs(pPos.x - mvPosition.x);
                float diffY = std::abs(pPos.y - mvPosition.y);
                
                if (diffX <= FeintFloorConstants::TriggerDistanceX && diffY <= FeintFloorConstants::TriggerDistanceY) {
                    // 発動！
                    mIsEvading = true;
                    // フェイントで動く方向を右のみにする
                    mMoveDirection = 1.0f;
                }
            }
        }
    } else {
        // 回避移動中
        if (mCurrentMovedDistance < FeintFloorConstants::MaxEvadeDistance) {
            float moveStep = FeintFloorConstants::EvadeSpeed;
            // 行き過ぎないように調整
            if (mCurrentMovedDistance + moveStep > FeintFloorConstants::MaxEvadeDistance) {
                moveStep = FeintFloorConstants::MaxEvadeDistance - mCurrentMovedDistance;
            }
            
            mvPosition.x += moveStep * mMoveDirection;
            mCurrentMovedDistance += moveStep;
        }
    }

    // 乗る判定
    CheckPlayerRiding();
}

void FeintFloor::CheckPlayerRiding()
{
    if (!mpPlayer) return;

    VECTOR pPos = mpPlayer->GetPosition();
    float pVy = mpPlayer->GetVelocityY();

    // 当たり判定の領域（上面は削り値を反映）
    float myTop = mvPosition.y - mHeight / 2.0f + FeintFloorConstants::CollisionTopOffset;
    float myBottom = mvPosition.y + mHeight / 2.0f;
    float myLeft = mvPosition.x - mWidth / 2.0f;
    float myRight = mvPosition.x + mWidth / 2.0f;

    float pWidth = PlayerConstants::PlayerCollisionWidth;
    float pHeight = PlayerConstants::PlayerCollisionHeight;

    float pLeft = pPos.x - pWidth / 2.0f;
    float pRight = pPos.x + pWidth / 2.0f;
    float pTop = pPos.y - pHeight / 2.0f;
    float pBottom = pPos.y + pHeight / 2.0f;

    // AABB による四角形の重なり判定（すり抜け防止）
    if (pRight > myLeft && pLeft < myRight && pBottom > myTop && pTop < myBottom) {
        
        // めり込み量を計算（どれくらいめり込んでいるか）
        float overlapLeft = pRight - myLeft;
        float overlapRight = myRight - pLeft;
        float overlapTop = pBottom - myTop;
        float overlapBottom = myBottom - pTop;

        // 一番めり込みが浅い方向（＝直前までいた方向）を基準に押し出す
        float min1 = min(overlapLeft, overlapRight);
        float min2 = min(overlapTop, overlapBottom);
        float minOverlap = min(min1, min2);

        // ブリンク中の壁激突判定
        bool isBlinking = mpPlayer->IsBlinking();
        bool isImmune = mpPlayer->mIsBlinkWallDeathImmune;

        // 足場の上に立っている（上面に乗っている）かどうかの判定
        // プレイヤーの中心が足場の上面より十分に上にあり、上面との重なりが一番浅い場合
        bool isRidingOnTop = (minOverlap == overlapTop) && (pPos.y < myTop - 20.0f);

        if (isRidingOnTop && pVy >= 0.0f) {
            // 上に乗った
            pPos.y = myTop - pHeight / 2.0f;
            mpPlayer->SetForceGrounded();
            mpPlayer->SetVelocityY(0.0f);
        } else if (minOverlap == overlapBottom) {
            // 下から頭をぶつけた
            pPos.y = myBottom + pHeight / 2.0f;
            if (pVy < 0.0f) {
                mpPlayer->SetVelocityY(0.0f);
            }
            if (isBlinking && !isImmune) {
                mpPlayer->TakeDamage(mpPlayer->GetHp());
            }
        } else if (minOverlap == overlapLeft) {
            // 左から壁としてぶつかった
            pPos.x = myLeft - pWidth / 2.0f;
            if (isBlinking && !isImmune) {
                mpPlayer->TakeDamage(mpPlayer->GetHp());
            }
        } else if (minOverlap == overlapRight) {
            // 右から壁としてぶつかった
            pPos.x = myRight + pWidth / 2.0f;
            if (isBlinking && !isImmune) {
                mpPlayer->TakeDamage(mpPlayer->GetHp());
            }
        } else {
            // minOverlap == overlapTop だが足場の上に乗っていない（角やすれすれで横から侵入した）場合
            if (isBlinking) {
                // ブリンク中なら進行方向に応じて押し戻して激突死
                if (pPos.x < mvPosition.x) {
                    pPos.x = myLeft - pWidth / 2.0f;
                } else {
                    pPos.x = myRight + pWidth / 2.0f;
                }
                if (!isImmune) {
                    mpPlayer->TakeDamage(mpPlayer->GetHp());
                }
            } else {
                // 通常時なら上面に乗せる
                pPos.y = myTop - pHeight / 2.0f;
                mpPlayer->SetForceGrounded();
                mpPlayer->SetVelocityY(0.0f);
            }
        }

        mpPlayer->SetPosition(pPos);
    }
}

void FeintFloor::Draw()
{
    if (IsDeleteFlag()) return;

    if (mpBlockMap != nullptr)
    {
        float screenX = ConvertToScreenX(mvPosition.x, mpBlockMap);
        float screenY = ConvertToScreenY(mvPosition.y, mpBlockMap);

        DrawRotaGraph(static_cast<int>(screenX), static_cast<int>(screenY), 1.0, 0.0, mImageHandle, TRUE);
    }
}

#include "MovingFloor.h"
#include "Player.h"
#include "ResourceManager.h"
#include "GameConstants.h"
#include <cmath>

std::list<MovingFloor*> MovingFloor::s_AllMovingFloors;

// パターンA：往復用（始点・終点を指定）
MovingFloor::MovingFloor(VECTOR startPos, VECTOR endPos, float speed, MovePattern pattern, FloorFeature feature, Player* player, BlockMap* blockMap, std::string imagePath)
    : Object2D(startPos)
    , mStartPos(startPos)
    , mEndPos(endPos)
    , mBaseSpeed(speed)
    , mCurrentSpeed(speed)
    , mPattern(pattern)
    , mFeature(feature)
    , mpPlayer(player)
    , mIsHeadingToEnd(true)
    , mIsPlayerRiding(false)
    , mpBlockMap(blockMap)
    , mIsSpedUp(false)
{
    s_AllMovingFloors.push_back(this);
    // 画像の読み込み
    // 画像パスが指定されていない場合はデフォルトを使用
    std::string path = imagePath.empty() ? CharacterGraphPath::MoveFloor : imagePath;
    mImageHandle = LoadGraph(path.c_str());
    int w, h;
    GetGraphSize(mImageHandle, &w, &h);
    mWidth = static_cast<float>(w);
    mHeight = static_cast<float>(h);
    
    // ベクトルの初期計算
    VECTOR dir = VSub(mEndPos, mStartPos);
    float length = VSize(dir);
    if(length != 0.0f) {
        dir = VScale(dir, 1.0f / length); // 正規化
        mVelocity = VScale(dir, mBaseSpeed);
    } else {
        mVelocity = VGet(0, 0, 0);
    }
    // もし同じ動き（Velocity）を持つ他の床がすでに加速済みなら、自分も最初から加速状態にする
    if (mFeature == FloorFeature::SpeedUpOnRide) {
        for (MovingFloor* floor : s_AllMovingFloors) {
            if (floor != this && floor->mIsSpedUp && 
                floor->mVelocity.x == this->mVelocity.x && floor->mVelocity.y == this->mVelocity.y) {
                mCurrentSpeed = mBaseSpeed * MovingFloorConstants::SpeedUpMultiplier;
                mIsSpedUp = true;
                break;
            }
        }
    }

}

// パターンB：一方向・無限移動用（始点・移動ベクトルを指定）
MovingFloor::MovingFloor(VECTOR startPos, VECTOR velocity, MovePattern pattern, FloorFeature feature, Player* player, BlockMap* blockMap, std::string imagePath)
    : Object2D(startPos)
    , mStartPos(startPos)
    , mEndPos(startPos) // 一方向の場合は使わない
    , mVelocity(velocity)
    , mBaseSpeed(VSize(velocity))
    , mCurrentSpeed(VSize(velocity))
    , mPattern(pattern)
    , mFeature(feature)
    , mpPlayer(player)
    , mIsHeadingToEnd(true)
    , mIsPlayerRiding(false)
    , mpBlockMap(blockMap)
    , mIsSpedUp(false)
{
    s_AllMovingFloors.push_back(this);
    // 画像の読み込み
    // 画像パスが指定されていない場合はデフォルトを使用
    std::string path = imagePath.empty() ? CharacterGraphPath::MoveFloor : imagePath;
    mImageHandle = LoadGraph(path.c_str());
    int w, h;
    GetGraphSize(mImageHandle, &w, &h);
    mWidth = static_cast<float>(w);
    mHeight = static_cast<float>(h);

    // もし同じ動き（Velocity）を持つ他の床がすでに加速済みなら、自分も最初から加速状態にする
    if (mFeature == FloorFeature::SpeedUpOnRide) {
        for (MovingFloor* floor : s_AllMovingFloors) {
            if (floor != this && floor->mIsSpedUp && 
                floor->mVelocity.x == this->mVelocity.x && floor->mVelocity.y == this->mVelocity.y) {
                mCurrentSpeed = mBaseSpeed * MovingFloorConstants::SpeedUpMultiplier;
                mIsSpedUp = true;
                break;
            }
        }
    }
}

MovingFloor::~MovingFloor()
{
    s_AllMovingFloors.remove(this);
    // リソース解放はResourceManagerに任せる
}

void MovingFloor::Update()
{
    if (IsDeleteFlag()) return;

    CheckPlayerRiding();
    Move();
}

void MovingFloor::Move()
{
    VECTOR prevPos = GetPosition();
    VECTOR nextPos = prevPos;

    // 現在の速度ベクトルを計算（罠などで速度が変わっている場合を考慮）
    VECTOR currentVelocity = mVelocity;
    if (mBaseSpeed > 0) {
        currentVelocity = VScale(mVelocity, mCurrentSpeed / mBaseSpeed);
    }

    if (mPattern == MovePattern::Loop) {
        // ループ移動
        nextPos = VAdd(prevPos, currentVelocity);
        
        // ターゲット座標（向かっている先）
        VECTOR targetPos = mIsHeadingToEnd ? mEndPos : mStartPos;
        
        // ターゲットを通り過ぎたかどうかを判定
        VECTOR toTarget = VSub(targetPos, prevPos);
        VECTOR toNext = VSub(targetPos, nextPos);
        
        // 内積が負になったら（＝通り過ぎたら）方向反転
        if (VDot(toTarget, toNext) <= 0.0f) {
            nextPos = targetPos; // はみ出しを補正
            mIsHeadingToEnd = !mIsHeadingToEnd;
            mVelocity = VScale(mVelocity, -1.0f); // ベクトル反転
        }
    } 
    else if (mPattern == MovePattern::OneWayDestroy) {
        // 一方向移動
        nextPos = VAdd(prevPos, currentVelocity);
        
        // X座標による消滅判定は除外し、Y座標が画面の上下200ピクセル外に出たら破棄する
        if (nextPos.y < -MovingFloorConstants::DestroyMarginY || nextPos.y > ScreenSize::ScrrenHeight + MovingFloorConstants::DestroyMarginY) {
            SetDeleteFlag(true);
        }
    }


    // 床自身の座標を更新
    SetPosition(nextPos);

    // 乗っているプレイヤーも一緒に動かす
    if (mIsPlayerRiding && mpPlayer) {
        VECTOR moveDelta = VSub(nextPos, prevPos);
        VECTOR playerPos = mpPlayer->GetPosition();
        
        // X座標は床の移動量分だけ追従させる
        playerPos.x += moveDelta.x;
        
        // Y座標は毎フレームの重力加算による「めり込み」を防止するため、床の上にぴったり強制配置する
        playerPos.y = nextPos.y - mHeight / 2.0f - PlayerConstants::PlayerCollisionHeight / 2.0f;
        
        mpPlayer->SetPosition(playerPos);
    }
}

void MovingFloor::CheckPlayerRiding()
{
    if (!mpPlayer) return;

    VECTOR myPos = GetPosition();
    VECTOR playerPos = mpPlayer->GetPosition();

    // プレイヤーの足元座標を計算
    float pxLeft = playerPos.x - PlayerConstants::PlayerCollisionWidth / 2.0f;
    float pxRight = playerPos.x + PlayerConstants::PlayerCollisionWidth / 2.0f;
    float pyBottom = playerPos.y + PlayerConstants::PlayerCollisionHeight / 2.0f;
    float pyTop = playerPos.y - PlayerConstants::PlayerCollisionHeight / 2.0f;

    // 床の範囲（画像サイズに基づく）
    float myLeft = myPos.x - mWidth / 2.0f;
    float myRight = myPos.x + mWidth / 2.0f;
    float myTop = myPos.y - mHeight / 2.0f;

    // プレイヤーが床の横幅に収まっているか
    // X軸の判定（プレイヤーが床の幅に収まっているか）
    bool inX = (pxRight > myLeft) && (pxLeft < myRight);
    
    // Y軸の判定（プレイヤーの足が床の上面付近にあるか）
    // 落下速度が大きくて突き抜けるのを防ぐため、下方向の許容範囲を広く取る(+30.0f)
    bool inY = (pyBottom >= myTop - MovingFloorConstants::RideMarginTop) && (pyBottom <= myTop + MovingFloorConstants::RideMarginBottom);

    // 乗ったと判定する条件：
    // 1. X軸とY軸の範囲内にいる
    // 2. プレイヤーが「落下中（あるいは静止）」である（GetVelocityY() >= 0.0f）
    // これにより下からジャンプで突き抜ける時に吸い付くのを防ぐ
    if (inX && inY && mpPlayer->GetVelocityY() >= 0.0f) {
        mIsPlayerRiding = true;
        // Player側で処理されるため削除
        mpPlayer->SetForceGrounded(); // 接地フラグを強制的にTrueにする予約を入れる
        mpPlayer->SetVelocityY(0.0f); // 着地時の落下速度の残存による次フレームのすり抜けを防ぐため必須
        // 乗った時に加速する機能（一度加速したら降りても戻らない、かつ同じ動きの他の床も連動する）
        if (mFeature == FloorFeature::SpeedUpOnRide && !mIsSpedUp) {
            mIsSpedUp = true;
            mCurrentSpeed = mBaseSpeed * MovingFloorConstants::SpeedUpMultiplier;
            
            // 同じ方向・同じ速度の他の床も一斉に加速させる
            for (MovingFloor* floor : s_AllMovingFloors) {
                if (floor != this && floor->mFeature == FloorFeature::SpeedUpOnRide && 
                    floor->mVelocity.x == this->mVelocity.x && floor->mVelocity.y == this->mVelocity.y) {
                    floor->mCurrentSpeed = floor->mBaseSpeed * MovingFloorConstants::SpeedUpMultiplier;
                    floor->mIsSpedUp = true;
                }
            }
        }
    } else {
        mIsPlayerRiding = false;
        

    }
}

void MovingFloor::Draw()
{
    if (IsDeleteFlag()) return;

    if ( mpBlockMap != nullptr)
    {
        VECTOR pos = GetPosition();
        float screenX = ConvertToScreenX(pos.x, mpBlockMap);

        int drawX = static_cast<int>(ConvertToScreenX(mvPosition.x, mpBlockMap));
        int drawY = static_cast<int>(ConvertToScreenY(mvPosition.y, mpBlockMap));

        DrawRotaGraph(static_cast<int>(drawX), static_cast<int>(drawY), 1.0, 0.0, mImageHandle, TRUE);
    }
}

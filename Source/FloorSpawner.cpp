#include "FloorSpawner.h"
#include "GameConstants.h"
#include "MovingFloor.h"
#include "Player.h"
#include "BlockMap.h"
#include <algorithm>
using namespace std;

FloorSpawner::FloorSpawner(VECTOR spawnPos, VECTOR velocity, int spawnIntervalFrames, Player* player, BlockMap* blockMap, FloorFeature feature, std::string imagePath)
    : Object2D(spawnPos) // 画像を持たない見えないオブジェクトとして生成
    , mSpawnPos(spawnPos)
    , mVelocity(velocity)
    , mSpawnIntervalFrames(spawnIntervalFrames)
    , mTimer(0)
    , mpPlayer(player)
    , mpBlockMap(blockMap)
    , mImagePath(imagePath)
	, mFeature(feature)
    , mIsReversed(false)
{
}

FloorSpawner::~FloorSpawner()
{
}

void FloorSpawner::Update()
{
    if (IsDeleteFlag()) return;


    int timerAdd = 1;
    
    // スポナー自身が SpeedUpOnRide の場合、同じ系統の床がすでに加速済みかチェックする
    if (mFeature == FloorFeature::SpeedUpOnRide) {
        for (MovingFloor* floor : MovingFloor::s_AllMovingFloors) {
            if (floor->GetFeature() == FloorFeature::SpeedUpOnRide && floor->IsSpedUp() &&
                floor->GetVelocity().x == mVelocity.x && floor->GetVelocity().y == mVelocity.y) {
                // 加速済みなら、床の速度が上がった倍率と同じだけタイマーの進みも速くする
                timerAdd = static_cast<int>(MovingFloorConstants::SpeedUpMultiplier);
                break;
            }
        }
    }
    // ReverseAndDash の場合
    if (mFeature == FloorFeature::ReverseAndDash) {
        // まだ反転していなければ、同系統の床が反転したかをチェック
        if (!mIsReversed) {
            VECTOR reversedVel = VScale(mVelocity, -1.0f);
            for (MovingFloor* floor : MovingFloor::s_AllMovingFloors) {
                if (floor->GetFeature() == FloorFeature::ReverseAndDash && floor->IsSpedUp() &&
                    floor->GetVelocity().x == reversedVel.x && floor->GetVelocity().y == reversedVel.y) {
                    
                    mIsReversed = true;
                    
                    // スポナーの出現Y座標を反転する（TopにいたならBottomへ、BottomにいたならTopへ）
                    if (mSpawnPos.y < 500.0f) {
                        mSpawnPos.y = max(MovingFloorConstants::SpawnYTop, MovingFloorConstants::SpawnYBottom);
                    } else {
                        mSpawnPos.y = min(MovingFloorConstants::SpawnYTop, MovingFloorConstants::SpawnYBottom);
                    }
                    
                    break;
                }
            }
        }
        
        // 反転状態なら、タイマーの進みを速める
        if (mIsReversed) {
            timerAdd = static_cast<int>(MovingFloorConstants::ReverseDashMultiplier);
        }
    }


    mTimer += timerAdd;

    
    // 指定されたフレーム数（間隔）に達したら床を生成する
    if (mTimer >= mSpawnIntervalFrames)
    {
        mTimer = 0; // タイマーリセット
        
        // 新しい床を生成（OneWayDestroyなので画面外に出れば自動で消滅する）
        new MovingFloor(
            mSpawnPos, 
            mVelocity, 
            MovePattern::OneWayDestroy, 
            mFeature, 
            mpPlayer, 
            mpBlockMap,
            mImagePath
        );
    }
}

void FloorSpawner::Draw()
{
    // 見えない装置なので描画処理は行わない
}

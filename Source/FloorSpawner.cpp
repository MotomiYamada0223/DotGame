#include "FloorSpawner.h"
#include "MovingFloor.h"
#include "Player.h"
#include "BlockMap.h"

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
{
}

FloorSpawner::~FloorSpawner()
{
}

void FloorSpawner::Update()
{
    if (IsDeleteFlag()) return;

    mTimer++;
    
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

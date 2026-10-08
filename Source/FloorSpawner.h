#pragma once
#include "Object2D.h"
#include "MovingFloor.h"
#include <DxLib.h>
#include <string>

class Player;
class BlockMap;

class FloorSpawner : public Object2D {
public:
    FloorSpawner(VECTOR spawnPos, VECTOR velocity, int spawnIntervalFrames, Player* player, BlockMap* blockMap, FloorFeature faeture, std::string imagePath = "");
    virtual ~FloorSpawner();

    virtual void Update() override;
    virtual void Draw() override;

private:
    VECTOR mSpawnPos;
    VECTOR mVelocity;
    int mSpawnIntervalFrames;
    int mTimer;
    std::string mImagePath;
	FloorFeature mFeature;
    
    Player* mpPlayer;
    BlockMap* mpBlockMap;
};

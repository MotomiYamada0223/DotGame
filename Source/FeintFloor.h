#pragma once
#include "Object2D.h"
#include <DxLib.h>
#include <string>

class Player;
class BlockMap;

class FeintFloor : public Object2D {
public:
    FeintFloor(VECTOR startPos, Player* player, BlockMap* blockMap, std::string imagePath = "");
    virtual ~FeintFloor();

    virtual void Update() override;
    virtual void Draw() override;

private:
    Player* mpPlayer;
    BlockMap* mpBlockMap;

    VECTOR mStartPosition;
    bool mIsEvading;
    float mMoveDirection;
    float mCurrentMovedDistance;

    int mImageHandle;
    float mWidth;
    float mHeight;

    // 当たり判定用（乗れるが一緒に運ばない）
    bool mIsPlayerRiding;
    void CheckPlayerRiding();
};

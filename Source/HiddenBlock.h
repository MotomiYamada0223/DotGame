#pragma once
#include "Object2D.h"
#include "BlockMap.h"

class HiddenBlock : public Object2D
{
public:
    // 初期座標とBlockMapのポインタを受け取る
    HiddenBlock(VECTOR initPos, BlockMap* blockMap);
    virtual ~HiddenBlock();

    virtual void Update() override;
    virtual void Draw() override;

private:
    bool mIsRevealed;
    bool mIsBouncing;
    int mBounceFrame;
    const int mMaxBounceFrame = 15;
    BlockMap* mpBlockMap;
    float mWidth;
    float mHeight;
};

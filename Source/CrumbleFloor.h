#pragma once
#include "Object2D.h"
#include <DxLib.h>
#include <string>
#include <vector>

class Player;
class BlockMap;

class CrumbleFloor : public Object2D {
public:
    enum class State {
        Waiting,    // 通常待機（乗れる状態）
        Falling,    // 崩壊・破片飛散中（当たり判定なし）
        Destroyed   // 落下完了・非表示
    };

    struct Fragment {
        int srcX;
        int srcY;
        int width;
        int height;
        float offsetX;
        float offsetY;
        float posX;
        float posY;
        float vx;
        float vy;
        float gravity;
        float angle;
        float rotSpeed;
        float alpha;
    };

    CrumbleFloor(VECTOR startPos, Player* player, BlockMap* blockMap, std::string imagePath = "");
    virtual ~CrumbleFloor();

    virtual void Update() override;
    virtual void Draw() override;

    // リセット（初期位置・待機状態へ復帰）
    void Reset();

    // 全ての崩落床を一括リセット（プレイヤーリスポーン時用）
    static void ResetAll();

    State GetState() const { return mState; }

private:
    static std::vector<CrumbleFloor*> s_AllCrumbleFloors;

    Player* mpPlayer;
    BlockMap* mpBlockMap;

    VECTOR mStartPosition;
    State mState;
    float mVelocityY;

    int mImageHandle;
    float mWidth;
    float mHeight;

    bool mIsPlayerRiding;
    std::vector<Fragment> mFragments;

    void InitFragments();
    void StartCrumbling();
    void CheckPlayerRiding();
};

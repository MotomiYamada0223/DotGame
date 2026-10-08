#pragma once
#include "Object2D.h"
#include <DxLib.h>
#include <list>
#include <string>

class Player;
class BlockMap; // 前方宣言

// 移動の挙動パターン
enum class MovePattern {
    Loop,           // 始点と終点を永遠に往復する
    OneWayDestroy   // 一方向に進み続け、一定距離や画面外で自身を消滅させる
};

// 床の特殊効果・罠
enum class FloorFeature {
    Normal,         // 通常（常に同じ速度）
    SpeedUpOnRide   // プレイヤーが乗っている間だけ速度が上昇する罠
};

class MovingFloor : public Object2D {
public:
    // パターンA：往復用（始点・終点を指定）
    MovingFloor(VECTOR startPos, VECTOR endPos, float speed, MovePattern pattern, FloorFeature feature, Player* player, BlockMap* blockMap, std::string imagePath = "");

    // パターンB：一方向・無限移動用（始点・移動ベクトルを指定）
    MovingFloor(VECTOR startPos, VECTOR velocity, MovePattern pattern, FloorFeature feature, Player* player, BlockMap* blockMap, std::string imagePath = "");

    virtual ~MovingFloor();
    virtual void Update() override;
    virtual void Draw() override;

public:
    static std::list<MovingFloor*> s_AllMovingFloors;

private:
    bool mIsSpedUp;
    // 座標・移動関連
    VECTOR mStartPos;
    VECTOR mEndPos;
    VECTOR mVelocity;       // 現在の移動ベクトル
    float mBaseSpeed;       // 基本速度
    float mCurrentSpeed;    // 現在の速度
    bool mIsHeadingToEnd;   // ループ用：終点へ向かっているか

    // 属性
    MovePattern mPattern;
    FloorFeature mFeature;
    
    // プレイヤー連動
    Player* mpPlayer;
    BlockMap* mpBlockMap;
    bool mIsPlayerRiding;

    float mWidth;
    float mHeight;
    int mImageHandle;

    // 内部処理関数
    void Move();                // 実際の移動計算
    void CheckPlayerRiding();   // プレイヤーが乗っているかどうかの判定と連動
};

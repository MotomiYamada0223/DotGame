#pragma once
#include "DxLib.h"
#include "BlockMap.h"

class CharacterPhysics
{
private:
    static  constexpr float HalfMultiply = 0.5f; // 半分にする時にかける数値

public:
    CharacterPhysics();
    ~CharacterPhysics();


    // 1軸ずつ移動と判定を処理して接触した特殊タイルの種類を返す
    BlockMap::CollisionType UpdateMoveAndCollision(
        VECTOR& position,
        float& velocityY,
        bool& isGrounded,
        bool& isJumping,
        const BlockMap& blockMap,
        float width,
        float height,
        float gravity,
        float moveSpeed,
        float moveDirection
    );

    // 指定した矩形が何かに触れているか調べる
    bool IsBlockCollision(
        const BlockMap& blockMap,
        float x,
        float y,
        float width,
        float height
    ) const;


    // 敵からも共通で呼び出せるピクセル単位の当たり判定
    BlockMap::CollisionType CheckCollision(
        const BlockMap& blockMap,
        float x,
        float y,
        float width,
        float height,
        int* blockX = nullptr,
        int* blockY = nullptr
    ) const;

    // 階段状スロープ上の段差ガタつきを抑えるための描画・カメラ用平滑化Yオフセットを算出する
    float CalcSmoothedSlopeOffsetY(
        const BlockMap& blockMap,
        const VECTOR& position,
        float width,
        float height
    ) const;

};
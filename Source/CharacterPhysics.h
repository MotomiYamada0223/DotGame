#pragma once
#include "DxLib.h"
#include "BlockMap.h"


// キャラクターの重力とブロックとの当たり判定と位置補正を行うため
// 落下やジャンプ時の床と天井へのめり込みを防ぐ処理
// Enemyにも使えるはず
class CharacterPhysics
{
public:
	CharacterPhysics();
	~CharacterPhysics();

	// プレイヤーや敵の位置、速度、サイズを受け取って、移動とブロックの当たり判定をまとめて処理する関数
	void UpdateMoveAndCollision(
		VECTOR& position,   // 現在の位置
		float& velocityY,   // 縦の速度
		bool& isGrounded,   // 接地フラグ
		bool& isJumping,    // ジャンプ中フラグ
		BlockMap& blockMap, // マップ情報
		float width,        // 幅
		float height,       // 高さ
		float gravity,      // 重力
		float moveSpeed     // 移動スピード
	);
};
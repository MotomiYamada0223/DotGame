#pragma once

#include "Object2D.h"
#include "BlockMap.h"
#include "CharacterPhysics.h"

class Enemy : public Object2D
{
public:
	Enemy(VECTOR initPos);
	virtual ~Enemy();

	virtual void Update() override;
	virtual void Draw() override;

	// 敵の移動処理
	void EnemyMove(BlockMap& blockMap);

	// 攻撃を受けた際に呼ばれる処理
	void OnDamaged();

private:
	// 移動・物理関連
	float moveSpeed;

	// CharacterPhysics用
	bool mbIsJumping;
	bool isGrounded;
	float velocityY;

	// 重力
	float gravity;

	// 敵の当たり判定サイズ
	float mfEnemyWidth;
	float mfEnemyHeight;


	// ダメージ関連
	bool isDamaged;
	int damageTimer;


	// アニメーション関連
	int mnCurrentFrame;
	int mnFrameTimer;

	static const int FRAME_WIDTH = 256;
	static const int FRAME_HEIGHT = 256;
	static const int TOTAL_FRAMES = 4;
	static const int FRAME_INTERVAL = 13;


	// マップ
	BlockMap* mpBlockMap;


	// 物理計算
	CharacterPhysics mCharacterPhysics;
};
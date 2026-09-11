#pragma once
#include "Object2D.h"
#include "BlockMap.h"
#include "CharacterPhysics.h" // ジャンプとかの当たり判定をしてくれる処理

class Player : public Object2D
{
public:
	Player(VECTOR initPos);
	virtual ~Player();

	virtual void Update() override;
	virtual void Draw() override;

	void DebugDraw(); // デバッグ用の描画関数

	// Mapを受け取って位置を更新する処理
	void PlayerMove(BlockMap& blockMap);

private:
	CharacterPhysics mCharacterPhysics; // 物理計算用のインスタンス

private:
	// プレイヤーの当たり判定サイズ
	float playerWidth;
	float playerHeight;

	// ジャンプ関連
	bool isJumping;
	float velocityY;
	const float gravity = 0.5f; // 元0.5
	const float jumpPower = -20.0f; // 元12
	bool isGrounded; // 地面に接地しているかどうか

	// 移動関連
	const float moveSpeed = 5.0f;
	bool isFacingRight; 

	// 攻撃関連
	bool isAttacking;
	int attackTimer;
	const int attackDuration = 15; 

	// 被ダメージ（衝突）フラグ
	bool isHitDamage;

	// アニメーション関連
	int mCurrentFrame;
	int mFrameTimer;
	static const int FRAME_WIDTH = 256; 
	static const int FRAME_HEIGHT = 256; 
	static const int TOTAL_FRAMES = 4; 
	static const int FRAME_INTERVAL = 8; 
};

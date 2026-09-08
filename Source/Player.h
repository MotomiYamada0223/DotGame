#pragma once
#include "Object2D.h"

class Player : public Object2D
{
public:
	Player(VECTOR initPos);
	virtual ~Player();

	virtual void Update() override;
	virtual void Draw() override;

private:
	// ジャンプ関連
	
	bool isJumping;
	float velocityY;
	const float gravity = 0.5f;
	const float jumpPower = -12.0f;
	float groundY; 

	// 移動関連
	
	const float moveSpeed = 5.0f;
	bool isFacingRight; 

	// 攻撃関連
	
	bool isAttacking;
	int attackTimer;
	const int attackDuration = 15; 

	// アニメーション関連
	
	int mCurrentFrame;
	int mFrameTimer;
	static const int FRAME_WIDTH = 256; 
	static const int FRAME_HEIGHT = 256; 
	static const int TOTAL_FRAMES = 4; 
	static const int FRAME_INTERVAL = 8; 
};

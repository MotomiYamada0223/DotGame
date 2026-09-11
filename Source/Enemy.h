#pragma once

#include "DxLib.h"
#include "Object2D.h"

class Enemy : public Object2D
{
public:
	Enemy(VECTOR initPos);
	~Enemy();

	void Update() override;
	void Draw() override;

private:
	const float MoveSpeed = 2.0f;
	static constexpr float SearchRange = 500.0f;

	VECTOR mvPlayerDirection;
	int mnCurrentFrame;
	int mnFrameTimer;
	static const int FRAME_WIDTH = 256;
	static const int FRAME_HEIGHT = 256;
	static const int TOTAL_FRAMES = 4;
	static const int FRAME_INTERVAL = 10;
};
#pragma once
#include "Object2D.h"

class Enemy : public Object2D
{
public:
	Enemy(VECTOR initPos);
	virtual ~Enemy();

	virtual void Update() override;
	virtual void Draw() override;

	// 攻撃を受けた際に呼ばれる処理
	
	void OnDamaged();

private:
	float moveSpeed;
	bool isDamaged; // ダメージを受けて赤くなっているか
	
	int damageTimer; // 赤くするフレーム時間
	
	VECTOR mvPlayerDirection;
	int mnCurrentFrame;
	int mnFrameTimer;
	static const int FRAME_WIDTH = 256;
	static const int FRAME_HEIGHT = 256;
	static const int TOTAL_FRAMES = 4;
	static const int FRAME_INTERVAL = 10;
};

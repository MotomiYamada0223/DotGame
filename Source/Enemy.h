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
	
};

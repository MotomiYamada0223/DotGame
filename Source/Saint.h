#pragma once
#include "Object2D.h"

class Saint
{
private:
	const int MAX_STATE = 3; // 目閉・口閉(0), 目開・口閉(1), 目開・口開(2)

public:
	Saint(VECTOR initPos);
	virtual ~Saint();

	 void Update();
	 void Draw();

private:
	int graphHandles[3]; 
	int currentState;    
	
	// アニメーション用タイマー
	int talkTimer;
	int blinkTimer;
	int blinkInterval;
	bool isBlinking;

	float mfScale;
};

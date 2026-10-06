#pragma once
#include "Object2D.h"

class Saint
{
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
};

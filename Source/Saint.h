#pragma once
#include "Object2D.h"

class Saint : public Object2D
{
public:
	Saint(VECTOR initPos);
	virtual ~Saint();

	virtual void Update() override;
	virtual void Draw() override;

private:
	int graphHandles[3]; 
	int currentState;    
	
	// アニメーション用タイマー
	
	int talkTimer;
	int blinkTimer;
	int blinkInterval;
	bool isBlinking;
};

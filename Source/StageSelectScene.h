#pragma once
#include "Scene.h"
#include <string>

class StageSelectScene : public Scene
{
public:
	StageSelectScene();
	virtual ~StageSelectScene();

	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw() override;
	virtual void Finalize() override;

private:
	int mapGraphHandle;

	// プレイヤーの画像と座標
	
	int playerGraphHandle;
	float playerX;
	float playerY;
	float playerSpeed;

	// 目標座標と移動フラグ
	
	float targetX;
	float targetY;
	bool isMoving;

	// デバッグ用：受け付けた入力信号
	
	std::string lastSignal;
	
};

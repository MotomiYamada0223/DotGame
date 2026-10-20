#pragma once
#include "Scene.h"
#include <string>
#include "BackgroundBox.h"

class StageSelectScene : public Scene
{

private:
	const int TitleStringX = 40;
	const int TitleStringY = 40;
	const int TitleStringSize = 40;
	const int TitleBoxPos = 20;
	const int TitleBoxWidth = 380;
	const int TitleBoxHeight = 100;


public:
	StageSelectScene();
	virtual ~StageSelectScene();

	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw() override;
	virtual void Finalize() override;

	void DrawStageString();
	void Move();
	void TitleDrawString();


private:
	// 背景のBox描画のクラス
	BackgroundBox mBackgroundBox;


	int mapGraphHandle;

	// プレイヤーの画像と座標
	int playerGraphHandles[16];
	int animCounter;
	bool facingRight;
	int caveTimer;
	bool isHidden;
	float playerX;
	float playerY;
	float playerSpeed;

	// 目標座標と移動フラグ
	float targetX;
	float targetY;
	bool isMoving;

	// デバッグ用：受け付けた入力信号
	std::string lastSignal;
	bool mIsEnteringStage;
	int mEnterTimer;
	bool mIsReturningTitle;
	int mReturnTimer;
	
};

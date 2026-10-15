#include <DxLib.h>
#include <iostream>
#include "Background.h"
#include "Master.h"
#include "GameConstants.h"

Background::Background()
	: mnBackgroundGraph(-1)
	, mnScrollX(0)
	, mbIsLoaded(false)
	, mfScrollSpeed(0.0f)
{

}

Background::~Background()
{

}

bool Background::Load(const std::string& image_path)
{
	if (mbIsLoaded) { return true; }
	mnBackgroundGraph = Master::mpGameManager->GetResourceManager()->LoadGraphics(image_path);

	if (mnBackgroundGraph == -1)
	{
		printfDx("背景画像の読み込み失敗");
		return false;
	}

	mbIsLoaded = true;
	return true;
}

// 背景画像の移動処理
void Background::Move(int speed, bool scrolling, float blockMap_moveDirection)
{
	if (!mbIsLoaded) { return; }
	if (!scrolling) { return; }

	int scrollSpeed = static_cast<int>(speed * MapScrollConstants::BackgroundScrollSpeedScale);

	// BlockMapの方向で判断する
	if (blockMap_moveDirection > 0.0f) { mnScrollX += scrollSpeed; }
	else if (blockMap_moveDirection < 0.0f) { mnScrollX -= scrollSpeed; }
	

	if (mnScrollX >= ScreenSize::ScrrenWidth)
	{
		mnScrollX -= ScreenSize::ScrrenWidth;
	}
	if (mnScrollX < 0)
	{
		mnScrollX += ScreenSize::ScrrenWidth;
	}
}

void Background::Draw(int shakeX, int shakeY)
{
	if (!mbIsLoaded) { return; }
	int scrollx = static_cast<int>(mnScrollX) - shakeX;

	// 1枚目のスクロール
	DrawGraph(
		-scrollx,
		shakeY,
		mnBackgroundGraph,
		TRUE
	);

	// 2枚目のスクロール
	DrawGraph(
		ScreenSize::ScrrenWidth - scrollx,
		shakeY,
		mnBackgroundGraph,
		TRUE
	);
}

int Background::GetScrollX() const
{
	return static_cast<int>(mnScrollX);
}
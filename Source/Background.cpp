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
		printfDx("”wŒi‰æ‘œ‚Ì“Ç‚İ‚İ¸”s");
		return false;
	}

	mbIsLoaded = true;
	return true;
}

// ”wŒi‰æ‘œ‚ÌˆÚ“®ˆ—
void Background::Move(int speed, bool scrolling, float blockMap_moveDirection)
{
	if (!mbIsLoaded) { return; }
	if (!scrolling) { return; }

	int scrollSpeed = static_cast<int>(speed * MapScrollConstants::BackgroundScrollSpeedScale);

	// BlockMap‚Ì•ûŒü‚Å”»’f‚·‚é
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

void Background::Draw()
{
	if (!mbIsLoaded) { return; }
	int scrollx = static_cast<int>(mnScrollX);

	// 1–‡–Ú‚ÌƒXƒNƒ[ƒ‹
	DrawGraph(
		-scrollx,
		0,
		mnBackgroundGraph,
		TRUE
	);

	// 2–‡–Ú‚ÌƒXƒNƒ[ƒ‹
	DrawGraph(
		ScreenSize::ScrrenWidth - scrollx,
		0,
		mnBackgroundGraph,
		TRUE
	);
}

int Background::GetScrollX() const
{
	return static_cast<int>(mnScrollX);
}
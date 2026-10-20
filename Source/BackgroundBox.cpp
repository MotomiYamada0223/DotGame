#include "BackgroundBox.h"

BackgroundBox::BackgroundBox()
{

}

BackgroundBox::~BackgroundBox()
{

}

void BackgroundBox::DrawBackgroundBox(
	int x1,
	int y1,
	int x2,
	int y2,
	unsigned int color,
	unsigned int frameColor,
	int alpha,
	int dist)
{
	// ”¼“§–¾‚Ì”wŒi•`‰æ
	if (alpha != 255)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	}

	// ”¼“§–¾‚ÌcolorF‚Å•`‰æ
	DrawBox(
		x1,
		y1,
		x2,
		y2,
		color,
		true
	);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);



	// ˜g‚Ì•`‰æ
	// “ñd‚É•`‰æ‚µ‚Ä‚¢‚é‚Ì‚Å­‚µ‘¾‚ß
	DrawBox(
		x1,
		y1,
		x2,
		y2,
		frameColor,
		false
	);

	DrawBox(
		x1 + dist,
		y1 + dist,
		x2 - dist,
		y2 - dist,
		frameColor,
		false
	);
}

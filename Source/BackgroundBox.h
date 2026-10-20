#pragma once
#include <DxLib.h>
// ”wŒiBOX‚Ì•`‰æ‚ğ‚·‚éŠÖ”

class BackgroundBox
{
public:

	BackgroundBox();
	~BackgroundBox();

	// O‚Â‚ÌBox‚ğ•`‰æ‚·‚éŠÖ”
	void DrawBackgroundBox(
		int x1,
		int y1,
		int x2,
		int y2,
		unsigned int color,
		unsigned int frameColor,
		int alpha,
		int dist = 0 // Šg‘å‚·‚éƒsƒNƒZƒ‹
	);
};
#include "CircleFade.h"
#include "GameConstants.h"

CircleFade::CircleFade()
	: mnState(1)
	, mnTimer(0)
{

}

CircleFade::~CircleFade()
{

}

bool CircleFade::GetStateFlag() const
{
	if (mnState == 2)
	{
		return true;
	}
	return false;
}


void CircleFade::DrawCircleFade(Player* player, BlockMap block)
{
	if (mnState == 1)
	{
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(0, 0, 0), TRUE);
	}
	else if (mnState == 2)
	{
		int cx = ScreenSize::CenterX;
		int cy = ScreenSize::CenterY;
		if (player) {
			cx = (int)(player->GetPosition().x - block.GetScrollX());
			cy = (int)(player->GetPosition().y - block.GetScrollY());
		}

		float t = (float)mnTimer / 60.0f;
		float radius = t * t * 1500.0f;
		float time = 3000.0f;

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		for (int i = 0; i < 60; i++) {
			float a1 = i * (DX_PI_F * 2.0f / 60.0f);
			float a2 = (i + 1) * (DX_PI_F * 2.0f / 60.0f);
			int x1 = cx + (int)(cosf(a1) * radius);
			int y1 = cy + (int)(sinf(a1) * radius);
			int x2 = cx + (int)(cosf(a2) * radius);
			int y2 = cy + (int)(sinf(a2) * radius);
			int ox1 = cx + (int)(cosf(a1) * time);
			int oy1 = cy + (int)(sinf(a1) * time);
			int ox2 = cx + (int)(cosf(a2) * time);
			int oy2 = cy + (int)(sinf(a2) * time);
			DrawQuadrangle(x1, y1, x2, y2, ox2, oy2, ox1, oy1, GetColor(0, 0, 0), TRUE);
		}
	}
}



void CircleFade::UpdateCircleFade()
{

	if (mnState == 1)
	{
		mnTimer++;
		if (mnTimer > 100)
		{
			mnState = 2;
			mnTimer = 0;
		}
		return;
	}
	else if (mnState == 2)
	{
		mnTimer++;
		if (mnTimer > 60)
		{
			mnState = 0;
		}
	}
}
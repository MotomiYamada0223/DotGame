#include "DeathCountEffect.h"
#include <cmath>
#include "GameConstants.h"

namespace
{
	// ”j•Ğ‚Ì•¨—ƒpƒ‰ƒ[ƒ^
	const float kGravity = 0.50f;        // d—Í
	const float kBounceX = -0.4f;        // ‰¡•ûŒü‚Ì”½”­ŒW”
	const float kBounceY = -0.4f;        // c•ûŒü‚Ì”½”­ŒW”
	const float kFriction = 0.9f;        // ÚG‚Ì‰¡•ûŒü–€C
	const int   kFragmentLife = 150;     // ”j•Ğ‚Ìõ–½
}

DeathCountEffect::DeathCountEffect()
{
}

DeathCountEffect::~DeathCountEffect()
{
}

// ‰æ‘œ‚ğ”j•Ğ‚É•ªŠ„‚µ‚ÄA’†SˆÊ’u‚©‚ç”ò‚ÑU‚ç‚¹‚é
void DeathCountEffect::StartShatter(const PlayerSpriteSource& src, float centerX, float centerY, int fragSize)
{
	mFragments.clear();

	// ‰æ‘œ‚Ì¶ãÀ•W
	const float topLeftX = centerX - src.width / 2.0f;
	const float topLeftY = centerY - src.height / 2.0f;

	for (int y = 0; y < src.height; y += fragSize)
	{
		for (int x = 0; x < src.width; x += fragSize)
		{
			Fragment frag;
			frag.x = topLeftX + x;
			frag.y = topLeftY + y;
			frag.vx = (static_cast<float>(GetRand(100)) / 100.0f * 14.0f) - 7.0f;
			frag.vy = (static_cast<float>(GetRand(100)) / 100.0f * -14.0f) - 4.0f;
			frag.srcX = src.srcX + x;
			frag.srcY = src.srcY + y;
			frag.width = fragSize;
			frag.height = fragSize;
			frag.graphHandle = src.graphHandle;
			frag.isActive = true;
			frag.timer = 0;

			mFragments.push_back(frag);
		}
	}
}

void DeathCountEffect::Update(const BlockHitFunc& isBlocked)
{
	for (auto& frag : mFragments)
	{
		if (!frag.isActive) continue;

		frag.timer++;
		if (frag.timer >= kFragmentLife)
		{
			frag.isActive = false;
			continue;
		}

		frag.vy += kGravity;

		// X•ûŒü‚ÌˆÚ“®‚ÆÕ“Ë
		frag.x += frag.vx;
		if (isBlocked && isBlocked(frag.x, frag.y, (float)frag.width, (float)frag.height))
		{
			frag.x -= frag.vx;     // Œ³‚É–ß‚·
			frag.vx *= kBounceX;   // ”½”­
		}

		// Y•ûŒü‚ÌˆÚ“®‚ÆÕ“Ë
		frag.y += frag.vy;
		if (isBlocked && isBlocked(frag.x, frag.y, (float)frag.width, (float)frag.height))
		{
			frag.y -= frag.vy;
			frag.vy *= kBounceY;
			frag.vx *= kFriction;
		}
	}
}
// —LŒø‚È”j•Ğ‚ğ•`‰æ‚·‚é
void DeathCountEffect::Draw()
{
	for (const auto& frag : mFragments)
	{
		if (!frag.isActive) continue;
		if (frag.graphHandle == -1) continue;

		DrawRectGraph(
			static_cast<int>(frag.x),
			static_cast<int>(frag.y),
			frag.srcX,
			frag.srcY,
			frag.width,
			frag.height,
			frag.graphHandle,
			TRUE
		);
	}
}

// ‘S‚Ä‚Ì”j•Ğ‚ªÁ‚¦‚½‚©
bool DeathCountEffect::IsFinished() const
{
	for (const auto& frag : mFragments)
	{
		if (frag.isActive) return false;
	}
	return true;
}
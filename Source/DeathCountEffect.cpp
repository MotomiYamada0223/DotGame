#include "DeathCountEffect.h"
#include <cmath>
#include "GameConstants.h"

namespace
{
	// 破片の物理パラメータ
	const float kGravity = 0.50f;        // 重力
	const float kBounceX = -0.4f;        // 横方向の反発係数
	const float kBounceY = -0.4f;        // 縦方向の反発係数
	const float kFriction = 0.9f;        // 接触時の横方向摩擦
	const int   kFragmentLife = 150;     // 破片の寿命
}

DeathCountEffect::DeathCountEffect()
{
}

DeathCountEffect::~DeathCountEffect()
{
}

// 破片を作り直す（既存の破片は消える）
void DeathCountEffect::StartShatter(const PlayerSpriteSource& src, float centerX, float centerY, int fragSize, float scale)
{
	mFragments.clear();
	AppendShatter(src, centerX, centerY, fragSize, scale);
}

// 破片を追加する（既存の破片は残す）
void DeathCountEffect::AppendShatter(const PlayerSpriteSource& src, float centerX, float centerY, int fragSize, float scale)
{
	// 表示上の全体サイズから左上を求める
	const float dispW = src.width * scale;
	const float dispH = src.height * scale;
	const float topLeftX = centerX - dispW / 2.0f;
	const float topLeftY = centerY - dispH / 2.0f;

	for (int y = 0; y < src.height; y += fragSize)
	{
		for (int x = 0; x < src.width; x += fragSize)
		{
			Fragment frag;
			frag.x = topLeftX + x * scale;
			frag.y = topLeftY + y * scale;
			frag.vx = (static_cast<float>(GetRand(100)) / 100.0f * 14.0f) - 7.0f;
			frag.vy = (static_cast<float>(GetRand(100)) / 100.0f * -14.0f) - 4.0f;
			frag.srcX = src.srcX + x;
			frag.srcY = src.srcY + y;
			frag.width = fragSize;
			frag.height = fragSize;
			frag.scale = scale;
			frag.graphHandle = src.graphHandle;
			frag.isActive = true;
			frag.timer = 0;

			mFragments.push_back(frag);
		}
	}
}

// 全ての破片を消す
void DeathCountEffect::Clear()
{
	mFragments.clear();
}

// 破片の移動・衝突・寿命を更新する
void DeathCountEffect::Update(const BlockHitFunc& isBlocked)
{
	for (auto& frag : mFragments)
	{
		if (!frag.isActive) continue;

	frag.timer++;
if (mLifeEnabled && frag.timer >= kFragmentLife)
{
	frag.isActive = false;
	continue;
}

		// 表示上のサイズで衝突を判定する
		const float w = frag.width * frag.scale;
		const float h = frag.height * frag.scale;

		frag.vy += kGravity;

		// X方向の移動と衝突
		frag.x += frag.vx;
		if (isBlocked && isBlocked(frag.x, frag.y, w, h))
		{
			frag.x -= frag.vx;
			frag.vx *= kBounceX;
		}

		// Y方向の移動と衝突
		frag.y += frag.vy;
		if (isBlocked && isBlocked(frag.x, frag.y, w, h))
		{
			frag.y -= frag.vy;
			frag.vy *= kBounceY;
			frag.vx *= kFriction;
		}
	}
}

// 有効な破片を描画する（scale に応じて縮小して描画）
void DeathCountEffect::Draw()
{
	for (const auto& frag : mFragments)
	{
		if (!frag.isActive) continue;
		if (frag.graphHandle == -1) continue;

		DrawRectExtendGraph(
			static_cast<int>(frag.x),
			static_cast<int>(frag.y),
			static_cast<int>(frag.x + frag.width * frag.scale),
			static_cast<int>(frag.y + frag.height * frag.scale),
			frag.srcX,
			frag.srcY,
			frag.width,
			frag.height,
			frag.graphHandle,
			TRUE
		);
	}
}

// 全ての破片が消えたか
bool DeathCountEffect::IsFinished() const
{
	for (const auto& frag : mFragments)
	{
		if (frag.isActive) return false;
	}
	return true;
}
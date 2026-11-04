#include "HotSpring.h"
#include "Player.h"
#include "BlockMap.h"
#include "GameConstants.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include "Master.h"

HotSpring::HotSpring()
{
}

HotSpring::~HotSpring()
{
}

void HotSpring::Initialize()
{
	Reset();
}

void HotSpring::Reset()
{
	mWaveTimer = 0;
	mHeatTimer = 0;
	mHealPulseTimer = 0;
	mbPlayerInside = false;
	mPlayerX = 0.0f;
	mPlayerY = 0.0f;
	mParticles.clear();
	mFloatingTexts.clear();
}

void HotSpring::SpawnSteam(float x, float y, bool boiling)
{
	Particle p;
	p.x = x;
	p.y = y;
	p.vx = ((float)GetRand(10) - 5.0f) * 0.06f;
	p.vy = -((float)GetRand(15) / 20.0f + (boiling ? 0.95f : 0.45f));
	p.size = (float)GetRand(4) + (boiling ? 6.0f : 4.0f);
	p.life = GetRand(20) + (boiling ? 25 : 35);
	p.maxLife = p.life;
	if (boiling)
	{
		int r = 255;
		int g = GetRand(80) + 50;
		int b = GetRand(30);
		p.color = GetColor(r, g, b);
		p.type = 5; // 沸騰・熱湯蒸気
	}
	else
	{
		int brightness = GetRand(20) + 235;
		p.color = GetColor(brightness, brightness + 10, brightness + 20);
		p.type = 0; // 通常湯気
	}
	mParticles.push_back(p);
}

void HotSpring::SpawnBubble(float x, float y)
{
	Particle p;
	p.x = x;
	p.y = y;
	p.vx = 0.0f;
	p.vy = -((float)GetRand(12) / 10.0f + 0.8f);
	p.size = (float)GetRand(2) + 2.0f;
	p.life = GetRand(30) + 40;
	p.maxLife = p.life;
	p.color = GetColor(200, 240, 255);
	p.type = 1; // 気泡
	mParticles.push_back(p);
}

void HotSpring::SpawnSparkle(float x, float y, unsigned int color)
{
	Particle p;
	p.x = x;
	p.y = y;
	p.vx = ((float)GetRand(20) - 10.0f) * 0.08f;
	p.vy = -((float)GetRand(20) / 10.0f + 0.6f);
	p.size = (float)GetRand(3) + 2.0f;
	p.life = GetRand(20) + 25;
	p.maxLife = p.life;
	p.color = color;
	p.type = 4; // キラキラ
	mParticles.push_back(p);
}

void HotSpring::SpawnSplash(float x, float y)
{
	// 掻き分け時の水しぶき
	for (int i = 0; i < 5; i++)
	{
		Particle p;
		p.x = x + (GetRand(24) - 12.0f);
		p.y = y + (GetRand(8) - 4.0f);
		p.vx = ((float)GetRand(30) - 15.0f) * 0.12f;
		p.vy = -((float)GetRand(25) / 10.0f + 1.2f);
		p.size = (float)GetRand(3) + 2.0f;
		p.life = GetRand(14) + 12;
		p.maxLife = p.life;
		p.color = GetColor(180, 240, 255);
		p.type = 6; // 水しぶき
		mParticles.push_back(p);
	}
}

void HotSpring::SpawnText(float x, float y, const char* str, unsigned int color, unsigned int outlineColor)
{
	FloatingText ft;
	ft.x = x;
	ft.y = y;
	ft.vy = -0.7f;
	ft.life = 60;
	ft.maxLife = 60;
	strncpy_s(ft.text, str, sizeof(ft.text) - 1);
	ft.color = color;
	ft.outlineColor = outlineColor;
	mFloatingTexts.push_back(ft);
}

void HotSpring::TriggerHealBurst(float px, float py)
{
	mHealPulseTimer = 28;

	// 緑のクロス(+)パーティクル
	for (int i = 0; i < 8; i++)
	{
		Particle p;
		p.x = px + (GetRand(36) - 18.0f);
		p.y = py + (GetRand(40) - 20.0f);
		p.vx = ((float)GetRand(30) - 15.0f) * 0.1f;
		p.vy = -((float)GetRand(25) / 10.0f + 1.2f);
		p.size = 5.0f;
		p.life = GetRand(20) + 35;
		p.maxLife = p.life;
		p.color = GetColor(70, 255, 130);
		p.type = 2; // Cross (+)
		mParticles.push_back(p);
	}

	// ピンクのハートパーティクル
	for (int i = 0; i < 4; i++)
	{
		Particle p;
		p.x = px + (GetRand(30) - 15.0f);
		p.y = py + (GetRand(30) - 15.0f);
		p.vx = ((float)GetRand(20) - 10.0f) * 0.08f;
		p.vy = -((float)GetRand(20) / 10.0f + 1.0f);
		p.size = 6.0f;
		p.life = GetRand(25) + 35;
		p.maxLife = p.life;
		p.color = GetColor(255, 110, 160);
		p.type = 3; // Heart
		mParticles.push_back(p);
	}

	// 浮遊テキスト (+0.5 HP)
	SpawnText(px - 26.0f, py - 65.0f, "+0.5 HP", GetColor(80, 255, 130), GetColor(10, 40, 10));
}

void HotSpring::TriggerBoilDeath(BlockMap& blockMap, float px, float py)
{
	// 猛烈な沸騰蒸気・炎・熱湯スプラッシュ
	for (int i = 0; i < 40; i++)
	{
		Particle p;
		p.x = px + (GetRand(40) - 20.0f);
		p.y = py + (GetRand(50) - 25.0f);
		p.vx = ((float)GetRand(50) - 25.0f) * 0.16f;
		p.vy = -((float)GetRand(45) / 10.0f + 2.2f);
		p.size = (float)GetRand(9) + 6.0f;
		p.life = GetRand(30) + 25;
		p.maxLife = p.life;
		int r = 255;
		int g = GetRand(140) + 40;
		int b = GetRand(30);
		p.color = GetColor(r, g, b);
		p.type = 5;
		mParticles.push_back(p);
	}
}

void HotSpring::Update(BlockMap& blockMap, Player* player)
{
	if (Master::gCurrentStageIndex != 1)
	{
		return;
	}

	mWaveTimer++;

	// 水面からの湯気生成
	if (GetRand(4) == 0)
	{
		float sx = mLeftX + 6.0f + GetRand(static_cast<int>(mRightX - mLeftX - 12.0f));
		float sy = mBaseWaterY + GetRand(6) - 3.0f;
		SpawnSteam(sx, sy, (mbPlayerInside && mHeatTimer >= 150));
	}

	// 底からの気泡生成
	if (GetRand(12) == 0)
	{
		float bx = mLeftX + 8.0f + GetRand(static_cast<int>(mRightX - mLeftX - 16.0f));
		SpawnBubble(bx, mFloorY - GetRand(6));
	}

	// パーティクルの更新
	for (auto it = mParticles.begin(); it != mParticles.end(); )
	{
		if (it->type == 0 || it->type == 5) // Steam / Boiling
		{
			it->x += it->vx + sinf(it->life * 0.12f) * 0.35f;
			it->y += it->vy;
			it->size += 0.09f;
		}
		else if (it->type == 1) // Bubble
		{
			it->x += sinf(it->life * 0.18f) * 0.4f;
			it->y += it->vy;
			if (it->y <= mBaseWaterY) it->life = 0;
		}
		else if (it->type == 2) // Cross (+)
		{
			it->x += it->vx;
			it->y += it->vy;
			it->vy += 0.025f;
		}
		else if (it->type == 3) // Heart
		{
			it->x += it->vx + sinf(it->life * 0.15f) * 0.45f;
			it->y += it->vy;
		}
		else if (it->type == 6) // Splash
		{
			it->x += it->vx;
			it->y += it->vy;
			it->vy += 0.28f; // 重力
		}
		else // Sparkle
		{
			it->x += it->vx;
			it->y += it->vy;
		}

		it->life--;
		if (it->life <= 0) it = mParticles.erase(it);
		else ++it;
	}

	// 浮遊テキストの更新
	for (auto it = mFloatingTexts.begin(); it != mFloatingTexts.end(); )
	{
		it->y += it->vy;
		it->life--;
		if (it->life <= 0) it = mFloatingTexts.erase(it);
		else ++it;
	}

	if (mHealPulseTimer > 0)
	{
		mHealPulseTimer--;
	}

	// プレイヤーとの判定
	mbPlayerInside = false;
	if (player && !player->GetIsDead())
	{
		VECTOR pPos = player->GetPosition();
		// プレイヤーの足元 (pPos.y + 50) がくぼみに入っており、X座標がくぼみ全体の範囲内
		if (pPos.x >= mLeftX - 5.0f && pPos.x <= mRightX + 5.0f && (pPos.y + 50.0f) >= mBaseWaterY + 15.0f)
		{
			mbPlayerInside = true;
			mPlayerX = pPos.x;
			mPlayerY = pPos.y;

			// プレイヤーに温泉のぬめりフラグをセット
			player->SetInHotSpring(true);

			mHeatTimer++;

			// ユーザー要望：文字を出すのではなく、鎧の色をだんだん赤色へ変えていく
			// 4秒（240フレーム）で最大熱（1.0）へ
			float heatRatio = static_cast<float>(mHeatTimer) / 240.0f;
			if (heatRatio > 1.0f) heatRatio = 1.0f;
			player->SetHotSpringHeat(heatRatio);

			// 1.5秒（90フレーム）で半ハート回復
			if (mHeatTimer == 90)
			{
				player->HealHalfHeart();
				TriggerHealBurst(pPos.x, pPos.y);
			}
			// 3.0秒（180フレーム）でさらに半ハート回復
			else if (mHeatTimer == 180)
			{
				player->HealHalfHeart();
				TriggerHealBurst(pPos.x, pPos.y);
			}

			// 2.5秒以降（過熱危険フェーズ）：赤熱した沸騰蒸気・火花がプレイヤーから激しく噴き出す
			if (mHeatTimer >= 150)
			{
				if (GetRand(2) == 0)
				{
					SpawnSteam(pPos.x + GetRand(28) - 14.0f, mBaseWaterY - 10.0f, true);
				}
				if (GetRand(4) == 0)
				{
					SpawnSparkle(pPos.x + GetRand(30) - 15.0f, pPos.y + GetRand(40) - 20.0f, GetColor(255, 80, 40));
				}
			}
			else
			{
				// 通常入浴の気持ちいい湯気・キラキラ
				if (GetRand(3) == 0)
				{
					SpawnSteam(pPos.x + GetRand(28) - 14.0f, mBaseWaterY - 8.0f, false);
				}
				if (GetRand(5) == 0)
				{
					SpawnSparkle(pPos.x + GetRand(32) - 16.0f, pPos.y + GetRand(40) - 20.0f, GetColor(100, 255, 160));
				}
			}

			// 4秒以上温泉にいた場合、暑すぎて熱中症で即死！
			// 4.0秒 = 240フレーム
			if (mHeatTimer >= 240)
			{
				TriggerBoilDeath(blockMap, pPos.x, pPos.y);
				player->TakeDamage(9999);
				player->SetDeathReason(DeathReason::DeathType::HotWater);
				player->SetInHotSpring(false);
				player->SetHotSpringHeat(0.0f);
				mHeatTimer = 0;
			}
		}
		else
		{
			// 脱出した場合：徐々に鎧の熱が冷めて元の青色に戻る
			float h = player->GetHotSpringHeat();
			if (h > 0.0f)
			{
				h -= 0.025f;
				if (h < 0.0f) h = 0.0f;
				player->SetHotSpringHeat(h);
				mHeatTimer = static_cast<int>(h * 240.0f);
			}
			player->SetInHotSpring(false);
		}
	}
	else
	{
		if (player)
		{
			player->SetInHotSpring(false);
			player->SetHotSpringHeat(0.0f);
		}
		mHeatTimer = 0;
	}
}

void HotSpring::Draw(int scrollX, int scrollY)
{

	if (Master::gCurrentStageIndex != 1)
	{
		return;
	}

	// 1. くぼみ全体（X: 6418 ～ 6755）へ隙間なく完全連続してお湯を描画
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160);

	float prevWaveY = mBaseWaterY;
	for (float wx = mLeftX; wx <= mRightX; wx += 2.0f)
	{
		float waveY = mBaseWaterY
			+ sinf(mWaveTimer * 0.05f + (wx - mLeftX) * 0.07f) * 3.0f
			+ cosf(mWaveTimer * 0.035f + (wx - mLeftX) * 0.045f) * 1.5f;

		// プレイヤーが浸かっている場合は水面に波紋干渉
		if (mbPlayerInside)
		{
			float dist = fabsf(wx - mPlayerX);
			if (dist < 42.0f)
			{
				float damp = 1.0f - (dist / 42.0f);
				waveY += sinf(mWaveTimer * 0.12f + dist * 0.2f) * 2.5f * damp;
			}
		}

		int sx1 = static_cast<int>(wx) - scrollX;
		int sy1 = static_cast<int>(waveY) - scrollY;
		int sx2 = static_cast<int>(wx + 2.5f) - scrollX;
		int sy2 = static_cast<int>(mFloorY) - scrollY;

		// 縦2マスの深さ（深層と表層で綺麗なグラデーション）
		int depth = sy2 - sy1;
		int midY = sy1 + (depth * 3) / 10;

		// 深層（濃いターコイズブルー）
		DrawBox(sx1, midY, sx2, sy2, GetColor(30, 130, 160), TRUE);
		// 表層（透明感のあるアクアグリーン）
		DrawBox(sx1, sy1, sx2, midY, GetColor(70, 205, 225), TRUE);

		// 水面の白い波頭ライン
		if (wx > mLeftX)
		{
			int prevSx = static_cast<int>(wx - 2.0f) - scrollX;
			int prevSy = static_cast<int>(prevWaveY) - scrollY;
			DrawLine(prevSx, prevSy, sx1, sy1, GetColor(225, 250, 255));
			DrawLine(prevSx, prevSy + 1, sx1, sy1 + 1, GetColor(160, 235, 245));
		}
		prevWaveY = waveY;
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// 2. 湯気・気泡・回復・熱湯パーティクルの描画
	for (const auto& p : mParticles)
	{
		int px = static_cast<int>(p.x) - scrollX;
		int py = static_cast<int>(p.y) - scrollY;

		if (p.type == 0 || p.type == 5) // Steam / Boiling
		{
			int alpha = static_cast<int>((float)p.life / p.maxLife * (p.type == 5 ? 180.0f : 125.0f));
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawCircle(px, py, static_cast<int>(p.size), p.color, TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		else if (p.type == 1) // Bubble
		{
			int alpha = static_cast<int>((float)p.life / p.maxLife * 180.0f);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawCircle(px, py, static_cast<int>(p.size), p.color, FALSE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		else if (p.type == 2) // Cross (+)
		{
			int alpha = static_cast<int>((float)p.life / p.maxLife * 255.0f);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawLine(px - 4, py, px + 5, py, p.color);
			DrawLine(px, py - 4, px, py + 5, p.color);
			DrawLine(px - 1, py, px + 2, py, GetColor(240, 255, 240));
			DrawLine(px, py - 1, px, py + 2, GetColor(240, 255, 240));
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		else if (p.type == 3) // Heart
		{
			int alpha = static_cast<int>((float)p.life / p.maxLife * 255.0f);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawCircle(px - 2, py - 2, 3, p.color, TRUE);
			DrawCircle(px + 2, py - 2, 3, p.color, TRUE);
			DrawTriangle(px - 5, py - 1, px + 5, py - 1, px, py + 5, p.color, TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		else if (p.type == 6) // Splash
		{
			int alpha = static_cast<int>((float)p.life / p.maxLife * 240.0f);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawCircle(px, py, static_cast<int>(p.size), p.color, TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		else // Sparkle
		{
			int alpha = static_cast<int>((float)p.life / p.maxLife * 220.0f);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawLine(px - 3, py, px + 4, py, p.color);
			DrawLine(px, py - 3, px, py + 4, p.color);
			DrawPixel(px, py, GetColor(255, 255, 255));
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	// 3. 回復時のオーラリングエフェクト
	if (mHealPulseTimer > 0)
	{
		int psx = static_cast<int>(mPlayerX) - scrollX;
		int psy = static_cast<int>(mPlayerY) - scrollY;
		int radius = (28 - mHealPulseTimer) * 2 + 15;
		int alpha = mHealPulseTimer * 8;
		if (alpha > 255) alpha = 255;

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawCircle(psx, psy, radius, GetColor(70, 255, 120), FALSE);
		DrawCircle(psx, psy, radius - 1, GetColor(150, 255, 180), FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// 4. 回復時の浮遊テキスト (+0.5 HP のみ)
	for (const auto& ft : mFloatingTexts)
	{
		int tx = static_cast<int>(ft.x) - scrollX;
		int ty = static_cast<int>(ft.y) - scrollY;
		int alpha = static_cast<int>((float)ft.life / ft.maxLife * 255.0f);

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		// フチ
		DrawString(tx - 1, ty, ft.text, ft.outlineColor);
		DrawString(tx + 1, ty, ft.text, ft.outlineColor);
		DrawString(tx, ty - 1, ft.text, ft.outlineColor);
		DrawString(tx, ty + 1, ft.text, ft.outlineColor);
		// 本文
		DrawString(tx, ty, ft.text, ft.color);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

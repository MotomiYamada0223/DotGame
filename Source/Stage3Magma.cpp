#include "Stage3Magma.h"
#include "BlockMap.h"
#include "Player.h"
#include "GameConstants.h"
#include <cmath>

Stage3Magma::Stage3Magma()
	: mWaveTimer(0)
	, mDrainRatio(0.0f)
	, mbMagmaGone(false)
	, mbPlayerInMagma(false)
	, mSplashCooldown(0)
	, mSplashTex(-1)
	, mFireTex{ -1, -1, -1, -1 }
	, mFireSingleTex(-1)
	, mBurstRedTex(-1)
{
}

Stage3Magma::~Stage3Magma()
{
	Finalize();
}

void Stage3Magma::Initialize()
{
	Finalize();
	mSplashTex = LoadGraph("Resource/Effect/Effekseer01/Effekseer01/Texture/Splash01.png");
	LoadDivGraph("Resource/Effect/Pierre01/Pierre01/Texture/Fire.png", 4, 2, 2, 256, 256, mFireTex);
	mFireSingleTex = LoadGraph("Resource/Effect/Pierre01/Pierre01/Texture/Fire_Single.png");
	mBurstRedTex   = LoadGraph("Resource/Effect/tktk01/tktk01/Texture/burst02_red.png");
	Reset();
}

void Stage3Magma::Reset()
{
	mWaveTimer  = 0;
	mDrainRatio = 0.0f;
	mbMagmaGone     = false;
	mbPlayerInMagma = false;
	mSplashCooldown = 0;
	mParticles.clear();
	mSplashEffects.clear();
}

void Stage3Magma::Finalize()
{
	mParticles.clear();
	mSplashEffects.clear();
	if (mSplashTex != -1)
	{
		DeleteGraph(mSplashTex);
		mSplashTex = -1;
	}
	for (int i = 0; i < 4; ++i)
	{
		if (mFireTex[i] != -1)
		{
			DeleteGraph(mFireTex[i]);
			mFireTex[i] = -1;
		}
	}
	if (mFireSingleTex != -1)
	{
		DeleteGraph(mFireSingleTex);
		mFireSingleTex = -1;
	}
	if (mBurstRedTex != -1)
	{
		DeleteGraph(mBurstRedTex);
		mBurstRedTex = -1;
	}
}

void Stage3Magma::SetDrainRatio(float ratio)
{
	if (mbMagmaGone) return;
	if (ratio < 0.0f) ratio = 0.0f;
	if (ratio > 1.0f) ratio = 1.0f;
	mDrainRatio = ratio;
}

void Stage3Magma::ClearMagma()
{
	mbMagmaGone = true;
	mDrainRatio = 1.0f;
	mParticles.clear();
}

void Stage3Magma::SpawnBubble(float x, float y)
{
	MagmaParticle p;
	p.x       = x;
	p.y       = y;
	p.vx      = 0.0f;
	p.vy      = -((GetRand(12) / 10.0f) + 0.6f);
	p.size    = static_cast<float>(GetRand(5) + 4);
	p.life    = GetRand(35) + 30;
	p.maxLife = p.life;
	p.color   = GetColor(255, 190 + GetRand(55), 40);
	p.type    = 0;
	mParticles.push_back(p);
}

void Stage3Magma::SpawnEmber(float x, float y)
{
	MagmaParticle p;
	p.x       = x;
	p.y       = y;
	p.vx      = (GetRand(20) - 10) * 0.08f;
	p.vy      = -((GetRand(22) / 10.0f) + 1.0f);
	p.size    = static_cast<float>(GetRand(4) + 3);
	p.life    = GetRand(40) + 35;
	p.maxLife = p.life;
	int g     = GetRand(140) + 80;
	p.color   = GetColor(255, g, 20);
	p.type    = 1;
	mParticles.push_back(p);
}

void Stage3Magma::SpawnSplash(float x, float y)
{
	MagmaSplashEffect sp;
	sp.x        = x;
	sp.y        = y;
	sp.timer    = 0;
	sp.maxTimer = 28;
	mSplashEffects.push_back(sp);

	for (int i = 0; i < 18; ++i)
	{
		MagmaParticle p;
		float side = (i % 2 == 0) ? -1.0f : 1.0f;
		float spread = 1.4f + static_cast<float>(GetRand(42)) * 0.10f;
		p.x       = x + side * static_cast<float>(GetRand(18));
		p.y       = y - 4.0f;
		p.vx      = side * spread;
		p.vy      = -(5.5f + static_cast<float>(GetRand(65)) * 0.10f);
		p.size    = static_cast<float>(GetRand(10) + 12);
		p.life    = GetRand(18) + 24;
		p.maxLife = p.life;
		int g     = GetRand(110) + 65;
		p.color   = GetColor(255, g, 12);
		p.type    = 2;
		mParticles.push_back(p);
	}
}

void Stage3Magma::Update(BlockMap& blockMap, Player* player)
{
	mWaveTimer++;

	// マグマが完全に消滅した後は、くぼみの底に降りたプレイヤーが左右の崖へ戻りやすいよう
	// くぼみ両端に風の余韻（上昇気流アシスト）を発生させ、マグマの即死判定は行わない
	if (IsMagmaGone())
	{
		if (player != nullptr && !player->GetIsDead() && !player->IsWindFloatMode())
		{
			VECTOR pPos = player->GetPosition();
			bool inLeftUpdraft  = (pPos.x >= MAGMA_LEFT_X  && pPos.x <= MAGMA_LEFT_X  + 140.0f && pPos.y >= 1120.0f);
			bool inRightUpdraft = (pPos.x >= MAGMA_RIGHT_X - 140.0f && pPos.x <= MAGMA_RIGHT_X && pPos.y >= 1150.0f);
			if ((inLeftUpdraft || inRightUpdraft) && player->GetVelocityY() < -1.0f)
			{
				// ジャンプ中にくぼみ両端の上昇気流に乗ると左右の足場（Y=1116/1152）まで軽々登れる
				player->SetVelocityY(player->GetVelocityY() - 0.18f);
			}
		}
		return;
	}

	float currentSurfY = GetCurrentSurfaceY();
	int pitWidth = static_cast<int>(MAGMA_RIGHT_X - MAGMA_LEFT_X - 24.0f);
	if (pitWidth > 0 && currentSurfY < MAGMA_BOTTOM_Y - 16.0f)
	{
		// マグマの底からグツグツ湧き上がる泡
		if (GetRand(4) == 0)
		{
			float bx = MAGMA_LEFT_X + 12.0f + static_cast<float>(GetRand(pitWidth));
			float by = MAGMA_BOTTOM_Y - 8.0f;
			if (by > currentSurfY + 8.0f)
			{
				SpawnBubble(bx, by);
			}
		}

		// マグマ表面から立ち昇る火の粉
		if (GetRand(2) == 0)
		{
			float ex = MAGMA_LEFT_X + 12.0f + static_cast<float>(GetRand(pitWidth));
			float ey = currentSurfY + static_cast<float>(GetRand(10) - 5);
			SpawnEmber(ex, ey);
		}
	}

	// パーティクルの更新
	for (auto it = mParticles.begin(); it != mParticles.end(); )
	{
		if (it->type == 0) // 溶岩の泡
		{
			it->x += sinf(it->life * 0.18f) * 0.35f;
			it->y += it->vy;
			if (it->y <= currentSurfY + 4.0f)
			{
				it->y = currentSurfY + 4.0f;
				it->size += 0.25f;
				it->life -= 3;
			}
		}
		else if (it->type == 1) // 火の粉
		{
			it->x += it->vx + sinf(it->life * 0.15f) * 0.45f;
			it->y += it->vy;
		}
		else // マグマしぶき
		{
			it->x  += it->vx;
			it->y  += it->vy;
			it->vy += 0.28f;
		}

		it->life--;
		if (it->life <= 0)
		{
			it = mParticles.erase(it);
		}
		else
		{
			++it;
		}
	}

	for (auto it = mSplashEffects.begin(); it != mSplashEffects.end(); )
	{
		it->timer++;
		if (it->timer >= it->maxTimer)
		{
			it = mSplashEffects.erase(it);
		}
		else
		{
			++it;
		}
	}

	if (mSplashCooldown > 0)
	{
		mSplashCooldown--;
	}

	// プレイヤーがマグマに触れたら入った瞬間だけマグマ爆発を出して即死！
	if (player != nullptr && !player->GetIsDead())
	{
		VECTOR pPos  = player->GetPosition();
		float pHalfW = PlayerConstants::PlayerCollisionWidth * 0.5f;  // 16.0f
		float pHalfH = PlayerConstants::PlayerCollisionHeight * 0.5f; // 50.0f
		float pFootY = pPos.y + pHalfH;

		bool inMagma = (pPos.x + pHalfW > MAGMA_LEFT_X + 4.0f &&
		                pPos.x - pHalfW < MAGMA_RIGHT_X - 4.0f &&
		                pFootY >= currentSurfY + 10.0f &&
		                pPos.y - pHalfH <= MAGMA_BOTTOM_Y);
		if (inMagma)
		{
			if (!mbPlayerInMagma && mSplashCooldown <= 0)
			{
				SpawnSplash(pPos.x, currentSurfY);
				mSplashCooldown = 45;
				blockMap.mShakeX = 14;
				blockMap.mShakeY = 18;
			}
			mbPlayerInMagma = true;
			player->TakeDamage(9999, DeathReason::DeathType::DragonFireBreath);
		}
		else
		{
			mbPlayerInMagma = false;
		}
	}
	else
	{
		mbPlayerInMagma = false;
	}
}

void Stage3Magma::Draw(const BlockMap& blockMap)
{
	int drawScrollX = blockMap.GetScrollX() + blockMap.mShakeX;
	int drawScrollY = blockMap.GetScrollY() + blockMap.mShakeY;

	int leftSX   = static_cast<int>(MAGMA_LEFT_X)   - drawScrollX;
	int rightSX  = static_cast<int>(MAGMA_RIGHT_X)  - drawScrollX;
	int bottomSY = static_cast<int>(MAGMA_BOTTOM_Y) - drawScrollY;

	// 画面外なら描画スキップ
	int viewW = ScreenSize::ScrrenWidth, viewH = ScreenSize::ScrrenHeight;
	int curScreen = GetDrawScreen();
	if (curScreen > 0)
	{
		GetGraphSize(curScreen, &viewW, &viewH);
	}
	if (rightSX < 0 || leftSX > viewW) return;

	// マグマ消滅後は地面のマグマを描画せず、くぼみ両端の微かな風の上昇気流ラインのみ描画
	if (IsMagmaGone())
	{
		SetDrawBlendMode(DX_BLENDMODE_ADD, 85);
		for (int i = 0; i < 4; ++i)
		{
			int offset = ((mWaveTimer * 4 + i * 70) % 260);
			int ly = bottomSY - offset;
			DrawLine(leftSX + 28 + i * 24, ly, leftSX + 28 + i * 24, ly - 32, GetColor(120, 255, 210), 2);
			DrawLine(rightSX - 28 - i * 24, ly, rightSX - 28 - i * 24, ly - 32, GetColor(120, 255, 210), 2);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		return;
	}

	float currentSurfY = GetCurrentSurfaceY();
	int   surfSY       = static_cast<int>(currentSurfY) - drawScrollY;
	int   remainH      = bottomSY - surfSY;
	if (remainH <= 2) return;

	// 1. マグマ表面から上方に立ち昇る赤い熱気グロー（加算合成）
	SetDrawBlendMode(DX_BLENDMODE_ADD, 55);
	for (int g = 0; g < 5; ++g)
	{
		int glowH = (g + 1) * 24;
		DrawBox(leftSX, surfSY - glowH, rightSX, surfSY, GetColor(220, 45, 5), TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// 2. マグマ本体の多層グラデーション描画（水位低下に合わせてクランプ）
	int mid1Y = (surfSY + 12 < bottomSY) ? (surfSY + 12) : bottomSY;
	int mid2Y = (surfSY + 78 < bottomSY) ? (surfSY + 78) : bottomSY;
	int mid3Y = (surfSY + 42 < bottomSY) ? (surfSY + 42) : bottomSY;
	int top3Y = (surfSY + 8  < bottomSY) ? (surfSY + 8)  : bottomSY;

	DrawBox(leftSX, mid1Y, rightSX, bottomSY, GetColor(135, 18, 5), TRUE);
	DrawBox(leftSX, mid1Y, rightSX, mid2Y,    GetColor(190, 38, 8), TRUE);
	DrawBox(leftSX, top3Y, rightSX, mid3Y,    GetColor(235, 78, 12), TRUE);

	// 3. うねるマグマ表面の波と白熱クラスト（8px刻みでサイン波アニメ）
	float phase = mWaveTimer * 0.07f;
	const int stepX = 8;
	int waveBottom = (surfSY + 22 < bottomSY) ? (surfSY + 22) : bottomSY;
	for (int sx = leftSX; sx < rightSX; sx += stepX)
	{
		int nx = (sx + stepX > rightSX) ? rightSX : (sx + stepX);
		float worldX = static_cast<float>(sx + drawScrollX);
		float wave1  = sinf(worldX * 0.025f + phase) * 5.0f;
		float wave2  = cosf(worldX * 0.055f - phase * 1.4f) * 3.0f;
		int wy       = surfSY + static_cast<int>(wave1 + wave2);
		if (wy > bottomSY - 4) wy = bottomSY - 4;

		// 表面の明るいオレンジ層
		if (waveBottom > wy)
		{
			DrawBox(sx, wy, nx, waveBottom, GetColor(255, 120, 18), TRUE);
		}
		// 最表面の白熱イエローライン
		DrawBox(sx, wy, nx, wy + 6, GetColor(255, 225, 75), TRUE);
		DrawBox(sx, wy, nx, wy + 2, GetColor(255, 250, 170), TRUE);
	}

	// 3.5 2D Effekseer (Fire.png / Fire_Single.png / burst02_red.png) によるマグマだまりのうねる溶岩・熱気演出
	SetDrawArea(leftSX, surfSY - 95, rightSX, bottomSY);
	int poolW = rightSX - leftSX;
	if (mFireTex[0] != -1)
	{
		int cols = (poolW / 100) + 2;
		for (int c = 0; c < cols; ++c)
		{
			int cx = leftSX + c * 100 + static_cast<int>(sinf(phase * 0.7f + c) * 24.0f);
			int cy = surfSY + remainH / 2 + static_cast<int>(cosf(phase * 1.0f + c * 1.3f) * 16.0f);
			int fTex1 = mFireTex[((mWaveTimer / 3) + c) & 3];
			int fTex2 = mFireTex[((mWaveTimer / 4) + c + 2) & 3];
			if (fTex1 != -1)
			{
				SetDrawBright(255, 95, 12);
				SetDrawBlendMode(DX_BLENDMODE_ADD, 175);
				DrawRotaGraph(cx, cy, 1.22, 1.5707963, fTex1, TRUE);
			}
			if (fTex2 != -1)
			{
				SetDrawBright(255, 200, 50);
				SetDrawBlendMode(DX_BLENDMODE_ADD, 120);
				DrawRotaGraph(cx + 42, surfSY + 20, 0.85, -1.5707963, fTex2, TRUE);
			}
		}
	}
	if (mBurstRedTex != -1)
	{
		int bCols = (poolW / 180) + 2;
		for (int c = 0; c < bCols; ++c)
		{
			int bx = leftSX + c * 180 + static_cast<int>(cosf(phase * 0.8f + c * 1.9f) * 22.0f);
			int by = surfSY + remainH / 3 + static_cast<int>(sinf(phase * 1.1f + c) * 10.0f);
			SetDrawBright(255, 120, 20);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 115);
			DrawRotaGraph(bx, by, 0.24 + sinf(phase + c) * 0.04, phase * 0.4f + c, mBurstRedTex, TRUE);
		}
	}
	if (mFireSingleTex != -1)
	{
		int cols = (poolW / 95) + 2;
		for (int c = 0; c < cols; ++c)
		{
			int fx = leftSX + c * 95 + static_cast<int>(sinf(phase * 1.2f + c * 1.7f) * 18.0f);
			int fy = surfSY - 20 + static_cast<int>(cosf(phase * 1.5f + c) * 7.0f);
			SetDrawBright(255, 130, 20);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 165);
			DrawRotaGraph(fx, fy, 0.78, sinf(phase + c) * 0.22, mFireSingleTex, TRUE);
			SetDrawBright(255, 225, 85);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 105);
			DrawRotaGraph(fx, fy + 6, 0.48, -sinf(phase + c) * 0.16, mFireSingleTex, TRUE);
		}
	}
	SetDrawBright(255, 255, 255);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	SetDrawArea(0, 0, viewW, viewH);

	// 4. マグマ内部をゆっくり流れる明るい溶岩脈の模様
	if (remainH > 40)
	{
		for (int intband = 0; intband < 6; ++intband)
		{
			float bandYRatio = (intband + 1) / 7.0f;
			int by = surfSY + 20 + static_cast<int>((remainH - 28) * bandYRatio);
			if (by >= bottomSY - 6) continue;
			for (int sx = leftSX + 16; sx < rightSX - 48; sx += 110)
			{
				float wX = static_cast<float>(sx + drawScrollX);
				int offsetX = static_cast<int>(sinf(phase * 0.8f + intband * 1.7f + wX * 0.01f) * 18.0f);
				int offsetY = static_cast<int>(cosf(phase * 1.1f + intband * 2.1f + wX * 0.02f) * 4.0f);
				DrawOval(sx + offsetX + 24, by + offsetY, 26, 4, GetColor(245, 95, 15), TRUE);
			}
		}
	}

	// 5. 泡・火の粉・マグマしぶきパーティクルの描画
	for (const auto& p : mParticles)
	{
		int px = static_cast<int>(p.x) - drawScrollX;
		int py = static_cast<int>(p.y) - drawScrollY;
		int r  = static_cast<int>(p.size);

		if (p.type == 0) // 溶岩の泡
		{
			DrawCircle(px, py, r, GetColor(255, 110, 15), TRUE);
			DrawCircle(px, py, (r > 2) ? (r - 1) : r, p.color, TRUE);
			DrawCircle(px - 1, py - 1, (r > 3) ? (r / 2) : 1, GetColor(255, 250, 180), TRUE);
		}
		else if (p.type == 1) // 立ち昇る火の粉
		{
			int alpha = (p.life * 255) / p.maxLife;
			SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
			DrawCircle(px, py, r + 2, GetColor(255, 80, 10), TRUE);
			DrawCircle(px, py, r, p.color, TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		else // 粘性のある溶岩のしぶき粒（液体本体＋発光コア）
		{
			int alpha = (p.life * 245) / p.maxLife;
			int dropR = (r > 4) ? (r / 2) : 3;
			int tailX = px - static_cast<int>(p.vx * 1.8f);
			int tailY = py - static_cast<int>(p.vy * 1.8f);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawLine(tailX, tailY, px, py, GetColor(220, 48, 8), dropR + 2);
			DrawCircle(px, py, dropR + 2, GetColor(235, 68, 10), TRUE);
			DrawCircle(px, py, dropR, GetColor(255, 155, 30), TRUE);
			DrawCircle(px - 1, py - 1, (dropR > 2) ? (dropR / 2) : 1, GetColor(255, 235, 125), TRUE);
			int fTex = mFireTex[p.life & 3];
			if (fTex != -1)
			{
				double sc = (p.size * 1.45) / 256.0;
				float rot = std::atan2(p.vy, p.vx) + 1.5707963f;
				SetDrawBright(255, 95, 15);
				SetDrawBlendMode(DX_BLENDMODE_ADD, alpha / 2);
				DrawRotaGraph(px, py, sc, rot, fTex, TRUE);
				SetDrawBright(255, 255, 255);
			}
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	// 6. ジャポンと跳ね上がる溶岩液体スプラッシュ（溶岩クラウン＋波紋＋ADD Splash01）
	for (const auto& sp : mSplashEffects)
	{
		float t = static_cast<float>(sp.timer) / static_cast<float>(sp.maxTimer);
		int sx  = static_cast<int>(sp.x) - drawScrollX;
		int sy  = static_cast<int>(sp.y) - drawScrollY;
		int alpha = static_cast<int>((1.0f - t) * 245.0f);
		float rise = std::sin(t * 3.14159265f);

		// (A) 液面の二重波紋リング
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawOval(sx, sy + 4, static_cast<int>(28.0f + t * 88.0f), static_cast<int>(7.0f + t * 14.0f), GetColor(200, 35, 5), FALSE, 5);
		DrawOval(sx, sy + 2, static_cast<int>(18.0f + t * 56.0f), static_cast<int>(5.0f + t * 9.0f),  GetColor(255, 155, 25), FALSE, 3);

		// (B) 液面から王冠状に盛り上がる粘性溶岩プルーム（5本の液体柱）
		for (int k = -2; k <= 2; ++k)
		{
			float kNorm = static_cast<float>(k) * 0.5f;
			int px = sx + static_cast<int>(kNorm * (24.0f + t * 54.0f));
			float hFactor = (k == 0) ? 1.0f : (1.0f - std::fabs(kNorm) * 0.35f);
			int plumeH = static_cast<int>((14.0f + rise * 48.0f) * hFactor);
			int plumeW = static_cast<int>((16.0f - t * 6.0f) * hFactor);
			DrawOval(px, sy - plumeH / 2, plumeW, plumeH / 2 + 4, GetColor(215, 42, 6), TRUE);
			DrawOval(px, sy - plumeH / 2 + 3, static_cast<int>(plumeW * 0.65f), static_cast<int>(plumeH * 0.42f) + 2, GetColor(255, 130, 18), TRUE);
			DrawCircle(px, sy - plumeH, (plumeW > 6) ? (plumeW / 2) : 4, GetColor(255, 210, 65), TRUE);
		}

		// (C) 2D Effekseer Splash01 の加算グロウ（黒背景を透明化して溶岩の飛沫感を強調）
		if (mSplashTex != -1)
		{
			double scaleX = 0.28 + t * 0.26;
			double scaleY = 0.16 + std::sin(t * 2.50f) * 0.34;
			int drawY = sy - static_cast<int>(512.0 * scaleY * 0.44);
			SetDrawBright(255, 90, 12);
			SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(alpha * 0.90f));
			DrawRotaGraph3(sx, drawY, 256, 256, scaleX, scaleY, 0.0, mSplashTex, TRUE);
			SetDrawBright(255, 200, 60);
			SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(alpha * 0.72f));
			DrawRotaGraph3(sx, drawY + 10, 256, 256, scaleX * 0.70, scaleY * 0.78, 0.0, mSplashTex, TRUE);
			SetDrawBright(255, 255, 255);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

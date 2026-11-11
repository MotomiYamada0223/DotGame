#include "DragonFireGimmick.h"
#include "BlockMap.h"
#include "Player.h"
#include <cmath>

namespace
{
	float ToScreenX(float worldX, const BlockMap* blockMap)
	{
		float scrollX = (blockMap != nullptr) ? static_cast<float>(blockMap->GetScrollX()) : 0.0f;
		return worldX - scrollX;
	}

	float ToScreenY(float worldY, const BlockMap* blockMap)
	{
		float scrollY = (blockMap != nullptr) ? static_cast<float>(blockMap->GetScrollY()) : 0.0f;
		return worldY - scrollY;
	}
}


void DragonFireGimmick::ClearAll()
{
	mFireProjectiles.clear();
	mFireSparks.clear();
	mBurningGrounds.clear();
	mArchFlameWaves.clear();
	mCenterFireball.active = false;
	mCenterFireball.exploding = false;
}

void DragonFireGimmick::SpawnBreathInhaleSparks(float mouthX, float mouthY)
{
	for (int i = 0; i < 2; ++i)
	{
		float ang = (GetRand(360) * DX_PI_F) / 180.0f;
		float dist = 45.0f + static_cast<float>(GetRand(45));
		FireSpark sp;
		sp.x = mouthX + std::cos(ang) * dist;
		sp.y = mouthY + std::sin(ang) * dist;
		sp.vx = (mouthX - sp.x) * 0.12f;
		sp.vy = (mouthY - sp.y) * 0.12f;
		sp.size = 3.0f + static_cast<float>(GetRand(4));
		sp.life = 10;
		sp.maxLife = 10;
		sp.color = (i % 2 == 0) ? GetColor(255, 120, 20) : GetColor(255, 230, 90);
		mFireSparks.push_back(sp);
	}
}

void DragonFireGimmick::SpawnBreathStream(float mouthX, float mouthY, float targetX, float targetY)
{
	float fdx = targetX - mouthX;
	float fdy = targetY - mouthY;
	if (fdy < 90.0f) fdy = 90.0f;
	float len = std::sqrt(fdx * fdx + fdy * fdy);
	if (len < 1.0f) len = 1.0f;

	// 毎フレーム濃密な火炎ストリーム弾を発射（口元は細く、地面に近づくほど巨大に膨張する）
	float speed = 17.5f + static_cast<float>(GetRand(30)) * 0.1f;
	FireProjectile fp;
	fp.x = mouthX + (fdx / len) * 12.0f;
	fp.y = mouthY + (fdy / len) * 12.0f;
	fp.vx = (fdx / len) * speed;
	fp.vy = (fdy / len) * speed;
	fp.radius = 15.0f;
	fp.maxRadius = 46.0f + static_cast<float>(GetRand(12));
	fp.swaySeed = static_cast<float>(GetRand(628)) * 0.01f;
	fp.animTick = 0;
	fp.active = true;
	mFireProjectiles.push_back(fp);

	// 口元から飛び散る火の粉スパーク
	for (int i = 0; i < 3; ++i)
	{
		FireSpark sp;
		sp.x = mouthX;
		sp.y = mouthY;
		float spread = (static_cast<float>(GetRand(60) - 30) * 3.14159265f) / 180.0f;
		float baseAng = std::atan2(fdy, fdx) + spread;
		float spSpeed = 9.0f + static_cast<float>(GetRand(100)) * 0.1f;
		sp.vx = std::cos(baseAng) * spSpeed;
		sp.vy = std::sin(baseAng) * spSpeed;
		sp.size = 3.0f + static_cast<float>(GetRand(5));
		sp.life = 16 + GetRand(14);
		sp.maxLife = sp.life;
		sp.color = (i == 0) ? GetColor(255, 255, 160) : GetColor(255, 110, 20);
		mFireSparks.push_back(sp);
	}
}

void DragonFireGimmick::SpawnCenterChargeSparks(float mouthX, float mouthY)
{
	for (int i = 0; i < 3; ++i)
	{
		float ang = (GetRand(360) * DX_PI_F) / 180.0f;
		float dist = 50.0f + static_cast<float>(GetRand(50));
		FireSpark sp;
		sp.x = mouthX + std::cos(ang) * dist;
		sp.y = mouthY + std::sin(ang) * dist;
		sp.vx = (mouthX - sp.x) * 0.15f;
		sp.vy = (mouthY - sp.y) * 0.15f;
		sp.size = 4.0f + static_cast<float>(GetRand(4));
		sp.life = 10;
		sp.maxLife = 10;
		sp.color = (i % 2 == 0) ? GetColor(255, 90, 20) : GetColor(255, 240, 120);
		mFireSparks.push_back(sp);
	}
}

void DragonFireGimmick::LaunchCenterFireball(float mouthX, float mouthY, float targetX, float targetY)
{
	mCenterFireball.active = true;
	mCenterFireball.exploding = false;
	mCenterFireball.x = mouthX;
	mCenterFireball.y = mouthY;

	float fdx = targetX - mouthX;
	float fdy = targetY - mouthY;
	float travelFrames = 32.0f;
	mCenterFireball.vx = fdx / travelFrames;
	mCenterFireball.vy = fdy / travelFrames;
	mCenterFireball.radius = 52.0f;
	mCenterFireball.explosionTimer = 0;
	mCenterFireball.explosionRadius = 0.0f;
}

void DragonFireGimmick::SpawnDiveTrailSparks(float x, float y, float diveVx, float diveVy)
{
	for (int i = 0; i < 2; ++i)
	{
		FireSpark sp;
		sp.x = x + static_cast<float>(GetRand(60) - 30);
		sp.y = y + static_cast<float>(GetRand(60) - 30);
		sp.vx = -diveVx * 0.25f + static_cast<float>(GetRand(20) - 10) * 0.2f;
		sp.vy = -diveVy * 0.25f + static_cast<float>(GetRand(20) - 10) * 0.2f;
		sp.size = 3.0f + static_cast<float>(GetRand(4));
		sp.life = 12 + GetRand(8);
		sp.maxLife = sp.life;
		sp.color = (i == 0) ? GetColor(255, 80, 80) : GetColor(140, 220, 255);
		mFireSparks.push_back(sp);
	}
}

void DragonFireGimmick::SpawnDiveLandingSparks(float x, float facingDir)
{
	for (int s = 0; s < 12; ++s)
	{
		FireSpark sp;
		sp.x = x + facingDir * 60.0f + static_cast<float>(GetRand(60) - 30);
		sp.y = 1900.0f;
		sp.vx = facingDir * (4.0f + static_cast<float>(GetRand(80)) * 0.1f) + static_cast<float>(GetRand(60) - 30) * 0.1f;
		sp.vy = -static_cast<float>(30 + GetRand(80)) * 0.1f;
		sp.size = 4.0f + static_cast<float>(GetRand(4));
		sp.life = 16 + GetRand(14);
		sp.maxLife = sp.life;
		sp.color = (s % 2 == 0) ? GetColor(255, 220, 100) : GetColor(255, 90, 30);
		mFireSparks.push_back(sp);
	}
}

void DragonFireGimmick::Update(Player* player, BlockMap* blockMap, int attackDamage)
{
	const float floorY = 1903.0f;

	UpdateFireProjectiles(player, blockMap, attackDamage, floorY);
	UpdateFireSparks();
	UpdateBurningGrounds(player, attackDamage);
	UpdateCenterFireball(player, blockMap, attackDamage, floorY);
	UpdateArchFlameWaves(player, attackDamage, floorY);
}

// 1. 空中から噴かれた火炎ブレス弾の移動・膨張・プレイヤー当たり判定・地面着弾判定
void DragonFireGimmick::UpdateFireProjectiles(Player* player, BlockMap* blockMap, int attackDamage, float floorY)
{
	for (auto& fp : mFireProjectiles)
	{
		if (!fp.active) continue;

		fp.x += fp.vx;
		fp.y += fp.vy;
		fp.animTick++;

		// 口元から離れるにつれて円錐状に大きく渦巻きながら膨張する
		if (fp.radius < fp.maxRadius)
		{
			fp.radius += 1.35f;
			if (fp.radius > fp.maxRadius) fp.radius = fp.maxRadius;
		}

		// プレイヤーへの直撃判定
		if (player && !player->GetIsDead())
		{
			float px = player->GetPosition().x;
			float py = player->GetPosition().y;
			float pLeft = px - 16.0f;
			float pRight = px + 16.0f;
			float pTop = py - 50.0f;
			float pBot = py + 50.0f;

			float hitR = fp.radius * 0.75f;
			if (fp.x + hitR > pLeft && fp.x - hitR < pRight &&
				fp.y + hitR > pTop && fp.y - hitR < pBot)
			{
				player->TakeDamage(attackDamage);
				// ドラゴンのブレス
				player->SetDeathReason(DeathReason::DeathType::DragonFireBreath);
			}
		}

		// 地面（Y >= 1903）またはブロックに着弾したら、激しい火柱と火花を散らして地面に燃え広がる！
		bool hitGround = (fp.y >= floorY - 6.0f);
		if (!hitGround && blockMap != nullptr && fp.y >= 1750.0f)
		{
			if (blockMap->GetCollisionType(static_cast<int>(fp.x), static_cast<int>(fp.y)) == BlockMap::CollisionType::Block)
			{
				hitGround = true;
			}
		}

		if (hitGround)
		{
			fp.active = false;
			float burnX = fp.x;
			if (burnX < 8000.0f) burnX = 8000.0f;
			if (burnX > 9780.0f) burnX = 9780.0f;

			// 着弾時に左右へ爆散する火花スパークを生成
			for (int s = 0; s < 5; ++s)
			{
				FireSpark sp;
				sp.x = burnX + static_cast<float>(GetRand(30) - 15);
				sp.y = floorY - 4.0f;
				sp.vx = static_cast<float>(GetRand(240) - 120) * 0.1f;
				sp.vy = -static_cast<float>(40 + GetRand(90)) * 0.1f;
				sp.size = 3.0f + static_cast<float>(GetRand(5));
				sp.life = 18 + GetRand(20);
				sp.maxLife = sp.life;
				sp.color = (s % 2 == 0) ? GetColor(255, 220, 80) : GetColor(255, 90, 15);
				mFireSparks.push_back(sp);
			}

			// 既に近く（150px以内）に燃焼エリアがある場合は、炎の勢い（広がり幅）をさらに拡大し、タイマーを更新
			bool merged = false;
			for (auto& bg : mBurningGrounds)
			{
				if (std::fabs(bg.x - burnX) < 150.0f)
				{
					bg.timer = 300; // 5秒間 (60fps × 5s)
					// 着弾が続くほど左右へさらにカッコよく燃え広がる（最大半径 220px = 全幅 440px）
					if (bg.targetHalfWidth < 220.0f)
					{
						bg.targetHalfWidth += 10.0f;
						if (bg.targetHalfWidth > 220.0f) bg.targetHalfWidth = 220.0f;
					}
					merged = true;
					break;
				}
			}

			if (!merged)
			{
				BurningGround bg;
				bg.x = burnX;
				bg.y = floorY;
				bg.currentHalfWidth = 18.0f;  // 最初は中心から始まり、そこから左右へ急速に燃え広がる！
				bg.targetHalfWidth = 150.0f; // 基本の広がり幅（全幅 300px、連続着弾で最大 440px まで拡大）
				bg.timer = 300;               // 5秒間 (60fps × 5s)
				bg.maxTimer = 300;
				bg.age = 0;
				mBurningGrounds.push_back(bg);
			}
		}
		else if (fp.x < 7800.0f || fp.x > 9900.0f || fp.y > 2100.0f)
		{
			fp.active = false;
		}
	}

	// 無効化された火炎弾を削除
	for (auto it = mFireProjectiles.begin(); it != mFireProjectiles.end(); )
	{
		if (!it->active) it = mFireProjectiles.erase(it);
		else ++it;
	}
}

// 2. 飛び散る火花パーティクル（FireSpark）の更新
void DragonFireGimmick::UpdateFireSparks()
{
	for (auto it = mFireSparks.begin(); it != mFireSparks.end(); )
	{
		it->x += it->vx;
		it->y += it->vy;
		it->vy += 0.22f; // 軽い重力で弧を描く
		it->life--;
		if (it->life <= 0) it = mFireSparks.erase(it);
		else ++it;
	}
}

// 3. 地面の燃焼エリア（着弾点から左右へダイナミックに広がり、ダメージ判定が持続）の更新
void DragonFireGimmick::UpdateBurningGrounds(Player* player, int attackDamage)
{
	for (auto it = mBurningGrounds.begin(); it != mBurningGrounds.end(); )
	{
		it->timer--;
		it->age++;
		if (it->timer <= 0)
		{
			it = mBurningGrounds.erase(it);
			continue;
		}

		// 着弾直後、中心から外側へ向かって炎の波がカッコよく急速に広がる！
		if (it->currentHalfWidth < it->targetHalfWidth)
		{
			float expandSpeed = (it->targetHalfWidth - it->currentHalfWidth) * 0.16f + 3.5f;
			it->currentHalfWidth += expandSpeed;
			if (it->currentHalfWidth > it->targetHalfWidth)
			{
				it->currentHalfWidth = it->targetHalfWidth;
			}

			// 燃え広がる先端（左右の火先）から火花が弾ける
			if (it->age % 2 == 0)
			{
				for (int side = -1; side <= 1; side += 2)
				{
					FireSpark sp;
					sp.x = it->x + side * it->currentHalfWidth;
					sp.y = it->y - 6.0f;
					sp.vx = side * (3.0f + static_cast<float>(GetRand(40)) * 0.1f);
					sp.vy = -(2.5f + static_cast<float>(GetRand(45)) * 0.1f);
					sp.size = 3.0f + static_cast<float>(GetRand(3));
					sp.life = 14 + GetRand(10);
					sp.maxLife = sp.life;
					sp.color = GetColor(255, 180, 50);
					mFireSparks.push_back(sp);
				}
			}
		}

		// 燃えている地面のダメージ判定（現在広がっている炎の幅に連動）
		if (player && !player->GetIsDead())
		{
			float px = player->GetPosition().x;
			float py = player->GetPosition().y;
			float pLeft = px - 16.0f;
			float pRight = px + 16.0f;
			float pTop = py - 50.0f;
			float pBot = py + 50.0f;

			float fireLeft = it->x - it->currentHalfWidth + 10.0f;
			float fireRight = it->x + it->currentHalfWidth - 10.0f;
			float fireTop = it->y - 58.0f;
			float fireBot = it->y + 12.0f;

			if (pRight > fireLeft && pLeft < fireRight && pBot > fireTop && pTop < fireBot)
			{
				player->TakeDamage(attackDamage);
				player->SetDeathReason(DeathReason::DeathType::DragonFireBreath);
			}
		}

		++it;
	}
}

// 4. 中央へ吐き出された巨大火の玉の落下・周辺大爆発・アーチ状火炎波の生成
void DragonFireGimmick::UpdateCenterFireball(Player* player, BlockMap* blockMap, int attackDamage, float floorY)
{
	if (!mCenterFireball.active) return;

	if (!mCenterFireball.exploding)
	{
		// 口元から地面の中央（8900, 1903）へ向かって落下
		mCenterFireball.x += mCenterFireball.vx;
		mCenterFireball.y += mCenterFireball.vy;

		// 落下中に火の粉を撒き散らす
		for (int i = 0; i < 2; ++i)
		{
			FireSpark sp;
			sp.x = mCenterFireball.x + static_cast<float>(GetRand(60) - 30);
			sp.y = mCenterFireball.y - 20.0f;
			sp.vx = static_cast<float>(GetRand(60) - 30) * 0.1f;
			sp.vy = -static_cast<float>(20 + GetRand(40)) * 0.1f;
			sp.size = 4.0f + static_cast<float>(GetRand(4));
			sp.life = 14 + GetRand(10);
			sp.maxLife = sp.life;
			sp.color = (i == 0) ? GetColor(255, 220, 90) : GetColor(255, 80, 15);
			mFireSparks.push_back(sp);
		}

		// 落下中の巨大火の玉の直撃判定
		if (player && !player->GetIsDead())
		{
			float dx = player->GetPosition().x - mCenterFireball.x;
			float dy = player->GetPosition().y - mCenterFireball.y;
			if (dx * dx + dy * dy <= (mCenterFireball.radius + 24.0f) * (mCenterFireball.radius + 24.0f))
			{
				player->TakeDamage(attackDamage);
				player->SetDeathReason(DeathReason::DeathType::DragonExplosion);
			}
		}

		// 地面に着弾したら周辺を大爆発させ、左右にアーチ状で地面を沿う炎を放つ！
		if (mCenterFireball.y >= floorY - 10.0f)
		{
			mCenterFireball.y = floorY;
			mCenterFireball.exploding = true;
			mCenterFireball.explosionTimer = mCenterFireball.maxExplosionTimer;
			mCenterFireball.explosionRadius = 40.0f;

			// 爆発の火花を四方八方に吹き飛ばす
			for (int s = 0; s < 40; ++s)
			{
				float ang = -(GetRand(180) * 3.14159265f) / 180.0f; // 上半球へ爆散
				float spd = 6.0f + static_cast<float>(GetRand(140)) * 0.1f;
				FireSpark sp;
				sp.x = mCenterFireball.x + static_cast<float>(GetRand(40) - 20);
				sp.y = floorY - 8.0f;
				sp.vx = std::cos(ang) * spd;
				sp.vy = std::sin(ang) * spd;
				sp.size = 4.0f + static_cast<float>(GetRand(6));
				sp.life = 22 + GetRand(20);
				sp.maxLife = sp.life;
				sp.color = (s % 3 == 0) ? GetColor(255, 255, 170) : ((s % 2 == 0) ? GetColor(255, 160, 30) : GetColor(255, 50, 10));
				mFireSparks.push_back(sp);
			}

			// 地面を沿って低いアーチ状で左右（-1.0f と +1.0f）に広がる炎の波を生成！
			for (int side = -1; side <= 1; side += 2)
			{
				ArchFlameWave wave;
				wave.startX = mCenterFireball.x;
				wave.x = mCenterFireball.x + side * 40.0f;
				wave.y = floorY;
				wave.dir = static_cast<float>(side);
				wave.speed = 9.0f;
				wave.archWidth = 185.0f;  // 1アーチの横幅
				wave.archHeight = 68.0f;  // 低めのアーチ高さ（地上プレイヤーにしっかり当たり、ジャンプで飛び越えられる高さ）
				wave.tick = 0;
				wave.active = true;
				mArchFlameWaves.push_back(wave);
			}
		}
	}
	else
	{
		// 中央大爆発の膨張＆画面揺れ＆周辺ダメージ判定
		mCenterFireball.explosionTimer--;
		float progress = 1.0f - (static_cast<float>(mCenterFireball.explosionTimer) / static_cast<float>(mCenterFireball.maxExplosionTimer));
		mCenterFireball.explosionRadius = 50.0f + progress * 200.0f; // 最大半径 250px の大爆発ドーム


		// 2波目のアーチ炎を少し遅れて放ち、連続アーチの迫力を出す
		if (mCenterFireball.explosionTimer == mCenterFireball.maxExplosionTimer - 14)
		{
			for (int side = -1; side <= 1; side += 2)
			{
				ArchFlameWave wave2;
				wave2.startX = mCenterFireball.x;
				wave2.x = mCenterFireball.x + side * 20.0f;
				wave2.y = floorY;
				wave2.dir = static_cast<float>(side);
				wave2.speed = 9.0f;
				wave2.archWidth = 185.0f;
				wave2.archHeight = 52.0f;
				wave2.tick = 0;
				wave2.active = true;
				mArchFlameWaves.push_back(wave2);
			}
		}

		// 大爆発の当たり判定（プレイヤーの全身矩形との重なり判定）
		if (mCenterFireball.explosionTimer > 6 && player && !player->GetIsDead())
		{
			float px = player->GetPosition().x;
			float py = player->GetPosition().y;
			float hitR = mCenterFireball.explosionRadius * 0.85f;
			if (px + 16.0f > mCenterFireball.x - hitR && px - 16.0f < mCenterFireball.x + hitR &&
				py + 50.0f > (floorY - hitR) && py - 50.0f < floorY + 20.0f)
			{
				player->TakeDamage(attackDamage);
				player->SetDeathReason(DeathReason::DeathType::DragonExplosion);
			}
		}

		if (mCenterFireball.explosionTimer <= 0)
		{
			mCenterFireball.active = false;
			mCenterFireball.exploding = false;
		}
	}
}

// 5. 地面を沿ってアーチ状で左右に広がる炎の波（ArchFlameWave）の更新
void DragonFireGimmick::UpdateArchFlameWaves(Player* player, int attackDamage, float floorY)
{
	for (auto it = mArchFlameWaves.begin(); it != mArchFlameWaves.end(); )
	{
		if (!it->active)
		{
			it = mArchFlameWaves.erase(it);
			continue;
		}

		it->tick++;
		it->x += it->dir * it->speed;

		// スタート地点からの距離に応じて、地面（floorY）を跳ねる放物線アーチ軌道を計算
		float dist = std::fabs(it->x - it->startX);
		float phase = std::fmod(dist, it->archWidth) / it->archWidth; // 0.0 ~ 1.0
		float archSin = std::sin(phase * 3.14159265f);
		it->y = floorY - archSin * it->archHeight;

		// アーチが地面に接地する瞬間（バウンド点）で火花を散らす
		if (phase < 0.08f || phase > 0.92f)
		{
			FireSpark sp;
			sp.x = it->x;
			sp.y = floorY - 4.0f;
			sp.vx = it->dir * (2.0f + static_cast<float>(GetRand(40)) * 0.1f);
			sp.vy = -(3.0f + static_cast<float>(GetRand(50)) * 0.1f);
			sp.size = 3.0f + static_cast<float>(GetRand(3));
			sp.life = 14 + GetRand(8);
			sp.maxLife = sp.life;
			sp.color = GetColor(255, 200, 70);
			mFireSparks.push_back(sp);
		}

		// プレイヤーとの当たり判定（描画されているアーチ炎の全ノードとプレイヤーの全身矩形[幅32×高さ100]で確実に判定）
		if (player && !player->GetIsDead())
		{
			float px = player->GetPosition().x;
			float py = player->GetPosition().y;
			float pLeft = px - 16.0f;
			float pRight = px + 16.0f;
			float pTop = py - 50.0f;
			float pBot = py + 50.0f;

			// 描画と同じ12ノード分のアーチ帯すべてに当たり判定を持たせる
			for (int k = 0; k < 12; ++k)
			{
				float sampleDist = dist - k * 15.0f;
				if (sampleDist < 0.0f) continue;
				float sampleX = it->startX + it->dir * sampleDist;
				float samplePhase = std::fmod(sampleDist, it->archWidth) / it->archWidth;
				float sampleY = floorY - std::sin(samplePhase * 3.14159265f) * it->archHeight;

				float hitR = (k <= 3) ? 28.0f : 20.0f;
				if (sampleX + hitR > pLeft && sampleX - hitR < pRight &&
					sampleY + hitR > pTop && sampleY - hitR < pBot)
				{
					player->TakeDamage(attackDamage);
					player->SetDeathReason(DeathReason::DeathType::DragonArchFlame);

					break;
				}
			}
		}

		// ボス部屋の左右の壁に到達したら消滅
		if (it->x < 7930.0f || it->x > 9860.0f)
		{
			it = mArchFlameWaves.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void DragonFireGimmick::Draw(BlockMap* blockMap, bool isAirBreathState, bool isCenterChargeState, int animFrame, float mouthWX, float mouthWY) const
{
	if (blockMap == nullptr) return;

	int now = GetNowCount();
	const float floorY = 1903.0f;

	// 1. 空中火炎ブレスの口元チャージ＆咆哮マズルフラッシュ
	if (isAirBreathState)
	{
		DrawAirBreathMuzzle(blockMap, animFrame, mouthWX, mouthWY, now);
	}

	SetDrawBlendMode(DX_BLENDMODE_ADD, 235);

	// 2. 着弾地点から左右へカッコよく燃え広がる紅蓮の地面
	DrawBurningGrounds(blockMap, now);

	// 3. 空中から地面へ突き刺さる豪快な火炎ブレスストリームの描画
	DrawFireProjectiles(blockMap);

	// 4. 中央巨大火の玉（チャージ・落下・周辺大爆発ドーム）の描画
	DrawCenterFireballAndCharge(blockMap, isCenterChargeState, animFrame, mouthWX, mouthWY, now);

	// 5. 地面を沿ってアーチ状で左右に広がる炎の波（ArchFlameWave）の描画
	DrawArchFlameWaves(blockMap, floorY, now);

	// 6. 飛び散る火花スパーク（FireSpark）の描画
	DrawFireSparks(blockMap);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void DragonFireGimmick::DrawAirBreathMuzzle(BlockMap* blockMap, int animFrame, float mouthWX, float mouthWY, int now) const
{
	int mouthSX = static_cast<int>(ToScreenX(mouthWX, blockMap));
	int mouthSY = static_cast<int>(ToScreenY(mouthWY, blockMap));


	SetDrawBlendMode(DX_BLENDMODE_ADD, 230);
	if (animFrame < 3)
	{
		// 息を吸い込んで口元に灼熱の収束リングと火球が集まる予兆
		int r = 14 + animFrame * 9 + (now / 30) % 6;
		int ringR = 54 - animFrame * 14;
		DrawCircle(mouthSX, mouthSY, ringR, GetColor(255, 90, 20), FALSE);
		DrawCircle(mouthSX, mouthSY, r, GetColor(255, 65, 10), TRUE);
		DrawCircle(mouthSX, mouthSY, static_cast<int>(r * 0.65f), GetColor(255, 175, 40), TRUE);
		DrawCircle(mouthSX, mouthSY, static_cast<int>(r * 0.35f), GetColor(255, 255, 200), TRUE);
	}
	else if (animFrame >= 3 && animFrame <= 5)
	{
		// 火を噴いている口元の爆発的な火炎マズルフラッシュ
		int r = 42 + (now / 20) % 14;
		DrawCircle(mouthSX, mouthSY, r + 14, GetColor(180, 25, 5), TRUE);
		DrawCircle(mouthSX, mouthSY, r, GetColor(255, 65, 10), TRUE);
		DrawCircle(mouthSX, mouthSY, static_cast<int>(r * 0.72f), GetColor(255, 170, 30), TRUE);
		DrawCircle(mouthSX, mouthSY, static_cast<int>(r * 0.45f), GetColor(255, 255, 200), TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void DragonFireGimmick::DrawBurningGrounds(BlockMap* blockMap, int now) const
{
	for (const auto& bg : mBurningGrounds)
	{
		int cx = static_cast<int>(ToScreenX(bg.x, blockMap));
		int cy = static_cast<int>(ToScreenY(bg.y, blockMap));
		int hw = static_cast<int>(bg.currentHalfWidth);
		if (hw < 6) hw = 6;

		// 残り1秒（60フレーム未満）で徐々に炎が小さくなって消えていく
		float fadeScale = (bg.timer < 60) ? (static_cast<float>(bg.timer) / 60.0f) : 1.0f;
		if (fadeScale < 0.12f) fadeScale = 0.12f;

		// (A) 地面を焦がすマグマ状の灼熱グロウ（床面が真っ赤～黄金色に発光）
		DrawOval(cx, cy, hw + 22, static_cast<int>(18 * fadeScale) + 4, GetColor(160, 20, 5), TRUE);
		DrawOval(cx, cy - 1, hw + 8, static_cast<int>(12 * fadeScale) + 3, GetColor(255, 70, 10), TRUE);
		DrawOval(cx, cy - 2, static_cast<int>(hw * 0.85f), static_cast<int>(7 * fadeScale) + 2, GetColor(255, 190, 50), TRUE);

		// (B) 着弾直後に左右へ広がる衝撃波リング演出
		if (bg.age < 24)
		{
			int waveH = static_cast<int>((24 - bg.age) * 1.8f);
			DrawOval(cx, cy - 4, hw + 12, waveH, GetColor(255, 140, 30), FALSE);
		}

		// (C) 中心から外側へ波打つように立ち昇る多層構造の紅蓮の火柱
		int numFlames = (hw / 9) + 5;
		if (numFlames > 38) numFlames = 38;
		for (int i = 0; i < numFlames; ++i)
		{
			float ratio = (numFlames > 1)
				? (static_cast<float>(i) / static_cast<float>(numFlames - 1) * 2.0f - 1.0f) // -1.0 ~ +1.0
				: 0.0f;

			int fx = cx + static_cast<int>(ratio * hw);

			// 中心ほど高く、外側に向かってうねりながら広がるシルエット＋進行波アニメーション
			float distFactor = 1.0f - std::fabs(ratio) * 0.45f;
			float wave1 = std::sin(now * 0.026f - std::fabs(ratio) * 5.0f + i * 1.3f);
			float wave2 = std::cos(now * 0.034f + i * 2.1f);
			// 両端（火先）がめくれ上がるような迫力をつける
			float edgeFlare = (std::fabs(ratio) > 0.78f) ? 1.28f : 1.0f;

			int flameH = static_cast<int>((68.0f * distFactor * edgeFlare + 24.0f * wave1) * fadeScale);
			if (flameH < 12) flameH = 12;
			int flameW = static_cast<int>((22.0f * distFactor + 6.0f) * fadeScale) + 4;

			// 炎の先端が風と熱気と広がり方向へしなやかに揺れる
			int outwardLean = static_cast<int>(ratio * 14.0f);
			int tipX = fx + outwardLean + static_cast<int>(wave2 * 10.0f);
			int midX = fx + outwardLean / 2 + static_cast<int>(wave1 * 5.0f);

			// 1層目：深紅の外焔
			DrawTriangle(
				fx - flameW, cy + 2,
				fx + flameW, cy + 2,
				tipX, cy - flameH,
				GetColor(220, 30, 5), TRUE
			);
			// 2層目：鮮やかな紅蓮オレンジの中焔
			DrawTriangle(
				fx - (flameW * 3) / 4, cy + 1,
				fx + (flameW * 3) / 4, cy + 1,
				tipX, cy - (flameH * 4) / 5,
				GetColor(255, 95, 15), TRUE
			);
			// 3層目：黄金色の内焔
			DrawTriangle(
				fx - flameW / 2, cy,
				fx + flameW / 2, cy,
				midX, cy - (flameH * 3) / 5,
				GetColor(255, 185, 35), TRUE
			);
			// 4層目：白熱するコア焔
			DrawTriangle(
				fx - flameW / 4, cy,
				fx + flameW / 4, cy,
				midX, cy - (flameH * 2) / 5,
				GetColor(255, 250, 180), TRUE
			);
		}

		// (D) 燃焼エリアから渦を巻いて舞い上がる無数の火の粉
		int numEmbers = (hw / 18) + 4;
		for (int p = 0; p < numEmbers; ++p)
		{
			int seed = static_cast<int>(bg.x) + p * 131;
			int cycle = (now / 9 + seed) % 60;
			float normX = ((seed * 37) % 200) / 100.0f - 1.0f; // -1.0 ~ +1.0
			int px = cx + static_cast<int>(normX * hw) + static_cast<int>(std::sin(cycle * 0.22f + p) * 12.0f);
			int py = cy - static_cast<int>(cycle * 2.1f * fadeScale) - 8;
			int pr = (cycle < 25) ? 4 : ((cycle < 45) ? 3 : 2);
			unsigned int col = (cycle < 20) ? GetColor(255, 240, 150) : GetColor(255, 120, 20);
			DrawCircle(px, py, pr, col, TRUE);
		}
	}
}

void DragonFireGimmick::DrawFireProjectiles(BlockMap* blockMap) const
{
	for (const auto& fp : mFireProjectiles)
	{
		if (!fp.active) continue;

		// 渦巻くような揺らぎを加えて流体のような火炎放射に見せる
		float swirl = std::sin(fp.animTick * 0.45f + fp.swaySeed) * (fp.radius * 0.25f);
		float perpX = -fp.vy;
		float perpY = fp.vx;
		float pLen = std::sqrt(perpX * perpX + perpY * perpY);
		if (pLen > 0.001f) { perpX /= pLen; perpY /= pLen; }

		int sx = static_cast<int>(ToScreenX(fp.x + perpX * swirl, blockMap));
		int sy = static_cast<int>(ToScreenY(fp.y + perpY * swirl, blockMap));
		int r = static_cast<int>(fp.radius);

		// 進行方向逆側へたなびく火炎の尾
		int tailX = static_cast<int>(ToScreenX(fp.x - fp.vx * 1.4f, blockMap));
		int tailY = static_cast<int>(ToScreenY(fp.y - fp.vy * 1.4f, blockMap));

		// 外側の灼熱オーラ（深紅）
		DrawCircle(tailX, tailY, static_cast<int>(r * 0.85f), GetColor(180, 25, 5), TRUE);
		DrawCircle(sx, sy, r + 6, GetColor(180, 25, 5), TRUE);
		// 中間の紅蓮炎（オレンジ）
		DrawCircle(sx, sy, r, GetColor(255, 75, 10), TRUE);
		// 内側の黄金炎（イエロー）
		DrawCircle(sx, sy, static_cast<int>(r * 0.68f), GetColor(255, 180, 35), TRUE);
		// 中心の白熱コア
		DrawCircle(sx, sy, static_cast<int>(r * 0.36f), GetColor(255, 255, 200), TRUE);
	}
}

void DragonFireGimmick::DrawCenterFireballAndCharge(BlockMap* blockMap, bool isCenterChargeState, int animFrame, float mouthWX, float mouthWY, int now) const
{
	if (isCenterChargeState && animFrame < 3)
	{
		// 口元で膨れ上がる巨大火の玉チャージ演出（スプライトの各フレームの口元座標に完全一致）
		int mouthSX = static_cast<int>(ToScreenX(mouthWX, blockMap));
		int mouthSY = static_cast<int>(ToScreenY(mouthWY, blockMap));
		int chargeR = 18 + animFrame * 16 + (now / 25) % 8;
		DrawCircle(mouthSX, mouthSY, chargeR + 12, GetColor(200, 30, 5), TRUE);
		DrawCircle(mouthSX, mouthSY, chargeR, GetColor(255, 85, 15), TRUE);
		DrawCircle(mouthSX, mouthSY, static_cast<int>(chargeR * 0.68f), GetColor(255, 195, 45), TRUE);
		DrawCircle(mouthSX, mouthSY, static_cast<int>(chargeR * 0.38f), GetColor(255, 255, 210), TRUE);
	}

	if (mCenterFireball.active)
	{
		int cx = static_cast<int>(ToScreenX(mCenterFireball.x, blockMap));
		int cy = static_cast<int>(ToScreenY(mCenterFireball.y, blockMap));

		if (!mCenterFireball.exploding)
		{
			// 落下中の巨大火球（太陽のようなコロナ＋尾）
			int r = static_cast<int>(mCenterFireball.radius) + (now / 20) % 8;
			DrawOval(cx, cy - 35, static_cast<int>(r * 0.75f), r + 30, GetColor(200, 30, 5), TRUE);
			DrawCircle(cx, cy, r + 14, GetColor(190, 25, 5), TRUE);
			DrawCircle(cx, cy, r, GetColor(255, 75, 10), TRUE);
			DrawCircle(cx, cy, static_cast<int>(r * 0.72f), GetColor(255, 185, 35), TRUE);
			DrawCircle(cx, cy, static_cast<int>(r * 0.42f), GetColor(255, 255, 210), TRUE);
		}
		else
		{
			// 中央着弾時の大爆発ドーム＆衝撃波リング！
			float alphaRatio = static_cast<float>(mCenterFireball.explosionTimer) / static_cast<float>(mCenterFireball.maxExplosionTimer);
			int expAlpha = static_cast<int>(255 * alphaRatio);
			SetDrawBlendMode(DX_BLENDMODE_ADD, expAlpha);

			int er = static_cast<int>(mCenterFireball.explosionRadius);
			// 外側の衝撃波リング
			for (int t = 0; t < 4; ++t)
			{
				DrawCircle(cx, cy - 20, er + 18 - t * 2, GetColor(255, 120, 30), FALSE);
			}
			// 地面に沿って広がる爆風フレア
			DrawOval(cx, cy - 6, er + 55, 32, GetColor(255, 70, 15), TRUE);
			// 巨大爆発球（多層グラデーション）
			DrawCircle(cx, cy - 25, er, GetColor(220, 35, 5), TRUE);
			DrawCircle(cx, cy - 25, static_cast<int>(er * 0.78f), GetColor(255, 105, 20), TRUE);
			DrawCircle(cx, cy - 25, static_cast<int>(er * 0.55f), GetColor(255, 205, 60), TRUE);
			DrawCircle(cx, cy - 25, static_cast<int>(er * 0.32f), GetColor(255, 255, 220), TRUE);

			SetDrawBlendMode(DX_BLENDMODE_ADD, 235);
		}
	}
}

void DragonFireGimmick::DrawArchFlameWaves(BlockMap* blockMap, float floorY, int now) const
{
	for (const auto& wave : mArchFlameWaves)
	{
		if (!wave.active) continue;

		float headDist = std::fabs(wave.x - wave.startX);

		// 先端から後方へ向かってアーチ軌道上に16個の炎ノードを連ね、美しい「炎のアーチ」を描く！
		const int numNodes = 16;
		for (int i = numNodes - 1; i >= 0; --i)
		{
			float nodeDist = headDist - i * 15.0f;
			if (nodeDist < 0.0f) continue;

			float wx = wave.startX + wave.dir * nodeDist;
			float phase = std::fmod(nodeDist, wave.archWidth) / wave.archWidth;
			float wy = floorY - std::sin(phase * 3.14159265f) * wave.archHeight;

			int sx = static_cast<int>(ToScreenX(wx, blockMap));
			int sy = static_cast<int>(ToScreenY(wy, blockMap));

			float ratio = 1.0f - (static_cast<float>(i) / static_cast<float>(numNodes)); // 先端1.0 -> 尾0.0
			int r = static_cast<int>(10.0f + 26.0f * ratio) + ((now / 25 + i) % 4);

			// 外焔（深紅～紅蓮オレンジ）
			DrawCircle(sx, sy, r + 4, GetColor(190, 25, 5), TRUE);
			DrawCircle(sx, sy, r, GetColor(255, 80, 15), TRUE);
			// 内焔（黄金イエロー）
			DrawCircle(sx, sy, static_cast<int>(r * 0.65f), GetColor(255, 190, 40), TRUE);
			// 先端付近は白熱コアと上向きの炎の舌を追加
			if (i <= 4)
			{
				DrawCircle(sx, sy, static_cast<int>(r * 0.38f), GetColor(255, 255, 210), TRUE);
			}
			// 各ノードから立ち昇る小さな炎の先端
			int tongueH = static_cast<int>(r * 1.35f);
			int swayX = static_cast<int>(std::sin(now * 0.03f + i) * 6.0f);
			DrawTriangle(
				sx - r / 2, sy,
				sx + r / 2, sy,
				sx + swayX, sy - tongueH,
				GetColor(255, 130, 20), TRUE
			);
		}
	}
}

void DragonFireGimmick::DrawFireSparks(BlockMap* blockMap) const
{
	for (const auto& sp : mFireSparks)
	{
		int sx = static_cast<int>(ToScreenX(sp.x, blockMap));
		int sy = static_cast<int>(ToScreenY(sp.y, blockMap));
		int sr = static_cast<int>(sp.size * (static_cast<float>(sp.life) / static_cast<float>(sp.maxLife)));
		if (sr < 1) sr = 1;
		DrawCircle(sx, sy, sr, sp.color, TRUE);
	}
}

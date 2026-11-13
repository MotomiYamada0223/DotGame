#include "Minecart.h"
#include "BlockMap.h"
#include "Player.h"
#include "GameConstants.h"
#include <algorithm>
#include <cmath>
#include "Master.h"


namespace
{
	void PlayMinecartSE(SoundManager::SOUND_SE se)
	{
		if (Master::mpGameManager && Master::mpGameManager->GetSoundManager())
		{
			Master::mpGameManager->GetSoundManager()->PlaySE(se);
		}
	}

	void StopMinecartSE(SoundManager::SOUND_SE se)
	{
		if (Master::mpGameManager && Master::mpGameManager->GetSoundManager())
		{
			Master::mpGameManager->GetSoundManager()->StopSE(se);
		}
	}
}


Minecart::Minecart()
{
}

Minecart::~Minecart()
{
	if (mGraphHandle != -1)
	{
		DeleteGraph(mGraphHandle);
		mGraphHandle = -1;
	}
	if (mRampGraphHandle != -1)
	{
		DeleteGraph(mRampGraphHandle);
		mRampGraphHandle = -1;
	}
}

// 初期化
void Minecart::Initialize()
{
	if (mGraphHandle == -1)
	{
		mGraphHandle = LoadGraph("Resource/Image/Cave_Minecart.png");
	}
	if (mRampGraphHandle == -1)
	{
		mRampGraphHandle = LoadGraph(CharacterGraphPath::CaveRamp.c_str());
	}
	Reset();
}

// リセット（初期位置・待機状態へ）
void Minecart::Reset()
{
	mState = State::Waiting;
	mX = 5840.0f; // プレイヤー初期位置付近のX座標
	mY = 1871.0f; // レール上のY座標
	mVx = 0.0f;
	mVy = 0.0f;
	mAngle = 0.0f;
	mbActive = true;
	mbSmashedTriangle = false;
	mbHasJumped = false;
	mBobTimer = 0;
	mParticles.clear();

	StopMinecartSE(SoundManager::SE_STAGE2_MINECART_START);
	StopMinecartSE(SoundManager::SE_STAGE2_MINECART_DASH);
}

// 三角台の当たり判定（コリジョン）を消去して破壊
void Minecart::SmashTriangleCollision(BlockMap& blockMap)
{
	using namespace MinecartConstants;
	// X=5350からX=5440まで、Y=1800〜1895の空間をすべて確実に消去して通り抜け可能にする
	blockMap.SetCollisionBlock(RampClearStartX, RampTopY, RampClearWidth, RampHeight, BlockMap::CollisionType::None);
}

// 三角台の当たり判定（コリジョン）を復元
void Minecart::RestoreTriangleCollision(BlockMap& blockMap)
{
	using namespace MinecartConstants;
	for (int x = RampStartX; x <= RampEndX; ++x)
	{
		int topY = RampTopY + static_cast<int>((x - RampStartX) * 1.2f);
		for (int y = topY; y < RampFloorY; ++y)
		{
			blockMap.SetCollisionBlock(x, y, 1, 1, BlockMap::CollisionType::Block);
		}
	}
}

// レール走行時の火花パーティクル
void Minecart::SpawnWheelSparks(float x, float y, bool facingLeft)
{
	CartParticle p;
	p.x = x + (GetRand(8) - 4);
	p.y = y + (GetRand(4) - 2);
	p.vx = facingLeft ? ((float)GetRand(20) / 10.0f + 0.5f) : -((float)GetRand(20) / 10.0f + 0.5f);
	p.vy = -((float)GetRand(15) / 10.0f + 0.2f);
	p.size = GetRand(4) + 2;
	p.life = GetRand(14) + 8;
	p.maxLife = p.life;
	p.color = (GetRand(2) == 0) ? GetColor(255, 230, 90) : GetColor(180, 170, 160);
	mParticles.push_back(p);
}

// ブースト時のロケット・ジェット噴射炎
void Minecart::SpawnBoostFlames(float x, float y, bool facingLeft)
{
	for (int i = 0; i < 3; i++)
	{
		CartParticle p;
		p.x = x + (GetRand(8) - 4);
		p.y = y + (GetRand(16) - 8);
		// 進行方向と逆向きに噴出
		p.vx = facingLeft ? ((float)GetRand(40) / 10.0f + 6.0f) : -((float)GetRand(40) / 10.0f + 6.0f);
		p.vy = ((float)GetRand(20) - 10) / 10.0f;
		p.size = GetRand(8) + 4;
		p.life = GetRand(16) + 10;
		p.maxLife = p.life;
		int r = 255;
		int g = GetRand(120) + 60;
		int b = GetRand(30);
		p.color = GetColor(r, g, b);
		mParticles.push_back(p);
	}
}

// 三角台破壊時の飛び散る岩片・煙パーティクル
void Minecart::SpawnRockDebris(float x, float y, int count)
{
	for (int i = 0; i < count; i++)
	{
		CartParticle p;
		p.x = x + (GetRand(40) - 20);
		p.y = y + (GetRand(40) - 20);
		p.vx = ((float)GetRand(50) - 30.0f) * 0.9f;
		p.vy = ((float)GetRand(40) - 25.0f) * 0.9f;
		p.size = GetRand(12) + 4;
		p.life = GetRand(35) + 20;
		p.maxLife = p.life;
		int c = GetRand(40) + 90;
		p.color = GetColor(c + 15, c, c - 15);
		mParticles.push_back(p);
	}
}

// 毎フレーム更新処理
void Minecart::Update(BlockMap& blockMap, Player* player)
{
	if (Master::gCurrentStageIndex != 1)
	{
		return;
	}

	// プレイヤーが死亡して初期位置に戻った時の自動リセット＆三角台復元
	if (player && player->GetPosition().x <= MinecartConstants::PlayerRespawnThresholdX && mState != State::Waiting)
	{
		Reset();
		RestoreTriangleCollision(blockMap);
	}

	if (!mbActive || mState == State::Finished) return;

	// パーティクルの更新
	for (auto it = mParticles.begin(); it != mParticles.end(); )
	{
		it->x += it->vx;
		it->y += it->vy;
		it->vy += 0.25f;
		it->life--;
		if (it->life <= 0) it = mParticles.erase(it);
		else ++it;
	}

	switch (mState)
	{
	case State::Waiting:
		// プレイヤーが線路手前（X >= 5180）まで進んだらトロッコ発進！
		if (player && !player->GetIsDead())
		{
			if (player->GetPosition().x >= 5180.0f)
			{
				mState = State::RollingLeft;
				mVx = -2.5f; // ゆっくり線路へ向かう
				PlayMinecartSE(SoundManager::SE_STAGE2_MINECART_START);
				SpawnWheelSparks(mX - 18.0f, mY + 20.0f, true);
				SpawnWheelSparks(mX + 18.0f, mY + 20.0f, true);
			}
		}
		break;

	case State::RollingLeft:
		mX += mVx;
		mBobTimer++;

		// 走行時のガタゴト火花
		if (GetRand(3) == 0)
		{
			SpawnWheelSparks(mX + 18.0f, mY + 20.0f, true);
		}

		// レールの赤色部分（X=5719～5746）に接触した瞬間、超加速発動！
		if (mX <= 5530.0f)
		{
			mState = State::SuperBoostLeft;
			StopMinecartSE(SoundManager::SE_STAGE2_MINECART_START);
			mVx = -26.0f; // 猛スピード！
			SpawnBoostFlames(mX + 35.0f, mY, true);
		}
		break;

	case State::SuperBoostLeft:
		mX += mVx;

		// 後方（右側）からロケット噴射
		SpawnBoostFlames(mX + 35.0f, mY, true);

		// 三角台（X=5359～5440）に到達したら、豪快にぶち壊して粉砕！
		if (mX <= 5440.0f && !mbSmashedTriangle)
		{
			mbSmashedTriangle = true;
			SmashTriangleCollision(blockMap);
			SpawnRockDebris(5400.0f, 1850.0f, 60);
			blockMap.StartCameraShake(MinecartConstants::SmashShakePower, 1.0f, 1.0f, MinecartConstants::SmashShakeFrames);
		}

		// 三角台通過中（X=5440～5330）も破壊の土煙・岩片・火花を連続発生
		if (mX <= 5440.0f && mX >= 5330.0f)
		{
			SpawnRockDebris(mX, 1860.0f, 4);
			SpawnWheelSparks(mX, 1871.0f, true);
		}

		// 水平高度（Y=1871）を維持したままロケット直進（落下しない）
		mY = 1871.0f;

		// プレイヤー即死判定
		if (player && !player->GetIsDead())
		{
			VECTOR pPos = player->GetPosition();
			float pLeft = pPos.x - 18.0f;
			float pRight = pPos.x + 18.0f;
			float pTop = pPos.y - 50.0f;
			float pBottom = pPos.y + 50.0f;

			float cLeft = mX - 25.0f;
			float cRight = mX + 25.0f;
			float cTop = mY - 15.0f;
			float cBottom = mY + 15.0f;

			if (pRight >= cLeft && pLeft <= cRight && pBottom >= cTop && pTop <= cBottom)
			{
				player->TakeDamage(9999);
				player->SetDeathReason(DeathReason::DeathType::Minecart);
				SpawnRockDebris(pPos.x, pPos.y, 40);
			}
		}

		// 画面左外（X < 4400）へ突き抜けたら、画面外潜伏へ移行！
		if (mX < 4400.0f)
		{
			mState = State::LurkingLeft;
			mVx = 0.0f;
		}
		break;

	case State::LurkingLeft:
		// プレイヤーがトロッコ初期位置（X >= 6000）付近まで来たら、背後から猛スピードで戻ってくる！
		if (player && !player->GetIsDead())
		{
			if (player->GetPosition().x >= 5780.0f)
			{
				mState = State::ReturningRight;
				// プレイヤーの画面外左（約950px手前）から超スピードで突入
				mX = player->GetPosition().x - 950.0f;
				mY = 1871.0f;
				mVx = 26.0f; // 超スピードで右へ直進突進！
				mVy = 0.0f;
				mAngle = 0.0f;
				mbHasJumped = false;
				SpawnBoostFlames(mX - 35.0f, mY, false);
			}
		}
		break;

	case State::ReturningRight:
		mX += mVx;

		// ユーザー要望：
		// 「戻ってくるトロッコは直線じゃなくてプレイヤーが近くのX座標まで来た時ジャンプしてほしい。
		//   これでそのまま歩いていれば勝手に避けられるのにビビってジャンプしたらトロッコもジャンプしてきて死ぬっていうことができる。」
		if (player && !player->GetIsDead())
		{
			VECTOR pPos = player->GetPosition();

			// トロッコが背後約230pxに迫った時、大ジャンプ発動！
			if (!mbHasJumped && mX >= pPos.x - 230.0f)
			{
				mbHasJumped = true;
				mVy = -19.0f; // 空中へ跳ね上がる初速
				mAngle = -0.22f; // やや上向きに機首上げ
				SpawnWheelSparks(mX - 10.0f, mY + 20.0f, false);
				SpawnWheelSparks(mX + 10.0f, mY + 20.0f, false);
			}
		}

		if (mbHasJumped)
		{
			mY += mVy;
			mVy += 0.8f; // 重力加速度

			// 空中での機首角度の自然な変化（上昇中は上向き、下降中は下向き）
			mAngle = mVy * 0.018f;

			// 空中でも後方からブースト炎
			SpawnBoostFlames(mX - 35.0f, mY, false);

			// 着地判定（元のレール高さに戻った時）
			if (mY >= 1871.0f)
			{
				mY = 1871.0f;
				mVy = 0.0f;
				mAngle = 0.0f;
				SpawnWheelSparks(mX - 18.0f, mY + 20.0f, false);
				SpawnWheelSparks(mX + 18.0f, mY + 20.0f, false);
			}
		}
		else
		{
			mY = 1871.0f;
			mAngle = 0.0f;
			SpawnBoostFlames(mX - 35.0f, mY, false);
		}

		// 当たり判定：
		// そのまま歩いていれば頭上を飛び越えるため絶対に当たらない
		// ビビってジャンプした場合は、空中のトロッコに直撃して即死！
		if (player && !player->GetIsDead())
		{
			VECTOR pPos = player->GetPosition();
			float pLeft = pPos.x - 18.0f;
			float pRight = pPos.x + 18.0f;
			float pTop = pPos.y - 50.0f;
			float pBottom = pPos.y + 50.0f;

			float cLeft = mX - 35.0f;
			float cRight = mX + 35.0f;
			float cTop = mY - 22.0f;
			float cBottom = mY + 22.0f;

			bool xOverlap = (pRight >= cLeft && pLeft <= cRight);
			bool yOverlap = (pBottom >= cTop && pTop <= cBottom);

			bool hit = false;
			if (xOverlap)
			{
				if (yOverlap)
				{
					// 空中のトロッコ本体とプレイヤーが直接接触
					hit = true;
				}
				else if (mbHasJumped && (!player->GetIsGrounded() || player->GetIsJumping()))
				{
					// 地上にいない（ジャンプした）場合、空中のトロッコに巻き込まれて即死！
					hit = true;
				}
			}

			if (hit)
			{
				player->TakeDamage(9999);
				player->SetDeathReason(DeathReason::DeathType::Minecart);
				SpawnRockDebris(pPos.x, pPos.y, 40);
			}
		}

		// 画面右外（X > 7600）へ突き抜けたら終了
		if (mX > 7600.0f)
		{
			mState = State::Finished;
		}
		break;

	case State::Finished:
		break;
	}
}

// 描画処理
void Minecart::Draw(int scrollX, int scrollY)
{
	if (Master::gCurrentStageIndex != 1)
	{
		return;
	}

	using namespace MinecartConstants;

	// 三角台がまだ破壊されていない場合は、三角台スプライトを描画
	if (!mbSmashedTriangle)
	{
		if (mRampGraphHandle != -1)
		{
			int rx = RampSpriteX - scrollX;
			int ry = RampSpriteY - scrollY;
			DrawGraph(rx, ry, mRampGraphHandle, TRUE);
		}
	}
	else
	{
		// 三角台が破壊された場合は、地面に砕け散った瓦礫を描画
		// （CaveMap.png 側の三角台は透明化されているため、背後の洞窟背景が自然に見え、
		//   プレイヤーの上に背景が覆い被さることは一切ありません）
		int tx = RampClearStartX - scrollX;
		int ty = RampTopY - scrollY;
		int tw = 100;
		int th = RampHeight;
		int groundY = ty + th;

		DrawBox(tx + 14, groundY - 12, tx + tw - 5, groundY, GetColor(80, 75, 70), TRUE);
		DrawBox(tx + 24, groundY - 8, tx + tw - 20, groundY, GetColor(110, 105, 95), TRUE);
		DrawBox(tx + 34, groundY - 15, tx + 54, groundY - 2, GetColor(90, 85, 80), TRUE);
		DrawBox(tx + 69, groundY - 10, tx + 89, groundY - 1, GetColor(100, 95, 85), TRUE);
		DrawLine(tx + 19, groundY - 6, tx + 44, groundY - 2, GetColor(40, 35, 30));
		DrawLine(tx + 54, groundY - 8, tx + 79, groundY - 3, GetColor(40, 35, 30));
	}

	if (!mbActive || mState == State::Finished || mState == State::LurkingLeft) return;

	// パーティクルの描画
	for (const auto& p : mParticles)
	{
		int px = static_cast<int>(p.x) - scrollX;
		int py = static_cast<int>(p.y) - scrollY;
		DrawBox(px, py, px + p.size, py + p.size, p.color, TRUE);
	}

	int sx = static_cast<int>(mX) - scrollX;
	int sy = static_cast<int>(mY) - scrollY;

	// 走行中のガタゴト振動
	if (mState == State::RollingLeft)
	{
		if ((mBobTimer / 5) % 2 == 1)
		{
			sy -= 1;
		}
	}

	// 超加速時の残像エフェクト（スピード感を演出）
	if (mState == State::SuperBoostLeft)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
		DrawRotaGraph(sx + 35, sy, 1.0, 0.0, mGraphHandle, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 50);
		DrawRotaGraph(sx + 70, sy, 1.0, 0.0, mGraphHandle, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
	else if (mState == State::ReturningRight)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
		DrawRotaGraph(sx - 35, sy, 1.0, static_cast<double>(mAngle), mGraphHandle, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 50);
		DrawRotaGraph(sx - 70, sy, 1.0, static_cast<double>(mAngle), mGraphHandle, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	if (mGraphHandle != -1)
	{
		DrawRotaGraph(sx, sy, 1.0, static_cast<double>(mAngle), mGraphHandle, TRUE);
	}
}

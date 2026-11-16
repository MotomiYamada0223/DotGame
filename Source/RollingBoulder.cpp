#include "RollingBoulder.h"
#include "BlockMap.h"
#include "Player.h"
#include "GameConstants.h"
#include <algorithm>
#include <cmath>
#include "Master.h"

namespace
{
	void PlayBoulderSE(SoundManager::SOUND_SE se)
	{
		if (Master::mpGameManager && Master::mpGameManager->GetSoundManager())
		{
			Master::mpGameManager->GetSoundManager()->PlaySE(se);
		}
	}

	void StopBoulderSE(SoundManager::SOUND_SE se)
	{
		if (Master::mpGameManager && Master::mpGameManager->GetSoundManager())
		{
			Master::mpGameManager->GetSoundManager()->StopSE(se);
		}
	}


}
RollingBoulder::RollingBoulder()
{
}

RollingBoulder::~RollingBoulder()
{
	if (mGraphHandle != -1)
	{
		DeleteGraph(mGraphHandle);
		mGraphHandle = -1;
	}
}

// 初期化処理
void RollingBoulder::Initialize()
{
	if (mGraphHandle == -1)
	{
		mGraphHandle = LoadGraph("Resource/Enemy/Rock/Cave_Boulder.png");
	}
	Reset();
}

void RollingBoulder::Reset()
{
	mState = State::Waiting;
	mX = 3128.0f;       // 天井穴の中央（X=3250）
	mY = -200.0f;       // 画面外の上空
	mVx = 0.0f;
	mVy = 0.0f;
	mAngle = 0.0f;
	mbTriggered = false;
	mRollTimer = 0;
	mbActive = true;
	mDustList.clear();

	StopBoulderSE(SoundManager::SE_STAGE2_BOULDER_LAND);
	StopBoulderSE(SoundManager::SE_STAGE2_BOULDER_ROLL);
}

// 土煙・破片パーティクルを生成
void RollingBoulder::SpawnDust(float x, float y, int count, float speedMul)
{
	for (int i = 0; i < count; i++)
	{
		BoulderDust d;
		d.x = x + (GetRand(40) - 20);
		d.y = y + (GetRand(20) - 10);
		d.vx = (GetRand(24) - 12) * 0.5f * speedMul;
		d.vy = (GetRand(20) - 16) * 0.6f * speedMul;
		d.size = GetRand(10) + 5;
		d.life = GetRand(28) + 16;
		d.maxLife = d.life;
		int c = GetRand(35) + 85;
		d.color = GetColor(c + 15, c, c - 15);
		mDustList.push_back(d);
	}
}

// 毎フレームの更新処理
void RollingBoulder::Update(BlockMap& blockMap, Player* player)
{
	if (Master::gCurrentStageIndex != 1)
	{
		return;
	}

	if (mState == State::Finished) return;

	// 1. パーティクル（土煙・トゲ）の更新
	for (auto it = mDustList.begin(); it != mDustList.end(); )
	{
		it->x += it->vx;
		it->y += it->vy;
		it->vy += 0.35f;
		it->life--;
		if (it->life <= 0) it = mDustList.erase(it);
		else ++it;
	}

	// 3. 岩の状態ごとの挙動
	switch (mState)
	{
	case State::Waiting:
		if (player && player->GetPosition().x >= 3350.0f)
		{
			mState = State::Falling;
			mbTriggered = true;
			mVy = 3.0f;
		}
		break;

	case State::Falling:
		mVy += 0.8f;
		mY += mVy;
		mAngle += 0.02f;

		if (mY + mRadius >= 1029.0f)
		{
			mY = 1029.0f - mRadius;
			mVy = 0.0f;
			mVx = 4.5f;
			mState = State::Rolling;
			mRollTimer = 0;
			PlayBoulderSE(SoundManager::SE_STAGE2_BOULDER_LAND);

			SpawnDust(mX, mY + mRadius, 45, 2.5f);
		}
		break;

	case State::Rolling:
	{
		mRollTimer++;
		if (mRollTimer == 15)
		{
			PlayBoulderSE(SoundManager::SE_STAGE2_BOULDER_ROLL);
		}


		auto GetFloorAtX = [&](float probeX) -> int {
			int searchStart = static_cast<int>(mY + mRadius - 40.0f);
			for (int py = searchStart; py < searchStart + 350 && py < 2160; ++py)
			{
				if (blockMap.GetCollisionType(static_cast<int>(probeX), py) == BlockMap::CollisionType::Block)
				{
					return py;
				}
			}
			return -1;
			};

		int floorLeft = GetFloorAtX(mX - 55.0f);
		int floorRight = GetFloorAtX(mX + 55.0f);
		int foundFloor = -1;

		if (floorLeft != -1 && floorRight != -1)
		{
			foundFloor = (std::min)(floorLeft, floorRight);
		}
		else if (floorLeft != -1)
		{
			foundFloor = floorLeft;
		}
		else if (floorRight != -1)
		{
			foundFloor = floorRight;
		}

		if (foundFloor == -1 || mX >= 4750.0f)
		{
			mState = State::Pitfall;
			StopBoulderSE(SoundManager::SE_STAGE2_BOULDER_ROLL);
		}
		else
		{
			float targetY = static_cast<float>(foundFloor) - mRadius;

			if (targetY > mY)
			{
				mVy = (std::min)(16.0f, mVy + 0.7f);
				mY += (std::min)(mVy, targetY - mY);
				mVx = (std::min)(8.2f, mVx + 0.035f);
			}
			else
			{
				mY = targetY;
				mVy = 0.0f;
			}

			mX += mVx;
			mAngle += mVx / mRadius;

			if (GetRand(3) == 0)
			{
				SpawnDust(mX - mRadius * 0.6f, mY + mRadius - 8.0f, 3, 1.0f);
			}

			int scrollX = blockMap.GetScrollX();
			if (mX >= scrollX - 150 && mX <= scrollX + ScreenSize::ScrrenWidth + 150)
			{
			}
		}
	}
	break;

	case State::Pitfall:
		mVy += 0.8f;
		mX += mVx;
		mY += mVy;
		mAngle += mVx / mRadius;

		if (mY > 2300.0f)
		{
			mState = State::Finished; // 岩は終了するが、トゲ罠は継続動作する
		}
		break;

	case State::Finished:
		break;
	}

	// 4. 岩とプレイヤーの接触判定
	if (mState == State::Falling && player && !player->GetIsDead())
	{
		VECTOR pPos = player->GetPosition();
		float pW = 44.0f;
		float pH = 100.0f;
		float pLeft = pPos.x - pW * 0.5f;
		float pRight = pPos.x + pW * 0.5f;
		float pTop = pPos.y - pH * 0.5f;
		float pBottom = pPos.y + pH * 0.5f;

		float closestX = (std::max)(pLeft, (std::min)(mX, pRight));
		float closestY = (std::max)(pTop, (std::min)(mY, pBottom));
		float dx = mX - closestX;
		float dy = mY - closestY;
		float hitRadius = mRadius * 0.80f;

		if (dx * dx + dy * dy < hitRadius * hitRadius)
		{
			// 落下中の岩に直撃して押し潰されたら即死
			player->TakeDamage(9999);
			player->SetDeathReason(DeathReason::DeathType::RockPress);

			SpawnDust(pPos.x, pPos.y, 35, 2.0f);
		}
	}
}

void RollingBoulder::Draw(int scrollX, int scrollY)
{
	if (Master::gCurrentStageIndex != 1)
	{
		return;
	}


	if (mState == State::Waiting || mState == State::Finished) return;

	int screenX = static_cast<int>(mX) - scrollX;
	int screenY = static_cast<int>(mY) - scrollY;

	// 土煙パーティクルの描画
	for (const auto& d : mDustList)
	{
		int dx = static_cast<int>(d.x) - scrollX;
		int dy = static_cast<int>(d.y) - scrollY;
		DrawBox(dx, dy, dx + d.size, dy + d.size, d.color, TRUE);
	}

	// 巨大な岩を回転描画
	if (mGraphHandle != -1)
	{
		DrawRotaGraph(screenX, screenY, 1.0, static_cast<double>(mAngle), mGraphHandle, TRUE);
	}
	else
	{
		DrawCircle(screenX, screenY, static_cast<int>(mRadius), GetColor(110, 95, 80), TRUE);
		DrawCircle(screenX, screenY, static_cast<int>(mRadius), GetColor(50, 40, 30), FALSE);
	}
}

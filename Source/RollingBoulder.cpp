#include "RollingBoulder.h"
#include "BlockMap.h"
#include "Player.h"
#include "GameConstants.h"
#include <algorithm>
#include <cmath>
#include "Master.h"

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

// 位置と状態を初期状態に戻す
void RollingBoulder::Reset()
{
	mState = State::Waiting;
	mX = 3128.0f;       // 天井穴の中央（X=3250）
	mY = -200.0f;       // 画面外の上空
	mVx = 0.0f;
	mVy = 0.0f;
	mAngle = 0.0f;
	mbTriggered = false;
	mbActive = true;
	mDustList.clear();

	// 2マスの単一巨大トゲ罠のリセット
	mSpikeState = SpikeTrapState::Hidden;
	mSpikeHeight = 0.0f;
	mSpikeY = 2006.0f;
	mSpikeVy = 0.0f;
	mSpikeParticles.clear();
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

// トゲ刺突・発射時のパーティクルを生成
void RollingBoulder::SpawnSpikeParticles(float x, float y, int count)
{
	for (int i = 0; i < count; i++)
	{
		SpikeParticle sp;
		sp.x = x + (GetRand(30) - 15);
		sp.y = y + (GetRand(20) - 10);
		sp.vx = (GetRand(24) - 12) * 0.7f;
		sp.vy = -(GetRand(18) + 8) * 0.8f; // 上方へ噴き出す
		sp.size = GetRand(7) + 3;
		sp.life = GetRand(30) + 20;
		sp.maxLife = sp.life;
		if (GetRand(3) == 0)
		{
			sp.color = GetColor(255, 230, 150); // 火花
		}
		else
		{
			sp.color = GetColor(GetRand(50) + 200, 20, 30); // 血飛沫
		}
		mSpikeParticles.push_back(sp);
	}
}

// 毎フレームの更新処理
void RollingBoulder::Update(BlockMap& blockMap, Player* player)
{
	if (Master::gCurrentStageIndex != 1)
	{
		return;
	}

	// 岩もトゲも両方とも完全に終了している場合のみ処理を抜ける
	if (mState == State::Finished && mSpikeState == SpikeTrapState::Finished) return;

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

	for (auto it = mSpikeParticles.begin(); it != mSpikeParticles.end(); )
	{
		it->x += it->vx;
		it->y += it->vy;
		it->vy += 0.45f;
		it->life--;
		if (it->life <= 0) it = mSpikeParticles.erase(it);
		else ++it;
	}

	// 2. 2マスのくぼみ巨大トゲ罠の更新
	// 出現トリガー：
	// ① プレイヤーがくぼみに入った時
	// ② または、転がる岩がそのくぼみの座標まで来た時（X >= 4350）
	if (mSpikeState == SpikeTrapState::Hidden)
	{
		bool playerInRecess = false;
		if (player && !player->GetIsDead())
		{
			VECTOR pPos = player->GetPosition();
			if (pPos.x >= 4270.0f && pPos.x <= 4360.0f && pPos.y >= 1910.0f)
			{
				playerInRecess = true;
			}
		}

		bool boulderAtRecess = (mState == State::Rolling && mX >= 4190.0f);

		if (playerInRecess || boulderAtRecess)
		{
			mSpikeState = SpikeTrapState::Emerging;
		}
	}

	// 下からトゲがせり上がるアニメーション
	if (mSpikeState == SpikeTrapState::Emerging)
	{
		mSpikeHeight += 18.0f;
		if (mSpikeHeight >= mSpikeMaxHeight)
		{
			mSpikeHeight = mSpikeMaxHeight;
			// 岩によって針が出現した後は、勝手に飛ばずくぼみで立って待機する！
			mSpikeState = SpikeTrapState::Ready;
		}
	}
	VECTOR pPos = player->GetPosition();

	// 針が出現中（せり上がり中または待機状態）のプレイヤー判定
	if ((mSpikeState == SpikeTrapState::Emerging || mSpikeState == SpikeTrapState::Ready) && player && !player->GetIsDead())
	{
		// ① プレイヤーがくぼみに入った場合：針に刺さって死亡！（針は飛ばない）
		if (pPos.x >= 4270.0f && pPos.x <= 4360.0f && pPos.y >= 1910.0f)
		{
			mSpikeHeight = mSpikeMaxHeight;
			SpawnSpikeParticles(pPos.x, 1960.0f, 35);
			player->TakeDamage(9999);
			player->SetDeathReason(DeathReason::DeathType::NeedleTrapHit);

		}
		// ② プレイヤーがくぼみに入る以外で針が出てきたあと、プレイヤーがそのくぼみの場所を通ろうとすると発射！
		// （飛び越えようとして上空を通過、またはくぼみの場所を通ろうとした瞬間）
		else if (pPos.x >= 4255.0f && pPos.x <= 4375.0f)
		{
			mSpikeState = SpikeTrapState::Launched;
			mSpikeHeight = mSpikeMaxHeight;
			mSpikeY = 2006.0f;
			mSpikeVy = -24.0f; // 上に向かって発射！
			SpawnSpikeParticles(mSpikeBaseX, 2006.0f, 30);
		}
	}

	// トゲが上空へ発射中の挙動と当たり判定（当たり判定はつけたまま、当たったら死亡）
	if (mSpikeState == SpikeTrapState::Launched)
	{
		mSpikeY += mSpikeVy;

		// 発射の推進エフェクト
		if (GetRand(2) == 0)
		{
			SpawnDust(mSpikeBaseX + (GetRand(20) - 10), mSpikeY + 5.0f, 2, 0.6f);
		}

		// 上空を飛ぶトゲとプレイヤーの当たり判定
		if (player && !player->GetIsDead())
		{
			VECTOR pPos = player->GetPosition();
			float pW = 44.0f;
			float pH = 100.0f;
			float pLeft = pPos.x - pW * 0.5f;
			float pRight = pPos.x + pW * 0.5f;
			float pTop = pPos.y - pH * 0.5f;
			float pBottom = pPos.y + pH * 0.5f;

			// 発射されたトゲの判定ボックス
			float spLeft = mSpikeBaseX - mSpikeWidth * 0.5f;
			float spRight = mSpikeBaseX + mSpikeWidth * 0.5f;
			float spTop = mSpikeY - mSpikeHeight;
			float spBottom = mSpikeY;

			// 矩形同士の交差判定（当たったら死亡）
			if (pRight >= spLeft && pLeft <= spRight && pBottom >= spTop && pTop <= spBottom)
			{
				player->TakeDamage(9999);
				player->SetDeathReason(DeathReason::DeathType::NeedleTrapHit);
				SpawnSpikeParticles(pPos.x, pPos.y, 40);
			}
		}

		// 上空画面外へ飛び去ったら消滅
		if (mSpikeY < -300.0f)
		{
			mSpikeState = SpikeTrapState::Finished;
		}
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

			SpawnDust(mX, mY + mRadius, 45, 2.5f);
		}
		break;

	case State::Rolling:
	{
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

// 描画処理
void RollingBoulder::Draw(int scrollX, int scrollY)
{
	if (Master::gCurrentStageIndex != 1)
	{
		return;
	}

	// 1. くぼみの「1本で横2マス分を取る巨大なトゲ」の描画
	if (mSpikeState != SpikeTrapState::Hidden && mSpikeState != SpikeTrapState::Finished && mSpikeHeight > 0.0f)
	{
		float leftX = mSpikeBaseX - mSpikeWidth * 0.5f;  // 4440.0f
		float rightX = mSpikeBaseX + mSpikeWidth * 0.5f; // 4515.0f
		float tipX = mSpikeBaseX;                        // 4477.5f（中央）
		float baseY = mSpikeY;
		float tipY = mSpikeY - mSpikeHeight;

		int sx1 = static_cast<int>(leftX) - scrollX;
		int sx2 = static_cast<int>(rightX) - scrollX;
		int stX = static_cast<int>(tipX) - scrollX;
		int sbY = static_cast<int>(baseY) - scrollY;
		int stY = static_cast<int>(tipY) - scrollY;

		// 巨大な1本の三角トゲ本体（シルバー）
		DrawTriangle(stX, stY, sx1, sbY, sx2, sbY, GetColor(200, 205, 215), TRUE);
		// 立体陰影（右半分をダークシルバーに）
		DrawTriangle(stX, stY, stX, sbY, sx2, sbY, GetColor(140, 145, 155), TRUE);
		// 中央のハイライト線
		DrawLine(stX, stY, stX, sbY, GetColor(245, 245, 255));
		// 先端の血染めチップ（鋭い赤色）
		int tipRedH = static_cast<int>(mSpikeHeight * 0.28f);
		DrawTriangle(stX, stY, stX - 8, stY + tipRedH, stX + 8, stY + tipRedH, GetColor(220, 20, 30), TRUE);
		// 外枠の引き締まった黒輪郭線
		DrawLine(stX, stY, sx1, sbY, GetColor(45, 45, 55));
		DrawLine(stX, stY, sx2, sbY, GetColor(45, 45, 55));
		DrawLine(sx1, sbY, sx2, sbY, GetColor(45, 45, 55));
	}

	// 2. トゲ刺突・発射パーティクルの描画
	for (const auto& sp : mSpikeParticles)
	{
		int dx = static_cast<int>(sp.x) - scrollX;
		int dy = static_cast<int>(sp.y) - scrollY;
		DrawBox(dx, dy, dx + sp.size, dy + sp.size, sp.color, TRUE);
	}

	// 3. 岩の描画
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

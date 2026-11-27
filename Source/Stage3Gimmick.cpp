#include "Stage3Gimmick.h"
#include "BlockMap.h"
#include "Player.h"
#include "GameConstants.h"
#include <cstdio>
#include <cmath>

Stage3Gimmick::Stage3Gimmick()
	: mGateWallGraphHandle(-1)
	, mbGateTriggered(false)
	, mbGateClosed(false)
	, mGateCurrentBottomY(static_cast<float>(GATE_TOP_Y))
	, mGateDropSpeed(0.0f)
	, mGateShakeTimer(0)
	, mRidingPlatformIndex(-1)
{
	for (int i = 0; i < PLATFORM_COUNT; ++i)
	{
		mPlatforms[i].graphHandle = -1;
	}
	SetupPlatformDefinitions();
}

Stage3Gimmick::~Stage3Gimmick()
{
	Finalize();
}

void Stage3Gimmick::SetupPlatformDefinitions()
{
	// 【Stage3.png 5724x1692拡大版 対応】拡大後マップ(5724x1692)上の5つの浮遊台座標
	const int coords[PLATFORM_COUNT][4] = {
		{ 3736, 1155, 3872, 1181 }, // 台1 (左・137x27)
		{ 4010, 1074, 4181, 1100 }, // 台2 (中左・172x27)
		{ 4250, 994,  4352, 1020 }, // 台3 (中央高・103x27)
		{ 4422, 1209, 4558, 1234 }, // 台4 (中右低・137x26)
		{ 4661, 1128, 4866, 1154 }  // 台5 (右・206x27)
	};

	mRidingPlatformIndex = -1;

	for (int i = 0; i < PLATFORM_COUNT; ++i)
	{
		mPlatforms[i].minX           = coords[i][0];
		mPlatforms[i].minY           = coords[i][1];
		mPlatforms[i].maxX           = coords[i][2];
		mPlatforms[i].maxY           = coords[i][3];
		mPlatforms[i].state          = PlatformState::Active;
		mPlatforms[i].standTimer     = 0;
		mPlatforms[i].respawnTimer   = 0;
		mPlatforms[i].floatPhase     = i * 1.35f; // 各台で少しずつ位相をずらして自然な浮遊感を出す
		mPlatforms[i].floatOffsetY   = sinf(mPlatforms[i].floatPhase) * 10.0f;
		mPlatforms[i].fallOffsetY    = 0.0f;
		mPlatforms[i].fallSpeedY     = 0.0f;
		mPlatforms[i].currentTopY    = static_cast<float>(coords[i][1]) + mPlatforms[i].floatOffsetY;
		mPlatforms[i].prevTopY       = mPlatforms[i].currentTopY;
		mPlatforms[i].deltaY         = 0.0f;
		mPlatforms[i].shakeOffsetX   = 0.0f;
		mPlatforms[i].shakeOffsetY   = 0.0f;
		mPlatforms[i].isPlayerRiding = false;
		mPlatforms[i].fadeAlpha      = 255;
	}
}

void Stage3Gimmick::Initialize(BlockMap& blockMap)
{
	Finalize();

	// 1. 退路封鎖用の降りてくる壁画像を読み込み
	mGateWallGraphHandle = LoadGraph("Resource/Stage3/Stage3_GateWall.png");
	if (mGateWallGraphHandle == -1)
	{
		mGateWallGraphHandle = LoadGraph("Resource/Image/Stage3_GateWall.png");
	}

	// 2. 5つの浮遊台の画像を読み込み
	for (int i = 0; i < PLATFORM_COUNT; ++i)
	{
		char path[128];
		sprintf_s(path, sizeof(path), "Resource/Stage3/Stage3_Platform%d.png", i + 1);
		mPlatforms[i].graphHandle = LoadGraph(path);
		if (mPlatforms[i].graphHandle == -1)
		{
			sprintf_s(path, sizeof(path), "Resource/Image/Stage3_Platform%d.png", i + 1);
			mPlatforms[i].graphHandle = LoadGraph(path);
		}
	}

	Reset(blockMap);
}

void Stage3Gimmick::Reset(BlockMap& blockMap)
{
	// 壁ギミックの初期化（開いた状態）
	mbGateTriggered     = false;
	mbGateClosed        = false;
	mGateCurrentBottomY = static_cast<float>(GATE_TOP_Y);
	mGateDropSpeed      = 0.0f;
	mGateShakeTimer     = 0;

	// 4つの浮遊台の初期化
	SetupPlatformDefinitions();

	// 当たり判定の初期状態をマップへ反映
	ApplyInitialCollision(blockMap);
}

void Stage3Gimmick::ApplyInitialCollision(BlockMap& blockMap)
{
	// 入口の壁は最初開いているので当たり判定を消去
	blockMap.ClearCollisionArea(GATE_MIN_X, GATE_TOP_Y, GATE_MAX_X, GATE_BOTTOM_Y - 1);

	// 4つの浮遊台はすべて動的当たり判定（HandlePlayerPlatformCollision）で処理するため、
	// BlockMap上の静的判定はクリアしておく（上下ふよふよ＆落下時のプレイヤー追従のため）
	for (int i = 0; i < PLATFORM_COUNT; ++i)
	{
		blockMap.ClearCollisionArea(
			mPlatforms[i].minX,
			mPlatforms[i].minY - 20,
			mPlatforms[i].maxX,
			mPlatforms[i].maxY + 20
		);
	}
}

void Stage3Gimmick::UpdatePlatformsBeforePlayerMove(Player* player)
{
	(void)player;

	for (int i = 0; i < PLATFORM_COUNT; ++i)
	{
		CrumblingPlatform& plat = mPlatforms[i];
		plat.prevTopY = plat.currentTopY;

		if (plat.state == PlatformState::Active)
		{
			// 上下にふよふよ浮遊する動き（振幅 ±10px、滑らかなサイン波）
			plat.floatPhase  += 0.045f;
			plat.floatOffsetY = sinf(plat.floatPhase) * 10.0f;
			plat.currentTopY  = static_cast<float>(plat.minY) + plat.floatOffsetY;
			plat.deltaY       = plat.currentTopY - plat.prevTopY;
		}
		else if (plat.state == PlatformState::Falling)
		{
			// 1.5秒乗った後の落下処理：台が下へ加速して落ち、乗っているプレイヤーも一緒に連れていく
			plat.respawnTimer++;
			plat.fallSpeedY  += 0.75f;
			if (plat.fallSpeedY > 15.0f) plat.fallSpeedY = 15.0f;
			plat.fallOffsetY += plat.fallSpeedY;
			plat.currentTopY  = static_cast<float>(plat.minY) + plat.floatOffsetY + plat.fallOffsetY;
			plat.deltaY       = plat.currentTopY - plat.prevTopY;

			// マグマの底（Y = 1360付近）まで落ちたらフェードアウトして復活待機へ移行
			// 【Stage3.png 5724x1692拡大版 対応】マグマ底(Y=1477)に合わせて消滅判定Yを調整
			if (plat.currentTopY >= 1440.0f)
			{
				plat.fadeAlpha -= 25;
				if (plat.fadeAlpha <= 0)
				{
					plat.fadeAlpha      = 0;
					plat.state          = PlatformState::Waiting;
					plat.isPlayerRiding = false;
					if (mRidingPlatformIndex == i)
					{
						mRidingPlatformIndex = -1;
					}
				}
			}
		}
		else if (plat.state == PlatformState::Waiting)
		{
			plat.deltaY = 0.0f;
			// 落下してから10秒（600フレーム）経過したら元の場所に復活！
			plat.respawnTimer++;
			if (plat.respawnTimer >= PLATFORM_RESPAWN_TIME)
			{
				plat.state          = PlatformState::Active;
				plat.standTimer     = 0;
				plat.respawnTimer   = 0;
				plat.fallOffsetY    = 0.0f;
				plat.fallSpeedY     = 0.0f;
				plat.floatOffsetY   = sinf(plat.floatPhase) * 10.0f;
				plat.currentTopY    = static_cast<float>(plat.minY) + plat.floatOffsetY;
				plat.prevTopY       = plat.currentTopY;
				plat.fadeAlpha      = 255;
				plat.isPlayerRiding = false;
			}
		}
	}
}

void Stage3Gimmick::HandlePlayerPlatformCollision(Player& player, VECTOR& position, float& velocityY, bool& isGrounded, bool& mbIsJumping, bool& hasJumped)
{
	if (player.GetIsDead())
	{
		mRidingPlatformIndex = -1;
		for (int i = 0; i < PLATFORM_COUNT; ++i)
		{
			mPlatforms[i].isPlayerRiding = false;
		}
		return;
	}

	const float pHalfW  = PlayerConstants::PlayerCollisionWidth * 0.5f;  // 16.0f
	const float pHalfH  = PlayerConstants::PlayerCollisionHeight * 0.5f; // 50.0f
	const float pLeft   = position.x - pHalfW;
	const float pRight  = position.x + pHalfW;
	float       pBottom = position.y + pHalfH;
	const float pTop    = position.y - pHalfH;

	// 1. 前フレームから引き続き乗っている台がある場合：
	//    プレイヤーがジャンプしておらず、左右の範囲内にいるなら、台の上下ふよふよ移動および落下移動にピタッと追従させる！
	if (mRidingPlatformIndex >= 0 && mRidingPlatformIndex < PLATFORM_COUNT)
	{
		CrumblingPlatform& ridingPlat = mPlatforms[mRidingPlatformIndex];
		if (ridingPlat.state == PlatformState::Active || ridingPlat.state == PlatformState::Falling)
		{
			bool hOverlap = (pRight >= static_cast<float>(ridingPlat.minX) + 2.0f &&
			                 pLeft  <= static_cast<float>(ridingPlat.maxX) - 2.0f);

			// ジャンプしていない（velocityY >= -1.0f）かつ左右が台の上に残っているなら、
			// 台が下にふよふよ動いても、また1.5秒経って台が急降下しても、プレイヤーが宙に浮いてふわっとならないよう台の上面にスナップ！
			if (hOverlap && !mbIsJumping && velocityY >= -1.0f)
			{
				ridingPlat.isPlayerRiding = true;
				isGrounded  = true;
				hasJumped   = false;
				mbIsJumping = false;
				velocityY   = 0.0f;
				position.y  = ridingPlat.currentTopY - pHalfH;
				return;
			}
		}
		// 台から降りた・ジャンプした・台が消滅した場合
		ridingPlat.isPlayerRiding = false;
		mRidingPlatformIndex = -1;
	}

	// 2. 新規に台へ着地する判定
	for (int i = 0; i < PLATFORM_COUNT; ++i)
	{
		CrumblingPlatform& plat = mPlatforms[i];
		plat.isPlayerRiding = false;

		if (plat.state != PlatformState::Active && plat.state != PlatformState::Falling)
		{
			continue;
		}

		float platLeft   = static_cast<float>(plat.minX);
		float platRight  = static_cast<float>(plat.maxX);
		float platTop    = plat.currentTopY;
		float platBottom = platTop + static_cast<float>(plat.maxY - plat.minY);

		bool hOverlap = (pRight >= platLeft + 2.0f && pLeft <= platRight - 2.0f);
		if (!hOverlap) continue;

		// 上から足場に着地した場合
		if (velocityY >= -1.0f)
		{
			float toleranceBottom = 26.0f + ((plat.deltaY > 0.0f) ? plat.deltaY : 0.0f);
			if (pBottom >= platTop - 10.0f && pBottom <= platTop + toleranceBottom)
			{
				plat.isPlayerRiding  = true;
				mRidingPlatformIndex = i;
				isGrounded           = true;
				hasJumped            = false;
				mbIsJumping          = false;
				velocityY            = 0.0f;
				position.y           = platTop - pHalfH;
				return;
			}
		}
		// 下からジャンプして台の底面に頭をぶつけた場合（Active時のみ）
		else if (plat.state == PlatformState::Active && velocityY < -1.0f)
		{
			if (pTop <= platBottom + 6.0f && pTop >= platTop)
			{
				velocityY  = 0.0f;
				position.y = platBottom + pHalfH;
			}
		}
	}
}

void Stage3Gimmick::Update(BlockMap& blockMap, Player* player)
{
	if (player == nullptr) return;

	// =====================================================================
	// ギミック1: ボス部屋に入ったら上から降りてきて後ろへ帰れなくする壁
	// =====================================================================
	if (!mbGateTriggered && !player->GetIsDead())
	{
		if (player->GetPosition().x >= GATE_TRIGGER_X)
		{
			mbGateTriggered = true;
			mGateDropSpeed = 14.0f;
		}
	}

	if (mbGateTriggered && !mbGateClosed)
	{
		mGateDropSpeed += 2.2f; // 重力加速でズドンと落ちる
		mGateCurrentBottomY += mGateDropSpeed;

		if (mGateCurrentBottomY >= static_cast<float>(GATE_BOTTOM_Y))
		{
			mGateCurrentBottomY = static_cast<float>(GATE_BOTTOM_Y);
			mbGateClosed = true;
			mGateShakeTimer = 18; // 床に激突した瞬間の振動

			// 着地時の破片パーティクル発生
			for (int i = 0; i < 20; ++i)
			{
				BlockMap::DebrisParticle d;
				d.x = static_cast<float>(GATE_MIN_X + GetRand(GATE_MAX_X - GATE_MIN_X + 1));
				d.y = static_cast<float>(GATE_BOTTOM_Y - 6);
				d.vx = (GetRand(24) - 12) * 0.6f;
				d.vy = -(GetRand(14) + 4) * 0.5f;
				d.size = GetRand(8) + 5;
				d.life = GetRand(25) + 20;
				d.maxLife = d.life;
				d.color = GetColor(200, 215, 245);
				blockMap.mDebris.push_back(d);
			}
		}

		// 降りてきた高さまでの当たり判定をBlockで埋める（引き返し防止）
		int fillBottomY = static_cast<int>(mGateCurrentBottomY) - 1;
		if (fillBottomY >= GATE_TOP_Y)
		{
			blockMap.FillCollisionArea(GATE_MIN_X, GATE_TOP_Y, GATE_MAX_X, fillBottomY, BlockMap::CollisionType::Block);
		}

		// 万が一プレイヤーが壁の真下に引き返そうとしていたら部屋内側へ押し出す
		if (player->GetPosition().x < static_cast<float>(GATE_MAX_X + 20))
		{
			VECTOR p = player->GetPosition();
			p.x = static_cast<float>(GATE_MAX_X + 20);
			player->SetPosition(p);
		}
	}

	if (mGateShakeTimer > 0)
	{
		mGateShakeTimer--;
		if (mGateShakeTimer > 0)
		{
			int amp = (mGateShakeTimer / 2) + 2;
			blockMap.mShakeX = GetRand(amp * 2) - amp;
			blockMap.mShakeY = GetRand(amp * 2) - amp;
		}
		else
		{
			blockMap.mShakeX = 0;
			blockMap.mShakeY = 0;
		}
	}

	// =====================================================================
	// ギミック2: ボス部屋の4つの浮遊台（乗っている間の震え＆1.5秒経過で落下開始）
	// =====================================================================
	for (int i = 0; i < PLATFORM_COUNT; ++i)
	{
		CrumblingPlatform& plat = mPlatforms[i];
		plat.shakeOffsetX = 0.0f;
		plat.shakeOffsetY = 0.0f;

		if (plat.state == PlatformState::Active)
		{
			if (!player->GetIsDead() && plat.isPlayerRiding)
			{
				plat.standTimer++;

				// 乗っている間、台がブルブル震える（1.5秒に近づくほど揺れが激しくなる）
				float ratio = static_cast<float>(plat.standTimer) / static_cast<float>(PLATFORM_FALL_FRAMES);
				int ampX = 2 + static_cast<int>(ratio * 5.0f); // ±2px ～ ±7px
				int ampY = 1 + static_cast<int>(ratio * 3.0f); // ±1px ～ ±4px
				plat.shakeOffsetX = static_cast<float>(GetRand(ampX * 2 + 1) - ampX);
				plat.shakeOffsetY = static_cast<float>(GetRand(ampY * 2 + 1) - ampY);

				// 震えている台から小さな欠片がポロポロ落ちる演出
				if (plat.standTimer % 8 == 0)
				{
					BlockMap::DebrisParticle d;
					d.x = static_cast<float>(plat.minX + GetRand(plat.maxX - plat.minX + 1));
					d.y = plat.currentTopY + static_cast<float>(plat.maxY - plat.minY);
					d.vx = (GetRand(10) - 5) * 0.3f;
					d.vy = (GetRand(6) + 2) * 0.4f;
					d.size = GetRand(5) + 3;
					d.life = GetRand(18) + 12;
					d.maxLife = d.life;
					d.color = GetColor(215, 226, 255);
					blockMap.mDebris.push_back(d);
				}

				// 1.5秒（90フレーム）乗っていたら台が落下開始！（プレイヤーも台に乗ったまま一緒に落ちる）
				if (plat.standTimer >= PLATFORM_FALL_FRAMES)
				{
					plat.state        = PlatformState::Falling;
					plat.standTimer   = 0;
					plat.respawnTimer = 0;
					plat.fallOffsetY  = 0.0f;
					plat.fallSpeedY   = 3.5f;
					plat.shakeOffsetX = 0.0f;
					plat.shakeOffsetY = 0.0f;

					// 落下開始時の破片エフェクト
					for (int k = 0; k < 12; ++k)
					{
						BlockMap::DebrisParticle d;
						d.x = static_cast<float>(plat.minX + GetRand(plat.maxX - plat.minX + 1));
						d.y = plat.currentTopY + static_cast<float>(GetRand(plat.maxY - plat.minY + 1));
						d.vx = (GetRand(20) - 10) * 0.5f;
						d.vy = -(GetRand(10) + 2) * 0.4f;
						d.size = GetRand(6) + 4;
						d.life = GetRand(22) + 15;
						d.maxLife = d.life;
						d.color = GetColor(215, 226, 255);
						blockMap.mDebris.push_back(d);
					}
				}
			}
			else
			{
				// 台から降りたらカウントをリセット
				plat.standTimer = 0;
			}
		}
	}
}

void Stage3Gimmick::Draw(const BlockMap& blockMap, Player* player)
{
	(void)player;

	int drawScrollX = blockMap.GetScrollX() + blockMap.mShakeX;
	int drawScrollY = blockMap.GetScrollY() + blockMap.mShakeY;

	// ---------------------------------------------------------------------
	// 1. 退路を塞ぐ壁の描画
	// ---------------------------------------------------------------------
	if (mbGateTriggered)
	{
		int curH = static_cast<int>(mGateCurrentBottomY) - GATE_TOP_Y;
		if (curH > 0)
		{
			if (curH > GATE_HEIGHT) curH = GATE_HEIGHT;
			int screenX = GATE_MIN_X - drawScrollX;
			int screenY = GATE_TOP_Y - drawScrollY;

			if (mGateWallGraphHandle != -1)
			{
				int srcY = GATE_HEIGHT - curH;
				DrawRectGraph(
					screenX, screenY,
					0, srcY,
					GATE_MAX_X - GATE_MIN_X + 1, curH,
					mGateWallGraphHandle, TRUE
				);
			}
			else
			{
				DrawBox(screenX, screenY, screenX + (GATE_MAX_X - GATE_MIN_X + 1), screenY + curH, GetColor(215, 226, 255), TRUE);
			}
		}
	}

	// ---------------------------------------------------------------------
	// 2. ボス部屋の4つの浮遊台の描画（上下ふよふよ＋震え＋落下）
	// ---------------------------------------------------------------------
	for (int i = 0; i < PLATFORM_COUNT; ++i)
	{
		const CrumblingPlatform& plat = mPlatforms[i];
		if (plat.state == PlatformState::Waiting)
		{
			// 復活直前（残り0.5秒＝30フレーム）はうっすら点滅して復活予兆を見せる
			int remain = PLATFORM_RESPAWN_TIME - plat.respawnTimer;
			if (remain <= 30 && (remain / 4) % 2 == 0 && plat.graphHandle != -1)
			{
				int previewY = static_cast<int>(static_cast<float>(plat.minY) + sinf(plat.floatPhase) * 10.0f) - drawScrollY;
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, 110);
				DrawGraph(plat.minX - drawScrollX, previewY, plat.graphHandle, TRUE);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			}
			continue;
		}

		int drawX = plat.minX - drawScrollX + static_cast<int>(plat.shakeOffsetX);
		int drawY = static_cast<int>(plat.currentTopY + plat.shakeOffsetY) - drawScrollY;

		if (plat.fadeAlpha < 255)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, plat.fadeAlpha);
		}

		if (plat.graphHandle != -1)
		{
			DrawGraph(drawX, drawY, plat.graphHandle, TRUE);
		}
		else
		{
			int w = plat.maxX - plat.minX + 1;
			int h = plat.maxY - plat.minY + 1;
			DrawBox(drawX, drawY, drawX + w, drawY + h, GetColor(215, 226, 255), TRUE);
		}

		if (plat.fadeAlpha < 255)
		{
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}
}

void Stage3Gimmick::Finalize()
{
	if (mGateWallGraphHandle != -1)
	{
		DeleteGraph(mGateWallGraphHandle);
		mGateWallGraphHandle = -1;
	}
	for (int i = 0; i < PLATFORM_COUNT; ++i)
	{
		if (mPlatforms[i].graphHandle != -1)
		{
			DeleteGraph(mPlatforms[i].graphHandle);
			mPlatforms[i].graphHandle = -1;
		}
	}
}

void Stage3Gimmick::DestroyRandomPlatform(BlockMap& blockMap, Player* player)
{
	// 現在Activeな（残っている）台のインデックスを集める
	int activeIndices[PLATFORM_COUNT];
	int activeCount = 0;
	for (int i = 0; i < PLATFORM_COUNT; ++i)
	{
		if (mPlatforms[i].state == PlatformState::Active)
		{
			activeIndices[activeCount++] = i;
		}
	}

	if (activeCount <= 0) return;

	int targetIdx = activeIndices[GetRand(activeCount - 1)];
	CrumblingPlatform& plat = mPlatforms[targetIdx];

	// 咆哮の衝撃で台が粉々に砕け散り、10秒（600フレーム）の復活待機へ移行！
	plat.state          = PlatformState::Waiting;
	plat.standTimer     = 0;
	plat.respawnTimer   = 0;
	plat.fallOffsetY    = 0.0f;
	plat.fallSpeedY     = 0.0f;
	plat.shakeOffsetX   = 0.0f;
	plat.shakeOffsetY   = 0.0f;
	plat.fadeAlpha      = 0;
	plat.isPlayerRiding = false;

	if (mRidingPlatformIndex == targetIdx)
	{
		mRidingPlatformIndex = -1;
		if (player != nullptr)
		{
			player->SetIsGrounded(false);
		}
	}

	// 台が砕け散る大量の破片エフェクト
	for (int k = 0; k < 28; ++k)
	{
		BlockMap::DebrisParticle d;
		d.x       = static_cast<float>(plat.minX + GetRand(plat.maxX - plat.minX + 1));
		d.y       = plat.currentTopY + static_cast<float>(GetRand(plat.maxY - plat.minY + 1));
		d.vx      = (GetRand(32) - 16) * 0.6f;
		d.vy      = -(GetRand(18) + 2) * 0.5f;
		d.size    = GetRand(8) + 5;
		d.life    = GetRand(28) + 20;
		d.maxLife = d.life;
		d.color   = GetColor(215, 226, 255);
		blockMap.mDebris.push_back(d);
	}
}

void Stage3Gimmick::DestroyRandomPlatforms(BlockMap& blockMap, Player* player, int count)
{
	for (int c = 0; c < count; ++c)
	{
		DestroyRandomPlatform(blockMap, player);
	}
}

void Stage3Gimmick::DestroyAllPlatforms(BlockMap& blockMap, Player* player)
{
	for (int i = 0; i < PLATFORM_COUNT; ++i)
	{
		CrumblingPlatform& plat = mPlatforms[i];
		plat.state          = PlatformState::Waiting;
		plat.standTimer     = 0;
		plat.respawnTimer   = 0;
		plat.fallOffsetY    = 0.0f;
		plat.fallSpeedY     = 0.0f;
		plat.shakeOffsetX   = 0.0f;
		plat.shakeOffsetY   = 0.0f;
		plat.fadeAlpha      = 0;
		plat.isPlayerRiding = false;

		// 全台破壊時の破片を軽量化（8個/台）
		for (int k = 0; k < 8; ++k)
		{
			BlockMap::DebrisParticle d;
			d.x       = static_cast<float>(plat.minX + GetRand(plat.maxX - plat.minX + 1));
			d.y       = plat.currentTopY + static_cast<float>(GetRand(plat.maxY - plat.minY + 1));
			d.vx      = (GetRand(64) - 32) * 0.85f;
			d.vy      = -(GetRand(32) + 6) * 0.75f;
			d.size    = GetRand(11) + 6;
			d.life    = GetRand(50) + 35;
			d.maxLife = d.life;
			d.color   = GetColor(215, 230, 255);
			blockMap.mDebris.push_back(d);
		}
	}

	if (mRidingPlatformIndex != -1)
	{
		mRidingPlatformIndex = -1;
		if (player != nullptr)
		{
			player->SetIsGrounded(false);
		}
	}
}

void Stage3Gimmick::ForceDetachPlayer(Player* player)
{
	if (mRidingPlatformIndex >= 0 && mRidingPlatformIndex < PLATFORM_COUNT)
	{
		mPlatforms[mRidingPlatformIndex].isPlayerRiding = false;
	}
	mRidingPlatformIndex = -1;
	for (int i = 0; i < PLATFORM_COUNT; ++i)
	{
		mPlatforms[i].isPlayerRiding = false;
	}
	if (player != nullptr)
	{
		player->SetIsGrounded(false);
	}
}

void Stage3Gimmick::RestoreAllPlatforms()
{
	SetupPlatformDefinitions();
}

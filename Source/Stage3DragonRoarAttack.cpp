#include "Stage3DragonRoarAttack.h"
#include "Effekseer2DManager.h"
#include "BlockMap.h"
#include "Player.h"
#include "Stage3Gimmick.h"
#include "GameConstants.h"
#include <cmath>

Stage3DragonRoarAttack::Stage3DragonRoarAttack()
	: mPhase(Phase::Inactive)
	, mTimer(0)
	, mAnimTimer(0)
	, mAnimFrame(0)
	, mbTargetIsLeftEdge(false)
	, mTargetX(RIGHT_LEDGE_STAND_X)
	, mTargetFloorY(RIGHT_LEDGE_FLOOR_Y)
	, mbHasKnockedBackPlayer(false)
	, mPlayerKnockbackVx(0.0f)
	, mKnockbackTimer(0)
{
}

Stage3DragonRoarAttack::~Stage3DragonRoarAttack()
{
}

void Stage3DragonRoarAttack::Initialize()
{
	Reset();
}

void Stage3DragonRoarAttack::Reset(Player* player)
{
	if (mKnockbackTimer > 0 && player != nullptr)
	{
		player->SetInputDisabled(false);
	}
	mPhase                 = Phase::Inactive;
	mTimer                 = 0;
	mAnimTimer             = 0;
	mAnimFrame             = 0;
	mbTargetIsLeftEdge     = false;
	mTargetX               = RIGHT_LEDGE_STAND_X;
	mTargetFloorY          = RIGHT_LEDGE_FLOOR_Y;
	mbHasKnockedBackPlayer = false;
	mPlayerKnockbackVx     = 0.0f;
	mKnockbackTimer        = 0;
	mShockRings.clear();
	mWindLines.clear();
}

void Stage3DragonRoarAttack::StartAttack(VECTOR& bossPos, float& facingDir)
{
	// ステージの左端上空（X=3630, Y=660）か右端上空（X=5440, Y=660）をランダムに選択
	mbTargetIsLeftEdge = (GetRand(1) == 0);
	if (mbTargetIsLeftEdge)
	{
		mTargetX      = LEFT_LEDGE_STAND_X;
		mTargetFloorY = LEFT_LEDGE_FLOOR_Y;
	}
	else
	{
		mTargetX      = RIGHT_LEDGE_STAND_X;
		mTargetFloorY = RIGHT_LEDGE_FLOOR_Y;
	}

	mbHasKnockedBackPlayer = false;
	mShockRings.clear();
	mWindLines.clear();

	// 常に離陸（Takeoff）から開始し、空中へ舞い上がってから咆哮する！
	if (std::fabs(bossPos.x - mTargetX) > 40.0f)
	{
		facingDir = (mTargetX >= bossPos.x) ? 1.0f : -1.0f;
	}
	else
	{
		facingDir = mbTargetIsLeftEdge ? 1.0f : -1.0f;
	}

	mPhase     = Phase::Takeoff;
	mTimer     = 0;
	mAnimTimer = 0;
	mAnimFrame = 0;
}

void Stage3DragonRoarAttack::UpdatePlayerKnockback(Player* player, BlockMap* blockMap)
{
	if (player == nullptr || blockMap == nullptr) return;
	if (player->GetIsDead())
	{
		if (mKnockbackTimer > 0)
		{
			player->SetInputDisabled(false);
		}
		mKnockbackTimer    = 0;
		mPlayerKnockbackVx = 0.0f;
		return;
	}

	if (mKnockbackTimer > 0)
	{
		mKnockbackTimer--;
		// 吹き飛ばし前半（残り12フレームより上）はキー入力による逆方向歩行で相殺されないようロック維持
		if (mKnockbackTimer == 12 || mKnockbackTimer == 0)
		{
			player->SetInputDisabled(false);
		}
	}

	if (std::fabs(mPlayerKnockbackVx) > 0.35f)
	{
		VECTOR pPos  = player->GetPosition();
		float pHalfW = PlayerConstants::PlayerCollisionWidth * 0.5f;  // 16.0f
		float pHalfH = PlayerConstants::PlayerCollisionHeight * 0.5f; // 50.0f

		// 壁にめり込まないよう1ステップずつ横移動させる
		int steps = static_cast<int>(std::fabs(mPlayerKnockbackVx)) + 1;
		float stepX = mPlayerKnockbackVx / static_cast<float>(steps);

		for (int s = 0; s < steps; ++s)
		{
			float nextX  = pPos.x + stepX;
			float checkX = (stepX > 0.0f) ? (nextX + pHalfW) : (nextX - pHalfW);
			bool hitWall = false;
			for (float cy = pPos.y - pHalfH + 20.0f; cy <= pPos.y + pHalfH - 20.0f; cy += 16.0f)
			{
				BlockMap::CollisionType ct = blockMap->GetCollisionType(static_cast<int>(checkX), static_cast<int>(cy));
				if (ct == BlockMap::CollisionType::Block || ct == BlockMap::CollisionType::BreakableBlock)
				{
					hitWall = true;
					break;
				}
			}
			if (hitWall)
			{
				// 強吹き飛ばし中にボス部屋端の壁（閉じ込め壁 X=3384 や右壁 X=5703）に激突した場合は、
				// 壁際で止まってしまわないよう反動で逆方向（中央の穴側）へ大きく跳ね返らせる！
				if (mKnockbackTimer > 10)
				{
					mPlayerKnockbackVx = -mPlayerKnockbackVx * 0.88f;
					stepX              = -stepX * 0.88f;
				}
				else
				{
					mPlayerKnockbackVx = 0.0f;
					break;
				}
			}
			else
			{
				pPos.x = nextX;
			}
		}

		player->SetPosition(pPos);

		// 吹き飛ばしによる高速移動にカメラスクロールがしっかり追従するように更新
		int scrollX    = blockMap->GetScrollX();
		int screenX    = static_cast<int>(pPos.x) - scrollX;
		int maxScrollX = blockMap->GetCurrentWidth() - ScreenSize::ScrrenWidth;
		if (maxScrollX < 0) maxScrollX = 0;

		if (!blockMap->IsCameraLocked())
		{
			if (screenX > MapScrollConstants::ScrollStartRightX)
			{
				scrollX = static_cast<int>(pPos.x) - MapScrollConstants::ScrollStartRightX;
			}
			else if (screenX < MapScrollConstants::ScrollStartLeftX)
			{
				scrollX = static_cast<int>(pPos.x) - MapScrollConstants::ScrollStartLeftX;
			}
			if (scrollX < 0) scrollX = 0;
			if (scrollX > maxScrollX) scrollX = maxScrollX;
			blockMap->SetScrollX(scrollX);
		}

		// 吹き飛ばし速度の減衰（遠くまでしっかり吹き飛ぶよう緩やかに減衰）
		mPlayerKnockbackVx *= 0.96f;
		if (std::fabs(mPlayerKnockbackVx) <= 0.35f)
		{
			mPlayerKnockbackVx = 0.0f;
			if (mKnockbackTimer > 0)
			{
				mKnockbackTimer = 0;
				player->SetInputDisabled(false);
			}
		}
	}
}

void Stage3DragonRoarAttack::Update(
	VECTOR& bossPos,
	float& facingDir,
	bool& isBossGrounded,
	int& outAnimType,
	int& outAnimFrame,
	Player* player,
	BlockMap* blockMap,
	Stage3Gimmick* gimmick)
{
	// エフェクトの更新
	for (auto it = mShockRings.begin(); it != mShockRings.end(); )
	{
		it->radius += (it->maxRadius - it->radius) * 0.16f + 8.0f;
		it->life--;
		if (it->life <= 0) it = mShockRings.erase(it);
		else ++it;
	}
	for (auto it = mWindLines.begin(); it != mWindLines.end(); )
	{
		it->x += it->vx;
		it->y += it->vy;
		it->life--;
		if (it->life <= 0) it = mWindLines.erase(it);
		else ++it;
	}

	if (mPhase == Phase::Inactive || mPhase == Phase::Finished)
	{
		return;
	}

	mTimer++;
	mAnimTimer++;

	switch (mPhase)
	{
	case Phase::Takeoff:
		// 1. 離陸して空中（高度 AIR_ROAR_Y = 660.0f）へ飛び上がる
		isBossGrounded = false;
		outAnimType    = ANIM_TAKEOFF;
		if (mAnimTimer >= 5)
		{
			mAnimTimer = 0;
			if (mAnimFrame < 6) mAnimFrame++;
		}
		outAnimFrame = mAnimFrame;

		if (mAnimFrame >= 1)
		{
			bossPos.y += (AIR_ROAR_Y - bossPos.y) * 0.14f - 5.0f;
			if (bossPos.y < AIR_ROAR_Y) bossPos.y = AIR_ROAR_Y;
		}
		if (mTimer >= 30)
		{
			mPhase     = Phase::FlyToEdge;
			mTimer     = 0;
			mAnimTimer = 0;
			mAnimFrame = 0;
		}
		break;

	case Phase::FlyToEdge:
	{
		// 2. 選ばれた左端または右端の上空（AIR_ROAR_Y）へ飛行移動し、着陸せず空中でホバリングへ移行！
		isBossGrounded = false;
		outAnimType    = ANIM_FLY;
		if (mAnimTimer >= 5)
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % 7;
		}
		outAnimFrame = mAnimFrame;

		float dx = mTargetX - bossPos.x;
		if (std::fabs(dx) > 20.0f)
		{
			facingDir = (dx >= 0.0f) ? 1.0f : -1.0f;
		}
		else
		{
			facingDir = mbTargetIsLeftEdge ? 1.0f : -1.0f;
		}
		bossPos.y += (AIR_ROAR_Y - bossPos.y) * 0.18f + std::sin(mTimer * 0.2f) * 2.5f;

		const float flySpeed = 26.0f;
		if (std::fabs(dx) <= flySpeed && mTimer >= 18)
		{
			// 空中の定位置（X = mTargetX, Y = AIR_ROAR_Y）に到着！そのまま空中で3秒チャージ開始！
			bossPos.x  = mTargetX;
			bossPos.y  = AIR_ROAR_Y;
			facingDir  = mbTargetIsLeftEdge ? 1.0f : -1.0f; // 部屋の内側（反対端）を向く
			mPhase     = Phase::Charge3Sec;
			mTimer     = 0;
			mAnimTimer = 0;
			mAnimFrame = 0;
		}
		else
		{
			if (std::fabs(dx) > flySpeed)
			{
				bossPos.x += (dx > 0.0f) ? flySpeed : -flySpeed;
			}
			else
			{
				bossPos.x = mTargetX;
			}
		}
		break;
	}

	case Phase::Charge3Sec:
	{
		// 3. 空中でホバリングしながら3秒間（180フレーム）咆哮を溜める！（roar1.png ～ roar4.png）
		isBossGrounded = false;
		outAnimType    = ANIM_ROAR;
		bossPos.x      = mTargetX;
		// 空中で羽ばたきながら上下にふよふよホバリング
		bossPos.y      = AIR_ROAR_Y + std::sin(mTimer * 0.14f) * 16.0f;
		facingDir      = mbTargetIsLeftEdge ? 1.0f : -1.0f;

		// 0..180フレームを 4段階に分けて roar1(0) -> roar2(1) -> roar3(2) -> roar4(3) を再生
		if (mTimer < 45)       mAnimFrame = 0;
		else if (mTimer < 95)  mAnimFrame = 1;
		else if (mTimer < 145) mAnimFrame = 2;
		else                   mAnimFrame = 3;
		outAnimFrame = mAnimFrame;

		// 溜め中、空中の口元へ2D Effekseerチャージ収束波＋放電
		float chargeMouthX = bossPos.x + facingDir * 135.0f;
		float chargeMouthY = bossPos.y - 72.0f;
		if (mTimer == 30 || mTimer == 85 || mTimer == 140)
		{
			Effekseer2DManager::Play(
				Effect2DId::Stage3_RoarWave,
				chargeMouthX + facingDir * 12.0f,
				chargeMouthY,
				(mTimer == 30) ? 0.80f : 1.05f,
				1.25f,
				0.0f,
				255, 95, 55, 210
			);
		}
		if (mTimer == 55 || mTimer == 115 || mTimer == 155)
		{
			Effekseer2DManager::Play(
				Effect2DId::Stage3_Thunder,
				chargeMouthX + facingDir * 15.0f,
				chargeMouthY,
				(mTimer == 55) ? 0.65f : ((mTimer == 115) ? 0.85f : 1.05f),
				1.65f,
				(facingDir >= 0.0f) ? 0.0f : 3.14159265f,
				255, 195, 80, 235
			);
		}

		// 3秒（180フレーム）溜めきったら、空中から咆哮放出（RoarBlast）へ！
		if (mTimer >= CHARGE_DURATION_FRAMES)
		{
			mPhase     = Phase::RoarBlast;
			mTimer     = 0;
			mAnimTimer = 0;
			mAnimFrame = 4; // roar5.png
		}
		break;
	}

	case Phase::RoarBlast:
	{
		// 4. 空中から大咆哮を放つ！（roar5.png(4) <-> roar6.png(5) で激しく咆哮し、最後に roar7.png(6)）
		isBossGrounded = false;
		outAnimType    = ANIM_ROAR;
		bossPos.x      = mTargetX;
		bossPos.y      = AIR_ROAR_Y + std::sin(mTimer * 0.35f) * 8.0f;
		facingDir      = mbTargetIsLeftEdge ? 1.0f : -1.0f;

		if (mTimer < 52)
		{
			// roar5(4) と roar6(5) を交互に切り替えながら空中大咆哮
			mAnimFrame = ((mTimer / 5) % 2 == 0) ? 4 : 5;
		}
		else
		{
			mAnimFrame = 6; // roar7(6)
		}
		outAnimFrame = mAnimFrame;

		// 画面振動と衝撃波リング＆空中から吹き下ろす暴風ライン生成
		if (blockMap != nullptr)
		{
			if (mTimer < 52)
			{
				blockMap->mShakeX = GetRand(18) - 9;
				blockMap->mShakeY = GetRand(16) - 8;
			}
			else
			{
				blockMap->mShakeX = 0;
				blockMap->mShakeY = 0;
			}
		}

		float mouthX = bossPos.x + facingDir * 145.0f;
		float mouthY = bossPos.y - 75.0f;
		float roarRotZ = (facingDir >= 0.0f) ? 0.0f : 3.14159265f;

		if (mTimer == 1)
		{
			Effekseer2DManager::Play(
				Effect2DId::Stage3_RoarWave,
				mouthX + facingDir * 55.0f,
				mouthY,
				1.95f,
				1.40f,
				0.0f,
				255, 110, 50, 235
			);
			Effekseer2DManager::Play(
				Effect2DId::Stage3_FlameBurst,
				mouthX + facingDir * 185.0f,
				mouthY + 10.0f,
				1.95f,
				1.55f,
				roarRotZ,
				255, 175, 80, 245
			);
			Effekseer2DManager::Play(
				Effect2DId::Stage3_SlashWind,
				mouthX + facingDir * 250.0f,
				mouthY + 10.0f,
				1.85f,
				1.55f,
				roarRotZ,
				255, 215, 105, 235
			);
		}
		else if (mTimer == 22)
		{
			Effekseer2DManager::Play(
				Effect2DId::Stage3_Thunder,
				mouthX + facingDir * 210.0f,
				mouthY + 10.0f,
				1.45f,
				1.75f,
				roarRotZ,
				255, 225, 100, 235
			);
		}

		if (mTimer < 50 && mTimer % 3 == 0)
		{
			for (int w = 0; w < 4; ++w)
			{
				WindLine wl;
				wl.x       = mbTargetIsLeftEdge ? (LEFT_SAFE_MAX_X - 40.0f) : (RIGHT_SAFE_MIN_X + 40.0f);
				wl.y       = 300.0f + static_cast<float>(GetRand(820));
				wl.vx      = facingDir * (32.0f + static_cast<float>(GetRand(16)));
				wl.vy      = 4.5f + static_cast<float>(GetRand(6)); // 空中から斜め下へ吹き荒れる暴風
				wl.length  = 110.0f + static_cast<float>(GetRand(90));
				wl.life    = 36;
				wl.maxLife = 36;
				mWindLines.push_back(wl);
			}
		}

		// 当たり判定：ドラゴンのいる方向の足場側「以外」の範囲にいるプレイヤーをダメージなしで大きく吹き飛ばす！
		// ・ドラゴンが左端上空にいるとき：左端の足場（X <= 3852）は安全、それより右（X > 3852）に咆哮が当たる
		// ・ドラゴンが右端上空にいるとき：右端の足場（X >= 5220）は安全、それより左（X < 5220）に咆哮が当たる
		if (!mbHasKnockedBackPlayer && mTimer >= 2 && mTimer <= 52 && player != nullptr && !player->GetIsDead())
		{
			float px = player->GetPosition().x;
			bool inRoarZone = mbTargetIsLeftEdge ? (px > LEFT_SAFE_MAX_X) : (px < RIGHT_SAFE_MIN_X);
			if (inRoarZone)
			{
				mbHasKnockedBackPlayer = true;
				if (gimmick != nullptr)
				{
					gimmick->ForceDetachPlayer(player);
				}
				// ダメージは0、空中からの咆哮の風圧でプレイヤーを大きく吹き飛ばす！
				mPlayerKnockbackVx = facingDir * 29.0f;
				mKnockbackTimer    = 40;
				player->SetInputDisabled(true);
				player->SetIsGrounded(false);
				player->SetVelocityY(-10.2f);
				player->SetFacingRight(facingDir < 0.0f);
			}
		}

		// 咆哮が終わるタイミングで、4つの台のうちランダムな台を1つ破壊する！（10秒経てば元に戻る）
		if (mTimer >= ROAR_BLAST_FRAMES)
		{
			if (blockMap != nullptr)
			{
				blockMap->mShakeX = 0;
				blockMap->mShakeY = 0;
			}
			if (gimmick != nullptr && blockMap != nullptr)
			{
				gimmick->DestroyRandomPlatform(*blockMap, player);
			}
			// 空中からの咆哮を終えたら端の足場へ着地するフェーズへ移行
			mPhase     = Phase::LandAtEdge;
			mTimer     = 0;
			mAnimTimer = 0;
			mAnimFrame = 0;
		}
		break;
	}

	case Phase::LandAtEdge:
		// 5. 空中での咆哮終了後、端の足場へ着陸する
		outAnimType = ANIM_LANDING;
		bossPos.x   = mTargetX;
		facingDir   = mbTargetIsLeftEdge ? 1.0f : -1.0f;

		if (!isBossGrounded && bossPos.y < mTargetFloorY - 100.0f)
		{
			bossPos.y += 16.0f;
			if (bossPos.y >= mTargetFloorY - 100.0f)
			{
				bossPos.y      = mTargetFloorY - 100.0f;
				isBossGrounded = true;
			}
			if (mAnimTimer >= 5 && mAnimFrame < 2)
			{
				mAnimTimer = 0;
				mAnimFrame++;
			}
		}
		else
		{
			bossPos.y      = mTargetFloorY - 100.0f;
			isBossGrounded = true;
			if (mAnimFrame < 3)
			{
				mAnimFrame = 3;
				mAnimTimer = 0;
			}
			if (mAnimTimer >= 4)
			{
				mAnimTimer = 0;
				mAnimFrame++;
				if (mAnimFrame >= 7)
				{
					mPhase = Phase::Finished;
					mTimer = 0;
				}
			}
		}
		outAnimFrame = (mAnimFrame < 7) ? mAnimFrame : 6;
		break;

	default:
		break;
	}
}

void Stage3DragonRoarAttack::Draw(BlockMap* blockMap, const VECTOR& bossPos, float facingDir) const
{
	if (blockMap == nullptr) return;
	if (mPhase == Phase::Inactive || mPhase == Phase::Finished) return;

	int drawScrollX = blockMap->GetScrollX() + blockMap->mShakeX;
	int drawScrollY = blockMap->GetScrollY() + blockMap->mShakeY;

	// 1. 3秒間の空中咆哮チャージ中：ドラゴンのいる足場側「以外」の危険範囲を赤く予告表示＆口元にエネルギー収束
	if (mPhase == Phase::Charge3Sec)
	{
		float ratio = static_cast<float>(mTimer) / static_cast<float>(CHARGE_DURATION_FRAMES);
		int pulseAlpha = 22 + static_cast<int>((std::sin(mTimer * 0.25f) * 0.5f + 0.5f) * 36.0f);

		// 【Stage3.png 5724x1692拡大版 対応】危険範囲の壁(3016..5690)・天井(483)・底(1477)座標を更新
		int zoneLeftMapX  = mbTargetIsLeftEdge ? static_cast<int>(LEFT_SAFE_MAX_X) : 3016;
		int zoneRightMapX = mbTargetIsLeftEdge ? 5690 : static_cast<int>(RIGHT_SAFE_MIN_X);
		int sx1 = zoneLeftMapX  - drawScrollX;
		int sx2 = zoneRightMapX - drawScrollX;

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, pulseAlpha);
		DrawBox(sx1, 483 - drawScrollY, sx2, 1477 - drawScrollY, GetColor(255, 40, 40), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		// 安全地帯（ドラゴンのいる足場側）との境界ライン表示
		int borderMapX = mbTargetIsLeftEdge ? static_cast<int>(LEFT_SAFE_MAX_X) : static_cast<int>(RIGHT_SAFE_MIN_X);
		int borderSX   = borderMapX - drawScrollX;
		DrawLine(borderSX, 483 - drawScrollY, borderSX, 1477 - drawScrollY, GetColor(255, 90, 90), 3);

		// 口元への赤熱エネルギー収束演出（二重収束リング＋10本スパーク＋脈動コア）
		int mouthSX = static_cast<int>(bossPos.x + facingDir * 135.0f) - drawScrollX;
		int mouthSY = static_cast<int>(bossPos.y - 72.0f) - drawScrollY;
		float ringT1 = static_cast<float>(mTimer % 28) / 28.0f;
		float ringT2 = static_cast<float>((mTimer + 14) % 28) / 28.0f;
		int ringR1 = static_cast<int>((1.0f - ringT1) * 96.0f + 12.0f);
		int ringR2 = static_cast<int>((1.0f - ringT2) * 96.0f + 12.0f);
		int coreR  = static_cast<int>(10.0f + ratio * 26.0f + std::sin(mTimer * 0.35f) * 3.0f);
		SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(135.0f + ratio * 105.0f));
		DrawCircle(mouthSX, mouthSY, coreR * 2 + 10, GetColor(255, 45, 20), FALSE, 2);
		DrawCircle(mouthSX, mouthSY, ringR1, GetColor(255, 85, 45), FALSE, 3);
		DrawCircle(mouthSX, mouthSY, ringR2, GetColor(255, 175, 75), FALSE, 2);
		for (int k = 0; k < 10; ++k)
		{
			float ang = mTimer * 0.09f + k * (3.14159265f * 0.2f);
			float phaseT = (k % 2 == 0) ? ringT1 : ringT2;
			float distOut = (1.0f - phaseT) * 105.0f + 16.0f;
			float distIn  = distOut * 0.42f;
			int x1 = mouthSX + static_cast<int>(std::cos(ang) * distOut);
			int y1 = mouthSY + static_cast<int>(std::sin(ang) * distOut);
			int x2 = mouthSX + static_cast<int>(std::cos(ang) * distIn);
			int y2 = mouthSY + static_cast<int>(std::sin(ang) * distIn);
			DrawLine(x1, y1, x2, y2, (k % 2 == 0) ? GetColor(255, 130, 60) : GetColor(255, 215, 110), 2);
		}
		DrawCircle(mouthSX, mouthSY, coreR + 8, GetColor(255, 65, 25), TRUE);
		DrawCircle(mouthSX, mouthSY, coreR,     GetColor(255, 185, 75), TRUE);
		DrawCircle(mouthSX, mouthSY, coreR / 2, GetColor(255, 250, 210), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// 1.5 咆哮炸裂時の前方音波リング
	if (mPhase == Phase::RoarBlast && mTimer < 42)
	{
		int mouthSX = static_cast<int>(bossPos.x + facingDir * 150.0f) - drawScrollX;
		int mouthSY = static_cast<int>(bossPos.y - 72.0f) - drawScrollY;
		for (int rIdx = 0; rIdx < 3; ++rIdx)
		{
			int localT = (mTimer + rIdx * 10) % 30;
			float rt = static_cast<float>(localT) / 30.0f;
			int rx = mouthSX + static_cast<int>(facingDir * (30.0f + rt * 260.0f));
			int radX = static_cast<int>(18.0f + rt * 48.0f);
			int radY = static_cast<int>(38.0f + rt * 140.0f);
			int a = static_cast<int>((1.0f - rt) * 210.0f);
			SetDrawBlendMode(DX_BLENDMODE_ADD, a);
			DrawOval(rx, mouthSY, radX, radY, GetColor(255, 110, 55), FALSE, 4);
			DrawOval(rx, mouthSY, static_cast<int>(radX * 0.8f), static_cast<int>(radY * 0.85f), GetColor(255, 220, 145), FALSE, 2);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// 2. 空中から吹き下ろす咆哮の暴風ライン描画
	for (const auto& wl : mWindLines)
	{
		float dirSign = (wl.vx >= 0.0f) ? 1.0f : -1.0f;
		int wx1 = static_cast<int>(wl.x) - drawScrollX;
		int wy1 = static_cast<int>(wl.y) - drawScrollY;
		int wx2 = static_cast<int>(wl.x + dirSign * wl.length) - drawScrollX;
		int wy2 = static_cast<int>(wl.y + wl.length * 0.16f) - drawScrollY;
		int alpha = (wl.life * 210) / wl.maxLife;
		SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
		DrawLine(wx1, wy1, wx2, wy2, GetColor(255, 90, 100), 3);
		DrawLine(wx1, wy1 - 2, wx2, wy2 - 2, GetColor(255, 215, 215), 1);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

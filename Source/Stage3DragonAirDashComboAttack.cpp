#include "Stage3DragonAirDashComboAttack.h"
#include "Player.h"
#include "BlockMap.h"
#include <cmath>
#include <cstdlib>

namespace
{
	float ConvertWorldToScreenX(float worldX, BlockMap* blockMap)
	{
		if (blockMap != nullptr)
		{
			return worldX - blockMap->GetScrollX() + static_cast<float>(blockMap->mShakeX);
		}
		return worldX;
	}

	float ConvertWorldToScreenY(float worldY, BlockMap* blockMap)
	{
		if (blockMap != nullptr)
		{
			return worldY - blockMap->GetScrollY() + static_cast<float>(blockMap->mShakeY);
		}
		return worldY;
	}
}

Stage3DragonAirDashComboAttack::Stage3DragonAirDashComboAttack()
	: mPhase(Phase::Inactive)
	, mPhaseTimer(0)
	, mAnimFrame(0)
	, mDashStartX(0.0f)
	, mDashStartY(0.0f)
	, mLockedPlayerX(4353.0f)
	, mLockedPlayerY(900.0f)
	, mDashGoalX(4353.0f)
	, mDashGoalY(900.0f)
	, mReturnToLeftLedge(false)
{
	Reset();
}

Stage3DragonAirDashComboAttack::~Stage3DragonAirDashComboAttack()
{
}

void Stage3DragonAirDashComboAttack::Initialize()
{
	Reset();
}

void Stage3DragonAirDashComboAttack::Reset()
{
	mPhase             = Phase::Inactive;
	mPhaseTimer        = 0;
	mAnimFrame         = 0;
	mDashStartX        = 0.0f;
	mDashStartY        = 0.0f;
	mLockedPlayerX     = 4353.0f;
	mLockedPlayerY     = 900.0f;
	mDashGoalX         = 4353.0f;
	mDashGoalY         = 900.0f;
	mReturnToLeftLedge = false;

	for (int i = 0; i < MAX_AFTERIMAGES; ++i)
	{
		mAfterimages[i].active = false;
		mAfterimages[i].alpha  = 0;
	}
}

void Stage3DragonAirDashComboAttack::StartAttack(VECTOR& bossPos, float& facingDir, Player* player)
{
	mPhase      = Phase::Takeoff;
	mPhaseTimer = 0;
	mAnimFrame  = 0;
	mDashStartX = bossPos.x;
	mDashStartY = bossPos.y;

	if (player != nullptr)
	{
		facingDir = (player->GetPosition().x >= bossPos.x) ? 1.0f : -1.0f;
	}

	for (int i = 0; i < MAX_AFTERIMAGES; ++i)
	{
		mAfterimages[i].active = false;
		mAfterimages[i].alpha  = 0;
	}
}

void Stage3DragonAirDashComboAttack::LockOnPlayerPosition(const VECTOR& bossPos, float& facingDir, Player* player)
{
	mDashStartX = bossPos.x;
	mDashStartY = bossPos.y;

	if (player != nullptr)
	{
		mLockedPlayerX = player->GetPosition().x;
		// プレイヤーの中心付近を狙う（ドラゴンの爪がちょうどプレイヤーの位置を通るように高さを合わせる）
		mLockedPlayerY = player->GetPosition().y - 30.0f;
	}
	else
	{
		mLockedPlayerX = 4353.0f;
		mLockedPlayerY = 920.0f;
	}

	// プレイヤーのいる方向を向く
	float dx  = mLockedPlayerX - bossPos.x;
	float dy  = mLockedPlayerY - bossPos.y;
	facingDir = (dx >= 0.0f) ? 1.0f : -1.0f;

	// プレイヤーの座標を少し駆け抜けるように突進終点を設定（最低でも一定距離はしっかり突進する）
	float dist = std::sqrt(dx * dx + dy * dy);
	if (dist < 1.0f)
	{
		dx   = facingDir;
		dy   = 0.0f;
		dist = 1.0f;
	}

	float nx = dx / dist;
	float ny = dy / dist;

	// 1回の突進でプレイヤーに向かって進める最大距離を短めに制限（強すぎないように最大 520px まで）
	float dashDistance = dist + 40.0f;
	if (dashDistance < 240.0f)
	{
		dashDistance = 240.0f;
	}
	if (dashDistance > 520.0f)
	{
		dashDistance = 520.0f;
	}

	mDashGoalX = bossPos.x + nx * dashDistance;
	mDashGoalY = bossPos.y + ny * dashDistance;

	// ボス部屋の壁・天井・マグマに埋まらないようにクランプ
	if (mDashGoalX < MIN_ARENA_X) mDashGoalX = MIN_ARENA_X;
	if (mDashGoalX > MAX_ARENA_X) mDashGoalX = MAX_ARENA_X;
	if (mDashGoalY < MIN_DASH_Y)  mDashGoalY = MIN_DASH_Y;
	if (mDashGoalY > MAX_DASH_Y)  mDashGoalY = MAX_DASH_Y;
}

void Stage3DragonAirDashComboAttack::PushAfterimage(const VECTOR& bossPos, float facingDir, int animFrame)
{
	(void)animFrame;
	for (int i = MAX_AFTERIMAGES - 1; i > 0; --i)
	{
		mAfterimages[i] = mAfterimages[i - 1];
	}
	mAfterimages[0].active    = true;
	mAfterimages[0].x         = bossPos.x;
	mAfterimages[0].y         = bossPos.y;
	mAfterimages[0].facingDir = facingDir;
	mAfterimages[0].alpha     = 150;
}

bool Stage3DragonAirDashComboAttack::IsSlashActive() const
{
	if (mPhase == Phase::DashAttack1 || mPhase == Phase::DashAttack2)
	{
		// 突進開始直後から斬撃振り抜き中（フレーム 3 ~ 7）に攻撃判定を有効にする
		return (mPhaseTimer >= 3 && mPhaseTimer <= 23);
	}
	return false;
}

void Stage3DragonAirDashComboAttack::Update(
	VECTOR& bossPos,
	float& facingDir,
	bool& isBossGrounded,
	int& outAnimType,
	int& outAnimFrame,
	Player* player,
	BlockMap* blockMap)
{
	if (mPhase == Phase::Inactive || mPhase == Phase::Finished)
	{
		return;
	}

	mPhaseTimer++;

	// 残像のフェードアウト更新
	for (int i = 0; i < MAX_AFTERIMAGES; ++i)
	{
		if (mAfterimages[i].active)
		{
			mAfterimages[i].alpha -= 14;
			if (mAfterimages[i].alpha <= 0)
			{
				mAfterimages[i].active = false;
				mAfterimages[i].alpha  = 0;
			}
		}
	}

	switch (mPhase)
	{
	// -----------------------------------------------------------------
	// 0. 足場から空中へ離陸
	// -----------------------------------------------------------------
	case Phase::Takeoff:
	{
		isBossGrounded = false;
		outAnimType    = ANIM_TAKEOFF;

		mAnimFrame = (mPhaseTimer * 7) / TAKEOFF_FRAMES;
		if (mAnimFrame > 6) mAnimFrame = 6;
		outAnimFrame = mAnimFrame;

		if (mPhaseTimer > 10)
		{
			bossPos.y -= 9.5f;
		}

		if (mPhaseTimer >= TAKEOFF_FRAMES)
		{
			mPhase      = Phase::FlyToReadyPos;
			mPhaseTimer = 0;
			mAnimFrame  = 0;
			mDashStartX = bossPos.x;
			mDashStartY = bossPos.y;
		}
		break;
	}

	// -----------------------------------------------------------------
	// 1. 空中待機ポジションへ移動（空中に飛んでいる状態を作る）
	// -----------------------------------------------------------------
	case Phase::FlyToReadyPos:
	{
		isBossGrounded = false;
		outAnimType    = ANIM_FLY;
		mAnimFrame     = (mPhaseTimer / 5) % 7;
		outAnimFrame   = mAnimFrame;

		// ステージ内側の見やすい空中位置へスッと移動
		float readyX = bossPos.x;
		if (readyX < 3850.0f) readyX = 3920.0f;
		if (readyX > 5150.0f) readyX = 5080.0f;

		float t    = static_cast<float>(mPhaseTimer) / static_cast<float>(FLY_READY_FRAMES);
		if (t > 1.0f) t = 1.0f;
		float ease = t * (2.0f - t);

		bossPos.x = mDashStartX + (readyX - mDashStartX) * ease;
		bossPos.y = mDashStartY + (AIR_READY_Y - mDashStartY) * ease;

		if (player != nullptr)
		{
			facingDir = (player->GetPosition().x >= bossPos.x) ? 1.0f : -1.0f;
		}

		if (mPhaseTimer >= FLY_READY_FRAMES)
		{
			// ① 1回目のプレイヤー座標を取得して溜め開始！
			LockOnPlayerPosition(bossPos, facingDir, player);
			mPhase      = Phase::Charge1;
			mPhaseTimer = 0;
			mAnimFrame  = 0;
		}
		break;
	}

	// -----------------------------------------------------------------
	// 2. 【1回目】プレイヤーの座標を取得し、空中で通常攻撃を溜める
	// -----------------------------------------------------------------
	case Phase::Charge1:
	{
		isBossGrounded = false;
		outAnimType    = ANIM_ATTACK;

		// 通常攻撃の溜めモーション（sprite_attack1(0) -> sprite_attack2(1) -> sprite_attack3(2) でタメ維持）
		if (mPhaseTimer < 12)      mAnimFrame = 0;
		else if (mPhaseTimer < 24) mAnimFrame = 1;
		else                       mAnimFrame = 2;
		outAnimFrame = mAnimFrame;

		// 突進の反動をつけるため、ロックオン方向と逆へじわっと引く（予備動作）
		float pullBackRatio = static_cast<float>(mPhaseTimer) / static_cast<float>(CHARGE1_FRAMES);
		float hoverBob      = std::sin(mPhaseTimer * 0.25f) * 6.0f;
		bossPos.x           = mDashStartX - facingDir * (42.0f * pullBackRatio);
		bossPos.y           = mDashStartY - (18.0f * pullBackRatio) + hoverBob;

		if (mPhaseTimer >= CHARGE1_FRAMES)
		{
			// 溜め完了時の位置を突進開始点として更新し、1回目の突進通常攻撃へ！
			mDashStartX = bossPos.x;
			mDashStartY = bossPos.y;
			mPhase      = Phase::DashAttack1;
			mPhaseTimer = 0;
		}
		break;
	}

	// -----------------------------------------------------------------
	// 3. 【1回目】取得した座標へ向かって突進通常攻撃！
	// -----------------------------------------------------------------
	case Phase::DashAttack1:
	{
		isBossGrounded = false;
		outAnimType    = ANIM_ATTACK;

		// 突進しながら通常攻撃の斬撃フレーム（sprite_attack4(3) ~ sprite_attack9(8)）を再生！
		if (mPhaseTimer < 4)       mAnimFrame = 3;
		else if (mPhaseTimer < 9)  mAnimFrame = 4;
		else if (mPhaseTimer < 15) mAnimFrame = 5;
		else if (mPhaseTimer < 20) mAnimFrame = 6;
		else if (mPhaseTimer < 24) mAnimFrame = 7;
		else                       mAnimFrame = 8;
		outAnimFrame = mAnimFrame;

		// 鋭い加速→減速イージングでロックオン座標へ一気に突進
		float t = static_cast<float>(mPhaseTimer) / static_cast<float>(DASH1_FRAMES);
		if (t > 1.0f) t = 1.0f;
		float ease = 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);

		bossPos.x = mDashStartX + (mDashGoalX - mDashStartX) * ease;
		bossPos.y = mDashStartY + (mDashGoalY - mDashStartY) * ease;

		if (mPhaseTimer % 2 == 0 && mPhaseTimer <= 22)
		{
			PushAfterimage(bossPos, facingDir, mAnimFrame);
		}

		// 斬撃振り抜き瞬間に軽い画面シェイクで迫力を出す
		if (mPhaseTimer >= 5 && mPhaseTimer <= 14 && blockMap != nullptr)
		{
			blockMap->mShakeX = GetRand(8) - 4;
			blockMap->mShakeY = GetRand(8) - 4;
		}
		else if (mPhaseTimer == 15 && blockMap != nullptr)
		{
			blockMap->mShakeX = 0;
			blockMap->mShakeY = 0;
		}

		if (mPhaseTimer >= DASH1_FRAMES)
		{
			if (blockMap != nullptr)
			{
				blockMap->mShakeX = 0;
				blockMap->mShakeY = 0;
			}
			// ② 1回目の突進が終わったら、再びプレイヤーの現在座標を取得して2回目の溜めへ！
			LockOnPlayerPosition(bossPos, facingDir, player);
			mPhase      = Phase::Charge2;
			mPhaseTimer = 0;
			mAnimFrame  = 0;
		}
		break;
	}

	// -----------------------------------------------------------------
	// 4. 【2回目】再びプレイヤーの現在座標を取得し、空中で通常攻撃を溜める
	// -----------------------------------------------------------------
	case Phase::Charge2:
	{
		isBossGrounded = false;
		outAnimType    = ANIM_ATTACK;

		// 再び通常攻撃の溜めモーション（sprite_attack1(0) -> sprite_attack2(1) -> sprite_attack3(2)）
		if (mPhaseTimer < 10)      mAnimFrame = 0;
		else if (mPhaseTimer < 20) mAnimFrame = 1;
		else                       mAnimFrame = 2;
		outAnimFrame = mAnimFrame;

		// 2回目の突進予備動作（ロックオン方向と逆へ少し引きつつ少し浮き上がる）
		float pullBackRatio = static_cast<float>(mPhaseTimer) / static_cast<float>(CHARGE2_FRAMES);
		float hoverBob      = std::sin(mPhaseTimer * 0.28f) * 6.0f;
		bossPos.x           = mDashStartX - facingDir * (46.0f * pullBackRatio);
		bossPos.y           = mDashStartY - (26.0f * pullBackRatio) + hoverBob;
		if (bossPos.y < MIN_DASH_Y) bossPos.y = MIN_DASH_Y;

		if (mPhaseTimer >= CHARGE2_FRAMES)
		{
			// 溜め完了時の位置から2回目の突進通常攻撃へ！
			mDashStartX = bossPos.x;
			mDashStartY = bossPos.y;
			mPhase      = Phase::DashAttack2;
			mPhaseTimer = 0;
		}
		break;
	}

	// -----------------------------------------------------------------
	// 5. 【2回目】取得した座標へ向かって2回目の突進通常攻撃！
	// -----------------------------------------------------------------
	case Phase::DashAttack2:
	{
		isBossGrounded = false;
		outAnimType    = ANIM_ATTACK;

		// 2回目の突進斬撃フレーム（sprite_attack4(3) ~ sprite_attack9(8)）
		if (mPhaseTimer < 4)       mAnimFrame = 3;
		else if (mPhaseTimer < 9)  mAnimFrame = 4;
		else if (mPhaseTimer < 15) mAnimFrame = 5;
		else if (mPhaseTimer < 20) mAnimFrame = 6;
		else if (mPhaseTimer < 24) mAnimFrame = 7;
		else                       mAnimFrame = 8;
		outAnimFrame = mAnimFrame;

		float t = static_cast<float>(mPhaseTimer) / static_cast<float>(DASH2_FRAMES);
		if (t > 1.0f) t = 1.0f;
		float ease = 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);

		bossPos.x = mDashStartX + (mDashGoalX - mDashStartX) * ease;
		bossPos.y = mDashStartY + (mDashGoalY - mDashStartY) * ease;

		if (mPhaseTimer % 2 == 0 && mPhaseTimer <= 22)
		{
			PushAfterimage(bossPos, facingDir, mAnimFrame);
		}

		if (mPhaseTimer >= 5 && mPhaseTimer <= 15 && blockMap != nullptr)
		{
			blockMap->mShakeX = GetRand(10) - 5;
			blockMap->mShakeY = GetRand(10) - 5;
		}
		else if (mPhaseTimer == 16 && blockMap != nullptr)
		{
			blockMap->mShakeX = 0;
			blockMap->mShakeY = 0;
		}

		if (mPhaseTimer >= DASH2_FRAMES)
		{
			if (blockMap != nullptr)
			{
				blockMap->mShakeX = 0;
				blockMap->mShakeY = 0;
			}
			// 2連続突進が完了したら、近い方の足場（左端または右端）へ戻って着陸する
			float distLeft     = std::fabs(bossPos.x - LEFT_LEDGE_STAND_X);
			float distRight    = std::fabs(bossPos.x - RIGHT_LEDGE_STAND_X);
			mReturnToLeftLedge = (distLeft < distRight);

			mPhase      = Phase::FlyToLedge;
			mPhaseTimer = 0;
			mAnimFrame  = 0;
		}
		break;
	}

	// -----------------------------------------------------------------
	// 6. 足場の上空へ飛行移動
	// -----------------------------------------------------------------
	case Phase::FlyToLedge:
	{
		isBossGrounded = false;
		outAnimType    = ANIM_FLY;
		mAnimFrame     = (mPhaseTimer / 5) % 7;
		outAnimFrame   = mAnimFrame;

		float targetX = mReturnToLeftLedge ? LEFT_LEDGE_STAND_X : RIGHT_LEDGE_STAND_X;
		float targetY = AIR_READY_Y;

		float dx   = targetX - bossPos.x;
		float dy   = targetY - bossPos.y;
		float dist = std::sqrt(dx * dx + dy * dy);

		if ( std::fabs(dx) > 4.0f )
		{
			facingDir = (dx >= 0.0f) ? 1.0f : -1.0f;
		}

		const float flySpeed = 24.0f;
		if (dist <= flySpeed)
		{
			bossPos.x   = targetX;
			bossPos.y   = targetY;
			mPhase      = Phase::LandAtLedge;
			mPhaseTimer = 0;
			mAnimFrame  = 0;
		}
		else
		{
			bossPos.x += (dx / dist) * flySpeed;
			bossPos.y += (dy / dist) * flySpeed;
		}
		break;
	}

	// -----------------------------------------------------------------
	// 7. 足場へ着陸して完了
	// -----------------------------------------------------------------
	case Phase::LandAtLedge:
	{
		isBossGrounded = false;
		outAnimType    = ANIM_LANDING;
		mAnimFrame     = (mPhaseTimer / 4) % 7;
		outAnimFrame   = mAnimFrame;

		float floorY = mReturnToLeftLedge ? LEFT_LEDGE_FLOOR_Y : RIGHT_LEDGE_FLOOR_Y;
		float standY = floorY - BOSS_HALF_HEIGHT;

		bossPos.y += 14.0f;
		if (bossPos.y >= standY)
		{
			bossPos.y      = standY;
			isBossGrounded = true;
			// ステージ中央側を向く
			facingDir      = mReturnToLeftLedge ? 1.0f : -1.0f;
			mPhase         = Phase::Finished;
			mPhaseTimer    = 0;
		}
		break;
	}

	default:
		break;
	}
}

void Stage3DragonAirDashComboAttack::Draw(
	BlockMap* blockMap,
	const VECTOR& bossPos,
	float facingDir,
	float drawScale,
	int currentSpriteHandle,
	float footOffset,
	float enemyHeight) const
{
	if (!IsActive()) return;

	float bossSx = ConvertWorldToScreenX(bossPos.x, blockMap);
	float bossSy = ConvertWorldToScreenY(bossPos.y, blockMap);

	// -----------------------------------------------------------------
	// 1. 溜め中（Charge1 / Charge2）：ロックオンしたプレイヤー座標への突進予告ライン＆溜めオーラ
	// -----------------------------------------------------------------
	if (mPhase == Phase::Charge1 || mPhase == Phase::Charge2)
	{
		int maxChargeFrames = (mPhase == Phase::Charge1) ? CHARGE1_FRAMES : CHARGE2_FRAMES;
		float chargeRatio   = static_cast<float>(mPhaseTimer) / static_cast<float>(maxChargeFrames);
		if (chargeRatio > 1.0f) chargeRatio = 1.0f;

		float targetSx = ConvertWorldToScreenX(mLockedPlayerX, blockMap);
		float targetSy = ConvertWorldToScreenY(mLockedPlayerY, blockMap);
		float goalSx   = ConvertWorldToScreenX(mDashGoalX, blockMap);
		float goalSy   = ConvertWorldToScreenY(mDashGoalY, blockMap);

		// 突進軌道の赤い予告ライン
		int lineAlpha = 70 + static_cast<int>(120.0f * chargeRatio);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, lineAlpha);
		for (int offset = -3; offset <= 3; ++offset)
		{
			DrawLine(
				static_cast<int>(bossSx),
				static_cast<int>(bossSy) + offset,
				static_cast<int>(goalSx),
				static_cast<int>(goalSy) + offset,
				GetColor(255, 70, 40)
			);
		}

		// 取得したプレイヤー座標にロックオンサイト（照準リング）を描画
		float ringRadius = 56.0f - 28.0f * chargeRatio;
		DrawCircle(static_cast<int>(targetSx), static_cast<int>(targetSy), static_cast<int>(ringRadius), GetColor(255, 90, 50), FALSE);
		DrawCircle(static_cast<int>(targetSx), static_cast<int>(targetSy), static_cast<int>(ringRadius + 3.0f), GetColor(255, 200, 90), FALSE);
		DrawLine(static_cast<int>(targetSx - 22.0f), static_cast<int>(targetSy), static_cast<int>(targetSx + 22.0f), static_cast<int>(targetSy), GetColor(255, 220, 120));
		DrawLine(static_cast<int>(targetSx), static_cast<int>(targetSy - 22.0f), static_cast<int>(targetSx), static_cast<int>(targetSy + 22.0f), GetColor(255, 220, 120));

		// ドラゴンの周囲に収束する赤い闘気オーラ
		SetDrawBlendMode(DX_BLENDMODE_ADD, 130);
		int auraRadius = static_cast<int>(115.0f * (1.15f - 0.55f * chargeRatio));
		DrawCircle(static_cast<int>(bossSx), static_cast<int>(bossSy), auraRadius, GetColor(255, 80, 30), FALSE);
		DrawCircle(static_cast<int>(bossSx), static_cast<int>(bossSy), auraRadius - 6, GetColor(255, 190, 80), FALSE);

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// -----------------------------------------------------------------
	// 2. 突進通常攻撃中（DashAttack1 / DashAttack2）：残像と爪斬撃エフェクト
	// -----------------------------------------------------------------
	if (currentSpriteHandle != -1)
	{
		for (int i = MAX_AFTERIMAGES - 1; i >= 0; --i)
		{
			if (!mAfterimages[i].active || mAfterimages[i].alpha <= 0) continue;

			int   sx         = static_cast<int>(ConvertWorldToScreenX(mAfterimages[i].x, blockMap));
			float feetWorldY = mAfterimages[i].y + (enemyHeight * 0.5f);
			int   sy         = static_cast<int>(ConvertWorldToScreenY(feetWorldY - (footOffset * drawScale), blockMap));
			bool  reverse    = (mAfterimages[i].facingDir > 0.0f);

			SetDrawBlendMode(DX_BLENDMODE_ADD, mAfterimages[i].alpha);
			SetDrawBright(255, 110, 60);
			DrawRotaGraph(sx, sy, drawScale, 0.0, currentSpriteHandle, TRUE, reverse);
		}
		SetDrawBright(255, 255, 255);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	if (IsSlashActive())
	{
		// 前方の爪に赤い斬撃衝撃エフェクトを描画
		float slashCenterX = bossSx + facingDir * 115.0f;
		float slashCenterY = bossSy + 10.0f;

		SetDrawBlendMode(DX_BLENDMODE_ADD, 175);
		for (int i = 0; i < 4; ++i)
		{
			float offsetX = facingDir * (i * 16.0f);
			float offsetY = (i - 1.5f) * 22.0f;
			DrawLine(
				static_cast<int>(slashCenterX - facingDir * 55.0f + offsetX),
				static_cast<int>(slashCenterY - 65.0f + offsetY),
				static_cast<int>(slashCenterX + facingDir * 55.0f + offsetX),
				static_cast<int>(slashCenterY + 65.0f + offsetY),
				GetColor(255, 160 - i * 25, 60)
			);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

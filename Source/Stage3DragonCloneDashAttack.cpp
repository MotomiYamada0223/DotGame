#include "Stage3DragonCloneDashAttack.h"
#include "Effekseer2DManager.h"
#include "BlockMap.h"
#include <cstdio>
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

	float GetSlotY(int slotIndex)
	{
		switch (slotIndex)
		{
		case 0:  return 550.0f;  // 上
		case 1:  return 795.0f;  // 真ん中
		default: return 1036.0f; // 下
		}
	}
}

Stage3DragonCloneDashAttack::Stage3DragonCloneDashAttack()
	: mPhase(Phase::Inactive)
	, mPhaseTimer(0)
	, mAnimType(ANIM_IDLE)
	, mAnimFrame(0)
	, mCurrentWave(0)
	, mStartFromLeft(false)
	, mRealDragonSlot(1)
	, mMoveStartX(0.0f)
	, mMoveStartY(0.0f)
	, mStartX(RIGHT_LEDGE_STAND_X)
	, mEndX(LEFT_LEDGE_STAND_X)
	, mCurrentDashX(RIGHT_LEDGE_STAND_X)
	, mCloneAlpha(0)
	, mCloneEyeGlintTimer(0)
	, mDashStreakEfk(-1)
{
	mCloneSlots[0]    = 0;
	mCloneSlots[1]    = 2;
	mCloneStreakEfk[0] = -1;
	mCloneStreakEfk[1] = -1;
	Reset();
}

Stage3DragonCloneDashAttack::~Stage3DragonCloneDashAttack()
{
	ReleaseCloneAnimations();
}

void Stage3DragonCloneDashAttack::Initialize()
{
	LoadCloneAnimations();
	Reset();
}

void Stage3DragonCloneDashAttack::Reset()
{
	mPhase          = Phase::Inactive;
	mPhaseTimer     = 0;
	mAnimType       = ANIM_IDLE;
	mAnimFrame      = 0;
	mCurrentWave    = 0;
	mStartFromLeft  = false;
	mRealDragonSlot = 1;
	mCloneSlots[0]  = 0;
	mCloneSlots[1]  = 2;
	mMoveStartX     = 0.0f;
	mMoveStartY     = 0.0f;
	mStartX         = RIGHT_LEDGE_STAND_X;
	mEndX           = LEFT_LEDGE_STAND_X;
	mCurrentDashX   = RIGHT_LEDGE_STAND_X;
	mCloneAlpha     = 0;

	for (int i = 0; i < MAX_TRAIL_POINTS; ++i)
	{
		mTrailPoints[i].active = false;
		mTrailPoints[i].alpha  = 0;
	}
	mSpeedLines.clear();
	if (mDashStreakEfk != -1)
	{
		Effekseer2DManager::Stop(mDashStreakEfk);
		mDashStreakEfk = -1;
	}
	for (int i = 0; i < 2; ++i)
	{
		if (mCloneStreakEfk[i] != -1)
		{
			Effekseer2DManager::Stop(mCloneStreakEfk[i]);
			mCloneStreakEfk[i] = -1;
		}
	}
}

void Stage3DragonCloneDashAttack::ReleaseCloneAnimations()
{
	for (int h : mCloneFlyAnim)    if (h != -1) DeleteGraph(h);
	for (int h : mCloneAttackAnim) if (h != -1) DeleteGraph(h);
	mCloneFlyAnim.clear();
	mCloneAttackAnim.clear();
}

// 本体（赤い目）に対して、分身は目のピクセルを鮮やかな「黄金～エメラルドグリーン」に書き換えたグラフィックを生成する
int Stage3DragonCloneDashAttack::LoadGraphWithRecoloredEyes(const char* filePath)
{
	int softImg = LoadSoftImage(filePath);
	if (softImg == -1)
	{
		return LoadGraph(filePath);
	}

	int w = 0, h = 0;
	GetSoftImageSize(softImg, &w, &h);

	std::vector< std::pair<int, int> > eyePixels;
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			int r = 0, g = 0, b = 0, a = 0;
			GetPixelSoftImage(softImg, x, y, &r, &g, &b, &a);
			if (a > 120 && r > g + 30 && r > b + 15 && r > 110)
			{
				eyePixels.push_back(std::make_pair(x, y));
			}
		}
	}

	for (size_t i = 0; i < eyePixels.size(); ++i)
	{
		int ex = eyePixels[i].first;
		int ey = eyePixels[i].second;
		for (int dy = -1; dy <= 1; ++dy)
		{
			for (int dx = -1; dx <= 1; ++dx)
			{
				int nx = ex + dx;
				int ny = ey + dy;
				if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;

				int r = 0, g = 0, b = 0, a = 0;
				GetPixelSoftImage(softImg, nx, ny, &r, &g, &b, &a);
				if (a > 80)
				{
					if (dx == 0 && dy == 0)
					{
						DrawPixelSoftImage(softImg, nx, ny, 80, 255, 180, a);
					}
					else
					{
						DrawPixelSoftImage(softImg, nx, ny, 40, 240, 140, a);
					}
				}
			}
		}
	}

	int graphHandle = CreateGraphFromSoftImage(softImg);
	DeleteSoftImage(softImg);
	return graphHandle;
}

void Stage3DragonCloneDashAttack::LoadCloneAnimations()
{
	ReleaseCloneAnimations();

	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_flying/sprite_fly%d.png", i);
		int h = LoadGraphWithRecoloredEyes(path);
		if (h != -1) mCloneFlyAnim.push_back(h);
	}

	for (int i = 1; i <= 9; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_attack/sprite_attack%d.png", i);
		int h = LoadGraphWithRecoloredEyes(path);
		if (h != -1) mCloneAttackAnim.push_back(h);
	}
}

void Stage3DragonCloneDashAttack::StartAttack(VECTOR& bossPos, float& facingDir)
{
	mPhase        = Phase::Takeoff;
	mPhaseTimer   = 0;
	mAnimType     = ANIM_TAKEOFF;
	mAnimFrame    = 0;
	mCurrentWave  = 0;
	mCloneAlpha   = 0;
	mCloneEyeGlintTimer = 0;

	for (int i = 0; i < MAX_TRAIL_POINTS; ++i)
	{
		mTrailPoints[i].active = false;
		mTrailPoints[i].alpha  = 0;
	}
	mSpeedLines.clear();
	if (mDashStreakEfk != -1)
	{
		Effekseer2DManager::Stop(mDashStreakEfk);
		mDashStreakEfk = -1;
	}
	for (int i = 0; i < 2; ++i)
	{
		if (mCloneStreakEfk[i] != -1)
		{
			Effekseer2DManager::Stop(mCloneStreakEfk[i]);
			mCloneStreakEfk[i] = -1;
		}
	}

	float distLeft  = std::fabs(bossPos.x - LEFT_LEDGE_STAND_X);
	float distRight = std::fabs(bossPos.x - RIGHT_LEDGE_STAND_X);
	mStartFromLeft  = (distLeft < distRight);

	if (mStartFromLeft)
	{
		mStartX   = LEFT_LEDGE_STAND_X;
		mEndX     = RIGHT_LEDGE_STAND_X;
		facingDir = 1.0f;
	}
	else
	{
		mStartX   = RIGHT_LEDGE_STAND_X;
		mEndX     = LEFT_LEDGE_STAND_X;
		facingDir = -1.0f;
	}

	mCurrentDashX = mStartX;

	// 1回目の上・真ん中・下配置をランダムにシャッフル
	ShuffleSlots();

	mMoveStartX = bossPos.x;
	mMoveStartY = bossPos.y;
}

void Stage3DragonCloneDashAttack::ShuffleSlots()
{
	// 上(0)・真ん中(1)・下(2) のうち、本体が入るレーンをランダムに決定し、残り2つを分身2体に割り当てる
	mRealDragonSlot = GetRand(2);
	int cloneIdx = 0;
	for (int s = 0; s < LANE_COUNT; ++s)
	{
		if (s != mRealDragonSlot && cloneIdx < 2)
		{
			mCloneSlots[cloneIdx++] = s;
		}
	}
}

void Stage3DragonCloneDashAttack::PushTrailPoint(float x)
{
	for (int i = MAX_TRAIL_POINTS - 1; i > 0; --i)
	{
		mTrailPoints[i] = mTrailPoints[i - 1];
	}
	mTrailPoints[0].active = true;
	mTrailPoints[0].x      = x;
	mTrailPoints[0].alpha  = 175;
}

void Stage3DragonCloneDashAttack::UpdateEffects(float facingDir)
{
	(void)facingDir;

	// 残像の減衰
	for (int i = 0; i < MAX_TRAIL_POINTS; ++i)
	{
		if (mTrailPoints[i].active)
		{
			mTrailPoints[i].alpha -= 20;
			if (mTrailPoints[i].alpha <= 0)
			{
				mTrailPoints[i].active = false;
				mTrailPoints[i].alpha  = 0;
			}
		}
	}

	// 高速スピードラインの更新
	for (int i = static_cast<int>(mSpeedLines.size()) - 1; i >= 0; --i)
	{
		mSpeedLines[i].x += mSpeedLines[i].vx;
		mSpeedLines[i].life--;
		if (mSpeedLines[i].life <= 0)
		{
			mSpeedLines.erase(mSpeedLines.begin() + i);
		}
	}
}

bool Stage3DragonCloneDashAttack::IsSlashActive() const
{
	if (mPhase == Phase::SimultaneousDash)
	{
		return (mPhaseTimer >= 3 && mPhaseTimer <= DASH_FRAMES - 3);
	}
	return false;
}

void Stage3DragonCloneDashAttack::Update(
	VECTOR& bossPos,
	float& facingDir,
	bool& isBossGrounded,
	int& outAnimType,
	int& outAnimFrame,
	BlockMap* blockMap)
{
	if (mPhase == Phase::Inactive || mPhase == Phase::Finished)
	{
		return;
	}

	mPhaseTimer++;
	UpdateEffects(facingDir);

	if (IsEyeGlintMoment())
	{
		mCloneEyeGlintTimer = 16;
	}
	else if (mCloneEyeGlintTimer > 0)
	{
		mCloneEyeGlintTimer--;
	}

	switch (mPhase)
	{
	// -----------------------------------------------------------------
	// 0. 足場から離陸
	// -----------------------------------------------------------------
	case Phase::Takeoff:
	{
		isBossGrounded = false;
		mAnimType      = ANIM_TAKEOFF;
		mAnimFrame     = (mPhaseTimer * 7) / TAKEOFF_FRAMES;
		if (mAnimFrame > 6) mAnimFrame = 6;

		if (mPhaseTimer > 8)
		{
			bossPos.y -= 8.0f;
		}

		if (mPhaseTimer >= TAKEOFF_FRAMES)
		{
			mPhase      = Phase::MoveToStartSlot;
			mPhaseTimer = 0;
			mAnimFrame  = 0;
			mMoveStartX = bossPos.x;
			mMoveStartY = bossPos.y;
		}
		break;
	}

	// -----------------------------------------------------------------
	// 1. 開始サイドの本体配置高さ（上・真ん中・下のどれか）へ移動
	// -----------------------------------------------------------------
	case Phase::MoveToStartSlot:
	{
		isBossGrounded = false;
		mAnimType      = ANIM_FLY;
		mAnimFrame     = (mPhaseTimer / 5) % 7;
		facingDir      = mStartFromLeft ? 1.0f : -1.0f;

		float targetX = mStartX;
		float targetY = GetSlotY(mRealDragonSlot);

		float t = static_cast<float>(mPhaseTimer) / static_cast<float>(MOVE_SLOT_FRAMES);
		if (t > 1.0f) t = 1.0f;
		float ease = t * (2.0f - t);

		bossPos.x     = mMoveStartX + (targetX - mMoveStartX) * ease;
		bossPos.y     = mMoveStartY + (targetY - mMoveStartY) * ease;
		mCurrentDashX = bossPos.x;

		if (mPhaseTimer >= MOVE_SLOT_FRAMES)
		{
			bossPos.x     = targetX;
			bossPos.y     = targetY;
			mCurrentDashX = targetX;
			mPhase        = Phase::SummonAndCharge;
			mPhaseTimer   = 0;
			mAnimFrame    = 0;
			mCloneAlpha   = 0;
	mCloneEyeGlintTimer = 0;
		}
		break;
	}

	// -----------------------------------------------------------------
	// 2. 分身体2体（目の色が違う）を上・真ん中・下の空きレーンに出現させ、3体同時に通常攻撃を溜める
	// -----------------------------------------------------------------
	case Phase::SummonAndCharge:
	{
		isBossGrounded = false;
		mAnimType      = ANIM_ATTACK;
		facingDir      = mStartFromLeft ? 1.0f : -1.0f;

		// 分身がスッと出現（255までフェードイン）
		if (mPhaseTimer <= 15)
		{
			mCloneAlpha = (mPhaseTimer * 255) / 15;
		}
		else
		{
			mCloneAlpha = 255;
		}

		// 通常攻撃の溜めフレーム（sprite_attack1(0) -> sprite_attack2(1) -> sprite_attack3(2)）
		if (mPhaseTimer < 16)      mAnimFrame = 0;
		else if (mPhaseTimer < 32) mAnimFrame = 1;
		else                       mAnimFrame = 2;

		// 3体同時に突進の予備動作でグッと後ろへ身を引く
		float pullRatio = static_cast<float>(mPhaseTimer) / static_cast<float>(SUMMON_CHARGE_FRAMES);
		mCurrentDashX   = mStartX - facingDir * (54.0f * pullRatio);
		bossPos.x       = mCurrentDashX;
		bossPos.y       = GetSlotY(mRealDragonSlot) + std::sin(mPhaseTimer * 0.28f) * 5.0f;

		// 溜め中に前方から3体のドラゴンへ向かって大気が吸い込まれる風圧ライン生成
		if (mPhaseTimer % 2 == 0)
		{
			for (int lane = 0; lane < LANE_COUNT; ++lane)
			{
				SpeedLine sl;
				sl.x         = mCurrentDashX + facingDir * (180.0f + static_cast<float>(GetRand(320)));
				sl.y         = GetSlotY(lane) + static_cast<float>(GetRand(110) - 55);
				sl.vx        = -facingDir * (22.0f + static_cast<float>(GetRand(14)));
				sl.length    = 65.0f + static_cast<float>(GetRand(65));
				sl.thickness = 2;
				sl.maxLife   = 10;
				sl.life      = 10;
				sl.laneIdx   = lane;
				sl.color     = (lane == mRealDragonSlot) ? GetColor(255, 110, 80) : GetColor(90, 255, 200);
				mSpeedLines.push_back(sl);
			}
		}

		if (mPhaseTimer >= SUMMON_CHARGE_FRAMES)
		{
			mPhase      = Phase::SimultaneousDash;
			mPhaseTimer = 0;
			mMoveStartX = mCurrentDashX;

			// 突進発射の瞬間：魔法陣リングは出さず、本体・分身の追従スピード斬撃ストリークのみ再生
			float dashRotZ = (facingDir >= 0.0f) ? 0.0f : 3.14159265f;
			if (mDashStreakEfk != -1)
			{
				Effekseer2DManager::Stop(mDashStreakEfk);
				mDashStreakEfk = -1;
			}
			for (int i = 0; i < 2; ++i)
			{
				if (mCloneStreakEfk[i] != -1)
				{
					Effekseer2DManager::Stop(mCloneStreakEfk[i]);
					mCloneStreakEfk[i] = -1;
				}
			}
			mDashStreakEfk = Effekseer2DManager::Play(
				Effect2DId::Stage3_FlameBurst,
				mCurrentDashX + facingDir * 95.0f,
				GetSlotY(mRealDragonSlot),
				1.45f,
				2.20f,
				dashRotZ,
				255, 205, 80, 245
			);
			for (int i = 0; i < 2; ++i)
			{
				mCloneStreakEfk[i] = Effekseer2DManager::Play(
					Effect2DId::Stage3_SlashWind,
					mCurrentDashX + facingDir * 90.0f,
					GetSlotY(mCloneSlots[i]),
					1.25f,
					2.20f,
					dashRotZ,
					85, 255, 220, 230
				);
			}
		}
		break;
	}

	// -----------------------------------------------------------------
	// 3. 3体同時に横方向へ向かい側の地面まで超高速突進攻撃！
	//    分身は当たり判定がなく、攻撃しながら徐々にフェードしていき、攻撃が終わると完全に消える
	// -----------------------------------------------------------------
	case Phase::SimultaneousDash:
	{
		isBossGrounded = false;
		mAnimType      = ANIM_ATTACK;
		facingDir      = mStartFromLeft ? 1.0f : -1.0f;

		// 突進進行度 (0.0 -> 1.0)
		float t = static_cast<float>(mPhaseTimer) / static_cast<float>(DASH_FRAMES);
		if (t > 1.0f) t = 1.0f;

		// 初速が鋭く爆発的な疾走感が出るイージング（1 - (1-t)^3）
		float ease = 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
		mCurrentDashX = mMoveStartX + (mEndX - mMoveStartX) * ease;

		bossPos.x = mCurrentDashX;
		bossPos.y = GetSlotY(mRealDragonSlot);

		// 毎フレーム残像座標を記録
		PushTrailPoint(mCurrentDashX);

		// 2D Effekseer ストリークを本体＆分身2体の突進位置に毎フレーム追従させて超高速トレイルを描画
		float dashRotZ = (facingDir >= 0.0f) ? 0.0f : 3.14159265f;
		if (mDashStreakEfk != -1 && Effekseer2DManager::IsPlaying(mDashStreakEfk))
		{
			Effekseer2DManager::SetTransform(
				mDashStreakEfk,
				mCurrentDashX + facingDir * 95.0f,
				GetSlotY(mRealDragonSlot),
				1.45f,
				dashRotZ
			);
		}
		for (int i = 0; i < 2; ++i)
		{
			if (mCloneStreakEfk[i] != -1 && Effekseer2DManager::IsPlaying(mCloneStreakEfk[i]))
			{
				Effekseer2DManager::SetTransform(
					mCloneStreakEfk[i],
					mCurrentDashX + facingDir * 90.0f,
					GetSlotY(mCloneSlots[i]),
					1.25f,
					dashRotZ
				);
			}
		}

		// 中盤の超音速黄金雷スパーク（軽量化のため本体に1発のみ）
		if (mPhaseTimer == 12)
		{
			Effekseer2DManager::Play(
				Effect2DId::Stage3_Thunder,
				mCurrentDashX + facingDir * 105.0f,
				GetSlotY(mRealDragonSlot),
				1.30f,
				2.05f,
				dashRotZ,
				255, 235, 95, 240
			);
		}

		// 3レーンに超軽量な高速スピードラインを間引き生成（4フレームごと）
		if (mPhaseTimer % 4 == 0)
		{
			for (int lane = 0; lane < LANE_COUNT; ++lane)
			{
				float laneY = GetSlotY(lane);
				bool isRealLane = (lane == mRealDragonSlot);
				SpeedLine sl;
				sl.x         = mCurrentDashX + facingDir * static_cast<float>(GetRand(120) - 60);
				sl.y         = laneY + static_cast<float>(GetRand(130) - 65);
				sl.vx        = -facingDir * (44.0f + static_cast<float>(GetRand(24)));
				sl.length    = 200.0f + static_cast<float>(GetRand(160));
				sl.thickness = 2;
				sl.maxLife   = 9 + GetRand(4);
				sl.life      = sl.maxLife;
				sl.laneIdx   = lane;
				sl.color     = isRealLane ? GetColor(255, 200, 95) : GetColor(130, 255, 215);
				mSpeedLines.push_back(sl);
			}
		}

		// 通常攻撃の斬撃フレーム（sprite_attack4(3) ~ sprite_attack9(8)）を再生
		if (mPhaseTimer < 6)       mAnimFrame = 3;
		else if (mPhaseTimer < 13) mAnimFrame = 4;
		else if (mPhaseTimer < 21) mAnimFrame = 5;
		else if (mPhaseTimer < 29) mAnimFrame = 6;
		else if (mPhaseTimer < 36) mAnimFrame = 7;
		else                       mAnimFrame = 8;

		// 分身は攻撃しながら徐々にフェードアウトしていき、突進終了時点で完全に消える（alpha: 255 -> 0）
		mCloneAlpha = static_cast<int>(255.0f * (1.0f - t));
		if (mCloneAlpha < 0) mCloneAlpha = 0;

		if (mPhaseTimer >= 3 && mPhaseTimer <= 32 && blockMap != nullptr)
		{
			blockMap->mShakeX = GetRand(10) - 5;
			blockMap->mShakeY = GetRand(10) - 5;
		}
		else if (blockMap != nullptr)
		{
			blockMap->mShakeX = 0;
			blockMap->mShakeY = 0;
		}

		if (mPhaseTimer >= DASH_FRAMES)
		{
			if (blockMap != nullptr)
			{
				blockMap->mShakeX = 0;
				blockMap->mShakeY = 0;
			}
			// 1回の突進終了時に分身は一旦完全に消滅！
			mCloneAlpha = 0;
			bossPos.x   = mEndX;

			for (int i = 0; i < MAX_TRAIL_POINTS; ++i)
			{
				mTrailPoints[i].active = false;
				mTrailPoints[i].alpha  = 0;
			}

			// 1回目の突進が終わったら、再び上・真ん中・下をランダムにシャッフルして反対側へ2回目の突進攻撃！
			if (mCurrentWave == 0)
			{
				mCurrentWave   = 1;
				mStartFromLeft = !mStartFromLeft;
				if (mStartFromLeft)
				{
					mStartX   = LEFT_LEDGE_STAND_X;
					mEndX     = RIGHT_LEDGE_STAND_X;
					facingDir = 1.0f;
				}
				else
				{
					mStartX   = RIGHT_LEDGE_STAND_X;
					mEndX     = LEFT_LEDGE_STAND_X;
					facingDir = -1.0f;
				}
				mCurrentDashX = mStartX;

				// もう一度ランダムにシャッフル！
				ShuffleSlots();

				mMoveStartX = bossPos.x;
				mMoveStartY = bossPos.y;
				mPhase      = Phase::MoveToStartSlot;
				mPhaseTimer = 0;
				mAnimFrame  = 0;
			}
			else
			{
				// 2回目の突進攻撃も完了したら向かい側の地面へ着陸して終了
				mPhase      = Phase::LandOnOppositeLedge;
				mPhaseTimer = 0;
				mAnimFrame  = 0;
			}
		}
		break;
	}

	// -----------------------------------------------------------------
	// 4. 向かい側の地面（足場）へ本体が着陸して完了
	// -----------------------------------------------------------------
	case Phase::LandOnOppositeLedge:
	{
		isBossGrounded = false;
		mAnimType      = ANIM_LANDING;
		mAnimFrame     = (mPhaseTimer / 4) % 7;
		mCloneAlpha    = 0;

		float oppositeFloorY = mStartFromLeft ? RIGHT_LEDGE_FLOOR_Y : LEFT_LEDGE_FLOOR_Y;
		float standY         = oppositeFloorY - BOSS_HALF_HEIGHT;

		bossPos.x = mEndX;
		if (bossPos.y < standY)
		{
			bossPos.y += 18.0f;
			if (bossPos.y > standY) bossPos.y = standY;
		}
		else if (bossPos.y > standY)
		{
			bossPos.y -= 18.0f;
			if (bossPos.y < standY) bossPos.y = standY;
		}

		if (std::fabs(bossPos.y - standY) <= 1.0f && mPhaseTimer >= 16)
		{
			bossPos.y      = standY;
			isBossGrounded = true;
			facingDir      = mStartFromLeft ? -1.0f : 1.0f;
			mPhase         = Phase::Finished;
			mPhaseTimer    = 0;
		}
		break;
	}

	default:
		break;
	}

	outAnimType  = mAnimType;
	outAnimFrame = mAnimFrame;
}

int Stage3DragonCloneDashAttack::SelectCloneSpriteHandle(float& outCloneFootOffset) const
{
	outCloneFootOffset = 79.0f;
	if (mAnimType == ANIM_ATTACK && !mCloneAttackAnim.empty())
	{
		outCloneFootOffset = 79.0f;
		return mCloneAttackAnim[mAnimFrame % mCloneAttackAnim.size()];
	}
	if (!mCloneFlyAnim.empty())
	{
		outCloneFootOffset = 57.0f;
		return mCloneFlyAnim[mAnimFrame % mCloneFlyAnim.size()];
	}
	return -1;
}

void Stage3DragonCloneDashAttack::GetEyeWorldPosition(
	float posX,
	float posY,
	float facingDir,
	float drawScale,
	float footOffset,
	float enemyHeight,
	bool isAttackAnim,
	int animFrame,
	float& outEyeX,
	float& outEyeY) const
{
	static const float kAttackEyeOffsetX[9] = { -88.0f, -94.0f, -98.0f, -85.0f, -73.0f, -91.0f, -93.0f, -74.0f, -70.0f };
	static const float kAttackEyeOffsetY[9] = { -40.0f,   9.0f, -16.0f, -40.0f, -13.0f,   5.0f,  14.0f, -12.0f, -12.0f };

	float localOffX = -88.0f;
	float localOffY = -40.0f;
	if (isAttackAnim && animFrame >= 0 && animFrame < 9)
	{
		localOffX = kAttackEyeOffsetX[animFrame];
		localOffY = kAttackEyeOffsetY[animFrame];
	}

	float feetWorldY    = posY + (enemyHeight * 0.5f);
	float spriteCenterY = feetWorldY - (footOffset * drawScale);

	float dirSign = (facingDir > 0.0f) ? -1.0f : 1.0f;
	outEyeX = posX + (localOffX * dirSign) * drawScale;
	outEyeY = spriteCenterY + localOffY * drawScale;
}

// 目の十字閃光（本体と同型の鋭い緑色ダイヤ「キランッ！」閃光）を描画
void Stage3DragonCloneDashAttack::DrawEyeCrossGlint(int eyeSx, int eyeSy, int alpha, bool isClone) const
{
	(void)isClone;
	if (alpha <= 0 || mCloneEyeGlintTimer <= 0) return;

	float t   = static_cast<float>(mCloneEyeGlintTimer) / 16.0f;
	float pop = std::sin(t * 3.14159265f);
	int   a   = (alpha * static_cast<int>(pop * 255.0f)) / 255;
	if (a <= 0) return;

	int hLen  = static_cast<int>(64.0f * pop);
	int hHalf = static_cast<int>(7.0f * pop) + 1;
	int vLen  = static_cast<int>(46.0f * pop);
	int vHalf = static_cast<int>(6.0f * pop) + 1;
	int dLen  = static_cast<int>(20.0f * pop);

	unsigned int outerColor = GetColor(15, 255, 145);
	unsigned int coreHot    = GetColor(225, 255, 235);

	SetDrawBlendMode(DX_BLENDMODE_ADD, a);
	DrawTriangle(eyeSx - hLen, eyeSy, eyeSx, eyeSy - hHalf, eyeSx, eyeSy + hHalf, outerColor, TRUE);
	DrawTriangle(eyeSx + hLen, eyeSy, eyeSx, eyeSy - hHalf, eyeSx, eyeSy + hHalf, outerColor, TRUE);
	DrawTriangle(eyeSx, eyeSy - vLen, eyeSx - vHalf, eyeSy, eyeSx + vHalf, eyeSy, outerColor, TRUE);
	DrawTriangle(eyeSx, eyeSy + vLen, eyeSx - vHalf, eyeSy, eyeSx + vHalf, eyeSy, outerColor, TRUE);
	DrawTriangle(eyeSx - (hLen * 2 / 3), eyeSy, eyeSx, eyeSy - 2, eyeSx, eyeSy + 2, coreHot, TRUE);
	DrawTriangle(eyeSx + (hLen * 2 / 3), eyeSy, eyeSx, eyeSy - 2, eyeSx, eyeSy + 2, coreHot, TRUE);
	DrawTriangle(eyeSx, eyeSy - (vLen * 2 / 3), eyeSx - 2, eyeSy, eyeSx + 2, eyeSy, coreHot, TRUE);
	DrawTriangle(eyeSx, eyeSy + (vLen * 2 / 3), eyeSx - 2, eyeSy, eyeSx + 2, eyeSy, coreHot, TRUE);
	DrawLine(eyeSx - dLen, eyeSy - dLen, eyeSx + dLen, eyeSy + dLen, outerColor, 2);
	DrawLine(eyeSx - dLen, eyeSy + dLen, eyeSx + dLen, eyeSy - dLen, outerColor, 2);
	DrawCircle(eyeSx, eyeSy, static_cast<int>(6.0f * pop) + 1, outerColor, TRUE);
	DrawCircle(eyeSx, eyeSy, static_cast<int>(3.0f * pop) + 1, coreHot, TRUE);
}

void Stage3DragonCloneDashAttack::Draw(
	BlockMap* blockMap,
	const VECTOR& bossPos,
	float facingDir,
	float drawScale,
	int realSpriteHandle,
	float footOffset,
	float enemyHeight) const
{
	if (!IsActive()) return;

	bool reverse = (facingDir > 0.0f);
	int  dirSign = (facingDir >= 0.0f) ? 1 : -1;

	// -----------------------------------------------------------------
	// 1. 突進中：連続残像トレイル（矢印・帯表示は撤去し、2D Effekseerで演出）
	// -----------------------------------------------------------------
	float cloneFootOffset = 79.0f;
	int   cloneHandle     = SelectCloneSpriteHandle(cloneFootOffset);

	if (mPhase == Phase::SimultaneousDash)
	{
		// 本体＋分身2体の間引き残像（2枚に削減してさらに軽量化）
		for (int tIdx = 4; tIdx >= 2; tIdx -= 2)
		{
			if (!mTrailPoints[tIdx].active || mTrailPoints[tIdx].alpha <= 0) continue;

			int trailSx = static_cast<int>(ConvertWorldToScreenX(mTrailPoints[tIdx].x, blockMap));

			// 本体の残像（灼熱の紅蓮残像）
			if (realSpriteHandle != -1)
			{
				float realFeetY = GetSlotY(mRealDragonSlot) + (enemyHeight * 0.5f);
				int   realSy    = static_cast<int>(ConvertWorldToScreenY(realFeetY - (footOffset * drawScale), blockMap));

				SetDrawBlendMode(DX_BLENDMODE_ADD, mTrailPoints[tIdx].alpha);
				SetDrawBright(255, 95, 50);
				DrawRotaGraph(trailSx, realSy, drawScale, 0.0, realSpriteHandle, TRUE, reverse);
			}

			// 分身2体の残像（エメラルドシアン残像・分身のフェード率に連動）
			if (cloneHandle != -1 && mCloneAlpha > 0)
			{
				int cTrailAlpha = (mTrailPoints[tIdx].alpha * mCloneAlpha) / 255;
				if (cTrailAlpha > 0)
				{
					SetDrawBlendMode(DX_BLENDMODE_ADD, cTrailAlpha);
					SetDrawBright(70, 255, 200);
					for (int i = 0; i < 2; ++i)
					{
						float cFeetY = GetSlotY(mCloneSlots[i]) + (enemyHeight * 0.5f);
						int   cSy    = static_cast<int>(ConvertWorldToScreenY(cFeetY - (cloneFootOffset * drawScale), blockMap));
						DrawRotaGraph(trailSx, cSy, drawScale, 0.0, cloneHandle, TRUE, reverse);
					}
				}
			}
		}
		SetDrawBright(255, 255, 255);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// -----------------------------------------------------------------
	// 3. 分身体2体（上・真ん中・下の残り2レーン）の本体スプライト描画
	// -----------------------------------------------------------------
	if ((mPhase == Phase::SummonAndCharge || mPhase == Phase::SimultaneousDash) && mCloneAlpha > 0 && cloneHandle != -1)
	{
		for (int i = 0; i < 2; ++i)
		{
			int   slotIndex = mCloneSlots[i];
			float cloneX    = mCurrentDashX;
			float cloneY    = GetSlotY(slotIndex);
			if (mPhase == Phase::SummonAndCharge)
			{
				cloneY += std::sin((mPhaseTimer + (i + 1) * 12) * 0.28f) * 5.0f;
			}

			int   sx         = static_cast<int>(ConvertWorldToScreenX(cloneX, blockMap));
			float feetWorldY = cloneY + (enemyHeight * 0.5f);
			int   sy         = static_cast<int>(ConvertWorldToScreenY(feetWorldY - (cloneFootOffset * drawScale), blockMap));

			// 分身スプライト本体を mCloneAlpha で半透明フェード描画
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, mCloneAlpha);
			DrawRotaGraph(sx, sy, drawScale, 0.0, cloneHandle, TRUE, reverse);

			if (mCloneEyeGlintTimer > 0)
			{
				// 分身の目の位置に緑色の「キランッ！」閃光を描画
				float cloneEyeX = 0.0f, cloneEyeY = 0.0f;
				GetEyeWorldPosition(
					cloneX,
					cloneY,
					facingDir,
					drawScale,
					cloneFootOffset,
					enemyHeight,
					(mAnimType == ANIM_ATTACK),
					mAnimFrame,
					cloneEyeX,
					cloneEyeY
				);
				int cEyeSx = static_cast<int>(ConvertWorldToScreenX(cloneEyeX, blockMap));
				int cEyeSy = static_cast<int>(ConvertWorldToScreenY(cloneEyeY, blockMap));
				DrawEyeCrossGlint(cEyeSx, cEyeSy, mCloneAlpha, true);
			}
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// -----------------------------------------------------------------
	// 5. 高速スピードラインの描画
	// -----------------------------------------------------------------
	for (size_t i = 0; i < mSpeedLines.size(); ++i)
	{
		const SpeedLine& sl = mSpeedLines[i];
		bool isClone = (sl.laneIdx != mRealDragonSlot);
		int  alpha   = (sl.life * 215) / sl.maxLife;
		if (isClone && mPhase == Phase::SimultaneousDash)
		{
			alpha = (alpha * mCloneAlpha) / 255;
		}
		if (alpha <= 0) continue;

		int x1 = static_cast<int>(ConvertWorldToScreenX(sl.x, blockMap));
		int y1 = static_cast<int>(ConvertWorldToScreenY(sl.y, blockMap));
		int x2 = static_cast<int>(ConvertWorldToScreenX(sl.x - dirSign * sl.length, blockMap));

		SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
		DrawLine(x1, y1, x2, y1, sl.color, sl.thickness);
		DrawLine(x1, y1, (x1 + x2) / 2, y1, GetColor(255, 255, 240), 1);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

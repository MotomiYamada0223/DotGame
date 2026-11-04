#pragma once

#include "Enemy.h"
#include "UnitStatus.h"
#include <vector>

class BossEnemyDragon : public Enemy, public UnitStatus
{
public:
	enum class State
	{
		EntranceDrop,
		EntranceWait,
		Idle,
		Move,
		Attack,
		Takeoff,
		Fly,
		AirAttack,
		Landing
	};

	struct FireProjectile
	{
		float x, y;
		float vx, vy;
		float radius;
		int animTick;
		bool active;
	};

	struct BurningGround
	{
		float x, y;
		float halfWidth;
		int timer;    // 480フレーム (8秒間)
		int maxTimer; // 480フレーム
	};

	BossEnemyDragon(VECTOR initPos);
	virtual ~BossEnemyDragon();

	virtual void UpdateStatusByProgress(GameProgress progress) override;
	virtual void Update() override;
	virtual void Draw() override;
	virtual void EnemyMove(BlockMap& blockMap) override;
	virtual void TakeDamage(int damage) override;

	void StartEntrance();
	void DrawHpBar() const;
	State GetState() const { return mState; }
	int GetEntranceTimer() const { return mEntranceTimer; }
	int GetPostEntranceTimer() const { return mPostEntranceTimer; }
	float GetSmoothHp() const { return mSmoothHp; }
	int GetHpShakeTimer() const { return mHpShakeTimer; }

	virtual bool IsAttacking() const override;
	virtual void GetAttackHitbox(float& left, float& top, float& right, float& bottom) const override;

private:
	void UpdateFireAndBurningGround(class Player* player);
	void DrawFireAndBurningGround();

	std::vector<int> mIdleAnim;
	std::vector<int> mMoveAnim;
	std::vector<int> mAttackAnim;
	std::vector<int> mTakeoffAnim;
	std::vector<int> mFlyAnim;
	std::vector<int> mAirAttackAnim;
	std::vector<int> mLandingAnim;

	std::vector<FireProjectile> mFireProjectiles;
	std::vector<BurningGround> mBurningGrounds;

	State mState;
	int mAnimFrame;
	int mAnimTimer;
	int mStateTimer;
	int mAttackPhase;
	int mAttackCooldown;
	int mFlightCooldown;

	int mEntranceTimer;
	int mPostEntranceTimer;
	bool mHasBrokenCeiling;
	bool mEntranceLandAnimDone;

	float mFacingDirection; // -1.0f = 左向き(デフォルト画像向き), +1.0f = 右向き(反転描画)
	float mDrawScale;
	float mHoverTargetY;

	float mSmoothHp;
	int mHpShakeTimer;
};

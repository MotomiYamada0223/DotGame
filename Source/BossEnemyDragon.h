#pragma once

#include "Enemy.h"
#include "UnitStatus.h"
#include "DragonFireGimmick.h"
#include <vector>

class Player;
class GameScene;

// =========================================================================
// ステージ2（地下洞窟ボス部屋）ボス：ドラゴン
// 登場・移動・地上攻撃・3種の空中攻撃ギミック・撃破帰還・ボスUIを制御するクラス
// =========================================================================
class BossEnemyDragon : public Enemy, public UnitStatus
{
public:
	enum class State
	{
		EntranceDrop,         // 登場演出1: 天井（X=9500）を突き破って落下
		EntranceWait,         // 登場演出2: 着地砂煙 → 待機 → 離陸 → 空中ホバリングで戦闘開始
		Idle,                 // 地上待機
		Move,                 // 地上歩行移動
		Attack,               // 地上通常攻撃（高速ステップ＋2連斬撃）
		Takeoff,              // 地上から空中への離陸
		Fly,                  // 空中ホバリング移動＆次攻撃への分岐
		AirAttack,            // 空中攻撃1: 空中火炎ブレス放射＆地面延焼
		AirDiveAttack,        // 空中攻撃2: レーザー照準ロックオン急降下斬撃
		CenterFireballAttack, // 空中攻撃3: 中央巨大火球＆大爆発＆2波アーチ炎（→急降下へ派生）
		Landing,              // 空中から地上への着陸
		DefeatRetreat         // 撃破演出: 天井の穴（X=9500）から上空へ帰還
	};

	// 既存の型名との互換エイリアス
	using FireProjectile  = DragonFireGimmick::FireProjectile;
	using FireSpark       = DragonFireGimmick::FireSpark;
	using BurningGround   = DragonFireGimmick::BurningGround;
	using CenterFireball  = DragonFireGimmick::CenterFireball;
	using ArchFlameWave   = DragonFireGimmick::ArchFlameWave;

	BossEnemyDragon(VECTOR initPos);
	virtual ~BossEnemyDragon();

	virtual void UpdateStatusByProgress(GameProgress progress) override;
	virtual void Update() override;
	virtual void Draw() override;
	virtual void EnemyMove(BlockMap& blockMap) override;
	virtual void TakeDamage(int damage) override;

	void StartEntrance();
	State GetState() const { return mState; }
	int GetEntranceTimer() const { return mEntranceTimer; }
	int GetPostEntranceTimer() const { return mPostEntranceTimer; }
	float GetSmoothHp() const { return mSmoothHp; }
	int GetHpShakeTimer() const { return mHpShakeTimer; }
	bool HasRetreated() const { return mHasRetreated; }

	void StartBattle(); // 追加：戦闘開始を有効化する関数
	bool IsBattleActive() const { return mIsBattleActive; } // 追加

	virtual bool HasCustomDeath() const override { return true; }
	virtual bool IsActiveTarget() const override { return mState != State::EntranceDrop && mState != State::DefeatRetreat && !mHasRetreated; }
	virtual bool CheckAttackCollision(const VECTOR& targetPos, const VECTOR& targetSize) const override;
	virtual bool IsAttacking() const override;
	virtual void GetAttackHitbox(float& left, float& top, float& right, float& bottom) const override;

	// ボス登場時の「ドラゴン」文字演出＆画面下部HPバー描画
	void DrawBossUI(Player* player, BlockMap& blockMap) const;

private:
	// --- 初期化・補助関数 ---
	void LoadAnimations();
	void GetAirAttackMouthWorldPos(float& outMouthX, float& outMouthY) const;
	void TriggerDefeatRetreat(GameScene* gameScene);

	// --- 状態別更新関数 (Update) ---
	void UpdateEntranceDrop(Player* player, GameScene* gameScene);
	void UpdateEntranceWait(Player* player);
	void UpdateIdle(Player* player, float dx, float absDx);
	void UpdateMove(Player* player, float dx, float absDx, float roomMinX, float roomMaxX);
	void UpdateGroundAttack(float roomMinX, float roomMaxX);
	void UpdateTakeoff(Player* player, float dx);
	void UpdateFly(Player* player, float dx, float absDx, float roomMinX, float roomMaxX);
	void UpdateAirBreathAttack(Player* player, float dx);
	void UpdateAirDiveAttack(Player* player, float dx, float roomMinX, float roomMaxX);
	void UpdateCenterFireballAttack(Player* player, float dx);
	void UpdateLanding();
	void UpdateDefeatRetreat();

	// --- 演出・本体描画補助関数 (Draw) ---
	void SelectCurrentSprite(int& outHandle, float& outFootOffset) const;
	void DrawDiveLockOnSight() const;
	void DrawAfterimages(int screenX, int screenY, int handle, bool reverse, double drawAngle) const;
	void DrawDiveEyeFlare(int screenX, int screenY) const;
	void DrawWarningExclamation() const;

	// --- アニメーション画像ハンドル ---
	std::vector<int> mIdleAnim;
	std::vector<int> mMoveAnim;
	std::vector<int> mAttackAnim;
	std::vector<int> mTakeoffAnim;
	std::vector<int> mFlyAnim;
	std::vector<int> mAirAttackAnim;
	std::vector<int> mLandingAnim;

	// --- 炎・火球・大爆発・アーチ波・延焼地面ギミック管理 ---
	DragonFireGimmick mFireGimmick;

	// --- 状態・移動・攻撃制御パラメータ ---
	State mState;
	int mAnimFrame;
	int mAnimTimer;
	int mStateTimer;
	int mAttackPhase;
	int mDivePhase;
	int mCenterAttackPhase;
	int mRetreatPhase;
	bool mHasRetreated;
	float mDiveVx;
	float mDiveVy;
	float mDiveAngle;
	float mDiveTargetX;
	float mDiveTargetY;
	int mAirAttackCycle; // 0: 中央火の玉大爆発＆アーチ炎（→急降下派生）, 1: 空中火炎ブレス＆地面延焼
	int mAttackCooldown;
	int mFlightCooldown;

	int mEntranceTimer;
	int mPostEntranceTimer;
	bool mHasBrokenCeiling;
	bool mEntranceLandAnimDone;

	float mFacingDirection; // -1.0f = 左向き(デフォルト画像向き), +1.0f = 右向き(反転描画)
	float mDrawScale;
	float mHoverTargetY;
	float mBreathTargetCenterX;

	float mSmoothHp;
	int mHpShakeTimer;

	bool mIsBattleActive;
};

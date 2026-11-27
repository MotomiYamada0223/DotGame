#pragma once
#include "Enemy.h"
#include "UnitStatus.h"
#include "Stage3DragonRoarAttack.h"
#include "Stage3DragonMagmaFireballAttack.h"
#include "Stage3DragonAirDashComboAttack.h"
#include "Stage3DragonBounceFireballAttack.h"
#include "Stage3DragonCloneDashAttack.h"
#include "Stage3DragonCloneMeteorDashAttack.h"
#include "Stage3DragonUltimateWindSlamAttack.h"
#include "Stage3DragonCloneWindRoarAttack.h"
#include <vector>
#include <unordered_map>
#include <utility>

class Player;
class BlockMap;

// =========================================================================
// Stage 3（ステージ3）奥の足場に出現するドラゴンボスクラス
// ・第1形態「ドラゴン」：
//     初回固定順序: 咆哮 → 2回突進 → 反射火玉 → 中央巨大火球 → 以降は直前と被らないランダム
// ・第1形態のHPが0になると中央へ飛翔し、黄金に輝く第2形態「アルティメットドラゴン」へ覚醒！
// ・第2形態「アルティメットドラゴン」：
//     初回固定順序: 分身突進 → 風の刃＆咆哮 → 分身突進＆火の玉メテオ → 以降は直前と被らないランダム
// ・第2形態でHPが1になると強制的にHP1で踏みとどまり、最後の風炎玉攻撃（UltimateWindSlamAttack）を発動！
// ・最後の風炎玉攻撃が終わると8秒間スタン状態（FinalStun）になり、勇者が攻撃して撃破・勝利！
// =========================================================================
class Stage3BossDragon : public Enemy, public UnitStatus
{
public:
	enum class State
	{
		CutsceneIdle,           // ムービー演出中の待機・咆哮状態
		AwakeningCutscene,      // 第2形態「アルティメットドラゴン」黄金覚醒ムービー状態
		Idle,                   // 戦闘中の地上待機状態
		Walk,                   // 地上移動状態
		Attack,                 // 近接噛みつき・引っ掻き攻撃状態
		RoarAttack,             // 空中咆哮＆浮遊台破壊攻撃（Stage3DragonRoarAttackに委譲）
		MagmaFireballAttack,    // 中央マグマへの巨大火球＆マグマ噴水攻撃（Stage3DragonMagmaFireballAttackに委譲）
		AirDashComboAttack,     // 空中ロックオン溜め＆2連続突進通常攻撃（Stage3DragonAirDashComboAttackに委譲）
		BounceFireballAttack,   // 地上ブレス壁反射火の玉攻撃（Stage3DragonBounceFireballAttackに委譲）
		CloneDashAttack,        // 分身2体＋本体の3体同時横突進攻撃（Stage3DragonCloneDashAttackに委譲）
		CloneMeteorDashAttack,  // 分身メテオ8秒間＆2秒ごと疾走突進攻撃（Stage3DragonCloneMeteorDashAttackに委譲）
		CloneWindRoarAttack,    // 分身2体の120度ランダム風の刃（1.5秒毎）＋本体の逆方向溜め咆哮（3秒毎）・10秒間
		UltimateWindSlamAttack, // 最後の大技：分身2匹の風の玉＆マグマ吸引＋本体地面叩きつけ（Stage3DragonUltimateWindSlamAttackに委譲）
		Down,                   // 空中咆哮溜め中に魔法攻撃を受けて墜落・ダウン中（一定時間攻撃し放題）
		FinalStun,              // 最後の大技終了後の8秒間（480フレーム）スタン状態（勇者のトドメ待ち）
		Defeated                // 撃破・勝利演出状態
	};

	Stage3BossDragon();
	Stage3BossDragon(VECTOR pos);
	virtual ~Stage3BossDragon();

	void Update() override;
	void EnemyMove(BlockMap& blockMap) override;
	void Draw() override;

	void UpdateStatusByProgress(GameProgress progress) override;
	void TakeDamage(int damage) override;
	void TakeMagicDamage(int damage);
	bool IsChargingAirRoar() const;
	bool HasCustomDeath() const override { return true; }
	bool IsActiveTarget() const override;
	bool CheckAttackCollision(const VECTOR& targetPos, const VECTOR& targetSize) const override;
	bool IsAttacking() const override;
	void GetAttackHitbox(float& left, float& top, float& right, float& bottom) const override;

	// プレイヤーとの体当たり接触ダメージが有効かどうか（ムービー中・スタン中・撃破時は無効）
	bool CanDealContactDamage() const override;

	// ムービー演出制御用
	void Reset();
	void SetCutsceneMode(bool inCutscene);
	bool IsInCutscene() const { return mState == State::CutsceneIdle || mState == State::AwakeningCutscene; }
	void TriggerCutsceneRoar();

	// 第2形態「アルティメットドラゴン」覚醒ムービー連携用
	bool ConsumeAwakeningMovieRequest();
	void UpdateAwakeningCutscenePose(float x, float y, float facingDir, bool roaring, float chargeRatio);
	void TriggerGoldenAwakening();
	void StartPhase2Battle();
	bool IsPhase2() const { return mbIsPhase2; }

	// 画面下部のボスHPバー描画
	void DrawBossUI(Player* player, BlockMap& blockMap) const;

	// 【Stage3.png 5724x1692拡大版 対応】右奥足場（X = 4970 .. 5690, 床 Y = 1289）の初期スポーン座標
	static constexpr float SPAWN_X = 5330.0f;
	static constexpr float SPAWN_Y = 1189.0f; // 床 Y=1289 - 高さ半分(100)

private:
	void LoadAnimations();
	int  LoadGoldenDragonGraph(const char* filePath);
	void SelectCurrentSprite(int& outHandle, float& outFootOffset, bool forceGolden = false) const;
	void SelectSpriteFromSet(
		int& outHandle,
		float& outFootOffset,
		const std::vector<int>& idleAnim,
		const std::vector<int>& moveAnim,
		const std::vector<int>& attackAnim,
		const std::vector<int>& takeoffAnim,
		const std::vector<int>& flyAnim,
		const std::vector<int>& airAttackAnim,
		const std::vector<int>& landingAnim,
		const std::vector<int>& roarAnim,
		const std::vector<int>& chargeAnim,
		const std::vector<int>& downAnim
	) const;

	State mState;
	bool  mbCutsceneRoaring;
	int   mRoarTimer;

	bool  mbIsPhase2;
	bool  mbIsGolden;
	bool  mbAwakeningMovieRequested;
	float mAwakenChargeRatio;
	int   mSsjFireTex[4];
	int   mSsjThunderTex[8];
	int   mEyeGlintTimer;
	std::unordered_map<int, std::pair<float, float>> mEyeOffsetsByHandle;
	void  RecordEyeOffset(const char* filePath, int handle1, int handle2);
	bool  mbTriggerFinalUltimate;
	int   mStunTimer;
	int   mDownPhase;
	float mDownTargetX;
	float mDownFloorY;
	float mDownVy;
	static constexpr int DOWN_GROUND_FRAMES = 360; // 地上ダウン時間：6秒間（60fps × 6）攻撃し放題

	std::vector<int> mIdleAnim;
	std::vector<int> mMoveAnim;
	std::vector<int> mAttackAnim;
	std::vector<int> mTakeoffAnim;
	std::vector<int> mFlyAnim;
	std::vector<int> mAirAttackAnim;
	std::vector<int> mLandingAnim;
	std::vector<int> mRoarAnim;
	std::vector<int> mChargeAnim;
	std::vector<int> mDownAnim;

	// 第2形態「アルティメットドラゴン」用の黄金スプライト群
	std::vector<int> mGoldenIdleAnim;
	std::vector<int> mGoldenMoveAnim;
	std::vector<int> mGoldenAttackAnim;
	std::vector<int> mGoldenTakeoffAnim;
	std::vector<int> mGoldenFlyAnim;
	std::vector<int> mGoldenAirAttackAnim;
	std::vector<int> mGoldenLandingAnim;
	std::vector<int> mGoldenRoarAnim;
	std::vector<int> mGoldenChargeAnim;
	std::vector<int> mGoldenDownAnim;

	int   mAnimFrame;
	int   mAnimTimer;
	int   mStateTimer;
	float mFacingDirection; // -1.0f: 左向き（プレイヤー側）, 1.0f: 右向き
	float mDrawScale;

	float mSmoothHp;
	int   mHpShakeTimer;
	int   mAttackCooldown;
	int   mSpecialAttackCooldown;
	int   mDefeatTimer;
	int   mAttackPatternIndex;
	int   mLastAttackId;

	// 1. 空中咆哮＆ランダム浮遊台破壊攻撃モジュール
	Stage3DragonRoarAttack mRoarAttack;
	int                    mRoarAttackAnimType;
	int                    mRoarAttackAnimFrame;

	// 2. 中央マグマ巨大火球＆縦マグマ噴水＋飛散攻撃モジュール
	Stage3DragonMagmaFireballAttack mMagmaFireballAttack;
	int                             mMagmaFireballAnimType;
	int                             mMagmaFireballAnimFrame;

	// 3. 空中ロックオン溜め＆2連続突進通常攻撃モジュール
	Stage3DragonAirDashComboAttack mAirDashComboAttack;
	int                            mAirDashComboAnimType;
	int                            mAirDashComboAnimFrame;

	// 4. 地上ブレス壁反射火の玉攻撃モジュール
	Stage3DragonBounceFireballAttack mBounceFireballAttack;
	int                              mBounceFireballAnimType;
	int                              mBounceFireballAnimFrame;

	// 5. 分身2体＋本体の3体同時横突進攻撃モジュール
	Stage3DragonCloneDashAttack mCloneDashAttack;
	int                         mCloneDashAnimType;
	int                         mCloneDashAnimFrame;

	// 6. 分身1匹の上空特大火球8秒間メテオ＆本体＋分身1体の2秒ごと疾走突進攻撃モジュール
	Stage3DragonCloneMeteorDashAttack mCloneMeteorDashAttack;
	int                               mCloneMeteorDashAnimType;
	int                               mCloneMeteorDashAnimFrame;

	// 7. 最後の大技：分身2匹の大きな風の玉＆勇者浮遊＋マグマ吸引＆本体地面叩きつけモジュール
	Stage3DragonUltimateWindSlamAttack mUltimateWindSlamAttack;
	int                                mUltimateWindSlamAnimType;
	int                                mUltimateWindSlamAnimFrame;

	// 8. 分身2体の120度ランダム風の刃（1.5秒毎）＋本体の逆方向溜め咆哮（3秒毎）・10秒間
	Stage3DragonCloneWindRoarAttack    mCloneWindRoarAttack;
	int                                mCloneWindRoarAnimType;
	int                                mCloneWindRoarAnimFrame;
};

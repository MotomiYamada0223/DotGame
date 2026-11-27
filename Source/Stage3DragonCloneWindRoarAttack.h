#pragma once
#include "DxLib.h"
#include <vector>
#include <unordered_map>
#include <utility>

class BlockMap;
class Player;

// =========================================================================
// Stage 3（ステージ3）ドラゴン用の新攻撃8（最後の追加攻撃）：
// 中央上空の分身2体（左右向き・120度ランダム風の刃 1.5秒毎）
// ＋ 本体の端からの溜め咆哮（3秒毎・咆哮単体攻撃と同等のド迫力演出）・10秒間
// （Stage3DragonCloneWindRoarAttack）
// =========================================================================
class Stage3DragonCloneWindRoarAttack
{
public:
	enum class Phase
	{
		Inactive,             // 非アクティブ
		SetupPositions,       // 分身2体が中央上空（左右向き）へ、本体が端の上空へ移動
		WindBladeAndRoarLoop, // 10秒間（600F）：分身が1.5秒毎に120度ランダム風の刃、本体が3秒毎に溜め大咆哮
		ReturnToLedge,        // 分身がフェードアウトし、本体が足場に着地
		Finished              // 攻撃完了
	};

	Stage3DragonCloneWindRoarAttack();
	~Stage3DragonCloneWindRoarAttack();

	void Initialize();
	void Reset(Player* player = nullptr);

	void StartAttack(VECTOR& bossPos, float& facingDir, Player* player);

	void UpdateDragon(
		VECTOR& bossPos,
		float& facingDir,
		bool& isBossGrounded,
		int& outAnimType,
		int& outAnimFrame,
		Player* player,
		BlockMap* blockMap
	);

	void UpdateProjectilesAndEffects(
		Player* player,
		BlockMap* blockMap,
		int attackDamage
	);

	void Draw(
		BlockMap* blockMap,
		const VECTOR& bossPos,
		float facingDir,
		float drawScale,
		int realSpriteHandle,
		float footOffset,
		float enemyHeight
	) const;

	bool IsActive() const { return mPhase != Phase::Inactive && mPhase != Phase::Finished; }
	bool IsFinished() const { return mPhase == Phase::Finished; }
	bool IsAirbornePhase() const { return mPhase != Phase::Inactive && mPhase != Phase::Finished; }
	bool IsChargingAirRoar() const { return mPhase == Phase::WindBladeAndRoarLoop && (mPhaseTimer % ROAR_INTERVAL) < ROAR_CHARGE_FRAMES; }
	int  GetRoarCycleTimer() const { return (mPhase == Phase::WindBladeAndRoarLoop) ? (mPhaseTimer % ROAR_INTERVAL) : -1; }
	bool IsEyeGlintMoment() const { return (mPhase == Phase::WindBladeAndRoarLoop && (mPhaseTimer % ROAR_INTERVAL) == (ROAR_CHARGE_FRAMES - 14)); }
	float GetRealTargetX() const { return mRealTargetX; }
	float GetRealFloorY() const { return mbRealOnLeftEdge ? LEFT_LEDGE_FLOOR_Y : RIGHT_LEDGE_FLOOR_Y; }

	static constexpr int ANIM_IDLE       = 0;
	static constexpr int ANIM_TAKEOFF    = 1;
	static constexpr int ANIM_FLY        = 2;
	static constexpr int ANIM_LANDING    = 3;
	static constexpr int ANIM_ROAR       = 4;
	static constexpr int ANIM_AIR_ATTACK = 5;
	static constexpr int ANIM_ATTACK     = 6;

private:
	// 1. 分身が放つ 2D Effekseer 風の刃（当たり判定付き）
	struct WindBladeProjectile
	{
		bool  active    = false;
		float x         = 0.0f;
		float y         = 0.0f;
		float vx        = 0.0f;
		float vy        = 0.0f;
		float angle     = 0.0f;
		float radius    = 44.0f;
		int   life        = 0;
		int   maxLife     = 210;
		int   bounceCount = 0;
		int   fadeTimer   = 0;
		std::vector<int> efkHandles;
	};

	// 2. 風の刃の発射衝撃波リング
	struct WindLaunchRing
	{
		float x       = 0.0f;
		float y       = 0.0f;
		float angle   = 0.0f;
		float radius  = 16.0f;
		int   life    = 0;
		int   maxLife = 18;
	};

	// 3. 風の刃の軌跡・着弾スパーク粒子
	struct WindSpark
	{
		float        x       = 0.0f;
		float        y       = 0.0f;
		float        vx      = 0.0f;
		float        vy      = 0.0f;
		float        length  = 22.0f;
		int          life    = 0;
		int          maxLife = 16;
		unsigned int color   = 0;
	};

	// 4. 本体の咆哮衝撃波リング（収束チャージ用 ＆ 巨大拡散咆哮用：Stage3DragonRoarAttackと同等以上）
	struct RoarShockRing
	{
		float x         = 0.0f;
		float y         = 0.0f;
		float radius    = 10.0f;
		float maxRadius = 1750.0f;
		int   life      = 28;
		int   maxLife   = 28;
	};

	// 5. 本体の咆哮暴風ライン（空中から斜め下へ吹き荒れる暴風）
	struct RoarWindLine
	{
		float x       = 0.0f;
		float y       = 0.0f;
		float vx      = 0.0f;
		float vy      = 0.0f;
		float length  = 150.0f;
		int   life    = 36;
		int   maxLife = 36;
	};

	// 6. 本体の咆哮チャージ吸引パーティクル
	struct RoarChargeParticle
	{
		float        angle = 0.0f;
		float        dist  = 140.0f;
		float        speed = 5.5f;
		float        size  = 4.0f;
		unsigned int color = 0;
	};

	// 【Stage3.png 5724x1692拡大版 対応】足場・中央・分身・咆哮高度へ更新
	static constexpr float LEFT_LEDGE_STAND_X       = 3340.0f;
	static constexpr float RIGHT_LEDGE_STAND_X      = 5330.0f;
	static constexpr float LEFT_LEDGE_FLOOR_Y       = 1262.0f;
	static constexpr float RIGHT_LEDGE_FLOOR_Y      = 1289.0f;
	static constexpr float LEFT_SAFE_MAX_X          = 3667.0f; // 左側の安全右端
	static constexpr float RIGHT_SAFE_MIN_X         = 4970.0f; // 右側の安全左端
	static constexpr float BOSS_HALF_HEIGHT         = 100.0f;

	static constexpr float STAGE_CENTER_X           = 4353.0f; // ステージ中央X
	static constexpr float CLONE_CENTER_OFFSET_X    = 210.0f;  // 中央での分身2体の左右間隔
	static constexpr float CLONE_TOP_Y              = 700.0f;  // 中央上の分身高度
	static constexpr float REAL_ROAR_Y              = 810.0f;  // 本体の空中咆哮高度（RoarAttackと同じ）

	static constexpr int   SETUP_DURATION_FRAMES    = 48;      // 配置移動フレーム
	static constexpr int   ATTACK_DURATION_FRAMES   = 600;     // 攻撃時間：10秒（60fps × 10）
	static constexpr int   WIND_BLADE_INTERVAL      = 90;      // 風の刃：1.5秒に1回（60fps × 1.5）
	static constexpr int   ROAR_INTERVAL            = 180;     // 咆哮：3秒に1回（60fps × 3.0）
	static constexpr int   ROAR_CHARGE_FRAMES       = 124;     // 咆哮の溜め時間
	static constexpr int   ROAR_BLAST_FRAMES        = 48;      // 咆哮の放出時間

	Phase mPhase;
	int   mPhaseTimer;
	int   mAnimType;
	int   mAnimFrame;

	// 本体ドラゴンの配置（左端または右端の上空）
	bool  mbRealOnLeftEdge;
	float mRealFacingDir;
	float mStartBossX;
	float mStartBossY;
	float mRealTargetX;
	float mRealTargetY;
	float mReturnTargetX;
	float mReturnTargetY;

	// 分身2体の配置（真ん中の上に左右向いた状態で配置）
	// 分身1：中央やや左で左向き(-1.0f)、分身2：中央やや右で右向き(+1.0f)
	float mClone1X;
	float mClone1Y;
	float mClone1FacingDir;
	float mClone2X;
	float mClone2Y;
	float mClone2FacingDir;
	int   mCloneAnimType;
	int   mCloneAnimFrame;
	int   mCloneAlpha;
	int   mCloneEyeGlintTimer;
	int   mCloneMagicCircleEfk[2];

	float mClone1NextAngle; // 分身1の次回発射角度（左向き120度の範囲）
	float mClone2NextAngle; // 分身2の次回発射角度（右向き120度の範囲）

	bool  mbHasKnockedBackInCycle;
	float mPlayerKnockbackVx;
	int   mKnockbackTimer;

	std::vector<WindBladeProjectile> mWindBlades;
	std::vector<WindLaunchRing>      mWindLaunchRings;
	std::vector<WindSpark>           mWindSparks;
	std::vector<RoarShockRing>       mRoarShockRings;
	std::vector<RoarWindLine>        mRoarWindLines;
	std::vector<RoarChargeParticle>  mRoarChargeParticles;

	std::vector<int> mCloneIdleAnim;
	std::vector<int> mCloneFlyAnim;
	std::vector<int> mCloneAirAttackAnim;
	std::unordered_map<int, std::pair<float, float>> mCloneEyeOffsetsByHandle;

	void LoadCloneAnimations();
	void ReleaseCloneAnimations();
	int  LoadGraphWithRecoloredEyes(const char* filePath);
	int  SelectCloneSpriteHandle(float& outFootOffset) const;

	float PickRandomAngleIn120DegCone(float cloneFacingDir) const;
	void  SpawnWindBlade(float startX, float startY, float angle);
	void  SpawnWindSlashSparks(float x, float y, float baseAngle, int count);
	void  UpdatePlayerKnockback(Player* player, BlockMap* blockMap);

	void  DrawEyeCrossGlint(int eyeSx, int eyeSy, int alpha, bool isClone) const;
	void  GetDragonEyeScreenPos(
		const VECTOR& pos,
		float facingDir,
		float drawScale,
		float footOffset,
		float enemyHeight,
		int scrollX,
		int scrollY,
		int& outEyeSx,
		int& outEyeSy
	) const;
};

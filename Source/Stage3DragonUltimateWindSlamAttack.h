#pragma once
#include "DxLib.h"
#include <vector>

class BlockMap;
class Player;
class Stage3Magma;
class Stage3Gimmick;

// =========================================================================
// Stage 3（ステージ3）ドラゴン用の最後の大技：
// 分身2匹の超暴風雷火球生成＆マグマ吸引＋勇者の風引力浮遊＆本体の破滅叩きつけ
// （Stage3DragonUltimateWindSlamAttack）
// =========================================================================
class Stage3DragonUltimateWindSlamAttack
{
public:
	enum class Phase
	{
		Inactive,                     // 非アクティブ
		SetupPositions,               // 分身2匹が中央左右へ、本体が中央上空へ飛翔
		CreateWindSphereAndSuckMagma, // 分身の羽ばたきで巨大風玉形成、勇者浮遊＆下のマグマを吸引
		RealDragonSlamWindup,         // マグマ吸尽後の暴風紅蓮球上で本体が光刃の爪を振りかぶる
		RealDragonSlamDown,           // 本体が巨大球とともに地面へ超音速急降下叩きつけ！
		SlamImpactFinish,             // 地面に激突！大爆発・光柱・全足場粉砕・マグマ完全消滅
		ReturnToLedge,                // 本体が足場へ帰還して着地
		Finished                      // 攻撃完了
	};

	Stage3DragonUltimateWindSlamAttack();
	~Stage3DragonUltimateWindSlamAttack();

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
		BlockMap* blockMap,
		Stage3Magma* magma,
		Stage3Gimmick* gimmick = nullptr
	);

	void UpdateProjectilesAndEffects(
		Player* player,
		BlockMap* blockMap,
		Stage3Magma* magma,
		int attackDamage
	);

	void Draw(
		BlockMap* blockMap,
		Player* player,
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
	bool IsEyeGlintMoment() const { return (mPhase == Phase::RealDragonSlamWindup && mPhaseTimer == 32); }

	static constexpr int ANIM_IDLE       = 0;
	static constexpr int ANIM_TAKEOFF    = 1;
	static constexpr int ANIM_FLY        = 2;
	static constexpr int ANIM_LANDING    = 3;
	static constexpr int ANIM_ROAR       = 4;
	static constexpr int ANIM_AIR_ATTACK = 5;
	static constexpr int ANIM_ATTACK     = 6;

private:
	// 1. 風の引力で下から吸い込まれるマグマ塊（彗星コメット状）
	struct SuckedMagmaGlob
	{
		bool  active     = false;
		float startX     = 0.0f;
		float startY     = 0.0f;
		float ctrlX      = 0.0f;
		float ctrlY      = 0.0f;
		float x          = 0.0f;
		float y          = 0.0f;
		float prevX[5]   = {0};
		float prevY[5]   = {0};
		float radius     = 24.0f;
		int   warnTimer  = 18;
		int   timer      = 0;
		int   duration   = 74;
	};

	// 3. 分身の羽ばたきから風の玉へ射出される鋭利な三日月風刃ブレード
	struct CrescentBlade
	{
		bool  active   = false;
		float startX   = 0.0f;
		float startY   = 0.0f;
		float x        = 0.0f;
		float y        = 0.0f;
		float t        = 0.0f;
		float speed    = 0.048f;
		float angle    = 0.0f;
		float scale    = 1.0f;
		bool  fromLeft = true;
		int   efkHandle = -1;
	};

	// 4-2. 足場側の安全地帯を狙い撃つサイド電撃
	struct SideLightningStrike
	{
		bool  active     = false;
		bool  fromLeft   = true;
		float startX     = 0.0f;
		float startY     = 0.0f;
		float targetX    = 0.0f;
		float targetY    = 0.0f;
		float x          = 0.0f;
		float y          = 0.0f;
		float vx         = 0.0f;
		float vy         = 0.0f;
		int   warnTimer  = 22; // 予兆フレーム
		int   timer      = 0;
		bool  hitGround  = false;
		int   burstTimer = 0;
		int   efkHandle  = -1;
	};

	// 12. 地面を這う電撃（カタクリズム後、中心から崖を登り足場へ流れる）
	struct GroundCrawlingShock
	{
		bool  active     = false;
		int   dir        = -1; // -1: 左向き, +1: 右向き
		float x          = 0.0f;
		float y          = 0.0f;
		int   stage      = 0;  // 0: 底面, 1: 崖登り, 2: 足場
		float speed      = 24.0f;
		int   life       = 0;
		int   efkHandle  = -1;
	};

	// 12. 軽量・高迫力な2D爆風プルーム（白飛び・魔法陣なし）
	struct ExplosionPlume
	{
		bool  active   = false;
		bool  isSmoke  = false;
		int   texIdx   = 0;
		float x        = 0.0f;
		float y        = 0.0f;
		float vx       = 0.0f;
		float vy       = 0.0f;
		float scale    = 1.0f;
		float scaleVel = 0.04f;
		float angle    = 0.0f;
		float angleVel = 0.0f;
		int   life     = 0;
		int   maxLife  = 30;
		int   r        = 255;
		int   g        = 110;
		int   b        = 20;
	};

	struct ExplosionSpark
	{
		float x       = 0.0f;
		float y       = 0.0f;
		float prevX   = 0.0f;
		float prevY   = 0.0f;
		float vx      = 0.0f;
		float vy      = 0.0f;
		int   life    = 0;
		int   maxLife = 28;
		int   size    = 4;
		int   r       = 255;
		int   g       = 140;
		int   b       = 20;
	};

	// 【Stage3.png 5724x1692拡大版 対応】足場・中央・風炎球・ピット座標へ更新
	static constexpr float LEFT_LEDGE_STAND_X  = 3340.0f;
	static constexpr float RIGHT_LEDGE_STAND_X = 5330.0f;
	static constexpr float LEFT_LEDGE_FLOOR_Y  = 1262.0f;
	static constexpr float RIGHT_LEDGE_FLOOR_Y = 1289.0f;
	static constexpr float BOSS_HALF_HEIGHT    = 100.0f;

	static constexpr float CENTER_X            = 4353.0f;
	static constexpr float WIND_SPHERE_INIT_Y  = 740.0f;
	static constexpr float CLONE_OFFSET_X      = 430.0f;
	static constexpr float CLONE_FLAP_Y        = 755.0f;
	static constexpr float REAL_WAIT_Y         = 560.0f;
	static constexpr float PIT_FLOOR_Y         = 1455.0f;

	static constexpr float MAGMA_PIT_LEFT_X    = 3690.0f;
	static constexpr float MAGMA_PIT_RIGHT_X   = 4946.0f;
	static constexpr int   SUCTION_DURATION    = 420;

	Phase mPhase;
	int   mPhaseTimer;
	int   mAnimType;
	int   mAnimFrame;

	float mStartBossX;
	float mStartBossY;
	float mReturnTargetX;
	float mReturnTargetY;

	float mLeftCloneX;
	float mLeftCloneY;
	float mRightCloneX;
	float mRightCloneY;
	int   mCloneAnimFrame;
	int   mCloneAlpha;

	bool  mbSphereActive;
	float mSphereX;
	float mSphereY;
	float mSphereRadius;
	float mMagmaAbsorbRatio;

	std::vector<SuckedMagmaGlob>     mSuckedMagma;
	std::vector<CrescentBlade>       mCrescentBlades;
	std::vector<SideLightningStrike> mSideLightnings;
	int                              mPlayerFloatWindEfk;
	int                              mSphereTexWindVortex;
	int                              mSphereTexWindSoft;
	int                              mSphereTexElectric;
	int                              mSphereTexThunderArcs[8];
	int                              mExpTexFire[4];
	int                              mExpTexSmoke[4];
	int                              mSphereTexWindVortexSize;
	int                              mSphereTexWindSoftSize;
	int                              mSphereTexElectricSize;
	std::vector<ExplosionPlume>      mExplosionPlumes;
	std::vector<ExplosionSpark>      mExplosionSparks;
	int                              mWhiteFlashAlpha;
	bool  mSphereElectrified;
	int   mSphereElectricityLevel;
	int   mSphereElectricTimer;

	std::vector<GroundCrawlingShock> mGroundShocks;

	std::vector<int> mCloneFlyAnim;
	std::vector<int> mCloneIdleAnim;

	void LoadCloneAnimations();
	void ReleaseCloneAnimations();
	int  LoadGraphWithRecoloredEyes(const char* filePath);

	void SpawnSuckedMagmaGlob(float surfaceY);
	void SpawnCrescentBlade(bool fromLeft);
	void StopSphereEffects();
	void SpawnSideLightning(Player* player);
	void SpawnGroundCrawlingShocks(float startX, float startY);
	void DrawEyeCrossGlint(int eyeSx, int eyeSy, int alpha, float scale = 1.0f, bool megaFlash = false) const;
	void GetDragonEyeScreenPos(
		const VECTOR& bossPos,
		float facingDir,
		float drawScale,
		float footOffset,
		float enemyHeight,
		int animFrame,
		int scrollX,
		int scrollY,
		int& outEyeSx,
		int& outEyeSy
	) const;
};

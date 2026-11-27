#pragma once
#include "DxLib.h"
#include <vector>

class BlockMap;
class Player;

// =========================================================================
// Stage 3（ステージ3）ドラゴン用の新攻撃6：
// 分身メテオシャワー（8秒間）＆ 本体＋分身1体の2秒ごと上・真ん中・下 疾走横突進攻撃
// （Stage3DragonCloneMeteorDashAttack）
// ・分身2体（目の色がエメラルド～イエロー）のうち1匹が地上で上空に特大火の玉を放つ
// ・特大火の玉が上空で炸裂し、8秒間（480フレーム）ステージのランダムな場所へ上から火の玉が降り注ぐ
// ・その8秒間、残りの「本体（赤い目・当たり判定あり）」と「もう1匹の分身（当たり判定なし・突進中フェード）」が
//   上・真ん中・下のどれかにランダム配置され、2秒に1回（計4回）あのかっこいい高速横突進攻撃を繰り出す
// =========================================================================
class Stage3DragonCloneMeteorDashAttack
{
public:
	enum class Phase
	{
		Inactive,               // 非アクティブ
		IntroCastGiantFireball, // 分身1匹が地上で上空へ特大火の玉を放つ（本体も開始位置へ移動）
		MeteorAndDashLoop,      // 8秒間（480フレーム）：上から火の玉が降り注ぎ、2秒（120フレーム）に1回 本体＋分身1体が突進攻撃
		LandAtLedge,            // 8秒終了後、分身が完全に消えて本体が足場に着陸する
		Finished                // 攻撃完了
	};

	Stage3DragonCloneMeteorDashAttack();
	~Stage3DragonCloneMeteorDashAttack();

	void Initialize();
	void Reset();

	// 攻撃開始
	void StartAttack(VECTOR& bossPos, float& facingDir, Player* player);

	// ドラゴンの移動・モーション更新
	void UpdateDragon(
		VECTOR& bossPos,
		float& facingDir,
		bool& isBossGrounded,
		int& outAnimType,
		int& outAnimFrame,
		BlockMap* blockMap
	);

	// 上空への特大火の玉・8秒間降り注ぐ火の玉・疾走エフェクトの毎フレーム更新（当たり判定含む）
	void UpdateProjectilesAndEffects(Player* player, BlockMap* blockMap, int attackDamage);

	// 分身2体・特大火の玉・降り注ぐ火の玉・疾走突進エフェクトの描画
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
	bool IsEyeGlintMoment() const { return (mPhase == Phase::MeteorAndDashLoop && mCycleTimer == 50); }

	// 本体（Stage3BossDragon）の突進斬撃判定が有効かどうか（分身には当たり判定なし）
	bool IsSlashActive() const;

	// Stage3BossDragon と共有するアニメーション種別
	static constexpr int ANIM_IDLE       = 0;
	static constexpr int ANIM_TAKEOFF    = 1;
	static constexpr int ANIM_FLY        = 2;
	static constexpr int ANIM_LANDING    = 3;
	static constexpr int ANIM_ROAR       = 4;
	static constexpr int ANIM_AIR_ATTACK = 5;
	static constexpr int ANIM_ATTACK     = 6;

private:
	// 1. 分身1匹が地上から上空へ打ち上げる特大火の玉
	struct SkyGiantFireball
	{
		bool  active = false;
		float x      = 0.0f;
		float y      = 0.0f;
		float vy     = -22.0f;
		float radius = 64.0f;
	};

	// 2. 8秒間、上空からランダムなステージの場所へ落ちてくる火の玉
	struct FallingMeteorFireball
	{
		bool  active    = false;
		float x         = 0.0f;
		float y         = 0.0f;
		float vx        = 0.0f;
		float vy        = 11.5f;
		float radius    = 26.0f;
		int   warnTimer = 18; // 落下開始前の天井発光予告フレーム
	};

	// 3. 突進中の高速スピードライン（疾走感演出）
	struct SpeedLine
	{
		float        x         = 0.0f;
		float        y         = 0.0f;
		float        vx        = 0.0f;
		float        length    = 180.0f;
		int          thickness = 3;
		int          life      = 0;
		int          maxLife   = 16;
		unsigned int color     = 0;
		bool         isClone   = false;
	};

	// 4. 火球着弾から飛び散る火花パーティクル
	struct FireSpark
	{
		float        x       = 0.0f;
		float        y       = 0.0f;
		float        vx      = 0.0f;
		float        vy      = 0.0f;
		float        length  = 20.0f;
		int          life    = 0;
		int          maxLife = 16;
		unsigned int color   = 0;
	};

	// 6. 本体＋突進分身のX座標残像履歴
	struct DashTrailPoint
	{
		bool  active = false;
		float x      = 0.0f;
		int   alpha  = 0;
	};

	static constexpr int MAX_TRAIL_POINTS = 8;
	static constexpr int LANE_COUNT       = 3; // 0: 上, 1: 真ん中, 2: 下

	// 目の色を変えた分身専用スプライトを読み込み生成する
	void LoadCloneAnimations();
	void ReleaseCloneAnimations();
	int  LoadGraphWithRecoloredEyes(const char* filePath);
	int  SelectDashCloneSpriteHandle(float& outCloneFootOffset) const;
	int  SelectCasterCloneSpriteHandle(float& outCasterFootOffset) const;

	// 2秒ごとの突進サイクル開始時に、上(0)・真ん中(1)・下(2)のどれかに本体と突進分身をランダム配置する
	void ShuffleTwoLanesForCycle();
	void ClearTrailPoints();
	void PushTrailPoint(float x);
	void SpawnMeteorFireball();
	void SpawnImpactSparks(float x, float y, int count, unsigned int mainCol);

	void GetEyeWorldPosition(float posX, float posY, float facingDir, float drawScale, float footOffset, float enemyHeight, bool isAttackAnim, int animFrame, float& outEyeX, float& outEyeY) const;
	void DrawEyeCrossGlint(int eyeSx, int eyeSy, int alpha, bool isClone) const;

	Phase                              mPhase;
	int                                mPhaseTimer;
	int                                mAnimType;
	int                                mAnimFrame;

	// 地上で特大火の玉を上空へ放つ分身1匹目（Caster Clone）の状態
	float                              mCasterCloneX;
	float                              mCasterCloneY;
	float                              mCasterFacingDir;
	int                                mCasterAnimType;
	int                                mCasterAnimFrame;
	int                                mCasterCloneAlpha;
	bool                               mbGiantFireballLaunched;

	// 8秒間（480フレーム）のメテオ＆2秒ごと（120フレーム周期）の本体＋分身2匹目突進の状態
	int                                mMeteorLoopTimer;     // 0 .. 480 (8秒間)
	int                                mCycleIndex;          // 0, 1, 2, 3 (2秒×4回 = 8秒)
	int                                mCycleTimer;          // 0 .. 119 (2秒 = 120フレーム周期)
	bool                               mDashStartFromLeft;   // 今回の突進が左→右か、右→左か
	int                                mRealDragonSlot;      // 本体のレーン (0:上, 1:真ん中, 2:下)
	int                                mDashCloneSlot;       // もう1匹の突進分身のレーン (0:上, 1:真ん中, 2:下)
	float                              mMoveStartX;
	float                              mMoveStartY;
	float                              mStartX;
	float                              mEndX;
	float                              mCurrentDashX;
	int                                mDashCloneAlpha;      // 突進分身の不透明度（突進しながら徐々にフェード）
	int                                mCloneEyeGlintTimer;
	bool                               mbSlashActiveInCycle;

	SkyGiantFireball                   mSkyGiantFireball;
	std::vector<FallingMeteorFireball> mMeteors;
	DashTrailPoint                     mTrailPoints[MAX_TRAIL_POINTS];
	std::vector<SpeedLine>             mSpeedLines;
	std::vector<FireSpark>             mSparks;
	int                                mDashStreakEfk;
	int                                mCloneStreakEfk;
	int                                mFireTex[4];

	// 目の色を変更した分身用アニメーションハンドル
	std::vector<int>                   mCloneIdleAnim;
	std::vector<int>                   mCloneFlyAnim;
	std::vector<int>                   mCloneAttackAnim;
	std::vector<int>                   mCloneRoarAnim;

	// ボス部屋の座標定数
	// 【Stage3.png 5724x1692拡大版 対応】足場・天井・マグマ・3レーン高さを更新
	static constexpr float LEFT_LEDGE_STAND_X  = 3340.0f;
	static constexpr float LEFT_LEDGE_FLOOR_Y  = 1262.0f;
	static constexpr float RIGHT_LEDGE_STAND_X = 5330.0f;
	static constexpr float RIGHT_LEDGE_FLOOR_Y = 1289.0f;
	static constexpr float LEFT_LEDGE_EDGE_X   = 3667.0f;
	static constexpr float RIGHT_LEDGE_EDGE_X  = 4970.0f;
	static constexpr float MAGMA_SURFACE_Y     = 1365.0f;
	static constexpr float ROOM_CEILING_Y      = 490.0f;
	static constexpr float BOSS_HALF_HEIGHT    = 100.0f;

	// 上・真ん中・下の3レーンのY座標
	static constexpr float SLOT_TOP_Y          = 700.0f;  // 上段
	static constexpr float SLOT_MID_Y          = 930.0f;  // 真ん中
	static constexpr float SLOT_BOTTOM_Y       = 1170.0f; // 下段

	// 時間定数（60fps基準）
	static constexpr int INTRO_CAST_FRAMES     = 76;  // 分身1匹目が上空へ特大火球を打ち上げる導入フレーム
	static constexpr int METEOR_TOTAL_FRAMES   = 480; // 8秒間（60fps × 8秒 = 480フレーム）
	static constexpr int DASH_CYCLE_FRAMES     = 120; // 2秒に1回（60fps × 2秒 = 120フレーム周期）
};

#pragma once
#include "DxLib.h"
#include <vector>

class BlockMap;
class Player;

// =========================================================================
// Stage 3（ステージ3）ドラゴン用の新攻撃5：
// 3体同時（本体＋目の色が違う分身体2体）上・真ん中・下 横突進攻撃（Stage3DragonCloneDashAttack）
// ・ドラゴンの分身体を2体出し、分身は本体（赤い目）と違って目の色（エメラルド～イエロー発光）が異なる
// ・上・真ん中・下の3箇所に本体1体と分身2体をランダム配置する
// ・3体同時に横方向へ向かい側の地面（反対側の足場）まで超高速で通常攻撃突進を行う
// ・移動中の疾走感（ソニックブーム衝撃波・高速スピードライン・ジェット気流帯・連続残像）と
//   攻撃エフェクト（三日月型の3連爪斬撃・斬撃火花・眼光の十字閃光）を演出
// ・分身に当たり判定は一切なく、突進攻撃をしながら徐々にフェードアウトし、攻撃が終わると完全に消える
// =========================================================================
class Stage3DragonCloneDashAttack
{
public:
	enum class Phase
	{
		Inactive,            // 非アクティブ
		Takeoff,             // 現在の足場から離陸
		MoveToStartSlot,     // 開始サイドの本体スロット（上・真ん中・下のいずれか）へ移動
		SummonAndCharge,     // 分身体2体（目の色が違う）を出現させ、3体並んで通常攻撃を溜める
		SimultaneousDash,    // 3体同時に横方向へ向かい側の地面まで突進攻撃（分身は徐々にフェードアウト）
		LandOnOppositeLedge, // 分身が完全に消え、本体が向かい側の地面（足場）に着地する
		Finished             // 攻撃完了
	};

	Stage3DragonCloneDashAttack();
	~Stage3DragonCloneDashAttack();

	void Initialize();
	void Reset();

	// 攻撃開始
	void StartAttack(VECTOR& bossPos, float& facingDir);

	// 毎フレーム更新
	void Update(
		VECTOR& bossPos,
		float& facingDir,
		bool& isBossGrounded,
		int& outAnimType,
		int& outAnimFrame,
		BlockMap* blockMap
	);

	// 分身体2体（目の色違い・フェードアウト）および疾走感・斬撃エフェクトの描画
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
	bool IsEyeGlintMoment() const { return (mPhase == Phase::SummonAndCharge && mPhaseTimer == SUMMON_CHARGE_FRAMES - 14); }

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
	// 1. 突進中の高速スピードライン（疾走感演出）
	struct SpeedLine
	{
		float        x       = 0.0f;
		float        y       = 0.0f;
		float        vx      = 0.0f;
		float        length  = 180.0f;
		int          thickness = 3;
		int          life    = 0;
		int          maxLife = 16;
		unsigned int color   = 0;
		int          laneIdx = 0; // 0:上, 1:真ん中, 2:下
	};

	// 2. 3体共通のX座標残像履歴
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
	int  SelectCloneSpriteHandle(float& outCloneFootOffset) const;

	// スプライト内の目のワールド座標を計算（目の輝き強調用）
	void GetEyeWorldPosition(float posX, float posY, float facingDir, float drawScale, float footOffset, float enemyHeight, bool isAttackAnim, int animFrame, float& outEyeX, float& outEyeY) const;

	// 疾走感・攻撃エフェクト描画ヘルパー
	void DrawEyeCrossGlint(int eyeSx, int eyeSy, int alpha, bool isClone) const;

	void ShuffleSlots();
	void PushTrailPoint(float x);
	void UpdateEffects(float facingDir);

	Phase                      mPhase;
	int                        mPhaseTimer;
	int                        mAnimType;
	int                        mAnimFrame;
	int                        mCurrentWave;       // 0: 1回目の突進, 1: 折り返しシャッフル後の2回目の突進
	bool                       mStartFromLeft;     // true: 左側から右側へ突進, false: 右側から左側へ突進
	int                        mRealDragonSlot;    // 本体が配置されるレーン (0:上, 1:真ん中, 2:下)
	int                        mCloneSlots[2];     // 分身2体が配置されるレーン
	float                      mMoveStartX;
	float                      mMoveStartY;
	float                      mStartX;            // 突進開始X座標
	float                      mEndX;              // 向かい側の地面（足場）の到達X座標
	float                      mCurrentDashX;      // 3体共通の現在X座標
	int                        mCloneAlpha;        // 分身の不透明度（255 → 攻撃しながら徐々にフェード → 終了時 0）
	int                        mCloneEyeGlintTimer;

	DashTrailPoint             mTrailPoints[MAX_TRAIL_POINTS];
	std::vector<SpeedLine>     mSpeedLines;
	int                        mDashStreakEfk;
	int                        mCloneStreakEfk[2];

	// 目の色を変更した分身用アニメーションハンドル
	std::vector<int>           mCloneFlyAnim;
	std::vector<int>           mCloneAttackAnim;

	// ボス部屋の座標定数
	// 【Stage3.png 5724x1692拡大版 対応】左右足場・3レーン高さを更新
	static constexpr float LEFT_LEDGE_STAND_X  = 3340.0f;
	static constexpr float LEFT_LEDGE_FLOOR_Y  = 1262.0f;
	static constexpr float RIGHT_LEDGE_STAND_X = 5330.0f;
	static constexpr float RIGHT_LEDGE_FLOOR_Y = 1289.0f;
	static constexpr float BOSS_HALF_HEIGHT    = 100.0f;

	static constexpr float SLOT_TOP_Y          = 700.0f;  // 上段
	static constexpr float SLOT_MID_Y          = 930.0f;  // 真ん中
	static constexpr float SLOT_BOTTOM_Y       = 1170.0f; // 下段（地面すれすれの高さ）

	// 各フェーズのフレーム数
	static constexpr int TAKEOFF_FRAMES        = 26;
	static constexpr int MOVE_SLOT_FRAMES      = 28;
	static constexpr int SUMMON_CHARGE_FRAMES  = 54; // 分身出現＆3体同時溜め時間
	static constexpr int DASH_FRAMES           = 42; // 向かい側の地面まで疾走突進する時間
};

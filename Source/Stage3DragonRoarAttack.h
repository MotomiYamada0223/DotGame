#pragma once
#include "DxLib.h"
#include <vector>

class BlockMap;
class Player;
class Stage3Gimmick;

// =========================================================================
// Stage 3（ステージ3）ドラゴン用の新攻撃：空中咆哮攻撃（Stage3DragonRoarAttack）
// ・ステージの左端の上空（X=3630, Y=660）か右端の上空（X=5440, Y=660）にランダムで飛んで移動
// ・空中でホバリングしながら3秒間（180フレーム）咆哮の力を溜める（予告演出＆範囲ガイド）
// ・空中から咆哮を放ち、ドラゴンのいる方向の足場側「以外」の全範囲にいるプレイヤーに
//   ダメージ0で強力な吹き飛ばし（ノックバック）を与える
// ・咆哮が終わるタイミングで、4つの浮遊台のうちランダムな台を1つ破壊する（10秒で復活）
// ・咆哮終了後に足場へ着地する
// =========================================================================
class Stage3DragonRoarAttack
{
public:
	enum class Phase
	{
		Inactive,   // 非アクティブ
		Takeoff,    // 現在の足場から上空へ離陸
		FlyToEdge,  // ランダムに選んだ左端または右端の上空へ飛行移動
		Charge3Sec, // 空中で3秒間（180フレーム）咆哮を溜める
		RoarBlast,  // 空中から咆哮放出！（反対側全域にダメージなし強吹き飛ばし＋終了時に台1つ破壊）
		LandAtEdge, // 咆哮終了後に端の足場へ着陸
		Finished    // 完了
	};

	Stage3DragonRoarAttack();
	~Stage3DragonRoarAttack();

	void Initialize();
	void Reset(Player* player = nullptr);

	// 空中咆哮攻撃を開始（ステージの左端上空か右端上空かをランダムに選ぶ）
	void StartAttack(VECTOR& bossPos, float& facingDir);

	// 毎フレームの咆哮攻撃更新
	void Update(
		VECTOR& bossPos,
		float& facingDir,
		bool& isBossGrounded,
		int& outAnimType,
		int& outAnimFrame,
		Player* player,
		BlockMap* blockMap,
		Stage3Gimmick* gimmick
	);

	// プレイヤーに付与された吹き飛ばし速度の毎フレーム適用（壁跳ね返り＆カメラ追従付き）
	void UpdatePlayerKnockback(Player* player, BlockMap* blockMap);

	// 咆哮溜めエフェクト・衝撃波リング・暴風ライン・範囲予告の描画
	void Draw(BlockMap* blockMap, const VECTOR& bossPos, float facingDir) const;

	bool IsActive() const { return mPhase != Phase::Inactive && mPhase != Phase::Finished; }
	bool IsFinished() const { return mPhase == Phase::Finished; }
	// 咆哮攻撃中は常時ドラゴン側でY座標を制御するため重力落下を無効化
	bool IsAirbornePhase() const { return mPhase != Phase::Inactive && mPhase != Phase::Finished; }
	bool IsChargingRoar() const { return mPhase == Phase::Charge3Sec; }
	bool IsEyeGlintMoment() const { return (mPhase == Phase::Charge3Sec && mTimer == CHARGE_DURATION_FRAMES - 14); }
	float GetTargetX() const { return mTargetX; }
	float GetTargetFloorY() const { return mTargetFloorY; }

	// Stage3BossDragon へ通知するアニメーション種別
	static constexpr int ANIM_IDLE    = 0;
	static constexpr int ANIM_TAKEOFF = 1;
	static constexpr int ANIM_FLY     = 2;
	static constexpr int ANIM_LANDING = 3;
	static constexpr int ANIM_ROAR    = 4;

private:
	struct ShockRing
	{
		float x, y;
		float radius;
		float maxRadius;
		int   life;
		int   maxLife;
	};

	struct WindLine
	{
		float x, y;
		float vx, vy;
		float length;
		int   life;
		int   maxLife;
	};

	Phase mPhase;
	int   mTimer;
	int   mAnimTimer;
	int   mAnimFrame;

	bool  mbTargetIsLeftEdge; // true: 左端上空(X=3630), false: 右端上空(X=5440)
	float mTargetX;
	float mTargetFloorY;
	bool  mbHasKnockedBackPlayer; // 1回の咆哮につき吹き飛ばしは1回

	// プレイヤーの吹き飛ばし（ノックバック）速度と持続タイマー
	float mPlayerKnockbackVx;
	int   mKnockbackTimer;

	std::vector<ShockRing> mShockRings;
	std::vector<WindLine>  mWindLines;

	// 左端（X = 3384 .. 3851, 床 Y = 1116）と右端（X = 5220 .. 5700, 床 Y = 1152）および空中高度の定数
	// 【Stage3.png 5724x1692拡大版 対応】左右足場(Y=1262/1289)・安全地帯・空中咆哮高度へ更新
	static constexpr float LEFT_LEDGE_STAND_X  = 3340.0f;
	static constexpr float LEFT_LEDGE_FLOOR_Y  = 1262.0f;
	static constexpr float LEFT_SAFE_MAX_X     = 3667.0f; // ドラゴンが左端にいるとき、X <= 3667 は安全

	static constexpr float RIGHT_LEDGE_STAND_X = 5330.0f;
	static constexpr float RIGHT_LEDGE_FLOOR_Y = 1289.0f;
	static constexpr float RIGHT_SAFE_MIN_X    = 4970.0f; // ドラゴンが右端にいるとき、X >= 4970 は安全

	static constexpr float AIR_ROAR_Y          = 810.0f;  // 空中から咆哮する高度

	static constexpr int   CHARGE_DURATION_FRAMES = 180; // 3秒間（60fps × 3 = 180フレーム）空中で溜める
	static constexpr int   ROAR_BLAST_FRAMES      = 65;  // 咆哮放出フレーム数
};

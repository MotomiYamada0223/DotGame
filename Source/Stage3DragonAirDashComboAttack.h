#pragma once
#include "DxLib.h"

class BlockMap;
class Player;

// =========================================================================
// Stage 3（ステージ3）ドラゴン用の新攻撃3：
// 空中溜め2連続突進通常攻撃（Stage3DragonAirDashComboAttack）
// ・空中に飛んでいる状態で通常攻撃（sprite_attack1~3）を溜める
// ・「①プレイヤーの座標を取る → ①突進通常攻撃 → ②またプレイヤーの座標を取る → ②突進通常攻撃」
//   の順番で2回連続でプレイヤーへ向かって空中突進斬撃を行うクラス
// =========================================================================
class Stage3DragonAirDashComboAttack
{
public:
	enum class Phase
	{
		Inactive,       // 非アクティブ
		Takeoff,        // 足場から空中へ離陸
		FlyToReadyPos,  // 空中待機高度へ移動
		Charge1,        // 1回目：開始時にプレイヤーの座標を取得し、空中で通常攻撃を溜める
		DashAttack1,    // 1回目：取得した座標へ向かって突進通常攻撃！
		Charge2,        // 2回目：再びプレイヤーの現在座標を取得し、空中で通常攻撃を溜める
		DashAttack2,    // 2回目：取得した座標へ向かって2回目の突進通常攻撃！
		FlyToLedge,     // 左右どちらかの足場の上空へ飛行移動
		LandAtLedge,    // 足場へ着陸
		Finished        // 攻撃完了
	};

	Stage3DragonAirDashComboAttack();
	~Stage3DragonAirDashComboAttack();

	void Initialize();
	void Reset();

	// 攻撃開始
	void StartAttack(VECTOR& bossPos, float& facingDir, Player* player);

	// 毎フレーム更新
	void Update(
		VECTOR& bossPos,
		float& facingDir,
		bool& isBossGrounded,
		int& outAnimType,
		int& outAnimFrame,
		Player* player,
		BlockMap* blockMap
	);

	// 突進軌道予告・溜めオーラ・斬撃エフェクトの描画
	void Draw(BlockMap* blockMap, const VECTOR& bossPos, float facingDir, float drawScale, int currentSpriteHandle, float footOffset, float enemyHeight) const;

	bool IsActive() const { return mPhase != Phase::Inactive && mPhase != Phase::Finished; }
	bool IsFinished() const { return mPhase == Phase::Finished; }
	bool IsAirbornePhase() const { return mPhase != Phase::Inactive && mPhase != Phase::Finished; }
	bool IsEyeGlintMoment() const { return (mPhase == Phase::Charge1 && mPhaseTimer == CHARGE1_FRAMES - 14) || (mPhase == Phase::Charge2 && mPhaseTimer == CHARGE2_FRAMES - 14); }

	// 現在、突進通常攻撃の斬撃当たり判定が有効かどうか
	bool IsSlashActive() const;

	// Stage3BossDragon と共有するアニメーション種別
	static constexpr int ANIM_IDLE       = 0;
	static constexpr int ANIM_TAKEOFF    = 1;
	static constexpr int ANIM_FLY        = 2;
	static constexpr int ANIM_LANDING    = 3;
	static constexpr int ANIM_ROAR       = 4;
	static constexpr int ANIM_AIR_ATTACK = 5;
	static constexpr int ANIM_ATTACK     = 6; // 通常攻撃アニメーション (sprite_attack1 ~ 9)

private:
	// プレイヤーの現在座標を取得して突進目標座標をロックオンする
	void LockOnPlayerPosition(const VECTOR& bossPos, float& facingDir, Player* player);

	// 残像履歴の更新
	void PushAfterimage(const VECTOR& bossPos, float facingDir, int animFrame);

	struct Afterimage
	{
		bool  active    = false;
		float x         = 0.0f;
		float y         = 0.0f;
		float facingDir = -1.0f;
		int   alpha     = 0;
	};

	static constexpr int MAX_AFTERIMAGES = 6;

	Phase      mPhase;
	int        mPhaseTimer;
	int        mAnimFrame;
	float      mDashStartX;
	float      mDashStartY;
	float      mLockedPlayerX; // 取得したプレイヤー座標X
	float      mLockedPlayerY; // 取得したプレイヤー座標Y
	float      mDashGoalX;     // 突進到達目標X
	float      mDashGoalY;     // 突進到達目標Y
	bool       mReturnToLeftLedge;
	Afterimage mAfterimages[MAX_AFTERIMAGES];

	// ボス部屋の座標定数
	// 【Stage3.png 5724x1692拡大版 対応】5724x1692ボス部屋サイズに合わせて足場・空中突進範囲を更新
	static constexpr float LEFT_LEDGE_STAND_X  = 3340.0f;
	static constexpr float LEFT_LEDGE_FLOOR_Y  = 1262.0f;
	static constexpr float RIGHT_LEDGE_STAND_X = 5330.0f;
	static constexpr float RIGHT_LEDGE_FLOOR_Y = 1289.0f;
	static constexpr float BOSS_HALF_HEIGHT    = 100.0f;

	static constexpr float AIR_READY_Y         = 780.0f;
	static constexpr float MIN_ARENA_X         = 3140.0f;
	static constexpr float MAX_ARENA_X         = 5540.0f;
	static constexpr float MIN_DASH_Y          = 640.0f;
	static constexpr float MAX_DASH_Y          = 1140.0f;

	// 各フェーズのフレーム数
	static constexpr int TAKEOFF_FRAMES        = 28;
	static constexpr int FLY_READY_FRAMES      = 24;
	static constexpr int CHARGE1_FRAMES        = 45; // 1回目の溜め時間（約0.75秒）
	static constexpr int DASH1_FRAMES          = 28; // 1回目の突進通常攻撃時間
	static constexpr int CHARGE2_FRAMES        = 42; // 2回目の溜め時間（約0.7秒）
	static constexpr int DASH2_FRAMES          = 28; // 2回目の突進通常攻撃時間
};

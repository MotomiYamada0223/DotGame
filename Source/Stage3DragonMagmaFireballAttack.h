#pragma once
#include "DxLib.h"
#include <vector>

class BlockMap;
class Player;

// =========================================================================
// Stage 3（ステージ3）ドラゴン用の新攻撃：
// 中央巨大火の玉 ＆ 縦マグマ噴水 ＆ 周囲マグマ飛び散り攻撃（Stage3DragonMagmaFireballAttack）
// ・ステージ2のドラゴンと同様にステージ中央（X=4536）の上空へ飛んで移動
// ・口元に炎をチャージして中央のマグマへ向けてでっかい火の玉を放つ
// ・マグマ（Y=1236）に着弾した瞬間、まず着弾地点に縦にマグマが噴水のように高く吹き上がる
// ・そのあと噴水が弾けて、周囲（左右の浮遊台・足場方向）にマグマ塊が放物線を描いて飛び散る
// =========================================================================
class Stage3DragonMagmaFireballAttack
{
public:
	enum class Phase
	{
		Inactive,        // 非アクティブ
		Takeoff,         // 現在の足場から離陸
		FlyToCenter,     // ステージ中央（X=4536, Y=600）の上空へ飛行移動
		ChargeAndShoot,  // 口元にでっかい火の玉を溜めて真下のマグマへ発射
		HoverWatch,      // マグマ噴水が吹き上がるのを上空で見届ける
		FlyToLedge,      // 左端または右端の足場の上空へ飛行移動
		LandAtLedge,     // 足場へ着陸
		Finished         // ドラゴンの行動完了（飛び散ったマグマ塊は消えるまで更新継続）
	};

	Stage3DragonMagmaFireballAttack();
	~Stage3DragonMagmaFireballAttack();

	void Initialize();
	void Reset();

	// 攻撃開始
	void StartAttack(VECTOR& bossPos, float& facingDir);

	// ドラゴンの移動・モーション更新
	void UpdateDragon(
		VECTOR& bossPos,
		float& facingDir,
		bool& isBossGrounded,
		int& outAnimType,
		int& outAnimFrame,
		float drawScale,
		Player* player
	);

	// 巨大火の玉・縦マグマ噴水・周囲へ飛び散るマグマ塊・火花エフェクトの毎フレーム更新（当たり判定含む）
	void UpdateProjectilesAndEffects(Player* player, BlockMap* blockMap, int attackDamage);

	// 巨大火の玉・縦マグマ噴水・飛び散るマグマ塊・火花エフェクトの描画
	void Draw(BlockMap* blockMap, const VECTOR& bossPos, float facingDir, float drawScale) const;

	bool IsActive() const { return mPhase != Phase::Inactive && mPhase != Phase::Finished; }
	bool IsFinished() const { return mPhase == Phase::Finished; }
	bool IsAirbornePhase() const { return mPhase != Phase::Inactive && mPhase != Phase::Finished; }

	// Stage3BossDragon と共有するアニメーション種別
	static constexpr int ANIM_IDLE       = 0;
	static constexpr int ANIM_TAKEOFF    = 1;
	static constexpr int ANIM_FLY        = 2;
	static constexpr int ANIM_LANDING    = 3;
	static constexpr int ANIM_ROAR       = 4;
	static constexpr int ANIM_AIR_ATTACK = 5;

private:
	// 1. 中央へ撃ち下ろすでっかい火の玉
	struct GiantFireball
	{
		bool  active = false;
		float x      = 4536.0f;
		float y      = 600.0f;
		float vx     = 0.0f;
		float vy     = 0.0f;
		float radius = 58.0f;
	};

	// 2. 着弾地点から縦に吹き上がるマグマの噴水
	struct MagmaFountain
	{
		bool  active        = false;
		float x             = 4536.0f;
		float baseY         = 1236.0f;
		float currentHeight = 0.0f;
		// 【Stage3.png 5724x1692拡大版 対応】マグマ表面(Y=1365)～天井(Y=483)に合わせて噴水高さを調整
		float maxHeight     = 760.0f; // Y=1365 から Y=605 まで高くそびえ立つ縦のマグマ噴水
		float halfWidth     = 58.0f;
		int   timer         = 0;
		int   maxTimer      = 64;
		bool  hasSplashed   = false;
	};

	// 3. 噴水のあとに周囲へ放物線を描いて飛び散るマグマ塊
	struct MagmaSplashGlob
	{
		bool  active = false;
		float x, y;
		float vx, vy;
		float radius;
		int   age;
	};

	// 4. 火の粉・マグマ飛沫パーティクル
	struct LavaSpark
	{
		float        x, y;
		float        vx, vy;
		float        size;
		int          life;
		int          maxLife;
		unsigned int color;
	};

	void GetAirAttackMouthWorldPos(
		const VECTOR& bossPos,
		float facingDir,
		float drawScale,
		int animFrame,
		float& outMouthX,
		float& outMouthY
	) const;

	void SpawnChargeSparks(float mouthX, float mouthY);
	void LaunchGiantFireball(float mouthX, float mouthY);
	void StartMagmaFountain(float impactX);
	void SpawnMagmaSplashWave(float originX, float originY, int waveIndex);

	Phase mPhase;
	int   mTimer;
	int   mAnimTimer;
	int   mAnimFrame;

	bool  mbReturnToLeftLedge;
	float mReturnTargetX;
	float mReturnFloorY;

	GiantFireball                mFireball;
	MagmaFountain                mFountain;
	std::vector<MagmaSplashGlob> mSplashGlobs;
	std::vector<LavaSpark>       mSparks;
	int                          mFireSingleTex;
	int                          mBurstRedTex;
	int                          mSplashTex;

	// 【Stage3.png 5724x1692拡大版 対応】ボス部屋中央・マグマ表面・左右足場座標へ更新
	static constexpr float CENTER_X            = 4353.0f; // ボス部屋・マグマ地帯のど真ん中
	static constexpr float TARGET_HOVER_Y      = 760.0f;  // 中央で火球を撃つ上空高度
	static constexpr float MAGMA_SURFACE_Y     = 1365.0f; // マグマ表面のY座標

	static constexpr float LEFT_LEDGE_STAND_X  = 3340.0f;
	static constexpr float LEFT_LEDGE_FLOOR_Y  = 1262.0f;
	static constexpr float RIGHT_LEDGE_STAND_X = 5330.0f;
	static constexpr float RIGHT_LEDGE_FLOOR_Y = 1289.0f;
};

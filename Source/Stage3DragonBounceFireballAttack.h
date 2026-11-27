#pragma once
#include "DxLib.h"
#include <vector>

class BlockMap;
class Player;

// =========================================================================
// Stage 3（ステージ3）ドラゴン用の新攻撃4：
// 地上バウンド火の玉ブレス ＆ 反射数連動マグマ噴水・飛散攻撃（Stage3DragonBounceFireballAttack）
// ・地上でプレイヤーに向かって火の玉を1つ吐き出す
// ・火の玉は発射角度のまま自然反射で壁・天井に最大4回まで跳ね返り、
//   1回跳ね返るごとに速度と大きさが段階的に増加する
// ・4回反射した後に再び壁にぶつかる（5回目の壁衝突）か、床にぶつかると消滅する
// ・もしマグマ（X=3852..5220, Y=1236）に火の玉が入ると着弾地点から縦にマグマ噴水が噴き出し、その後周囲へ飛び散る
// ・反射回数（0～4回）に応じて噴水の高さ・太さ・マグマ飛び散り数と飛距離が変化し、
//   4反射状態でマグマに入ると中央巨大火の玉攻撃と同等の大噴水＆大飛散が発生する
// =========================================================================
class Stage3DragonBounceFireballAttack
{
public:
	enum class Phase
	{
		Inactive,       // 非アクティブ
		ChargeBreath,   // 地上で口元に火の玉をチャージ
		SpitFireball,   // プレイヤーに向かって火の玉を1つ吐き出す
		Recovery,       // 吐き出し後の地上フォロースルー
		Finished        // ドラゴンのモーション完了（火の玉・噴水・飛び散りマグマは独立して更新継続）
	};

	Stage3DragonBounceFireballAttack();
	~Stage3DragonBounceFireballAttack();

	void Initialize();
	void Reset();

	// 攻撃開始（地上の現在位置から開始）
	void StartAttack(const VECTOR& bossPos, float& facingDir, Player* player);

	// ドラゴンの地上モーション更新
	void UpdateDragon(
		const VECTOR& bossPos,
		float& facingDir,
		bool& isBossGrounded,
		int& outAnimType,
		int& outAnimFrame,
		float drawScale,
		Player* player
	);

	// 反射火の玉・縦マグマ噴水・飛び散るマグマ塊・エフェクトの毎フレーム更新（当たり判定含む）
	void UpdateProjectilesAndEffects(Player* player, BlockMap* blockMap, int attackDamage);

	// 反射火の玉・縦マグマ噴水・飛び散るマグマ塊・エフェクトの描画
	void Draw(BlockMap* blockMap, const VECTOR& bossPos, float facingDir, float drawScale) const;

	bool IsActive() const { return mPhase != Phase::Inactive && mPhase != Phase::Finished; }
	bool IsFinished() const { return mPhase == Phase::Finished; }

	// 火の玉や噴水・飛散マグマがまだ画面内で動作中かどうか
	bool HasActiveProjectiles() const;

	// Stage3BossDragon と共有するアニメーション種別
	static constexpr int ANIM_IDLE       = 0;
	static constexpr int ANIM_TAKEOFF    = 1;
	static constexpr int ANIM_FLY        = 2;
	static constexpr int ANIM_LANDING    = 3;
	static constexpr int ANIM_ROAR       = 4;
	static constexpr int ANIM_AIR_ATTACK = 5;
	static constexpr int ANIM_ATTACK     = 6;
	static constexpr int ANIM_CHARGE     = 7;

private:
	static constexpr int MAX_BOUNCES = 4; // 最大4回まで壁に跳ね返る

	// 1. 壁に跳ね返る火の玉（最大4反射、1回跳ね返るごとに速度と大きさが変化）
	// 反射ごとに 50% の確率でプレイヤーの座標を狙い、50% の確率でマグマへ向かう軌道をとる
	struct BouncingFireball
	{
		bool  active           = false;
		float x                = 0.0f;
		float y                = 0.0f;
		float vx               = 0.0f;
		float vy               = 0.0f;
		float gravityY         = 0.0f; // マグマ軌道選択時に足場の角を越えてマグマへ落ちる放物線重力
		float speed            = 14.5f;
		float radius           = 22.0f;
		int   bounceCount      = 0;    // 現在の反射回数 (0 .. 4)
		bool  isHeadingToMagma = false;
	};

	// 2. マグマに入った地点から縦に吹き上がるマグマ噴水（反射回数0～4に応じて高さ・太さが変化）
	struct MagmaFountain
	{
		bool  active        = false;
		float x             = 4536.0f;
		float baseY         = 1236.0f;
		float currentHeight = 0.0f;
		float maxHeight     = 760.0f;
		float halfWidth     = 76.0f;
		float riseSpeed     = 38.0f;
		int   timer         = 0;
		int   holdFrames    = 26;
		int   bouncePower   = 4; // マグマ突入時の反射回数 (0 .. 4)
		bool  hasSplashed   = false;
	};

	// 3. 噴水が吹き上がったあとに周囲へ飛び散るマグマ塊
	struct MagmaGlob
	{
		bool  active = false;
		float x      = 0.0f;
		float y      = 0.0f;
		float vx     = 0.0f;
		float vy     = 0.0f;
		float radius = 18.0f;
	};

	// 4. 火花・壁バウンド衝撃・消滅エフェクト用パーティクル
	struct FireSpark
	{
		float        x       = 0.0f;
		float        y       = 0.0f;
		float        vx      = 0.0f;
		float        vy      = 0.0f;
		float        size    = 4.0f;
		int          life    = 0;
		int          maxLife = 20;
		unsigned int color   = 0;
	};

	void GetGroundMouthWorldPos(const VECTOR& bossPos, float facingDir, float drawScale, float& outMouthX, float& outMouthY) const;
	static float GetRadiusForBounce(int bounceCount);
	static float GetSpeedForBounce(int bounceCount);

	void TriggerMagmaFountain(float impactX, int bounceCount, BlockMap* blockMap);
	void SpawnMagmaSplashGlobs(float originX, float fountainTopY, int bounceCount);
	void SpawnBurstSparks(float x, float y, int count, float speedScale, unsigned int mainColor);

	Phase                   mPhase;
	int                     mPhaseTimer;
	int                     mAnimFrame;
	bool                    mbFireballShot;
	BouncingFireball        mFireball;
	MagmaFountain           mFountain;
	std::vector<MagmaGlob>  mMagmaGlobs;
	std::vector<FireSpark>  mSparks;
	int                     mFireSingleTex;
	int                     mBurstRedTex;
	int                     mSplashTex;

	// ボス部屋の地形境界定数
	// 【Stage3.png 5724x1692拡大版 対応】ボス部屋の壁・天井・足場・マグマ表面座標へ更新
	static constexpr float ROOM_LEFT_WALL_X    = 3016.0f; // ボス部屋左壁（閉まったゲート壁の内側）
	static constexpr float ROOM_RIGHT_WALL_X   = 5690.0f; // ボス部屋右壁
	static constexpr float ROOM_CEILING_Y      = 483.0f;  // ボス部屋天井
	static constexpr float LEFT_LEDGE_EDGE_X   = 3667.0f; // 左足場の右端（ここから右がマグマピット）
	static constexpr float LEFT_LEDGE_FLOOR_Y  = 1262.0f; // 左足場の床Y
	static constexpr float RIGHT_LEDGE_EDGE_X  = 4970.0f; // 右足場の左端（ここから左がマグマピット）
	static constexpr float RIGHT_LEDGE_FLOOR_Y = 1289.0f; // 右足場の床Y
	static constexpr float MAGMA_SURFACE_Y     = 1365.0f; // マグマ表面Y
	static constexpr float BOSS_HALF_HEIGHT    = 100.0f;

	// モーションフレーム定数
	static constexpr int CHARGE_FRAMES   = 42; // 口元チャージ時間 (dragon_charge1..6 を各7フレーム再生)
	static constexpr int SPIT_FRAMES     = 18; // 火の玉発射フレーム
	static constexpr int RECOVERY_FRAMES = 22; // 発射後の硬直フレーム
};

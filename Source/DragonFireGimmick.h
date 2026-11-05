#pragma once

#include "DxLib.h"
#include <vector>

class Player;
class BlockMap;

// =========================================================================
// ドラゴンの炎・火球・大爆発・アーチ火炎波・延焼地面・火花エフェクト管理クラス
// =========================================================================
class DragonFireGimmick
{
public:
	// 空中火炎ブレスの火炎弾
	struct FireProjectile
	{
		float x, y;
		float vx, vy;
		float radius;
		float maxRadius;
		float swaySeed;
		int animTick;
		bool active;
	};

	// 飛び散る火花スパークパーティクル
	struct FireSpark
	{
		float x, y;
		float vx, vy;
		float size;
		int life;
		int maxLife;
		unsigned int color;
	};

	// 着弾地点から左右へ燃え広がる紅蓮の地面
	struct BurningGround
	{
		float x, y;
		float currentHalfWidth; // 着弾地点から左右へカッコよく燃え広がる現在の幅
		float targetHalfWidth;  // 最大まで広がったときの幅
		int timer;              // 持続フレーム
		int maxTimer;           // 最大持続フレーム
		int age;                // 発生してからの経過フレーム（広がり演出用）
	};

	// 中央に吐き出す巨大火の玉＆周辺大爆発ドーム
	struct CenterFireball
	{
		bool active = false;
		bool exploding = false;
		float x = 8900.0f;
		float y = 1400.0f;
		float vx = 0.0f;
		float vy = 0.0f;
		float radius = 20.0f;
		int explosionTimer = 0;
		int maxExplosionTimer = 32;
		float explosionRadius = 0.0f;
	};

	// 大爆発のあと地面を沿ってアーチ状で左右に広がる炎の波
	struct ArchFlameWave
	{
		float startX;
		float x;
		float y;
		float dir;        // -1.0f = 左へ広がる, +1.0f = 右へ広がる
		float speed;      // 横方向の進行速度
		float archWidth;  // 1アーチの横幅
		float archHeight; // アーチの高さ
		int tick;
		bool active;
	};

	DragonFireGimmick() = default;
	~DragonFireGimmick() = default;

	// 全エフェクト・攻撃オブジェクトをクリア
	void ClearAll();

	// 1. 空中火炎ブレス用：口元への火の粉吸い込み＆火炎ストリーム発射
	void SpawnBreathInhaleSparks(float mouthX, float mouthY);
	void SpawnBreathStream(float mouthX, float mouthY, float targetX, float targetY);

	// 2. 中央巨大火球用：口元へのチャージ火花吸い込み＆巨大火球発射
	void SpawnCenterChargeSparks(float mouthX, float mouthY);
	void LaunchCenterFireball(float mouthX, float mouthY, float targetX, float targetY);

	// 3. 空中急降下ダイブ用：突進軌跡の火花＆着地時の爆散火花
	void SpawnDiveTrailSparks(float x, float y, float diveVx, float diveVy);
	void SpawnDiveLandingSparks(float x, float facingDir);

	// 毎フレームの更新処理（移動・膨張・地面延焼・大爆発・アーチ波・プレイヤー当たり判定）
	void Update(Player* player, BlockMap* blockMap, int attackDamage);

	// 描画処理
	void Draw(BlockMap* blockMap, bool isAirBreathState, bool isCenterChargeState, int animFrame, float mouthWX, float mouthWY) const;

private:
	void UpdateFireProjectiles(Player* player, BlockMap* blockMap, int attackDamage, float floorY);
	void UpdateFireSparks();
	void UpdateBurningGrounds(Player* player, int attackDamage);
	void UpdateCenterFireball(Player* player, BlockMap* blockMap, int attackDamage, float floorY);
	void UpdateArchFlameWaves(Player* player, int attackDamage, float floorY);

	void DrawAirBreathMuzzle(BlockMap* blockMap, int animFrame, float mouthWX, float mouthWY, int now) const;
	void DrawBurningGrounds(BlockMap* blockMap, int now) const;
	void DrawFireProjectiles(BlockMap* blockMap) const;
	void DrawCenterFireballAndCharge(BlockMap* blockMap, bool isCenterChargeState, int animFrame, float mouthWX, float mouthWY, int now) const;
	void DrawArchFlameWaves(BlockMap* blockMap, float floorY, int now) const;
	void DrawFireSparks(BlockMap* blockMap) const;

	std::vector<FireProjectile> mFireProjectiles;
	std::vector<FireSpark> mFireSparks;
	std::vector<BurningGround> mBurningGrounds;
	CenterFireball mCenterFireball;
	std::vector<ArchFlameWave> mArchFlameWaves;
};

#pragma once
#include "DxLib.h"
#include <vector>

class BlockMap;
class Player;

// =========================================================================
// Stage 3（ステージ3）ボス部屋くぼみ用 マグマ（溶岩）ギミッククラス
// 4つの浮遊台の下にあるくぼみ（X = 3852 .. 5219, 床 Y = 1404）に敷き詰められ、
// 波打ち・マグマの泡・火の粉・熱気グローの描画と、触れたプレイヤーへの即死判定を行う
// ・ドラゴン最後の大技で風の引力に吸い込まれると水位が低下し、最後の叩きつけで完全に消滅する
// =========================================================================
class Stage3Magma
{
public:
	Stage3Magma();
	~Stage3Magma();

	// 初期化
	void Initialize();

	// リセット（リトライ時）
	void Reset();

	// 毎フレームの更新（波タイマー、泡・火の粉の生成、プレイヤーとの接触判定）
	void Update(BlockMap& blockMap, Player* player);

	// マグマの描画（くぼみの溶岩層・波・泡・火の粉・照り返し光）
	void Draw(const BlockMap& blockMap);

	// リソース解放
	void Finalize();

	// 風の玉によるマグマ吸引率（0.0f = 通常満水 ～ 1.0f = 完全吸引）
	void  SetDrainRatio(float ratio);
	float GetDrainRatio() const { return mDrainRatio; }

	// 最後の大技フィニッシュ時に地面のマグマを完全に消滅させる
	void  ClearMagma();
	bool  IsMagmaGone() const { return mbMagmaGone || mDrainRatio >= 0.995f; }

	// 現在のマグマ表面Y座標を取得
	float GetCurrentSurfaceY() const
	{
		return MAGMA_SURFACE_Y + (MAGMA_BOTTOM_Y - MAGMA_SURFACE_Y) * mDrainRatio;
	}

private:
	// くぼみの範囲定数（Stage 3 ボス部屋の下部ピット）
	// 【Stage3.png 5724x1692拡大版 対応】マグマピット（X=3667..4970, 穴底Y=1477）に合わせて座標を調整
	static constexpr float MAGMA_LEFT_X    = 3667.0f;
	static constexpr float MAGMA_RIGHT_X   = 4970.0f;
	static constexpr float MAGMA_SURFACE_Y = 1365.0f; // マグマ表面の高さ（左右足場 Y=1262/1289 より少し下）
	static constexpr float MAGMA_BOTTOM_Y  = 1477.0f; // くぼみの底（Y = 1477）

	struct MagmaParticle
	{
		float x, y;
		float vx, vy;
		float size;
		int   life;
		int   maxLife;
		unsigned int color;
		int   type; // 0: 溶岩の泡（表面で弾ける）, 1: 立ち昇る火の粉, 2: マグマしぶき
	};

	int                        mWaveTimer;
	float                      mDrainRatio;
	bool                       mbMagmaGone;
	bool                       mbPlayerInMagma;
	int                        mSplashCooldown;
	struct MagmaSplashEffect
	{
		float x;
		float y;
		int   timer;
		int   maxTimer;
	};

	int                        mSplashTex;
	int                        mFireTex[4];
	int                        mFireSingleTex;
	int                        mBurstRedTex;
	std::vector<MagmaParticle> mParticles;
	std::vector<MagmaSplashEffect> mSplashEffects;

	void SpawnBubble(float x, float y);
	void SpawnEmber(float x, float y);
	void SpawnSplash(float x, float y);
};

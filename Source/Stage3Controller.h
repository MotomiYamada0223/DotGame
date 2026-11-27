#pragma once
#include "DxLib.h"
#include "Stage3Gimmick.h"
#include "Stage3Magma.h"
#include "Stage3BossMovie.h"
#include "Stage3BossCamera.h"
#include "Stage3EffectManager.h"

class BlockMap;
class Player;
class Stage3BossDragon;

// =========================================================================
// Stage 3（ステージ3）専用管理クラス
// ステージ全体の初期化・マップ読み込み・カメラ初期設定・背景描画を担当し、
// ・閉じ込め壁＆上下ふよふよ崩落台ギミック → Stage3Gimmick
// ・くぼみのマグマギミック               → Stage3Magma
// ・奥の足場のドラゴンボス               → Stage3BossDragon
// ・壁が落ちた後のカメラパン演出ムービー → Stage3BossMovie
// ・ボス戦時の引きカメラ制御             → Stage3BossCamera
// にそれぞれ分離委譲する
// =========================================================================
class Stage3Controller
{
public:
	Stage3Controller();
	~Stage3Controller();

	// ステージ3の背景・マップ読み込みと初期化
	void Initialize(BlockMap& blockMap);

	// ステージ3開始時のプレイヤースポーン位置を取得
	VECTOR GetInitialSpawnPosition(const BlockMap& blockMap) const;

	// プレイヤー生成後のカメラ・スクロール初期設定＆ボス配置
	void SetupInitialCamera(BlockMap& blockMap, Player* player);

	// プレイヤー移動前の浮遊台位置更新
	void UpdateBeforePlayerMove(Player* player);

	// 毎フレームのステージ3更新処理
	void Update(BlockMap& blockMap, Player* player);

	// ボス戦用の引きカメラ描画の開始・終了
	void BeginBossCameraDraw();
	void EndBossCameraDraw();

	// マップ奥の背景描画
	void DrawBackground(const BlockMap& blockMap);

	// マップ手前のステージ3ギミック・マグマ描画
	void DrawForeground(const BlockMap& blockMap, Player* player);

	// ステージ3専用の2D Effekseerエフェクト描画
	void DrawEffects(const BlockMap& blockMap);

	// 画面最前面のムービー黒帯・ボスHPバー描画
	void DrawUI(BlockMap& blockMap, Player* player);

	// プレイヤー死亡リトライ時のステージ3再初期化処理
	void ResetOnRespawn(BlockMap& blockMap, Player* player);

	// リソース解放
	void Finalize();

	Stage3Gimmick&       GetGimmick()       { return mGimmick; }
	Stage3Magma&         GetMagma()         { return mMagma; }
	Stage3BossMovie&     GetBossMovie()     { return mBossMovie; }
	Stage3BossCamera&    GetBossCamera()    { return mBossCamera; }
	float GetBossCameraScaleX() const { return mBossCamera.GetViewScaleX(); }
	float GetBossCameraScaleY() const { return mBossCamera.GetViewScaleY(); }
	Stage3EffectManager& GetEffectManager() { return mEffectManager; }
	Stage3BossDragon*    GetBoss()          { return mpBossDragon; }

private:
	void SpawnBossOnFarLedge(BlockMap& blockMap);

	int                 mSkyBackgroundHandle;
	Stage3Gimmick       mGimmick;        // 閉じ込め壁＆4つの浮遊台ギミック
	Stage3Magma         mMagma;          // 台の下のくぼみのマグマギミック
	Stage3BossMovie     mBossMovie;      // 壁が落ちた後のボス紹介カメラムービー
	Stage3BossCamera    mBossCamera;     // ボス戦中の引きカメラ管理
	Stage3EffectManager mEffectManager;  // ステージ3専用Effekseerエフェクト管理
	Stage3BossDragon*   mpBossDragon;    // 奥の足場に配置されるドラゴン
	bool                mbMovieStarted;  // ムービーを開始済みかどうか
};

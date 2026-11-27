#pragma once
#include "DxLib.h"
#include "GameConstants.h"

class BlockMap;
class Player;
class Stage3BossDragon;

// =========================================================================
// Stage 3（ステージ3）ボス戦専用 引きカメラ管理クラス（Stage3BossCamera）
// ・通常道中は等倍（1920x1080）カメラ
// ・ボス戦開始時（壁が閉まり登場ムービーが終わった後）にカメラを滑らかに引き目（2400x1350視野）へズームアウトし、
//   ボス部屋全体（左足場・4つの浮遊台・マグマ・右足場・上空）を一画面に収めて
//   ドラゴンがどこで何の攻撃を準備しているか常に把握できるようにするクラス
// =========================================================================
class Stage3BossCamera
{
public:
	Stage3BossCamera();
	~Stage3BossCamera();

	// 初期化（オフスクリーン描画キャンバスの生成）
	void Initialize();

	// リセット（リトライ時など）
	void Reset();

	// リソース解放
	void Finalize();

	// 毎フレームのボス戦カメラ更新（スクロール座標とズームアウト率の補間）
	void Update(BlockMap& blockMap, Player* player, Stage3BossDragon* boss, bool isBossFightActive);

	// ワールド描画開始（ボス戦カメラ有効時は広角オフスクリーンキャンバスへ描画先を切り替え）
	void BeginWorldDraw();

	// ワールド描画終了（広角オフスクリーンキャンバスを画面 1920x1080 に縮小転送して引きカメラ映像にする）
	void EndWorldDraw();

	bool IsActive() const { return mbActive && mZoomProgress > 0.001f; }
	int  GetCurrentViewWidth()  const { return mCurrentViewWidth; }
	int  GetCurrentViewHeight() const { return mCurrentViewHeight; }
	float GetViewScaleX() const
	{
		return (IsActive() && mCurrentViewWidth > 0)
			? (static_cast<float>(ScreenSize::ScrrenWidth) / static_cast<float>(mCurrentViewWidth))
			: 1.0f;
	}
	float GetViewScaleY() const
	{
		return (IsActive() && mCurrentViewHeight > 0)
			? (static_cast<float>(ScreenSize::ScrrenHeight) / static_cast<float>(mCurrentViewHeight))
			: 1.0f;
	}

	// 【Stage3.png 5724x1692拡大版 対応】少し近づけた広角視野 2880x1620・基準スクロール(2844, 72)に設定
	static constexpr int BOSS_VIEW_WIDTH    = 2880;
	static constexpr int BOSS_VIEW_HEIGHT   = 1620;

	// ボス部屋全域が収まる基準スクロール座標
	static constexpr int BOSS_ROOM_SCROLL_X = 2844;
	static constexpr int BOSS_ROOM_SCROLL_Y = 72;

private:
	int   mCanvasScreenHandle; // 2400x1350 のオフスクリーン描画ハンドル
	bool  mbActive;            // ボス戦カメラが有効か
	bool  mbDrawingToCanvas;   // 現在フレームでオフスクリーンキャンバスに描画中か
	float mZoomProgress;       // 0.0f（通常 1920x1080）～ 1.0f（ボス戦引きカメラ 2400x1350）
	int   mStartScrollX;       // ズームアウト開始時のスクロールX
	int   mStartScrollY;       // ズームアウト開始時のスクロールY
	int   mCurrentViewWidth;   // 現在フレームの視野幅（1920 .. 2400）
	int   mCurrentViewHeight;  // 現在フレームの視野高（1080 .. 1350）
};

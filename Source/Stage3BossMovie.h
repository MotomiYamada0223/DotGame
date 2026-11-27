#pragma once
#include "DxLib.h"

class BlockMap;
class Player;
class Stage3BossDragon;

// =========================================================================
// Stage 3（ステージ3）専用ボス登場＆第2形態覚醒カメラムービークラス（Stage3BossMovie）
// 1. 閉じ込め壁が降りてきた直後の初登場ムービー（StartMovie）
// 2. 第1形態撃破時の第2形態「アルティメットドラゴン」黄金覚醒ムービー（StartAwakeningMovie）
// =========================================================================
class Stage3BossMovie
{
public:
	Stage3BossMovie();
	~Stage3BossMovie();

	void Initialize();
	void Reset();

	// 閉じ込め壁が閉まりきった瞬間に初登場ムービーを開始する
	void StartMovie(BlockMap& blockMap, Player* player, Stage3BossDragon* boss);

	// 第1形態HPが0になった瞬間に「アルティメットドラゴン」黄金覚醒ムービーを開始する
	void StartAwakeningMovie(BlockMap& blockMap, Player* player, Stage3BossDragon* boss);

	// 毎フレームのムービーカメラ＆演出更新（PlayerMoveの後に呼ぶことでカメラ位置を上書き制御）
	void Update(BlockMap& blockMap, Player* player, Stage3BossDragon* boss);

	// ムービー演出の描画（上下のシネマ黒帯・ドラゴン紹介テロップ・覚醒フラッシュ）
	void Draw(const BlockMap& blockMap, Stage3BossDragon* boss) const;

	bool IsPlaying() const { return mPhase != Phase::None && mPhase != Phase::Finished; }
	bool HasFinished() const { return mPhase == Phase::Finished; }

private:
	enum class Phase
	{
		None,                    // 未開始
		WaitAfterGate,           // 壁が閉まった直後の小休止（25フレーム）
		PanToBoss,               // プレイヤー視点 → 奥の足場のドラゴンへカメラを寄せる（85フレーム）
		FocusOnBoss,             // ドラゴンを映して咆哮＆ネーム表示（110フレーム）
		PanBackToPlayer,         // ドラゴン → 元のプレイヤー視点へカメラを戻す（75フレーム）
		AwakenMoveToCenter,      // 第2形態覚醒：ドラゴンが中央へ移動＆カメラが中央へ寄る（70フレーム）
		AwakenFocusAndTransform, // 第2形態覚醒：中央で黄金化して咆哮＆「アルティメットドラゴン」表示（150フレーム）
		AwakenReturnToBattle,    // 第2形態覚醒：定位置へ着地しカメラ復帰＆第2形態バトル開始（65フレーム）
		Finished                 // ムービー完了（プレイヤー操作再開）
	};

	Phase mPhase;
	int   mTimer;
	bool  mbIsAwakeningMovie;

	float mSavedPlayerScrollX; // ムービー開始時のプレイヤーカメラScrollX
	float mSavedPlayerScrollY; // ムービー開始時のプレイヤーカメラScrollY
	float mTargetBossScrollX;  // ドラゴンを映すときの目標ScrollX
	float mTargetBossScrollY;  // ドラゴンを映すときの目標ScrollY
	float mCurrentScrollX;     // ムービー中の現在ScrollX
	float mCurrentScrollY;     // ムービー中の現在ScrollY
	float mLetterboxHeight;    // 上下のシネマ黒帯の現在の高さ（0 .. 90px）

	float mBossStartX;         // 覚醒ムービー開始時のドラゴンX
	float mBossStartY;         // 覚醒ムービー開始時のドラゴンY

	// 【Stage3.png 5724x1692拡大版 対応】覚醒ムービーの中央座標を5724x1692ボス部屋中央へ調整
	static constexpr float AWAKEN_CENTER_X = 4353.0f;
	static constexpr float AWAKEN_CENTER_Y = 860.0f;

	static float SmoothStep(float t);
};

#pragma once
#include "DeathCountEffect.h"
#include <vector>

// ステージ選択の枠の中に、前回の死亡回数分のプレイヤー画像を並べて爆発させるクラス
class SelectDeathDisplay
{
public:
	SelectDeathDisplay();
	~SelectDeathDisplay();

	// 死亡回数と枠の位置・サイズを設定し、演出を最初から始める
	void Setup(int deathCount, int boxX, int boxY, int boxW, int boxH);

	// 画像の切り出し（画像の所有はしない）
	void SetTexture(int graphHandle, int srcX, int srcY, int srcW, int srcH);

	// 出現・割れる演出の更新
	void Update();

	// 枠の中だけに描画する
	void Draw();

	// 演出が全て終わったか
	bool IsFinished() const;

private:
	enum class State
	{
		Idle,    // 設定なし
		Appear,  // 左上から順番に出現
		Hold,    // 全員出現後の待機
		Shatter, // 右側から順番に割れる
		Done     // 演出終了
	};

	DeathCountEffect mEffect;

	State mState = State::Idle;
	int mTimer = 0;

	// 表示する死亡回数
	int mDeathCount = 0;

	// 枠の位置とサイズ
	int mBoxX = 0;
	int mBoxY = 0;
	int mBoxW = 0;
	int mBoxH = 0;

	// アイコン配置
	int mIconsPerRow = 1;
	int mRowHeight = 1;

	// スクロール
	float mMaxScrollY = 0.0f;
	float mScrollY = 0.0f;
	float mScrollTarget = 0.0f;

	// 出現・割れた状態
	int mSpawnedCount = 0;
	int mShatterIndex = 0;
	std::vector<bool> mShattered;
	std::vector<int> mShatterOrder;

	// 画像
	int mTextureHandle = -1;
	int mSrcX = 0;
	int mSrcY = 0;
	int mSrcW = 0;
	int mSrcH = 0;

	// 補助関数
	int ColOf(int idx) const;
	int RowOf(int idx) const;
	float IconScreenLeft(int idx) const;
	float IconScreenTop(int idx) const;
	void ScrollToRow(int row);
	void ShatterIcon(int idx);
};
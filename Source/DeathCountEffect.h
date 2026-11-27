#pragma once
#include "DxLib.h"
#include <vector>
#include <functional>

// 死亡時の破片1枚分
struct Fragment
{
	float x, y;        // 表示位置（画面座標）
	float vx, vy;      // 速度
	int srcX, srcY;    // 元画像の切り出し位置
	int width, height; // 元画像上の切り出しサイズ
	float scale;       // 表示時の倍率（1.0 で等倍）
	int graphHandle;   // 画像ハンドル
	bool isActive;     // 有効かどうか
	int timer;         // 経過フレーム
};

// 矩形（画面座標）がブロックに当たるかを判定する関数
using BlockHitFunc = std::function<bool(float x, float y, float w, float h)>;

class DeathCountEffect
{
public:
	// 破片の元情報
	struct PlayerSpriteSource
	{
		int graphHandle;
		int srcX;
		int srcY;
		int width;
		int height;
	};

	DeathCountEffect();
	~DeathCountEffect();

	// 寿命で消えるかどうか（既定では消える。選択画面では消えないようにする）
	void SetLifeEnabled(bool enabled) { mLifeEnabled = enabled; }

	// 破片を作り直す（既存の破片は消える）
	// scale は表示時の倍率（1.0 で元サイズ）
	void StartShatter(const PlayerSpriteSource& src, float centerX, float centerY, int fragSize, float scale = 1.0f);

	// 破片を追加する（既存の破片は残す）
	void AppendShatter(const PlayerSpriteSource& src, float centerX, float centerY, int fragSize, float scale = 1.0f);

	// 全ての破片を消す
	void Clear();

	// 破片を更新する（isBlocked でブロック衝突を判定する）
	void Update(const BlockHitFunc& isBlocked);

	// 破片を描画する
	void Draw();

	// 全ての破片が消えたか
	bool IsFinished() const;

	// 破片の数（デバッグ用）
	size_t GetFragmentCount() const { return mFragments.size(); }

private:
	std::vector<Fragment> mFragments;
	bool mLifeEnabled = true;
};
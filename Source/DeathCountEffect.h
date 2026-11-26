#pragma once
#include "DxLib.h"
#include <vector>
#include <functional>


// 死亡時の破片1枚分
struct Fragment
{
	float x, y;
	float vx, vy;
	int srcX, srcY;
	int width, height;
	int graphHandle;
	bool isActive;
	int timer;
};

// 死亡回数でプレイヤーを表示させる枠など
class DeathCountEffect
{
public:

	// 矩形（画面座標）がブロックに当たるかを判定する関数
	using BlockHitFunc = std::function<bool(float x, float y, float w, float h)>;

	size_t GetFragmentCount() const { return mFragments.size(); }


	// 破片の元情報
	struct PlayerSpriteSource
	{
		int graphHandle; // 画像ハンドル
		int srcX;        // 切り出し開始X
		int srcY;        // 切り出し開始Y
		int width;       // 幅
		int height;      // 高さ
	};

	DeathCountEffect();
	~DeathCountEffect();

	// 破片を作る（Player 側は画像情報と位置だけ渡す）
	void StartShatter(const PlayerSpriteSource& src, float centerX, float centerY, int fragSize);
	void Update(const BlockHitFunc& isBlocked);
	void Draw();
	bool IsFinished() const;

private:
	std::vector<Fragment> mFragments;
};
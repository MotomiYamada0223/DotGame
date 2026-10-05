#pragma once
#include <string>

/// <summary>
/// 背景画像のスクロールをするクラス
/// 一枚の画像を、描画方法を反転してループさせる。
/// </summary>
class Background
{
public:
	Background();
	~Background();

	// 背景読み込み
	bool Load(const std::string& image_path);

	void Move(int speed, bool scrolling, float blockMap_moveDirection);
	void Draw();
	int GetScrollX() const;

private:
	int mnBackgroundGraph;
	int mnScrollX;
	bool mbIsLoaded; // 読み込み済みか
	float mfScrollSpeed;
};

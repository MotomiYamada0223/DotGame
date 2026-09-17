#pragma once
#include "DxLib.h"

// フォルダにあるフォントをまとめるためのクラス
// Master依存にして、どのクラスからでも呼び出せるように

class FontManager
{
public:
	FontManager();
	~FontManager();

	// Mainで初期化と終了処理を呼び出すため
	void Initialize();
	void Finalize();

	//読み込んだフォントを呼び出すゲッター 大きさ 100
	int GetDotFont_100() const { return mnDotFont100; }

	// 大きさ200
	int GetDotFont_200() const { return mnDotFont200; }

private:

	// フォントを代入するメンバ変数
	int mnDotFont100; // ドットのフォント

	int mnDotFont200; // 大きめのフォント
};
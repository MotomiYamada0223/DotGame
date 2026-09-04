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

	//読み込んだフォントを呼び出すゲッター
	int GetDotFont() const { return mnDotFont; }

private:

	// フォントを代入するメンバ変数
	int mnDotFont; // ドットのフォント
};
#include "FontManager.h"

FontManager::FontManager()
	: mnDotFont(-1)
{

}

FontManager::~FontManager()
{

}

void FontManager::Initialize()
{
	// Resourceにあるフォントを読み込んで使用できるようにするため
	AddFontResourceEx(
		"Resource/Font/dotFont/x12y16pxMaruMonica.ttf", // 読み込むフォントのパス
		FR_PRIVATE,
		NULL
	);


	// 読み込んだフォントの作成
	mnDotFont = CreateFontToHandle(
		"x12y16pxMaruMonica",  // フォントの名前
		100, // フォントの大きさ
		5
	);

	// フォントが読み込まれなかった場合
	if (mnDotFont == -1)
	{
		printfDx("ドットフォントの読み込みに失敗しました。");
		mnDotFont = CreateFontToHandle(
			NULL,
			100,
			5
		);
	}
}

void FontManager::Finalize()
{
	DeleteFontToHandle(mnDotFont);
}
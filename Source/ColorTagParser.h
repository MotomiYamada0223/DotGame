#pragma once
#include <string>
#include <DxLib.h>

class ColorTagParser
{
public:
	// タグ名から対応する色を取得する
	static bool ParseColorTag(const std::string& tagContent, unsigned int& outColor);

	// 文字列中のタグ（<red>や</>など）の長さを取得する（タグでなければ0を返す）
	static int GetTagLength(const std::string& text, int index);
};
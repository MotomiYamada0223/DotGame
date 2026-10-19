#include "ColorTagParser.h"

bool ColorTagParser::ParseColorTag(const std::string& tagContent, unsigned int& outColor)
{
	if (tagContent == "red" || tagContent == "r" || tagContent == "Ô")
	{
		outColor = GetColor(255, 80, 80);
		return true;
	}
	if (tagContent == "blue" || tagContent == "b" || tagContent == "Â" || tagContent == "…")
	{
		outColor = GetColor(80, 200, 255);
		return true;
	}
	if (tagContent == "green" || tagContent == "g" || tagContent == "—Î")
	{
		outColor = GetColor(80, 255, 120);
		return true;
	}
	if (tagContent == "white" || tagContent == "w" || tagContent == "”’")
	{
		outColor = GetColor(255, 255, 255);
		return true;
	}
	if (tagContent == "orange" || tagContent == "o" || tagContent == "žò")
	{
		outColor = GetColor(255, 165, 50);
		return true;
	}
	if (tagContent == "pink" || tagContent == "p" || tagContent == "“")
	{
		outColor = GetColor(255, 130, 220);
		return true;
	}
	if (tagContent == "/" || tagContent == "yellow" || tagContent == "y" || tagContent == "‰©" || tagContent == "default")
	{
		outColor = GetColor(255, 255, 0);
		return true;
	}
	return false;
}

int ColorTagParser::GetTagLength(const std::string& text, int index)
{
	if (index >= static_cast<int>(text.size()) || text[index] != '<') return 0;
	size_t closePos = text.find('>', index);
	if (closePos != std::string::npos && closePos - index <= 12)
	{
		return static_cast<int>(closePos - index + 1);
	}
	return 0;
}
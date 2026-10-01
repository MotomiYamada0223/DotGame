import sys

# TextureAnimation.h
try:
    with open('Source/TextureAnimation.h', 'r', encoding='utf-8-sig') as f:
        h_content = f.read()
except UnicodeDecodeError:
    with open('Source/TextureAnimation.h', 'r', encoding='cp932') as f:
        h_content = f.read()

h_content = h_content.replace('void Draw();   //', 'void Draw(bool turnFlag = false); //')

with open('Source/TextureAnimation.h', 'w', encoding='utf-8-sig') as f:
    f.write(h_content)


# TextureAnimation.cpp
try:
    with open('Source/TextureAnimation.cpp', 'r', encoding='utf-8-sig') as f:
        cpp_content = f.read()
except UnicodeDecodeError:
    with open('Source/TextureAnimation.cpp', 'r', encoding='cp932') as f:
        cpp_content = f.read()

old_draw = """void TextureAnimation::Draw()// `
{
	
	// centerPositionɂΗǂ
	// ̂܂܂ƁA㒆Sł甼aƂȂĂ܂Ă
	// centerPositionɂƁA^񒆂̍W甼aɂȂ̂Ŕ肪Ȃ
	DrawGraph(static_cast<int>(mvPosition.x - (mnxNum / 2)), static_cast<int>(mvPosition.y - (mnyNum / 2)), mnHandleList[mnCurrentNum], true);
}"""

new_draw = """void TextureAnimation::Draw(bool turnFlag)// 描画
{
	int x = static_cast<int>(mvPosition.x - (mnxNum / 2));
	int y = static_cast<int>(mvPosition.y - (mnyNum / 2));
	if (turnFlag)
	{
		DrawTurnGraph(x, y, mnHandleList[mnCurrentNum], true);
	}
	else
	{
		DrawGraph(x, y, mnHandleList[mnCurrentNum], true);
	}
}"""

# 文字化けなどに対応するため正規表現で置換
import re
pattern = r"void TextureAnimation::Draw\(\)//[^\n]*\n\{\s*//[\s\S]*?DrawGraph\([^;]+;\s*\n\}"
match = re.search(pattern, cpp_content)
if match:
    cpp_content = cpp_content[:match.start()] + new_draw + cpp_content[match.end():]
else:
    # 失敗した場合は単純な文字列置換を試す
    cpp_content = cpp_content.replace('void TextureAnimation::Draw()//', 'void TextureAnimation::Draw(bool turnFlag)//')
    cpp_content = cpp_content.replace('DrawGraph(static_cast<int>(mvPosition.x - (mnxNum / 2)), static_cast<int>(mvPosition.y - (mnyNum / 2)), mnHandleList[mnCurrentNum], true);', 
        """int x = static_cast<int>(mvPosition.x - (mnxNum / 2));
\tint y = static_cast<int>(mvPosition.y - (mnyNum / 2));
\tif (turnFlag) DrawTurnGraph(x, y, mnHandleList[mnCurrentNum], true);
\telse DrawGraph(x, y, mnHandleList[mnCurrentNum], true);""")

with open('Source/TextureAnimation.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(cpp_content)


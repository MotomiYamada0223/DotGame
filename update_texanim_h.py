import sys

try:
    with open('Source/TextureAnimation.h', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/TextureAnimation.h', 'r', encoding='cp932') as f:
        content = f.read()

import re

# TextureAnimation.h に関数を追加
new_methods = """	// アニメーションの現在の状態を取得・操作
	int GetCurrentFrame() const { return mnCurrentNum; }
	void ResetAnimation() { mnCurrentNum = 0; mnCounter = 0; }

	// vC[Ŏg"""

content = content.replace("// vC[Ŏg", new_methods)

with open('Source/TextureAnimation.h', 'w', encoding='utf-8-sig') as f:
    f.write(content)


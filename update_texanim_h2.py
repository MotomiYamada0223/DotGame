import sys
import re

try:
    with open('Source/TextureAnimation.h', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/TextureAnimation.h', 'r', encoding='cp932') as f:
        content = f.read()

pattern = r"VECTOR GetPosition\(\) \{ return mvPosition; \}"
replacement = """VECTOR GetPosition() { return mvPosition; }

	// アニメーション制御
	int GetCurrentFrame() const { return mnCurrentNum; }
	void ResetAnimation() { mnCurrentNum = 0; mnCounter = 0; }"""

content = re.sub(pattern, replacement, content)

with open('Source/TextureAnimation.h', 'w', encoding='utf-8-sig') as f:
    f.write(content)


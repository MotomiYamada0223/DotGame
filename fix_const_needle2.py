import sys
import re

def modify():
    try:
        with open('Source/GameConstants.h', 'r', encoding='utf-8-sig') as f:
            content = f.read()
    except UnicodeDecodeError:
        with open('Source/GameConstants.h', 'r', encoding='cp932') as f:
            content = f.read()

    if "Needle.png" not in content:
        content = re.sub(r'(static const std::string Dragon = [^\n]*\n)', r'\1\tstatic const std::string Needle = "Resource/Image/Needle.png"; // 針の罠画像\n', content)

    with open('Source/GameConstants.h', 'w', encoding='utf-8-sig') as f:
        f.write(content)

modify()

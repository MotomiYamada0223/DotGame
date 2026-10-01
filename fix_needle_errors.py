import sys
import re

# 1. Player.cpp のインクルードと攻撃回避ロジック修正
try:
    with open('Source/Player.cpp', 'r', encoding='utf-8-sig') as f:
        p_content = f.read()
except UnicodeDecodeError:
    with open('Source/Player.cpp', 'r', encoding='cp932') as f:
        p_content = f.read()

if '#include "NeedleTrap.h"' not in p_content:
    p_content = p_content.replace('#include "Player.h"', '#include "Player.h"\n#include "NeedleTrap.h"')

with open('Source/Player.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(p_content)


# 2. NeedleTrap.cpp のコンパイルエラー修正
try:
    with open('Source/NeedleTrap.cpp', 'r', encoding='utf-8-sig') as f:
        n_content = f.read()
except UnicodeDecodeError:
    with open('Source/NeedleTrap.cpp', 'r', encoding='cp932') as f:
        n_content = f.read()

# UpdateRect(); を削除
n_content = n_content.replace("UpdateRect();", "// UpdateRect();")

# mpBlockMap->ConvertToScreenX を ConvertToScreenX に変更
n_content = n_content.replace("mpBlockMap->ConvertToScreenX(mvPosition.x, mpBlockMap)", "ConvertToScreenX(mvPosition.x, mpBlockMap)")

# include "Scene.h" の追加
if '#include "Scene.h"' not in n_content:
    n_content = n_content.replace('#include "SceneManager.h"', '#include "SceneManager.h"\n#include "Scene.h"')

with open('Source/NeedleTrap.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(n_content)


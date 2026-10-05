import sys

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        content = f.read()

# ダブっている再定義を削除
content = content.replace("        if (isDamaged) { SetDrawBright(255, 100, 100); }\n\n        const float screenX = ConvertToScreenX(mvPosition.x, mpBlockMap);\n        const float screenY = mvPosition.y;\n", "        if (isDamaged) { SetDrawBright(255, 100, 100); }\n\n")

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

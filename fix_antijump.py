import sys

try:
    with open('Source/NeedleTrap.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/NeedleTrap.cpp', 'r', encoding='cp932') as f:
        content = f.read()

old_cond = "if (distX < 150.0f && playerY < mvPosition.y)"
# 明確にジャンプしている（針の中心より100ピクセル以上高い）場合のみ発動するようにする
new_cond = "if (distX < 150.0f && playerY < mvPosition.y - 100.0f)"

content = content.replace(old_cond, new_cond)

with open('Source/NeedleTrap.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

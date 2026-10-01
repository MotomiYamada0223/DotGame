import sys

try:
    with open('Source/NeedleTrap.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/NeedleTrap.cpp', 'r', encoding='cp932') as f:
        content = f.read()

# スケールを 3.0 に変更
content = content.replace("DrawRotaGraph(drawX, drawY, 2.0, 0.0, mGraphHandle, TRUE);", "DrawRotaGraph(drawX, drawY, 3.0, 0.0, mGraphHandle, TRUE);")

# 当たり判定も 96x96 に合わせる
content = content.replace("mfEnemyWidth = 64.0f;", "mfEnemyWidth = 96.0f;")
content = content.replace("mfEnemyHeight = 64.0f;", "mfEnemyHeight = 96.0f;")

with open('Source/NeedleTrap.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

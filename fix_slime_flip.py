import sys

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        content = f.read()

# !mIsFacingRight を mIsFacingRight に変更
old_turn = "bool turnFlag = !mIsFacingRight;"
new_turn = "bool turnFlag = mIsFacingRight;"
content = content.replace(old_turn, new_turn)

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)


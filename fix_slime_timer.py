import sys

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        content = f.read()

old_logic = """        // 1b3bŎ̍s
        mActionTimer = 60 + GetRand(120);"""

new_logic = """        // 1枚8フレーム × 6枚 = 1ループ48フレーム
        // 1〜3回モーション(ループ)を行ったら次の行動へ移るようにする
        int loopCount = 1 + GetRand(2); // 1, 2, 3
        mActionTimer = loopCount * 48;"""

if old_logic in content:
    content = content.replace(old_logic, new_logic)
else:
    # コメントが文字化けしている可能性があるので正規表現
    import re
    pattern = r"mActionTimer\s*=\s*60\s*\+\s*GetRand\(120\);"
    content = re.sub(pattern, new_logic, content)

# さらに初期化部分も60から48に変更しておく
content = content.replace("mActionTimer = 60;", "mActionTimer = 48;")

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

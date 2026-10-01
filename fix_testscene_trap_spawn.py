import sys

try:
    with open('Source/TestScene.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/TestScene.cpp', 'r', encoding='cp932') as f:
        content = f.read()

old_str = """				EnemySlime* slime = new EnemySlime(VGet(1000.0f, 300, 0.0f));
		slime->UpdateStatusByProgress(mProgress);
	}"""
new_str = """				EnemySlime* slime = new EnemySlime(VGet(1000.0f, 300, 0.0f));
		slime->UpdateStatusByProgress(mProgress);

		// トラップのテスト配置 (スライムと同じY座標 300 付近に配置)
		new NeedleTrap(VGet(500.0f, 300.0f, 0.0f), TrapType::PopUp);
		new NeedleTrap(VGet(1500.0f, 300.0f, 0.0f), TrapType::AntiJump);
	}"""

if "new NeedleTrap" not in content:
    content = content.replace(old_str, new_str)

with open('Source/TestScene.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

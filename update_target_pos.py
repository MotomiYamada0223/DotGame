import sys
import re

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        content = f.read()

old_logic = """            // 2枚目(インデックス1)の時にプレイヤー座標を記録
            if (currentFrame == 1 && mTargetPlayerPos.x == 0.0f && mTargetPlayerPos.y == 0.0f)
            {
                mTargetPlayerPos = playerObj->GetPosition();
            }"""

new_logic = """            // 2枚目(インデックス1)の時にプレイヤー座標を記録
            if (currentFrame == 1 && mTargetPlayerPos.x == 0.0f && mTargetPlayerPos.y == 0.0f)
            {
                mTargetPlayerPos = playerObj->GetPosition();
                // プレイヤーの少し奥(100.0f)を目標にする
                if (mTargetPlayerPos.x > enemyX) {
                    mTargetPlayerPos.x += 100.0f;
                } else {
                    mTargetPlayerPos.x -= 100.0f;
                }
            }"""

if "mTargetPlayerPos = playerObj->GetPosition();" in content:
    content = content.replace(old_logic, new_logic)
else:
    print("Failed to replace target pos logic")

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)


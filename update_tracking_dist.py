import sys

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        content = f.read()

import re

old_logic = r"// 逃げ切られたかの判定（距離か、扇状のY幅から外れたか）\s*float dx = std::abs\(playerX - enemyX\);\s*float dy = std::abs\(playerY - enemyY\);\s*float maxDy = \(mVisionBaseHeight / 2\.0f\) \+ dx \* std::tan\(mVisionAngle \* 0\.01745329f\);\s*// 視界の長さから逃げ切られたか、高さ\(ジャンプ等\)で視界から外れたら見失う\s*if \(dx > mVisionLength || dy > maxDy\)\s*\{\s*mIsChasing = false;\s*\}"

new_logic = """// 逃げ切られたかの判定（直線距離が600以上になったら見失う）
            float dx = std::abs(playerX - enemyX);
            float dy = std::abs(playerY - enemyY);
            float dist = std::sqrt(dx * dx + dy * dy);
            
            if (dist >= mVisionLength)
            {
                mIsChasing = false;
            }"""

match = re.search(old_logic, content)
if match:
    content = content[:match.start()] + new_logic + content[match.end():]
else:
    print("Failed to replace tracking distance logic")

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)


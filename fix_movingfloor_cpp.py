import sys

with open('Source/MovingFloor.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()

# mIsPlayerRiding = true; の後に SetVelocityY(0.0f); を追加
lines = content.split('\n')
for i, line in enumerate(lines):
    if "mIsPlayerRiding = true;" in line:
        lines.insert(i + 1, "        mpPlayer->SetVelocityY(0.0f); // 重力リセット")
        break

with open('Source/MovingFloor.cpp', 'w', encoding='utf-8-sig') as f:
    f.write('\n'.join(lines))

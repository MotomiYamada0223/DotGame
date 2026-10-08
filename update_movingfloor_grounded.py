import sys

with open('Source/MovingFloor.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()

lines = content.split('\n')
for i, line in enumerate(lines):
    if "mpPlayer->SetVelocityY(0.0f);" in line:
        lines.insert(i + 1, "        mpPlayer->SetGrounded(true); // 接地フラグを強制的にTrueにしてジャンプや歩行を可能にする")
        break

with open('Source/MovingFloor.cpp', 'w', encoding='utf-8-sig') as f:
    f.write('\n'.join(lines))

import sys

with open('Source/MovingFloor.cpp', 'r', encoding='utf-8-sig') as f:
    lines = f.readlines()

for i, line in enumerate(lines):
    if "mpPlayer->SetForceGrounded();" in line:
        lines.insert(i + 1, "        mpPlayer->SetVelocityY(0.0f); // 着地時の落下速度の残存による次フレームのすり抜けを防ぐため必須\n")
        break

with open('Source/MovingFloor.cpp', 'w', encoding='utf-8-sig') as f:
    f.writelines(lines)

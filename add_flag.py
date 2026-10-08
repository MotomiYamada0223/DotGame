import sys

with open('Source/Player.h', 'r', encoding='utf-8-sig') as f:
    lines = f.readlines()

for i, line in enumerate(lines):
    if "bool isGrounded;" in line:
        lines.insert(i + 1, "\tbool mForceGroundedThisFrame; // 外部オブジェクトにより強制接地させるフラグ\n")
        break

with open('Source/Player.h', 'w', encoding='utf-8-sig') as f:
    f.writelines(lines)

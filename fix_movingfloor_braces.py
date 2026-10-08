import sys

with open('Source/MovingFloor.cpp', 'r', encoding='utf-8-sig') as f:
    lines = f.readlines()

start_idx = -1
for i, line in enumerate(lines):
    if "else if (mPattern == MovePattern::OneWayDestroy) {" in line:
        start_idx = i
        break

if start_idx != -1:
    lines[start_idx+4] = "        // X座標による消滅判定は除外し、Y座標が画面の上下200ピクセル外に出たら破棄する\n"
    lines[start_idx+5] = "        if (nextPos.y < -200.0f || nextPos.y > ScreenSize::ScrrenHeight + 200.0f) {\n"
    lines[start_idx+6] = "            SetDeleteFlag(true);\n"
    lines[start_idx+7] = "        }\n"
    lines[start_idx+8] = "    }\n"
    lines[start_idx+9] = "\n"

with open('Source/MovingFloor.cpp', 'w', encoding='utf-8-sig') as f:
    f.writelines(lines)

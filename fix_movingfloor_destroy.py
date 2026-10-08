import sys

with open('Source/MovingFloor.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()

old_logic = """    else if (mPattern == MovePattern::OneWayDestroy) {
        // 移動
        nextPos = VAdd(prevPos, currentVelocity);
        
        // 画面外に大きく出たら破棄
        if (nextPos.y < -1500.0f || nextPos.y > ScreenSize::ScrrenHeight + 1500.0f ||
            nextPos.x < -1500.0f || nextPos.x > ScreenSize::ScrrenWidth + 1500.0f) {
            SetDeleteFlag(true);
        }
    }"""

# 文字化け対策で柔軟に置換
lines = content.split('\n')
for i, line in enumerate(lines):
    if "mPattern == MovePattern::OneWayDestroy" in line:
        # この行から数行を書き換える
        lines[i+4] = "        float screenX = ConvertToScreenX(nextPos.x, mpBlockMap);"
        lines[i+5] = "        if (nextPos.y < -1500.0f || nextPos.y > ScreenSize::ScrrenHeight + 1500.0f ||"
        lines[i+6] = "            screenX < -1500.0f || screenX > ScreenSize::ScrrenWidth + 1500.0f) {"
        break

with open('Source/MovingFloor.cpp', 'w', encoding='utf-8-sig') as f:
    f.write('\n'.join(lines))

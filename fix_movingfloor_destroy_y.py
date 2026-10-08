import sys
import re

with open('Source/MovingFloor.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()

old_logic = """          float screenX = ConvertToScreenX(nextPos.x, mpBlockMap);
          if (nextPos.y < -1500.0f || nextPos.y > ScreenSize::ScrrenHeight + 1500.0f ||
              screenX < -1500.0f || screenX > ScreenSize::ScrrenWidth + 1500.0f) {
              SetDeleteFlag(true);
          }"""

new_logic = """          // X座標による消滅判定は除外し、Y座標が画面の上下200ピクセル外に出たら破棄する
          if (nextPos.y < -200.0f || nextPos.y > ScreenSize::ScrrenHeight + 200.0f) {
              SetDeleteFlag(true);
          }"""

# 正規表現で置換
content = re.sub(r"          float screenX = ConvertToScreenX\(nextPos\.x, mpBlockMap\);\s*if \(nextPos\.y < -1500\.0f.*?SetDeleteFlag\(true\);\s*\}", new_logic, content, flags=re.DOTALL)

with open('Source/MovingFloor.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)


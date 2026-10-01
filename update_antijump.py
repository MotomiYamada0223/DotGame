import sys
import re

try:
    with open('Source/NeedleTrap.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/NeedleTrap.cpp', 'r', encoding='cp932') as f:
        content = f.read()

# AntiJumpの判定部分を探して置換
old_antijump_cond = """        else if (mType == TrapType::AntiJump)
        {
            // プレイヤーが針より高く飛んでいる時に反応
            if (distX < 150.0f && playerY < mvPosition.y - 100.0f)
            {
                mState = TrapState::Active;
            }
        }"""

new_antijump_cond = """        else if (mType == TrapType::AntiJump)
        {
            // x座標の感知範囲を狭める (150 -> 60)
            if (distX < 60.0f && playerY < mvPosition.y - 100.0f)
            {
                mState = TrapState::Active;
                // 感知した瞬間のプレイヤーのY座標を基準とし、さらに100ピクセル上（マイナス方向）を目標にする
                mTargetY = playerY - 100.0f;
            }
        }"""

# コメントの文字化けなども考慮して正規表現で置換
pattern = r"else if \(mType == TrapType::AntiJump\)\s*\{\s*//[^\n]*\n\s*if \(distX < 150\.0f && playerY < mvPosition\.y - 100\.0f\)\s*\{\s*mState = TrapState::Active;\s*\}\s*\}"
match = re.search(pattern, content)
if match:
    content = content[:match.start()] + new_antijump_cond + content[match.end():]
else:
    # パターンが見つからなかった場合、手動で直接置換を試みる
    # distX < 150.0f を distX < 60.0f に
    # mState = TrapState::Active; の後に mTargetY = playerY - 100.0f; を追加
    content = re.sub(
        r"if \(distX < 150\.0f && playerY < mvPosition\.y - 100\.0f\)\s*\{\s*mState = TrapState::Active;\s*\}",
        "if (distX < 60.0f && playerY < mvPosition.y - 100.0f)\n            {\n                mState = TrapState::Active;\n                mTargetY = playerY - 100.0f;\n            }",
        content
    )

with open('Source/NeedleTrap.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

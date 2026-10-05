import sys

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        content = f.read()

import re

old_logic = r"// 目標座標へ向かうX速度を計算 \(着地まで約30フレームと仮定\)\s*mJumpSpeedX = \(mTargetPlayerPos\.x - enemyX\) / 30\.0f;\s*// 最大速度を制限\s*if \(mJumpSpeedX > 8\.0f\) mJumpSpeedX = 8\.0f;\s*if \(mJumpSpeedX < -8\.0f\) mJumpSpeedX = -8\.0f;"

new_logic = """// アニメーションは全7枚、インデックス3(4枚目)から飛びつき開始
                // SetInterval(6)の場合、残り4枚(3,4,5,6) × 6フレーム = 約24フレーム
                // 攻撃モーション終了時(約24フレーム後)に目標のX座標へ必ず到達するように速度を算出
                mJumpSpeedX = (mTargetPlayerPos.x - enemyX) / 24.0f;
                // ※制限は設けず、算出した速度で目標座標に確実に到達させる"""

match = re.search(old_logic, content)
if match:
    content = content[:match.start()] + new_logic + content[match.end():]
else:
    print("Failed to replace jump speed logic")

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)


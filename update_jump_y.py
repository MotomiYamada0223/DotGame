import sys
import re

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        content = f.read()

old_logic = """                mbIsJumping = true;
                velocityY = -10.0f; // 弧を描くための上方向への初速
                mHasJumped = true;
                
                // アニメーションは全7枚、インデックス3(4枚目)から飛びつき開始
                // SetInterval(6)の場合、残り4枚(3,4,5,6) × 6フレーム = 約24フレーム
                // 攻撃モーション終了時(約24フレーム後)に目標のX座標へ必ず到達するように速度を算出
                mJumpSpeedX = (mTargetPlayerPos.x - enemyX) / 24.0f;
                // ※制限は設けず、算出した速度で目標座標に確実に到達させる"""

new_logic = """                mbIsJumping = true;
                mHasJumped = true;
                
                // アニメーションは全7枚、インデックス3(4枚目)から飛びつき開始
                // SetInterval(6)の場合、残り4枚(3,4,5,6) × 6フレーム = 約24フレーム
                float jumpFrames = 24.0f;
                
                // 攻撃モーション終了時に目標のX座標へ必ず到達するように速度を算出
                mJumpSpeedX = (mTargetPlayerPos.x - enemyX) / jumpFrames;
                
                // 攻撃モーション終了時に目標のY座標（あるいは着地）に必ず到達するように初速を算出
                // 等加速度直線運動: Δy = v0 * t + 0.5 * g * t^2
                // v0 = (Δy / t) - 0.5 * g * t
                float deltaY = mTargetPlayerPos.y - enemyY;
                velocityY = (deltaY / jumpFrames) - (0.5f * gravity * jumpFrames);"""

if "velocityY = -10.0f;" in content:
    content = content.replace(old_logic, new_logic)
else:
    print("Failed to replace jump logic")

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)


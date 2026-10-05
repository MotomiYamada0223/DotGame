import sys

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        content = f.read()

import re

old_logic = r"if \(mIsChasing\)\s*\{\s*// --- 追跡中の処理 ---\s*// 常にプレイヤーの方向に視界\(自分の向き\)と移動方向を合わせる（振り向く）\s*if \(playerX > enemyX\) \{\s*mIsFacingRight = true;\s*mCurrentMoveDirection = 1\.0f;\s*\} else \{\s*mIsFacingRight = false;\s*mCurrentMoveDirection = -1\.0f;\s*\}\s*// 逃げ切られたかの判定（直線距離が600以上になったら見失う）\s*float dx = std::abs\(playerX - enemyX\);\s*float dy = std::abs\(playerY - enemyY\);\s*float dist = std::sqrt\(dx \* dx \+ dy \* dy\);\s*if \(dist >= mVisionLength\)\s*\{\s*mIsChasing = false;\s*\}\s*\}\s*else\s*\{\s*// --- 未発見（徘徊中）の処理 ---\s*// 扇状の視界判定\s*float dx = mIsFacingRight \? \(playerX - enemyX\) : \(enemyX - playerX\);\s*float dy = std::abs\(playerY - enemyY\);\s*// 向きの方向にいて、長さ以内か\s*if \(dx >= 0\.0f && dx <= mVisionLength\)\s*\{\s*float maxDy = \(mVisionBaseHeight / 2\.0f\) \+ dx \* std::tan\(mVisionAngle \* 0\.01745329f\);\s*if \(dy <= maxDy\)\s*\{\s*mIsChasing = true;\s*// 発見した瞬間に向きを更新\s*mCurrentMoveDirection = mIsFacingRight \? 1\.0f : -1\.0f;\s*\}\s*\}\s*\}"

new_logic = """if (mIsChasing)
        {
            // --- 追跡中の処理 ---
            mActionTimer--;
            if (mActionTimer <= 0)
            {
                if (mCurrentMoveDirection == 0.0f)
                {
                    // Idle終わり -> 追跡開始 (3〜4秒)
                    mActionTimer = 180 + GetRand(60);
                    mCurrentMoveDirection = mIsFacingRight ? 1.0f : -1.0f;
                }
                else
                {
                    // 追跡終わり -> Idle開始 (1〜2ループ = 48〜96フレーム)
                    int idleLoops = 1 + GetRand(1);
                    mActionTimer = idleLoops * 48;
                    mCurrentMoveDirection = 0.0f;
                }
            }
            
            // 常にプレイヤーの方向に視界(自分の向き)を合わせる（振り向く）
            if (playerX > enemyX) {
                mIsFacingRight = true;
                if (mCurrentMoveDirection != 0.0f) mCurrentMoveDirection = 1.0f;
            } else {
                mIsFacingRight = false;
                if (mCurrentMoveDirection != 0.0f) mCurrentMoveDirection = -1.0f;
            }
            
            // 逃げ切られたかの判定（直線距離が600以上になったら見失う）
            float dx = std::abs(playerX - enemyX);
            float dy = std::abs(playerY - enemyY);
            float dist = std::sqrt(dx * dx + dy * dy);
            
            if (dist >= mVisionLength)
            {
                mIsChasing = false;
            }
        }
        else
        {
            // --- 未発見（徘徊中）の処理 ---
            // 扇状の視界判定
            float dx = mIsFacingRight ? (playerX - enemyX) : (enemyX - playerX);
            float dy = std::abs(playerY - enemyY);
            
            // 向きの方向にいて、長さ以内か
            if (dx >= 0.0f && dx <= mVisionLength)
            {
                float maxDy = (mVisionBaseHeight / 2.0f) + dx * std::tan(mVisionAngle * 0.01745329f);
                
                if (dy <= maxDy)
                {
                    mIsChasing = true;
                    // 発見した瞬間に追跡モード(Move)のタイマーをセットする
                    mActionTimer = 180 + GetRand(60); // 3〜4秒
                    mCurrentMoveDirection = mIsFacingRight ? 1.0f : -1.0f;
                }
            }
        }"""

match = re.search(old_logic, content)
if match:
    content = content[:match.start()] + new_logic + content[match.end():]
else:
    print("Failed to replace chasing idle logic")

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)


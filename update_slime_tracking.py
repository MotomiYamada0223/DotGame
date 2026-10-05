import sys

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        content = f.read()

import re

# 追跡ロジックの入れ替え
old_logic = r"ObjectManager\* objManager = Master::mpGameManager->GetSceneManager\(\)->GetCurrentScene\(\)->GetObjectManager\(\);\s*Object2D\* playerObj = objManager->GetObject2DByTag\(Object2D::Tag::Player2D\);\s*mIsChasing = false;\s*mVisionLength = 600\.0f;\s*mVisionBaseHeight = 50\.0f;\s*mVisionAngle = 15\.0f;\s*if \(playerObj != nullptr\)\s*\{\s*float playerX = playerObj->GetPosition\(\)\.x;\s*float playerY = playerObj->GetPosition\(\)\.y;\s*float enemyX = mvPosition\.x;\s*float enemyY = mvPosition\.y;\s*// 扇状の視界判定\s*float dx = mIsFacingRight \? \(playerX - enemyX\) : \(enemyX - playerX\);\s*float dy = std::abs\(playerY - enemyY\);\s*// 向きの方向にいて、長さ以内か\s*if \(dx >= 0\.0f && dx <= mVisionLength\)\s*\{\s*// 距離に応じたY方向の許容幅を計算 \(tanを使用、角度はラジアンに変換\)\s*// 3\.14159265f / 180\.0f = 0\.01745329f\s*float maxDy = \(mVisionBaseHeight / 2\.0f\) \+ dx \* std::tan\(mVisionAngle \* 0\.01745329f\);\s*if \(dy <= maxDy\)\s*\{\s*mIsChasing = true;\s*mCurrentMoveDirection = mIsFacingRight \? 1\.0f : -1\.0f;\s*\}\s*\}\s*\}"

new_logic = """ObjectManager* objManager = Master::mpGameManager->GetSceneManager()->GetCurrentScene()->GetObjectManager();
    Object2D* playerObj = objManager->GetObject2DByTag(Object2D::Tag::Player2D);
    
    // コンストラクタで初期化されているのでここで再代入しなくてよいが、一応残しておく
    mVisionLength = 600.0f;
    mVisionBaseHeight = 50.0f;
    mVisionAngle = 15.0f;
    
    if (playerObj != nullptr)
    {
        float playerX = playerObj->GetPosition().x;
        float playerY = playerObj->GetPosition().y;
        float enemyX = mvPosition.x;
        float enemyY = mvPosition.y;
        
        if (mIsChasing)
        {
            // --- 追跡中の処理 ---
            // 常にプレイヤーの方向に視界(自分の向き)と移動方向を合わせる（振り向く）
            if (playerX > enemyX) {
                mIsFacingRight = true;
                mCurrentMoveDirection = 1.0f;
            } else {
                mIsFacingRight = false;
                mCurrentMoveDirection = -1.0f;
            }
            
            // 逃げ切られたかの判定（距離か、扇状のY幅から外れたか）
            float dx = std::abs(playerX - enemyX);
            float dy = std::abs(playerY - enemyY);
            float maxDy = (mVisionBaseHeight / 2.0f) + dx * std::tan(mVisionAngle * 0.01745329f);
            
            // 視界の長さから逃げ切られたか、高さ(ジャンプ等)で視界から外れたら見失う
            if (dx > mVisionLength || dy > maxDy)
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
                    // 発見した瞬間に向きを更新
                    mCurrentMoveDirection = mIsFacingRight ? 1.0f : -1.0f;
                }
            }
        }
    }"""

match = re.search(old_logic, content)
if match:
    content = content[:match.start()] + new_logic + content[match.end():]
else:
    print("Failed to replace tracking logic")

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

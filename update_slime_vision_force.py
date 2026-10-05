import sys
import re

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        lines = f.readlines()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        lines = f.readlines()

new_lines = []
skip = False

# Drawメソッドの中身を書き換え
for line in lines:
    if "// ---" in line and "デバッグ表示" in line:
        skip = True
        new_lines.append("""        // --- 視界の扇状(台形)デバッグ表示 ---
        float baseHalfY = mVisionBaseHeight / 2.0f;
        float farHalfY = baseHalfY + mVisionLength * std::tan(mVisionAngle * 0.01745329f);
        
        int x1, y1, x2, y2, x3, y3, x4, y4;
        
        if (mIsFacingRight)
        {
            // 時計回りに頂点を指定
            x1 = static_cast<int>(screenX);                   y1 = static_cast<int>(screenY - baseHalfY); // 左上
            x2 = static_cast<int>(screenX + mVisionLength);   y2 = static_cast<int>(screenY - farHalfY);  // 右上
            x3 = static_cast<int>(screenX + mVisionLength);   y3 = static_cast<int>(screenY + farHalfY);  // 右下
            x4 = static_cast<int>(screenX);                   y4 = static_cast<int>(screenY + baseHalfY); // 左下
        }
        else
        {
            // 時計回りに指定 (DXライブラリではDrawQuadrangleは時計回りまたは反時計回りでねじれないように)
            x1 = static_cast<int>(screenX);                   y1 = static_cast<int>(screenY - baseHalfY); // 右上
            x2 = static_cast<int>(screenX);                   y2 = static_cast<int>(screenY + baseHalfY); // 右下
            x3 = static_cast<int>(screenX - mVisionLength);   y3 = static_cast<int>(screenY + farHalfY);  // 左下
            x4 = static_cast<int>(screenX - mVisionLength);   y4 = static_cast<int>(screenY - farHalfY);  // 左上
        }
        
        // 追跡中は赤、待機中は黄色
        int color = mIsChasing ? GetColor(255, 0, 0) : GetColor(255, 255, 0);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawQuadrangle(x1, y1, x2, y2, x3, y3, x4, y4, color, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        // ------------------------\n""")
        continue
        
    if skip and "// ------------------------" in line:
        skip = False
        continue
        
    if not skip:
        new_lines.append(line)

lines = new_lines
new_lines = []
skip = False

# EnemyMoveメソッドの中身を書き換え
in_enemy_move = False
for line in lines:
    if "void EnemySlime::EnemyMove(BlockMap& blockMap)" in line:
        in_enemy_move = True
    
    if in_enemy_move and "if (playerObj != nullptr)" in line:
        skip = True
        new_lines.append(line)
        new_lines.append("""    {
        float playerX = playerObj->GetPosition().x;
        float playerY = playerObj->GetPosition().y;
        float enemyX = mvPosition.x;
        float enemyY = mvPosition.y;
        
        // 扇状の視界判定
        float dx = mIsFacingRight ? (playerX - enemyX) : (enemyX - playerX);
        float dy = std::abs(playerY - enemyY);
        
        // 向きの方向にいて、長さ以内か
        if (dx >= 0.0f && dx <= mVisionLength)
        {
            // 距離に応じたY方向の許容幅を計算 (tanを使用、角度はラジアンに変換)
            // 3.14159265f / 180.0f = 0.01745329f
            float maxDy = (mVisionBaseHeight / 2.0f) + dx * std::tan(mVisionAngle * 0.01745329f);
            
            if (dy <= maxDy)
            {
                mIsChasing = true;
                mCurrentMoveDirection = mIsFacingRight ? 1.0f : -1.0f;
            }
        }
    }\n""")
        continue
        
    if skip and "if (mIsChasing)" in line:
        skip = False
        in_enemy_move = False
        
    if not skip:
        new_lines.append(line)

content = "".join(new_lines)
if "<cmath>" not in content:
    content = content.replace('#include "Master.h"', '#include "Master.h"\n#include <cmath>')

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

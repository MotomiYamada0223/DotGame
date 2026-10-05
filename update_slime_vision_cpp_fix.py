import sys

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        content = f.read()

# mVisionLength 等の初期化
if "mVisionLength" not in content:
    content = content.replace("mIsChasing = false;", "mIsChasing = false;\n    mVisionLength = 600.0f;\n    mVisionBaseHeight = 50.0f;\n    mVisionAngle = 15.0f;")

import re

# EnemyMoveの視界判定の置換
old_vision = r"if \(std::abs\(playerY - enemyY\) < 100\.0f\)\s*\{\s*if \(mIsFacingRight\)\s*\{\s*// 右向き：前方に200ピクセル\s*if \(playerX > enemyX && \(playerX - enemyX\) <= 200\.0f\)\s*\{\s*mIsChasing = true;\s*mCurrentMoveDirection = 1\.0f; // 右へ追いかける\s*\}\s*\}\s*else\s*\{\s*// 左向き：前方に200ピクセル\s*if \(playerX < enemyX && \(enemyX - playerX\) <= 200\.0f\)\s*\{\s*mIsChasing = true;\s*mCurrentMoveDirection = -1\.0f; // 左へ追いかける\s*\}\s*\}\s*\}"

new_vision = """// 扇状の視界判定
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
        }"""

match = re.search(old_vision, content)
if match:
    content = content[:match.start()] + new_vision + content[match.end():]
else:
    print("Failed to replace vision logic in EnemyMove")

# Drawメソッドのデバッグ表示の置換
old_draw = r"// --- 視界のデバッグ表示 ---\s*int visionLeft, visionRight, visionTop, visionBottom;\s*visionTop = static_cast<int>\(screenY - 100\.0f\);\s*visionBottom = static_cast<int>\(screenY \+ 100\.0f\);\s*if \(mIsFacingRight\)\s*\{\s*visionLeft = static_cast<int>\(screenX\);\s*visionRight = static_cast<int>\(screenX \+ 200\.0f\);\s*\}\s*else\s*\{\s*visionLeft = static_cast<int>\(screenX - 200\.0f\);\s*visionRight = static_cast<int>\(screenX\);\s*\}\s*// 追跡中は赤、待機中は黄色\s*int color = mIsChasing \? GetColor\(255, 0, 0\) : GetColor\(255, 255, 0\);\s*SetDrawBlendMode\(DX_BLENDMODE_ALPHA, 100\);\s*DrawBox\(visionLeft, visionTop, visionRight, visionBottom, color, TRUE\);\s*SetDrawBlendMode\(DX_BLENDMODE_NOBLEND, 0\);\s*// ------------------------"

new_draw = """// --- 視界の扇状(台形)デバッグ表示 ---
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
        // ------------------------"""

match2 = re.search(old_draw, content)
if match2:
    content = content[:match2.start()] + new_draw + content[match2.end():]
else:
    print("Failed to replace Draw logic")

# cmathの追加
if "<cmath>" not in content:
    content = content.replace('#include "Master.h"', '#include "Master.h"\n#include <cmath>')

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)


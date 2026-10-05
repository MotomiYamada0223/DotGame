import sys
import re

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        content_cpp = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        content_cpp = f.read()

# コンストラクタでの初期化
if "mVisionLength =" not in content_cpp:
    content_cpp = content_cpp.replace("mIsChasing = false;", "mIsChasing = false;\n    mVisionLength = 200.0f;\n    mVisionBaseHeight = 50.0f;\n    mVisionAngle = 15.0f;")

# EnemyMoveの視界判定の置換
old_vision_logic = """        // 視界のY幅 (上下100以内)
        if (std::abs(playerY - enemyY) < 100.0f)
        {
            if (mIsFacingRight)
            {
                // 右向き：前方に200ピクセル
                if (playerX > enemyX && (playerX - enemyX) <= 200.0f)
                {
                    mIsChasing = true;
                    mCurrentMoveDirection = 1.0f; // 右へ追いかける
                }
            }
            else
            {
                // 左向き：前方に200ピクセル
                if (playerX < enemyX && (enemyX - playerX) <= 200.0f)
                {
                    mIsChasing = true;
                    mCurrentMoveDirection = -1.0f; // 左へ追いかける
                }
            }
        }"""

new_vision_logic = """        // 扇状の視界判定
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
content_cpp = content_cpp.replace(old_vision_logic, new_vision_logic)

# Drawメソッドのデバッグ表示の置換
old_draw_logic = """        // --- 視界のデバッグ表示 ---
        int visionLeft, visionRight, visionTop, visionBottom;
        visionTop = static_cast<int>(screenY - 100.0f);
        visionBottom = static_cast<int>(screenY + 100.0f);
        
        if (mIsFacingRight)
        {
            visionLeft = static_cast<int>(screenX);
            visionRight = static_cast<int>(screenX + 200.0f);
        }
        else
        {
            visionLeft = static_cast<int>(screenX - 200.0f);
            visionRight = static_cast<int>(screenX);
        }
        
        // 追跡中は赤、待機中は黄色
        int color = mIsChasing ? GetColor(255, 0, 0) : GetColor(255, 255, 0);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawBox(visionLeft, visionTop, visionRight, visionBottom, color, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        // ------------------------"""

new_draw_logic = """        // --- 視界の扇状(台形)デバッグ表示 ---
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
            // 反時計回りに頂点を指定
            x1 = static_cast<int>(screenX);                   y1 = static_cast<int>(screenY - baseHalfY); // 右上
            x2 = static_cast<int>(screenX - mVisionLength);   y2 = static_cast<int>(screenY - farHalfY);  // 左上
            x3 = static_cast<int>(screenX - mVisionLength);   y3 = static_cast<int>(screenY + farHalfY);  // 左下
            x4 = static_cast<int>(screenX);                   y4 = static_cast<int>(screenY + baseHalfY); // 右下
        }
        
        // 追跡中は赤、待機中は黄色
        int color = mIsChasing ? GetColor(255, 0, 0) : GetColor(255, 255, 0);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawQuadrangle(x1, y1, x2, y2, x3, y3, x4, y4, color, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        // ------------------------"""
content_cpp = content_cpp.replace(old_draw_logic, new_draw_logic)

# インクルードに <cmath> を追加 (std::tan 用)
if "<cmath>" not in content_cpp:
    content_cpp = content_cpp.replace('#include "Master.h"', '#include "Master.h"\n#include <cmath>')

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content_cpp)


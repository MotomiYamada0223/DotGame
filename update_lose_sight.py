import sys
import re

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        content = f.read()

# 追跡解除の判定を距離のみにする
old_logic = """            float maxDy = (mVisionBaseHeight / 2.0f) + dx * std::tan(mVisionAngle * 0.01745329f);
            
            // E̒瓦؂ꂽA(Wv)ŎEOꂽ猩
            if (dx > mVisionLength || dy > maxDy)
            {
                mIsChasing = false;
            }"""

new_logic = """            float dist = std::sqrt(dx * dx + dy * dy);
            
            // 距離が視界の長さ(600)以上離れたら見失う（高低差は無視して追い続ける）
            if (dist >= mVisionLength)
            {
                mIsChasing = false;
            }"""

if "std::tan(mVisionAngle" in content:
    # 愚直なreplaceが文字化けコメントで失敗しないように正規表現を使う
    pattern = r"float\s*maxDy\s*=\s*\(mVisionBaseHeight\s*/\s*2\.0f\)\s*\+\s*dx\s*\*\s*std::tan\(mVisionAngle\s*\*\s*0\.01745329f\);\s*//[^\n]*\n\s*if\s*\(dx\s*>\s*mVisionLength\s*\|\|\s*dy\s*>\s*maxDy\)\s*\{\s*mIsChasing\s*=\s*false;\s*\}"
    content = re.sub(pattern, new_logic, content)

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

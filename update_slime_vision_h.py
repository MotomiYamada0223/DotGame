import sys

# EnemySlime.h
try:
    with open('Source/EnemySlime.h', 'r', encoding='utf-8-sig') as f:
        content_h = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.h', 'r', encoding='cp932') as f:
        content_h = f.read()

if "float mVisionLength;" not in content_h:
    content_h = content_h.replace("bool mIsChasing;", "bool mIsChasing;\n    float mVisionLength;\n    float mVisionBaseHeight;\n    float mVisionAngle;")

with open('Source/EnemySlime.h', 'w', encoding='utf-8-sig') as f:
    f.write(content_h)

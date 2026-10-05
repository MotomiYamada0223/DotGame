import sys
import re

# TextureAnimation.h に SetInterval を追加
try:
    with open('Source/TextureAnimation.h', 'r', encoding='utf-8-sig') as f:
        content_h = f.read()
except UnicodeDecodeError:
    with open('Source/TextureAnimation.h', 'r', encoding='cp932') as f:
        content_h = f.read()

if "void SetInterval(int interval)" not in content_h:
    content_h = content_h.replace("void SetPosition(VECTOR centerPosition)", "void SetInterval(int interval) { mnInterval = interval; }\n\tvoid SetPosition(VECTOR centerPosition)")

with open('Source/TextureAnimation.h', 'w', encoding='utf-8-sig') as f:
    f.write(content_h)

# EnemySlime.cpp に速度変更処理を追加
try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        content_cpp = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        content_cpp = f.read()

old_chase_logic = """    // 追跡中でない場合はランダム徘徊
    if (!mIsChasing)
    {
        mActionTimer--;"""
        
new_chase_logic = """    if (mIsChasing)
    {
        mMoveSpeed = 2.0f;
        if (mpAnimMove) mpAnimMove->SetInterval(7);
    }
    else
    {
        mMoveSpeed = 1.0f;
        if (mpAnimMove) mpAnimMove->SetInterval(8);
    }

    // 追跡中でない場合はランダム徘徊
    if (!mIsChasing)
    {
        mActionTimer--;"""

content_cpp = content_cpp.replace(old_chase_logic, new_chase_logic)

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content_cpp)


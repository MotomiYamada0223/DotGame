import sys
import re

try:
    with open('Source/EnemySlime.h', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.h', 'r', encoding='cp932') as f:
        content = f.read()

pattern = r"float mVisionAngle;"
replacement = """float mVisionAngle;

    bool mIsAttacking;
    int mAttackCooldownTimer;
    VECTOR mTargetPlayerPos;
    bool mHasJumped;
    float mJumpSpeedX;"""

content = re.sub(pattern, replacement, content)

with open('Source/EnemySlime.h', 'w', encoding='utf-8-sig') as f:
    f.write(content)


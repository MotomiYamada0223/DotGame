import sys

# --- UnitStatus.h ---
try:
    with open('Source/UnitStatus.h', 'r', encoding='utf-8-sig') as f:
        us_content = f.read()
except UnicodeDecodeError:
    with open('Source/UnitStatus.h', 'r', encoding='cp932') as f:
        us_content = f.read()

if "mIgnoresInvincibility" not in us_content:
    us_content = us_content.replace(
        "bool mHasInstantKillAttack;", 
        "bool mHasInstantKillAttack;\n\tbool mIgnoresInvincibility; // ブリンクや被弾後無敵を貫通するかどうか"
    )

with open('Source/UnitStatus.h', 'w', encoding='utf-8-sig') as f:
    f.write(us_content)


# --- UnitStatus.cpp ---
try:
    with open('Source/UnitStatus.cpp', 'r', encoding='utf-8-sig') as f:
        usc_content = f.read()
except UnicodeDecodeError:
    with open('Source/UnitStatus.cpp', 'r', encoding='cp932') as f:
        usc_content = f.read()

if "mIgnoresInvincibility" not in usc_content:
    usc_content = usc_content.replace(
        ", mHasInstantKillAttack(false)", 
        ", mHasInstantKillAttack(false), mIgnoresInvincibility(false)"
    )
    if "mIgnoresInvincibility(false)" not in usc_content:
        # 万が一見つからなかった場合
        usc_content = usc_content.replace(
            "mHasInstantKillAttack = false;", 
            "mHasInstantKillAttack = false;\n\tmIgnoresInvincibility = false;"
        )

with open('Source/UnitStatus.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(usc_content)


# --- NeedleTrap.cpp ---
try:
    with open('Source/NeedleTrap.cpp', 'r', encoding='utf-8-sig') as f:
        nt_content = f.read()
except UnicodeDecodeError:
    with open('Source/NeedleTrap.cpp', 'r', encoding='cp932') as f:
        nt_content = f.read()

if "mIgnoresInvincibility = true;" not in nt_content:
    nt_content = nt_content.replace(
        "mHasInstantKillAttack = true;", 
        "mHasInstantKillAttack = true;\n    mIgnoresInvincibility = true;"
    )

with open('Source/NeedleTrap.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(nt_content)


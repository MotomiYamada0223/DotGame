import sys

try:
    with open('Source/Player.cpp', 'r', encoding='utf-8-sig') as f:
        lines = f.readlines()
except UnicodeDecodeError:
    with open('Source/Player.cpp', 'r', encoding='cp932') as f:
        lines = f.readlines()

new_lines = []
for line in lines:
    if '#include "Player.h"' in line:
        new_lines.append(line)
        new_lines.append('#include "NeedleTrap.h"\n')
    else:
        new_lines.append(line)
lines = new_lines

# 衝突処理部分を特定して置き換える
# 412~438
start_idx = -1
end_idx = -1
for i, line in enumerate(lines):
    if "if (Collision::CheckRectToRect(myPos, mySize, enePos, eneSize))" in line:
        start_idx = i
        break

if start_idx != -1:
    for i in range(start_idx, len(lines)):
        if "if (hasAttackRect)" in lines[i]:
            end_idx = i - 2
            break

if start_idx != -1 and end_idx != -1:
    new_block = """		if (Collision::CheckRectToRect(myPos, mySize, enePos, eneSize))
		{
			UnitStatus* enemyStatus = dynamic_cast<UnitStatus*>(enemy);
			bool ignoresInvincible = (enemyStatus && enemyStatus->mIgnoresInvincibility);

			if (ignoresInvincible)
			{
				int dmg = enemyStatus ? enemyStatus->mAttack : 0;
				if (enemyStatus && enemyStatus->mHasInstantKillAttack) dmg = mHp;
				TakeDamage(dmg);
			}
			else if (isBlinking)
			{
				if (!mIsBlinkWallDeathImmune)
				{
					TakeDamage(mHp);
				}
			}
			else
			{
				isHitDamage = true;
				if (mInvincibleTimer <= 0 && mDeadState == 0)
				{
					mInvincibleTimer = 60;
					if (enemyStatus)
					{
						int dmg = enemyStatus->mAttack;
						if (enemyStatus->mHasInstantKillAttack) dmg = mHp;
						TakeDamage(dmg);
					}
				}
			}
		}\n"""
    lines[start_idx:end_idx+1] = [new_block]

# 攻撃無視の部分 (if (!alreadyHit))
atk_idx = -1
for i, line in enumerate(lines):
    if "if (!alreadyHit)" in line and "enemy->OnDamaged();" in lines[i+2]:
        atk_idx = i
        break

if atk_idx != -1:
    ignore_code = """				NeedleTrap* trap = dynamic_cast<NeedleTrap*>(enemy);
				if (trap != nullptr)
				{
					continue;
				}\n"""
    lines.insert(atk_idx, ignore_code)

with open('Source/Player.cpp', 'w', encoding='utf-8-sig') as f:
    f.writelines(lines)

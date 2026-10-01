import sys
import re

try:
    with open('Source/Player.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/Player.cpp', 'r', encoding='cp932') as f:
        content = f.read()

pattern = r"if \(Collision::CheckRectToRect\(myPos, mySize, enePos, eneSize\)\)\s*\{[\s\S]*?mInvincibleTimer = 60;[^\}]*\}\s*\}\s*\}"

new_collision = """if (Collision::CheckRectToRect(myPos, mySize, enePos, eneSize))
		{
			UnitStatus* enemyStatus = dynamic_cast<UnitStatus*>(enemy);
			bool ignoresInvincible = (enemyStatus && enemyStatus->mIgnoresInvincibility);

			if (ignoresInvincible)
			{
				int dmg = enemyStatus->mAttack;
				if (enemyStatus->mHasInstantKillAttack) dmg = mHp;
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
		}"""

content = re.sub(pattern, new_collision, content)

with open('Source/Player.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

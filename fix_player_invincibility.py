import sys

try:
    with open('Source/Player.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/Player.cpp', 'r', encoding='cp932') as f:
        content = f.read()

# 既存の判定部分を探して置換
old_collision = """		if (Collision::CheckRectToRect(myPos, mySize, enePos, eneSize))
		{
			if (isBlinking)
			{
				if (!mIsBlinkWallDeathImmune)
				{
					// ブリンク失敗なら即死
					TakeDamage(mHp);
				}
				// 無敵状態なら何もしない
			}
			else
			{
				isHitDamage = true;
				if (mInvincibleTimer <= 0 && mDeadState == 0)
				{
					mInvincibleTimer = 60; // 1秒無敵
					UnitStatus* enemyStatus = dynamic_cast<UnitStatus*>(enemy);
					if (enemyStatus)
					{
						int dmg = enemyStatus->mAttack;
						if (enemyStatus->mHasInstantKillAttack) dmg = mHp;
						TakeDamage(dmg);
					}
				}
			}
		}"""

new_collision = """		if (Collision::CheckRectToRect(myPos, mySize, enePos, eneSize))
		{
			UnitStatus* enemyStatus = dynamic_cast<UnitStatus*>(enemy);
			bool ignoresInvincible = (enemyStatus && enemyStatus->mIgnoresInvincibility);

			if (ignoresInvincible)
			{
				// 無敵を貫通する罠などの場合、常にダメージを受ける
				int dmg = enemyStatus->mAttack;
				if (enemyStatus->mHasInstantKillAttack) dmg = mHp;
				TakeDamage(dmg);
			}
			else if (isBlinking)
			{
				if (!mIsBlinkWallDeathImmune)
				{
					// ブリンク失敗なら即死
					TakeDamage(mHp);
				}
				// 無敵状態なら何もしない
			}
			else
			{
				isHitDamage = true;
				if (mInvincibleTimer <= 0 && mDeadState == 0)
				{
					mInvincibleTimer = 60; // 1秒無敵
					if (enemyStatus)
					{
						int dmg = enemyStatus->mAttack;
						if (enemyStatus->mHasInstantKillAttack) dmg = mHp;
						TakeDamage(dmg);
					}
				}
			}
		}"""

if "bool ignoresInvincible =" not in content:
    # 古い文字列が見つからなかった場合のために、もうちょっと柔軟に探す
    if old_collision in content:
        content = content.replace(old_collision, new_collision)
    else:
        # 古い文字列が少し違う可能性があるので正規表現で
        import re
        pattern = r"if \(Collision::CheckRectToRect\(myPos, mySize, enePos, eneSize\)\)\s*\{[\s\S]*?\}\s*\}\s*\}"
        match = re.search(pattern, content)
        if match:
            # 危険なので直接の文字列置換ができなかったらログに出す
            print("Regex match found, but standard replace failed.")
            print(match.group(0))

with open('Source/Player.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

import sys

try:
    with open('Source/Player.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/Player.cpp', 'r', encoding='cp932') as f:
        content = f.read()

# NeedleTrap.h の include 追加
if '#include "NeedleTrap.h"' not in content:
    content = content.replace('#include "Player.h"', '#include "Player.h"\n#include "NeedleTrap.h"')

# 攻撃無視のロジック追加 (2. 攻撃と敵の衝突判定内)
old_atk = """				if (!alreadyHit)
				{
					enemy->OnDamaged();"""
new_atk = """				// 針トラップには攻撃無効
				NeedleTrap* trap = dynamic_cast<NeedleTrap*>(enemy);
				if (trap != nullptr)
				{
					continue;
				}

				if (!alreadyHit)
				{
					enemy->OnDamaged();"""
if "NeedleTrap* trap" not in content:
    content = content.replace(old_atk, new_atk)


# 無敵貫通ロジック追加 (1. プレイヤーと敵の衝突判定内)
old_player_dmg = """		// 1. プレイヤーと敵の衝突判定 (ダメージを受ける)
		if (Collision::CheckRectToRect(myPos, mySize, enePos, eneSize))
		{
			if (isBlinking)
			{
				if (!mIsBlinkWallDeathImmune)
				{
					// ブリンク失敗なら即死
					TakeDamage(mHp);
				}
				// 免疫ありなら何もしない(すり抜け)
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

new_player_dmg = """		// 1. プレイヤーと敵の衝突判定 (ダメージを受ける)
		if (Collision::CheckRectToRect(myPos, mySize, enePos, eneSize))
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

import re
# 文字化け対策として、正規表現でブロックを特定する
pattern = r"if \(Collision::CheckRectToRect\(myPos, mySize, enePos, eneSize\)\)\s*\{[\s\S]*?mInvincibleTimer = 60;[\s\S]*?TakeDamage\(dmg\);\s*\}\s*\}\s*\}"
match = re.search(pattern, content)
if match:
    content = content[:match.start()] + new_player_dmg.replace('// 1. プレイヤーと敵の衝突判定 (ダメージを受ける)\n\t\t', '') + content[match.end():]

with open('Source/Player.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

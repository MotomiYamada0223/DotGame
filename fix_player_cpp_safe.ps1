import sys
import re

def modify():
    with open('Source/Player.cpp', 'r', encoding='cp932') as f:
        content = f.read()

    # mInvincibleTimerの初期化 (Player::Player の最初の方)
    if "mInvincibleTimer = 0;" not in content:
        content = content.replace("isHitDamage = false;\n\tisDead = false;", "isHitDamage = false;\n\tmInvincibleTimer = 0;\n\tisDead = false;")

    # Updateの最初
    # --- 当たり判定 --- の下に isHitDamage = false; がある
    if "mInvincibleTimer--;" not in content:
        content = content.replace("isHitDamage = false;\n\n\tObjectManager", "isHitDamage = false;\n\tif (mInvincibleTimer > 0) mInvincibleTimer--;\n\n\tObjectManager")

    # 1. プレイヤー自身と敵の衝突判定 (ダメージで赤くする)
    # これを置き換える。
    old_player_col = """		// 1. プレイヤー自身と敵の衝突判定 (ダメージで赤くする)
		if (Collision::CheckRectToRect(myPos, mySize, enePos, eneSize))
		{
			isHitDamage = true;
		}"""
    
    new_player_col = """		// 1. プレイヤー自身と敵の衝突判定 (ダメージで赤くする)
		if (Collision::CheckRectToRect(myPos, mySize, enePos, eneSize))
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
		}"""
    content = content.replace(old_player_col, new_player_col)

    # 2. 攻撃判定と敵の衝突判定 (敵を赤く光らせる)
    old_enemy_col = """		// 2. 攻撃判定と敵の衝突判定 (敵を赤く光らせる)
		if (hasAttackRect)
		{
			if (Collision::CheckRectToRect(atkPos, atkSize, enePos, eneSize))
			{
				enemy->OnDamaged();
			}
		}"""
        
    new_enemy_col = """		// 2. 攻撃判定と敵の衝突判定 (敵を赤く光らせる)
		if (hasAttackRect)
		{
			if (Collision::CheckRectToRect(atkPos, atkSize, enePos, eneSize))
			{
				enemy->OnDamaged();
				UnitStatus* enemyStatus = dynamic_cast<UnitStatus*>(enemy);
				if (enemyStatus)
				{
					enemyStatus->TakeDamage(mAttack);
					if (enemyStatus->mHp <= 0)
					{
						enemy->SetDeleteFlag(true);
					}
				}
			}
		}"""
    content = content.replace(old_enemy_col, new_enemy_col)

    with open('Source/Player.cpp', 'w', encoding='cp932') as f:
        f.write(content)

modify()

import sys

def modify():
    with open('Source/Player.cpp', 'r', encoding='cp932', errors='ignore') as f:
        content = f.read()

    # コンストラクタ初期化
    if "mInvincibleTimer = 0;" not in content:
        content = content.replace("isHitDamage = false;", "isHitDamage = false;\n\tmInvincibleTimer = 0;")

    # タイマー更新
    if "mInvincibleTimer > 0" not in content:
        content = content.replace("isHitDamage = false;", "isHitDamage = false;\n\tif (mInvincibleTimer > 0) mInvincibleTimer--;")

    # 衝突処理の書き換え (正規表現か直接置換か)
    # 元:
    # 		if (Collision::CheckRectToRect(myPos, mySize, enePos, eneSize))
    #		{
    #			isHitDamage = true;
    #		}
    old_col_player = """		// 1. vC[gƓG̏Փ˔ (_[WŐԂ)
		if (Collision::CheckRectToRect(myPos, mySize, enePos, eneSize))
		{
			isHitDamage = true;
		}"""

    new_col_player = """		// 1. プレイヤーと敵の衝突
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
    
    if "TakeDamage(dmg)" not in content:
        content = content.replace(old_col_player, new_col_player)
    
    # 敵へのダメージ処理
    # 		if (hasAttackRect)
    #		{
    #			if (Collision::CheckRectToRect(atkPos, atkSize, enePos, eneSize))
    #			{
    #				enemy->OnDamaged();
    #			}
    #		}
    old_col_enemy = """		// 2. UƓG̏Փ˔ (GԂ点)
		if (hasAttackRect)
		{
			if (Collision::CheckRectToRect(atkPos, atkSize, enePos, eneSize))
			{
				enemy->OnDamaged();
			}
		}"""
        
    new_col_enemy = """		// 2. 攻撃と敵の衝突
		if (hasAttackRect)
		{
			if (Collision::CheckRectToRect(atkPos, atkSize, enePos, eneSize))
			{
				enemy->OnDamaged();
				UnitStatus* enemyStatus = dynamic_cast<UnitStatus*>(enemy);
				if (enemyStatus)
				{
					enemyStatus->TakeDamage(mAttack);
					// 敵の死亡処理
					if (enemyStatus->mHp <= 0)
					{
						enemy->Destroy(); // または何らかの死亡処理
					}
				}
			}
		}"""

    if "TakeDamage(mAttack)" not in content:
        content = content.replace(old_col_enemy, new_col_enemy)
        
    with open('Source/Player.cpp', 'w', encoding='cp932') as f:
        f.write(content)

modify()

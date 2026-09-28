$content = [System.IO.File]::ReadAllText("Source\Player.cpp")

# 1
if ($content -notmatch 'mInvincibleTimer = 0;') {
    $content = $content -replace 'isHitDamage = false;\r?\n\tisDead = false;', "isHitDamage = false;`r`n`tmInvincibleTimer = 0;`r`n`tisDead = false;"
}

# 2
if ($content -notmatch 'mInvincibleTimer--;') {
    $content = $content -replace 'isHitDamage = false;\r?\n\r?\n\tObjectManager', "isHitDamage = false;`r`n`tif (mInvincibleTimer > 0) mInvincibleTimer--;`r`n`r`n`tObjectManager"
}

# 3
$old_col_player = '(?s)// 1\. [^\r\n]*\r?\n\t\tif \(Collision::CheckRectToRect\(myPos, mySize, enePos, eneSize\)\)\r?\n\t\t\{\r?\n\t\t\tisHitDamage = true;\r?\n\t\t\}'
$new_col_player = @"
		// 1. プレイヤー自身と敵の衝突判定 (ダメージで赤くする)
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
		}
"@
$content = $content -replace $old_col_player, $new_col_player

# 4
$old_col_enemy = '(?s)// 2\. [^\r\n]*\r?\n\t\tif \(hasAttackRect\)\r?\n\t\t\{\r?\n\t\t\tif \(Collision::CheckRectToRect\(atkPos, atkSize, enePos, eneSize\)\)\r?\n\t\t\t\{\r?\n\t\t\t\tenemy->OnDamaged\(\);\r?\n\t\t\t\}\r?\n\t\t\}'
$new_col_enemy = @"
		// 2. 攻撃判定と敵の衝突判定 (敵を赤く光らせる)
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
		}
"@
$content = $content -replace $old_col_enemy, $new_col_enemy

[System.IO.File]::WriteAllText("Source\Player.cpp", $content, [System.Text.Encoding]::UTF8)

$content = git show HEAD:Source/Player.cpp
$content = $content -join "`n"

if ($content -notmatch 'mInvincibleTimer = 0;') {
    $content = $content -replace 'isHitDamage = false;\n\tisDead = false;', "isHitDamage = false;`n`tmInvincibleTimer = 0;`n`tisDead = false;"
}

if ($content -notmatch 'mInvincibleTimer--;') {
    $content = $content -replace 'isHitDamage = false;\n\n\tObjectManager', "isHitDamage = false;`n`tif (mInvincibleTimer > 0) mInvincibleTimer--;`n`n`tObjectManager"
}

$old_col_player = '(?s)// 1\. [^\n]*\n\t\tif \(Collision::CheckRectToRect\(myPos, mySize, enePos, eneSize\)\)\n\t\t\{\n\t\t\tisHitDamage = true;\n\t\t\}'
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

$old_col_enemy = '(?s)// 2\. [^\n]*\n\t\tif \(hasAttackRect\)\n\t\t\{\n\t\t\tif \(Collision::CheckRectToRect\(atkPos, atkSize, enePos, eneSize\)\)\n\t\t\t\{\n\t\t\t\tenemy->OnDamaged\(\);\n\t\t\t\}\n\t\t\}'
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

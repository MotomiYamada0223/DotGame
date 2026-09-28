$content = [System.IO.File]::ReadAllText("Source\Player.cpp", [System.Text.Encoding]::UTF8)

# mHitEnemies のクリア処理を追加
if ($content -notmatch 'mHitEnemies\.clear\(\);') {
    $content = $content -replace 'isAttacking = true;\r?\n\t\tattackTimer = attackDuration;', "isAttacking = true;`r`n`t`tattackTimer = attackDuration;`r`n`t`tmHitEnemies.clear();"
}

# 当たり判定処理の書き換え
$old_col_enemy = '(?s)// 2\. [^\n]*\n\t\tif \(hasAttackRect\)\n\t\t\{\n\t\t\tif \(Collision::CheckRectToRect\(atkPos, atkSize, enePos, eneSize\)\)\n\t\t\t\{\n\t\t\t\tenemy->OnDamaged\(\);\n\t\t\t\tUnitStatus\* enemyStatus = dynamic_cast<UnitStatus\*>\(enemy\);\n\t\t\t\tif \(enemyStatus\)\n\t\t\t\t\{\n\t\t\t\t\tenemyStatus->TakeDamage\(mAttack\);\n\t\t\t\t\tif \(enemyStatus->mHp <= 0\)\n\t\t\t\t\t\{\n\t\t\t\t\t\tenemy->SetDeleteFlag\(true\);\n\t\t\t\t\t\}\n\t\t\t\t\}\n\t\t\t\}\n\t\t\}'

$new_col_enemy = @"
		// 2. 攻撃判定と敵の衝突判定 (敵を赤く光らせる)
		if (hasAttackRect)
		{
			if (Collision::CheckRectToRect(atkPos, atkSize, enePos, eneSize))
			{
				bool alreadyHit = false;
				for (auto* hitEnemy : mHitEnemies)
				{
					if (hitEnemy == enemy)
					{
						alreadyHit = true;
						break;
					}
				}
				if (!alreadyHit)
				{
					enemy->OnDamaged();
					mHitEnemies.push_back(enemy);
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
		}
"@

$content = $content -replace $old_col_enemy, $new_col_enemy

[System.IO.File]::WriteAllText("Source\Player.cpp", $content, [System.Text.Encoding]::UTF8)

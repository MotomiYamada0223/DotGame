$content = [System.IO.File]::ReadAllText("Source\Player.cpp", [System.Text.Encoding]::GetEncoding(932))
$content = $content -replace 'isHitDamage = false;\r?\n\tif \(mInvincibleTimer > 0\) mInvincibleTimer--;\r?\n\tmInvincibleTimer = 0;\r?\n\tisDead = false;', "isHitDamage = false;`r`n`tmInvincibleTimer = 0;`r`n`tisDead = false;"
$content = $content -replace '// --- 当たり判定 ---\r?\n\tisHitDamage = false;\r?\n\tif \(mInvincibleTimer > 0\) mInvincibleTimer--;\r?\n\tmInvincibleTimer = 0;', "// --- 当たり判定 ---`r`n`tisHitDamage = false;`r`n`tif(mInvincibleTimer > 0) mInvincibleTimer--;"

[System.IO.File]::WriteAllText("Source\Player.cpp", $content, [System.Text.Encoding]::GetEncoding(932))

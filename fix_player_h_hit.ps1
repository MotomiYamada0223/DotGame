$content = [System.IO.File]::ReadAllText("Source\Player.h", [System.Text.Encoding]::GetEncoding(932))
if ($content -notmatch 'std::vector<Object2D\*> mHitEnemies;') {
    $content = $content -replace 'bool isAttacking;\r?\n\tint attackTimer;', "bool isAttacking;`r`n`tint attackTimer;`r`n`tstd::vector<Object2D*> mHitEnemies;"
    [System.IO.File]::WriteAllText("Source\Player.h", $content, [System.Text.Encoding]::GetEncoding(932))
}

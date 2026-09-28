$content = [System.IO.File]::ReadAllText("Source\Player.h", [System.Text.Encoding]::GetEncoding(932))
if ($content -notmatch 'int mInvincibleTimer;') {
    $content = $content -replace 'bool isHitDamage;', "bool isHitDamage;`r`n`tint mInvincibleTimer;"
    [System.IO.File]::WriteAllText("Source\Player.h", $content, [System.Text.Encoding]::GetEncoding(932))
}

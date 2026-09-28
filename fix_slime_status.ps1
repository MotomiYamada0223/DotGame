$content = [System.IO.File]::ReadAllText("Source\EnemySlime.cpp", [System.Text.Encoding]::GetEncoding(932))

$new_update_status = @"
void EnemySlime::UpdateStatusByProgress(GameProgress progress)
{
    switch (progress)
    {
    case GameProgress::Tutorial1:
        mMaxHp = 15; mHp = 15;
        mAttack = 9999; 
        mHasInstantKillAttack = true;
        break;
    default: 
        mMaxHp = 15; mHp = 15;
        mAttack = 5;
        mHasInstantKillAttack = false;
        break;
    }
}
"@

$content = $content -replace '(?s)void EnemySlime::UpdateStatusByProgress\(GameProgress progress\)\r?\n\{.*?\}', $new_update_status

$content = $content -replace 'mfEnemyHeight = 64\.0f;\r?\n\}', "mfEnemyHeight = 64.0f;`r`n`r`n    UpdateStatusByProgress(GameProgress::Tutorial1);`r`n}"

[System.IO.File]::WriteAllText("Source\EnemySlime.cpp", $content, [System.Text.Encoding]::GetEncoding(932))

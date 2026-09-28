$files = "Source\BossEnemyDragon.cpp", "Source\BossEnemySnake.cpp", "Source\BossEnemyTree.cpp", "Source\EnemySkeleton.cpp", "Source\EnemyWolf.cpp"
foreach ($file in $files) {
    $content = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::GetEncoding(932))
    $content = "#include `"GameConstants.h`"`r`n" + $content
    [System.IO.File]::WriteAllText($file, $content, [System.Text.Encoding]::GetEncoding(932))
}

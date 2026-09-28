$files = "Source\BossEnemyDragon.cpp", "Source\BossEnemySnake.cpp", "Source\BossEnemyTree.cpp", "Source\EnemySkeleton.cpp", "Source\EnemyWolf.cpp"
foreach ($file in $files) {
    $content = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::GetEncoding(932))
    $content = $content -replace ': Enemy\(initPos\)', ': Enemy(CharacterGraphPath::Dragon, initPos)'
    [System.IO.File]::WriteAllText($file, $content, [System.Text.Encoding]::GetEncoding(932))
}

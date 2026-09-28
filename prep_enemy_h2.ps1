$content = [System.IO.File]::ReadAllText("Source\Enemy.h", [System.Text.Encoding]::GetEncoding(932))
$content = $content -replace 'Enemy\(VECTOR initPos\);', 'Enemy(const std::string& graphPath, VECTOR initPos);'
$content = $content -replace 'virtual void EnemyMove\(BlockMap& blockMap\);', 'virtual void EnemyMove(BlockMap& blockMap) = 0;'
[System.IO.File]::WriteAllText("Source\Enemy.h", $content, [System.Text.Encoding]::GetEncoding(932))

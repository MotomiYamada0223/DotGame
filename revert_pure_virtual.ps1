$content = [System.IO.File]::ReadAllText("Source\Enemy.h", [System.Text.Encoding]::GetEncoding(932))
$content = $content -replace 'virtual void EnemyMove\(BlockMap& blockMap\) = 0;', 'virtual void EnemyMove(BlockMap& blockMap) {}'
[System.IO.File]::WriteAllText("Source\Enemy.h", $content, [System.Text.Encoding]::GetEncoding(932))

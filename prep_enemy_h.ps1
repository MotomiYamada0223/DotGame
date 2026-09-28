$content = [System.IO.File]::ReadAllText("Source\Enemy.h", [System.Text.Encoding]::GetEncoding(932))
$content = $content -replace 'private:', 'protected:'
$content = $content -replace 'float moveSpeed;\r?\n', ''
$content = $content -replace 'void EnemyMove\(BlockMap& blockMap\);', 'virtual void EnemyMove(BlockMap& blockMap);'
[System.IO.File]::WriteAllText("Source\Enemy.h", $content, [System.Text.Encoding]::GetEncoding(932))

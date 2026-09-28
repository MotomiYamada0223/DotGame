$content = [System.IO.File]::ReadAllText("Source\Enemy.cpp", [System.Text.Encoding]::GetEncoding(932))
$content = $content -replace 'Enemy::Enemy\(VECTOR initPos\)\r?\n\t: Object2D\(CharacterGraphPath::Dragon, initPos\)', "Enemy::Enemy(const std::string& graphPath, VECTOR initPos)`r`n`t: Object2D(graphPath, initPos)"

# moveSpeed の初期化を削除
$content = $content -replace '(?s)// .*?moveSpeed = 2\.0f;\r?\n', ""

# EnemyMoveの実装を削除 (純粋仮想関数にしたため)
$content = $content -replace '(?s)// EnemyMove\r?\nvoid Enemy::EnemyMove\(BlockMap& blockMap\)\r?\n\{.*?\}\r?\n\r?\nvoid Enemy::OnDamaged', "void Enemy::OnDamaged"

[System.IO.File]::WriteAllText("Source\Enemy.cpp", $content, [System.Text.Encoding]::GetEncoding(932))

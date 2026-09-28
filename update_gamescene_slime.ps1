$content = [System.IO.File]::ReadAllText("Source\GameScene.cpp", [System.Text.Encoding]::GetEncoding(932))
$content = $content -replace 'new Enemy\(VGet\(1000\.0f, mpPlayer->GetPosition\(\)\.y - 500, 0\.0f\)\);', 'new EnemySlime(VGet(1000.0f, mpPlayer->GetPosition().y - 500, 0.0f));'
[System.IO.File]::WriteAllText("Source\GameScene.cpp", $content, [System.Text.Encoding]::GetEncoding(932))

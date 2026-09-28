$content = [System.IO.File]::ReadAllText("Source\Player.cpp", [System.Text.Encoding]::GetEncoding(932))
$content = $content -replace 'enemy->Destroy\(\);', 'enemy->SetDeleteFlag(true);'
[System.IO.File]::WriteAllText("Source\Player.cpp", $content, [System.Text.Encoding]::GetEncoding(932))

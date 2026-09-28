$files = Get-ChildItem -Path Source\ -Filter "*Enemy*.cpp" -Exclude "Enemy.cpp", "EnemySlime.cpp"
foreach ($file in $files) {
    $content = [System.IO.File]::ReadAllText($file.FullName, [System.Text.Encoding]::GetEncoding(932))
    $content = $content -replace ': Enemy\(initPos\)', ': Enemy(CharacterGraphPath::Dragon, initPos)'
    [System.IO.File]::WriteAllText($file.FullName, $content, [System.Text.Encoding]::GetEncoding(932))
}

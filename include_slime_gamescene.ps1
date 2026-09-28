$content = [System.IO.File]::ReadAllText("Source\GameScene.cpp", [System.Text.Encoding]::GetEncoding(932))
if ($content -notmatch '#include "EnemySlime.h"') {
    $content = $content -replace '#include "Enemy.h"', "#include `"Enemy.h`"`r`n#include `"EnemySlime.h`""
    [System.IO.File]::WriteAllText("Source\GameScene.cpp", $content, [System.Text.Encoding]::GetEncoding(932))
}

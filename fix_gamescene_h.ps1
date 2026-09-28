$content = [System.IO.File]::ReadAllText("Source\GameScene.h", [System.Text.Encoding]::GetEncoding(932))
if ($content -notmatch 'GameProgress mProgress;') {
    $content = $content -replace 'Player\* mpPlayer;', "Player* mpPlayer;`r`n`tGameProgress mProgress;"
    $content = $content -replace '#include "EnemySlime.h"', "#include `"EnemySlime.h`"`r`n#include `"UnitStatus.h`""
    [System.IO.File]::WriteAllText("Source\GameScene.h", $content, [System.Text.Encoding]::GetEncoding(932))
}

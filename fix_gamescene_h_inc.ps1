$content = [System.IO.File]::ReadAllText("Source\GameScene.h", [System.Text.Encoding]::GetEncoding(932))
if ($content -notmatch '#include "UnitStatus.h"') {
    $content = $content -replace '#pragma once', "#pragma once`r`n#include `"UnitStatus.h`""
    [System.IO.File]::WriteAllText("Source\GameScene.h", $content, [System.Text.Encoding]::GetEncoding(932))
}

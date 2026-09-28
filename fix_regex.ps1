$content = [System.IO.File]::ReadAllText("Source\Player.cpp", [System.Text.Encoding]::GetEncoding(932))
$content = [System.Text.RegularExpressions.Regex]::Replace($content, 'if\s*\(mInvincibleTimer\s*>\s*0\)\s*mInvincibleTimer--;\s*mInvincibleTimer\s*=\s*0;', 'if (mInvincibleTimer > 0) mInvincibleTimer--;')
[System.IO.File]::WriteAllText("Source\Player.cpp", $content, [System.Text.Encoding]::GetEncoding(932))

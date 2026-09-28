$content = [System.IO.File]::ReadAllText("Source\GameScene.cpp", [System.Text.Encoding]::GetEncoding(932))
if ($content -notmatch 'mProgress = GameProgress::Tutorial1;') {
    $replacement = @"
	mProgress = GameProgress::Tutorial1;
	
	mpPlayer = new Player(VGet(150.0f, 400.0f, 0.0f));
	mpPlayer->UpdateStatusByProgress(mProgress);
"@
    $content = $content -replace 'mpPlayer = new Player\(VGet\(150\.0f, 400\.0f, 0\.0f\)\);', $replacement

    $replacement2 = @"
		EnemySlime* slime = new EnemySlime(VGet(1000.0f, mpPlayer->GetPosition().y - 500, 0.0f));
		slime->UpdateStatusByProgress(mProgress);
"@
    $content = $content -replace 'new EnemySlime\(VGet\(1000\.0f, mpPlayer->GetPosition\(\)\.y - 500, 0\.0f\)\);', $replacement2

    [System.IO.File]::WriteAllText("Source\GameScene.cpp", $content, [System.Text.Encoding]::GetEncoding(932))
}

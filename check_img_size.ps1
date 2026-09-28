Add-Type -AssemblyName System.Drawing
$img = [System.Drawing.Image]::FromFile("C:\Users\student\Desktop\DotGame\Resource\Image\SampleSlime.png")
Write-Output ("Width: " + $img.Width + ", Height: " + $img.Height)
$img.Dispose()

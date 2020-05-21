git ls-files -s | Select-String -Pattern "120000 " | ForEach-Object {
    $filename = ($_.Line -split '\s+')[3];
    $target = Get-Content $filename;
    $z = [System.IO.Path]::Combine([System.IO.Path]::GetDirectoryName($filename), $target);
    if ([System.IO.File]::Exists($z)) {
        Copy-Item -Path $z -Destination $filename
    }
}

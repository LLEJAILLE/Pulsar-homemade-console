# clean.ps1

if (Test-Path "build") {
    Remove-Item "build" -Recurse -Force
    Write-Host "Build deleted."
}
param([string]$BuildDir="build-release")
$ErrorActionPreference='Stop'
$ProjectRoot=(Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$Out=Join-Path $ProjectRoot $BuildDir
cmake -S $ProjectRoot -B $Out -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build $Out
cmake --install $Out --prefix (Join-Path $Out "install")
Push-Location $Out
cpack -G ZIP
try { cpack -G NSIS } catch { Write-Warning "NSIS not installed; ZIP package was still generated." }
Pop-Location

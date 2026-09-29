$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Write-Host "PS2 Studio 0.17 validation" -ForegroundColor Cyan

& "$PSScriptRoot\Verify-Dependencies.ps1"

$required = @(
  "src\ClangdClient.cpp",
  "src\ProblemsWidget.cpp",
  "src\DebuggerWidget.cpp",
  "src\RuntimeTemplates.cpp",
  "docs\FULL_STACK_0.17.md"
)
foreach($rel in $required) {
  $p = Join-Path $root $rel
  if(-not (Test-Path $p)) { throw "Missing required 0.17 file: $rel" }
  Write-Host "[OK] $rel"
}

if(Get-Command cmake -ErrorAction SilentlyContinue) {
  Write-Host ""
  Write-Host "Run the host compile with:" -ForegroundColor Yellow
  Write-Host "  .\scripts\Build-PS2Studio.ps1"
}

[CmdletBinding()]
param(
    [ValidateSet('All','Host','PS2','Optional')]
    [string]$Mode = 'All',
    [string]$QtVersion = '6.11.1',
    [string]$QtRoot = 'C:\Qt',
    [string]$PS2DevRoot = 'C:\ps2dev',
    [switch]$SkipVisualStudio,
    [switch]$SkipQt,
    [switch]$SkipPCSX2,
    [switch]$NonInteractive
)

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'

function Write-Step([string]$Text) { Write-Host "`n==> $Text" -ForegroundColor Cyan }
function Write-Ok([string]$Text) { Write-Host "[OK] $Text" -ForegroundColor Green }
function Write-Warn([string]$Text) { Write-Host "[WARN] $Text" -ForegroundColor Yellow }
function Test-Command([string]$Name) { return [bool](Get-Command $Name -ErrorAction SilentlyContinue) }

function Install-WingetPackage([string]$Id, [string]$ExtraArgs = '') {
    if (-not (Test-Command 'winget')) { throw 'winget is required. Install Microsoft App Installer, then rerun this script.' }
    Write-Step "Installing/updating $Id"
    $args = @('install','--id',$Id,'--exact','--accept-package-agreements','--accept-source-agreements','--silent')
    if ($ExtraArgs) { $args += @('--override', $ExtraArgs) }
    & winget @args
    if ($LASTEXITCODE -ne 0) {
        # winget may return a non-zero code when already installed/current. Verify by list before failing.
        & winget list --id $Id --exact | Out-Null
        if ($LASTEXITCODE -ne 0) { throw "winget failed for $Id (exit $LASTEXITCODE)" }
    }
    Write-Ok $Id
}

function Add-UserPath([string]$PathValue) {
    if (-not $PathValue) { return }
    $full = [Environment]::ExpandEnvironmentVariables($PathValue).TrimEnd('\\')
    $current = [Environment]::GetEnvironmentVariable('Path','User')
    $parts = @($current -split ';' | Where-Object { $_ })
    if ($parts -notcontains $full) {
        $newPath = (($parts + $full) -join ';')
        [Environment]::SetEnvironmentVariable('Path',$newPath,'User')
    }
    if (($env:Path -split ';') -notcontains $full) { $env:Path += ";$full" }
}

function Set-UserEnvironment([string]$Name,[string]$Value) {
    [Environment]::SetEnvironmentVariable($Name,$Value,'User')
    Set-Item -Path "Env:$Name" -Value $Value
}

function Refresh-Path {
    $machine = [Environment]::GetEnvironmentVariable('Path','Machine')
    $user = [Environment]::GetEnvironmentVariable('Path','User')
    $env:Path = "$machine;$user"
}

function Install-HostTools {
    Write-Step 'Installing Windows host build tools'
    Install-WingetPackage 'Git.Git'
    Install-WingetPackage 'Kitware.CMake'
    Install-WingetPackage 'Ninja-build.Ninja'
    Install-WingetPackage 'Python.Python.3.13'
    Install-WingetPackage 'MSYS2.MSYS2'
    $bash='C:\msys64\usr\bin\bash.exe'
    if (Test-Path $bash) {
        Write-Step 'Installing GNU Make in MSYS2'
        & $bash -lc 'pacman -S --needed --noconfirm make'
        if ($LASTEXITCODE -ne 0) { throw 'Failed to install GNU Make through MSYS2.' }
        Add-UserPath 'C:\msys64\usr\bin'
        Write-Ok 'GNU Make / MSYS2'
    } else { Write-Warn 'MSYS2 was installed but bash.exe was not found at C:\msys64\usr\bin\bash.exe.' }

    if (-not $SkipVisualStudio) {
        # C++ Build Tools workload. If VS is already present, winget keeps the existing installation.
        Install-WingetPackage 'Microsoft.VisualStudio.2022.BuildTools' '--wait --passive --norestart --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended'
    }
    Refresh-Path

    if (-not $SkipQt) {
        Write-Step "Installing Qt $QtVersion (MSVC 2022 x64) using aqtinstall"
        $python = (Get-Command python -ErrorAction SilentlyContinue)
        if (-not $python) { $python = (Get-Command py -ErrorAction SilentlyContinue) }
        if (-not $python) { throw 'Python was installed but is not visible yet. Open a new terminal and rerun the bootstrap.' }
        $pyExe = $python.Source
        if ($python.Name -eq 'py.exe') {
            & $pyExe -3 -m pip install --user --upgrade aqtinstall
            if ($LASTEXITCODE -ne 0) { throw 'Failed to install aqtinstall.' }
            & $pyExe -3 -m aqt install-qt windows desktop $QtVersion win64_msvc2022_64 -O $QtRoot
        } else {
            & $pyExe -m pip install --user --upgrade aqtinstall
            if ($LASTEXITCODE -ne 0) { throw 'Failed to install aqtinstall.' }
            & $pyExe -m aqt install-qt windows desktop $QtVersion win64_msvc2022_64 -O $QtRoot
        }
        if ($LASTEXITCODE -ne 0) { throw "Qt installation failed. If $QtVersion is no longer available, rerun with -QtVersion <available version>." }
        $qtPrefix = Join-Path $QtRoot "$QtVersion\msvc2022_64"
        if (-not (Test-Path $qtPrefix)) { throw "Qt install completed but $qtPrefix was not found." }
        Set-UserEnvironment 'QTDIR' $qtPrefix
        Set-UserEnvironment 'Qt6_DIR' (Join-Path $qtPrefix 'lib\cmake\Qt6')
        Set-UserEnvironment 'CMAKE_PREFIX_PATH' $qtPrefix
        Add-UserPath (Join-Path $qtPrefix 'bin')
        Write-Ok "Qt installed at $qtPrefix"
    }
}

function Install-PS2Dev {
    Write-Step 'Installing the official PS2DEV prebuilt Windows environment'
    New-Item -ItemType Directory -Force -Path $PS2DevRoot | Out-Null
    $temp = Join-Path $env:TEMP 'ps2dev-windows-latest.tar.gz'
    $url = 'https://github.com/ps2dev/ps2dev/releases/latest/download/ps2dev-windows-latest.tar.gz'
    Invoke-WebRequest -Uri $url -OutFile $temp -UseBasicParsing
    if (-not (Test-Path $temp)) { throw 'PS2DEV download failed.' }
    if (-not (Test-Command 'tar')) { throw 'Windows tar.exe is required to extract PS2DEV.' }
    & tar -xzf $temp --strip-components=1 -C $PS2DevRoot
    if ($LASTEXITCODE -ne 0) { throw 'Failed to extract the PS2DEV archive.' }
    Remove-Item $temp -Force -ErrorAction SilentlyContinue

    $sdk = Join-Path $PS2DevRoot 'ps2sdk'
    $gskit = Join-Path $PS2DevRoot 'gsKit'
    Set-UserEnvironment 'PS2DEV' $PS2DevRoot
    Set-UserEnvironment 'PS2SDK' $sdk
    Set-UserEnvironment 'GSKIT' $gskit
    @(
        (Join-Path $PS2DevRoot 'bin'),
        (Join-Path $PS2DevRoot 'ee\bin'),
        (Join-Path $PS2DevRoot 'iop\bin'),
        (Join-Path $PS2DevRoot 'dvp\bin'),
        (Join-Path $sdk 'bin')
    ) | ForEach-Object { Add-UserPath $_ }

    $ee = Join-Path $PS2DevRoot 'ee\bin\mips64r5900el-ps2-elf-g++.exe'
    if (-not (Test-Path $ee)) { Write-Warn 'EE C++ compiler was not found at the expected path; run Verify-Dependencies.ps1 for details.' }
    else { Write-Ok "PS2DEV installed at $PS2DevRoot" }
}


function Install-Tyra {
    $root='C:\tyra'
    Write-Step 'Installing/updating Tyra Engine'
    if(Test-Path "$root\.git"){ git -C $root pull --ff-only }
    else { if(Test-Path $root){Remove-Item -Recurse -Force $root}; git clone --recursive https://github.com/h4570/tyra.git $root }
    if($LASTEXITCODE -ne 0){throw 'Tyra install/update failed.'}
    Set-UserEnvironment 'TYRA' $root
    Write-Ok "Tyra installed at $root"
}

function Install-OptionalTools {
    if (-not $SkipPCSX2) {
        Install-WingetPackage 'PCSX2Team.PCSX2'
    }
    # clangd powers F12/navigation and project code intelligence.
    Install-WingetPackage 'LLVM.LLVM'
    # Git LFS is useful for large game assets but is not required to compile.
    Install-WingetPackage 'GitHub.GitLFS'
    Refresh-Path
}

try {
    switch ($Mode) {
        'All' { Install-HostTools; Install-PS2Dev; Install-OptionalTools }
        'Host' { Install-HostTools }
        'PS2' { Install-PS2Dev }
    Install-Tyra
        'Optional' { Install-OptionalTools }
    }
    Write-Step 'Bootstrap complete'
    Write-Host 'Dependencies were persisted to your user environment.' -ForegroundColor Green
    Write-Host 'You can build immediately; Build-PS2Studio.ps1 reloads persisted values automatically.' -ForegroundColor Yellow
    Write-Host 'Run: .\scripts\Build-PS2Studio.ps1' -ForegroundColor White
    exit 0
}
catch {
    Write-Host "`n[FAILED] $($_.Exception.Message)" -ForegroundColor Red
    if (-not $NonInteractive) { Read-Host 'Press Enter to close' | Out-Null }
    exit 1
}

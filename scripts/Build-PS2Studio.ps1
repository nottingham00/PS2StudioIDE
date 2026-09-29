[CmdletBinding()]
param(
  [ValidateSet('Debug','RelWithDebInfo','Release')] [string]$Configuration='Release',
  [string]$BuildDir='build',
  [switch]$Package
)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
Set-Location $root

function Refresh-PersistedEnvironment {
    # BOOTSTRAP.cmd runs in a child PowerShell process. Environment variables it
    # persists with SetEnvironmentVariable(User) are not injected into an already
    # open parent terminal. Import the persisted values here so users can build
    # immediately without reopening PowerShell.
    foreach($name in @('QTDIR','Qt6_DIR','CMAKE_PREFIX_PATH','PS2DEV','PS2SDK','GSKIT')) {
        if(-not (Get-Item -Path "Env:$name" -ErrorAction SilentlyContinue)) {
            $value=[Environment]::GetEnvironmentVariable($name,'User')
            if(-not $value){$value=[Environment]::GetEnvironmentVariable($name,'Machine')}
            if($value){Set-Item -Path "Env:$name" -Value $value}
        }
    }
    $machine=[Environment]::GetEnvironmentVariable('Path','Machine')
    $user=[Environment]::GetEnvironmentVariable('Path','User')
    if($machine -or $user){$env:Path="$machine;$user"}
}

function Resolve-QtPrefix {
    $candidates=New-Object System.Collections.Generic.List[string]
    foreach($value in @($env:CMAKE_PREFIX_PATH,$env:QTDIR)) {
        if($value){
            foreach($part in ($value -split ';')) { if($part){$candidates.Add($part.Trim())} }
        }
    }
    $userPrefix=[Environment]::GetEnvironmentVariable('CMAKE_PREFIX_PATH','User')
    $userQt=[Environment]::GetEnvironmentVariable('QTDIR','User')
    foreach($value in @($userPrefix,$userQt)) {
        if($value){ foreach($part in ($value -split ';')) { if($part){$candidates.Add($part.Trim())} } }
    }

    # Known bootstrap layout, then a generic scan for installed MSVC Qt versions.
    $candidates.Add('C:\Qt\6.11.1\msvc2022_64')
    if(Test-Path 'C:\Qt') {
        Get-ChildItem 'C:\Qt' -Directory -ErrorAction SilentlyContinue |
            Sort-Object Name -Descending | ForEach-Object {
                $msvc=Join-Path $_.FullName 'msvc2022_64'
                if(Test-Path $msvc){$candidates.Add($msvc)}
            }
    }
    foreach($candidate in $candidates | Select-Object -Unique) {
        if(-not $candidate){continue}
        $qtConfig=Join-Path $candidate 'lib\cmake\Qt6\Qt6Config.cmake'
        if(Test-Path $qtConfig){return (Resolve-Path $candidate).Path}
    }
    return $null
}

Refresh-PersistedEnvironment

function Import-MsvcEnvironment {
    if(Get-Command cl -ErrorAction SilentlyContinue){return}
    $vswhere="${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if(-not (Test-Path $vswhere)){return}
    $install=& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if(-not $install){return}
    $dev=Join-Path $install 'Common7\Tools\VsDevCmd.bat'
    if(-not (Test-Path $dev)){return}
    $lines=& cmd.exe /s /c "`"$dev`" -no_logo -arch=x64 && set"
    foreach($line in $lines){
        if($line -match '^([^=]+)=(.*)$'){Set-Item -Path "Env:$($matches[1])" -Value $matches[2]}
    }
}
Import-MsvcEnvironment
if(-not (Get-Command cl -ErrorAction SilentlyContinue)){throw 'MSVC C++ compiler was not found. Run BOOTSTRAP.cmd first.'}
if(-not (Get-Command cmake -ErrorAction SilentlyContinue)){throw 'CMake was not found. Run BOOTSTRAP.cmd first.'}
if(-not (Get-Command ninja -ErrorAction SilentlyContinue)){throw 'Ninja was not found. Run BOOTSTRAP.cmd first.'}

$prefix=Resolve-QtPrefix
if(-not $prefix){throw 'Qt 6 MSVC installation was not found. Expected QTDIR/CMAKE_PREFIX_PATH or C:\Qt\<version>\msvc2022_64. Run BOOTSTRAP.cmd first.'}
$env:QTDIR=$prefix
$env:Qt6_DIR=Join-Path $prefix 'lib\cmake\Qt6'
$env:CMAKE_PREFIX_PATH=$prefix
Write-Host "[OK] Using Qt: $prefix" -ForegroundColor Green
$args=@('-S','.','-B',$BuildDir,'-G','Ninja',"-DCMAKE_BUILD_TYPE=$Configuration", "-DCMAKE_PREFIX_PATH=$prefix")
& cmake @args
if($LASTEXITCODE -ne 0){throw 'CMake configure failed.'}
& cmake --build $BuildDir --config $Configuration
if($LASTEXITCODE -ne 0){throw 'Build failed.'}
# Make the build directory directly runnable on Windows by deploying the Qt runtime.
$exe=Join-Path (Join-Path $root $BuildDir) 'PS2Studio.exe'
$windeploy=Join-Path $prefix 'bin\windeployqt.exe'
if((Test-Path $exe) -and (Test-Path $windeploy)){
    Write-Host '[INFO] Deploying Qt runtime beside PS2Studio.exe...'
    $wdMode=if($Configuration -eq 'Debug'){'--debug'}else{'--release'}
    & $windeploy $wdMode --no-translations $exe
    if($LASTEXITCODE -ne 0){throw 'windeployqt failed.'}
}
if($Package){Push-Location $BuildDir; & cpack -C $Configuration; if($LASTEXITCODE -ne 0){throw 'CPack failed.'}; Pop-Location}
Write-Host "[OK] PS2 Studio build complete: $root\$BuildDir" -ForegroundColor Green

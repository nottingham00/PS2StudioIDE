[CmdletBinding()]
param([switch]$Json)
$ErrorActionPreference='SilentlyContinue'
function Cmd($n){ [bool](Get-Command $n -ErrorAction SilentlyContinue) }
function Exists($p){ $p -and (Test-Path $p) }
function Persisted($n){
    $v=(Get-Item -Path "Env:$n" -ErrorAction SilentlyContinue).Value
    if(-not $v){$v=[Environment]::GetEnvironmentVariable($n,'User')}
    if(-not $v){$v=[Environment]::GetEnvironmentVariable($n,'Machine')}
    return $v
}
function FindQt {
    foreach($p in @((Persisted 'QTDIR'),(Persisted 'CMAKE_PREFIX_PATH'),'C:\Qt\6.11.1\msvc2022_64')) {
        if($p -and (Test-Path (Join-Path $p 'lib\cmake\Qt6\Qt6Config.cmake'))){return $p}
    }
    if(Test-Path 'C:\Qt'){
        foreach($d in (Get-ChildItem 'C:\Qt' -Directory -ErrorAction SilentlyContinue | Sort-Object Name -Descending)){
            $p=Join-Path $d.FullName 'msvc2022_64'
            if(Test-Path (Join-Path $p 'lib\cmake\Qt6\Qt6Config.cmake')){return $p}
        }
    }
    return $null
}
$ps2dev=Persisted 'PS2DEV'; $ps2sdk=Persisted 'PS2SDK'; $gskit=Persisted 'GSKIT'; $qt=FindQt
$checks=[ordered]@{
    winget = Cmd 'winget'
    git = Cmd 'git'
    cmake = Cmd 'cmake'
    ninja = (Cmd 'ninja') -or (Cmd 'ninja-build')
    python = (Cmd 'python') -or (Cmd 'py')
    make = (Cmd 'make') -or (Exists 'C:\msys64\usr\bin\make.exe')
    msvc = (Cmd 'cl') -or (Test-Path "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe")
    qt6 = Exists $qt
    PS2DEV = Exists $ps2dev
    PS2SDK = Exists $ps2sdk
    GSKIT = Exists $gskit
    TYRA = (Exists (Persisted 'TYRA')) -or (Exists 'C:\tyra\Makefile.base')
    ee_gxx = Exists (Join-Path $ps2dev 'ee\bin\mips64r5900el-ps2-elf-g++.exe')
    iop_gcc = Exists (Join-Path $ps2dev 'iop\bin\mipsel-none-elf-gcc.exe')
    dvp_as = Exists (Join-Path $ps2dev 'dvp\bin\dvp-as.exe')
    ps2client = (Cmd 'ps2client') -or (Exists (Join-Path $ps2dev 'bin\ps2client.exe'))
    clangd = (Cmd 'clangd') -or (Exists 'C:\Program Files\LLVM\bin\clangd.exe')
    pcsx2 = (Cmd 'pcsx2-qt') -or (Cmd 'pcsx2') -or (Test-Path "$env:ProgramFiles\PCSX2\pcsx2-qt.exe") -or (Test-Path "$env:LOCALAPPDATA\Microsoft\WinGet\Links\pcsx2-qt.exe")
}
if($Json){$checks|ConvertTo-Json;exit}
$checks.GetEnumerator()|ForEach-Object{ $m=if($_.Value){'[OK]'}else{'[--]'}; '{0,-5} {1}' -f $m,$_.Key }
if(($checks.Values|Where-Object{$_ -eq $false}).Count -gt 0){exit 2}else{exit 0}


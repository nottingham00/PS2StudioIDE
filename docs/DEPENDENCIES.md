# PS2 Studio dependencies

## Windows — automatic setup

On a fresh Windows 10/11 development machine, open the source folder and run:

```powershell
.\BOOTSTRAP.cmd
```

or from PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\Bootstrap-Windows.ps1 -Mode All
```

The bootstrap installs or configures:

### Host build tools
- Git for Windows
- CMake
- Ninja
- Python
- Visual Studio 2022 Build Tools with the C++ workload
- Qt 6 (MSVC x64) through `aqtinstall`

### PS2 development environment
- Official PS2DEV prebuilt Windows environment
- PS2SDK
- gsKit
- EE compiler
- IOP compiler
- DVP/VU assembler
- ps2client and the other tools shipped by PS2DEV

### Optional testing tools
- PCSX2
- Git LFS

The script installs PS2DEV under `C:\ps2dev` by default and sets the user environment variables:

- `PS2DEV=C:\ps2dev`
- `PS2SDK=C:\ps2dev\ps2sdk`
- `GSKIT=C:\ps2dev\gsKit`

It also appends the PS2DEV compiler/tool directories to the user PATH.

PS2DEV officially publishes a prebuilt Windows archive. The bootstrap downloads `ps2dev-windows-latest.tar.gz` from the project's latest GitHub release instead of compiling the complete toolchain locally.

## Partial installation

```powershell
.\scripts\Bootstrap-Windows.ps1 -Mode Host
.\scripts\Bootstrap-Windows.ps1 -Mode PS2
.\scripts\Bootstrap-Windows.ps1 -Mode Optional
```

Change Qt version if necessary:

```powershell
.\scripts\Bootstrap-Windows.ps1 -Mode Host -QtVersion 6.11.1
```

Change installation locations:

```powershell
.\scripts\Bootstrap-Windows.ps1 -QtRoot D:\Qt -PS2DevRoot D:\ps2dev
```

PS2DEV paths must not contain spaces or special characters.

## Verify

```powershell
.\scripts\Verify-Dependencies.ps1
```

After installation, open a new terminal so Windows loads the updated user PATH/environment variables.

## Build PS2 Studio

```powershell
.\scripts\Build-PS2Studio.ps1 -Configuration Release
```

To create the CPack package as well:

```powershell
.\scripts\Build-PS2Studio.ps1 -Configuration Release -Package
```

## In the IDE

Once PS2 Studio itself is built, open **Tools → Dependency Manager**. It can rerun the same installer for repairs or optional tools and display the PowerShell output in the IDE.

## Notes

Qt and the MSVC build tools cannot be installed *by an IDE binary that has not been compiled yet*. That is why `BOOTSTRAP.cmd` is included outside the application. Once the application runs, the in-IDE manager can maintain the environment.

Disc-authoring software is intentionally not forced as a mandatory dependency. PS2 Studio detects supported ISO/burn backends through Environment Doctor and Disc / Deploy Manager because optical-disc tooling varies between Windows installations.

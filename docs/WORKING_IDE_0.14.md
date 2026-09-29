# PS2 Studio 0.14 — Working IDE Core

This release focuses on making the development workflow usable rather than adding placeholder controls.

## Working IDE workflow

- Tabbed source/header editor using QPlainTextEdit
- C/C++/PS2 syntax highlighting
- Line numbers and current-line highlighting
- Basic C++/PS2 API completion (Ctrl+Space)
- New C++ source/header creation
- Save, Save As and Save All with dirty-file prompts
- Find, Replace and Go to Line
- Editor breakpoints and an integrated GDB console
- GCC/MSVC build error parsing and source-line navigation
- Automatic PS2DEV/MSYS2/PCSX2 tool search
- GNU Make installation in BOOTSTRAP.cmd
- EE / gsKit builds
- IOP IRX target generated from the current PS2SDK IOP skeleton layout
- VU/DVP `.vsm` assembly target
- PCSX2 launch and ps2client deployment
- Environment Doctor / Validation Center use the same executable names and search paths as the real build path
- windeployqt runs after the Windows IDE build so the build directory is directly runnable

## Runtime improvements

- Text components now have a small built-in 3x5 bitmap renderer for A-Z, 0-9 and common punctuation.
- Textures, OBJ+UV meshes and 16-bit PCM WAV remain the supported compiled asset formats.
- Unsupported formats are rejected at import instead of being accepted without a runtime compiler.

## Still experimental / limited

These do not block using PS2 Studio as an IDE, but they are not production-complete engine subsystems yet:

- VU1 accelerated mesh rendering: DVP assembly works, but the scene renderer still uses the verified EE fallback until a real VU1 upload/dispatch path is hardware-validated.
- GDB UI: console, breakpoints, continue/interrupt/step/next/backtrace/register commands are wired; PS2-side debugging still requires the ps2gdb stub/ps2link setup expected by ps2gdb.
- Physics: editor component data and AABB helpers exist; this is not a full rigid-body solver.
- Animation: asset fields exist; skeletal skinning is not yet implemented.
- Audio: audsrv PCM playback exists; this is not a full multi-voice SPU2 mixer.
- Hot reload is rebuild/relaunch, not live code patching.

## First validation

On Windows run:

```powershell
.\scripts\Verify-Dependencies.ps1
.\scripts\Build-PS2Studio.ps1
```

The build script deploys Qt DLLs beside `build\PS2Studio.exe` using `windeployqt`.

Then launch `build\PS2Studio.exe`, open a project, open a `.cpp` file, edit/save it, select `EE / gsKit`, and build.

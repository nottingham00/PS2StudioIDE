# PS2 Studio 0.18.1 – Tyra Backend Hotfix

PS2 Studio is a Qt 6 C++ IDE/editor for PS2DEV, PS2SDK and gsKit projects. 0.16 carries the 0.15 engine/runtime systems forward and adds production-oriented code navigation, debugging, conversion and run workflows.

## IDE workflow

- Tabbed C/C++/VU code editor with save, line numbers, highlighting, basic completion and breakpoints.
- **Ctrl+Shift+F** project-wide search.
- **F12** Go to Definition using a real `clangd` JSON-RPC process when LLVM is installed.
- Build-error navigation back to source.
- EE/gsKit, IOP IRX and DVP/VU build routes.
- Visual GDB dock: console, locals, call stack, registers, memory command output and watches.
- PCSX2 run/stop/restart and captured output.
- Optional live rebuild + relaunch on watched asset changes.
- Modern model conversion hook using Assimp CLI or Blender, emitting OBJ for the PS2 asset compiler.
- Environment Doctor, dependency bootstrap, support bundles, profiler, real-PS2 deploy and disc/ISO workflow.

## Engine/runtime carried from 0.15

- Generated C++ gameplay components and callbacks.
- Mutable runtime entities and scene switching.
- AABB 3D physics with gravity, layers/masks, triggers, friction/restitution and collision callbacks.
- Materials/basic lighting, particles and controller-driven UI widgets.
- PCM WAV plus ADPCM multi-voice SFX when `adpenc` is available.
- `.ps2skin` CPU skeletal skinning and `.ps2anim` animation clips.
- Runtime profiler counters routed back into the IDE.

## Build PS2 Studio

```powershell
.\scripts\Verify-Dependencies.ps1
.\scripts\Build-PS2Studio.ps1
```

If clangd is missing, rerun the optional dependency bootstrap; 0.16 adds LLVM to that set.

## Important validation status

`EE / gsKit` is the primary runtime path. The VU/DVP assembler target is real, but VU1 mesh dispatch remains explicitly experimental: the generated renderer still falls back to EE until VIF/DMA upload, microprogram dispatch and synchronization are verified on PCSX2 and physical PS2 hardware. Live reload is rebuild/relaunch rather than executable-memory patching.

See `docs/PRODUCTION_TOOLS_0.16.md`, `docs/ENGINE_SYSTEMS_0.15.md`, and `docs/FEATURE_STATUS.md`.


## 0.17 Full Stack additions

- clangd diagnostics -> Problems dock + editor wave underlines
- smart clangd completion
- hover/type information
- Find References
- Rename Symbol workspace edits
- existing Go to Definition and project search retained
- extended runtime profiler counters for physics/skinning/VIF/VU1
- runtime interfaces for animation state machines, audio buses, UI anchors/layout metadata, collider shape selection
- explicit VU1 backend verification/fallback layer

See `docs/FULL_STACK_0.17.md`.

### Validation note

PS2 Studio 0.17 intentionally distinguishes **implemented IDE/runtime code** from **verified PS2 hardware behavior**. Real VIF/DMA VU1 dispatch, deep physics stability, streaming-from-disc audio, true in-process code hot patching, and burn/console behavior must still be proven through PCSX2 and hardware tests.


## Tyra Engine backend
New Project now supports **Tyra Engine** alongside the native PS2SDK + gsKit backend. `BOOTSTRAP.cmd` clones upstream Tyra to `C:\tyra`, sets `TYRA`, generates Tyra's standard `src/inc/obj/bin/res` layout, links `-ltyra`, includes `engine/inc`, and uses `Makefile.base`. Built Tyra ELFs under `bin/` are recognized by PCSX2/deploy actions.


## 0.18.1 compile hotfix
Fixes Qt 6.11/MSVC compilation in `AssetCompiler.cpp`: instance-based `QFileInfo::isExecutable()` and bounds-safe `QJsonArray` indexing for `.ps2skin` data.

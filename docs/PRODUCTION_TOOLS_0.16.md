# PS2 Studio 0.16 Production Tools

## Added
- clangd process integration with F12 Go to Definition.
- Project-wide text/symbol search (Ctrl+Shift+F).
- Visual GDB panes for locals, call stack, registers, memory console, watches and raw commands.
- Live rebuild + PCSX2 relaunch mode for changed assets/scenes.
- PCSX2 restart command and captured stdout/stderr feeding the profiler.
- Modern model conversion hook: Assimp CLI first, Blender fallback, emitting OBJ for the existing PS2 compiler.
- LLVM/clangd detection in the environment layer.

## Engine status carried from 0.15
Working code paths include generated gameplay components, AABB 3D physics with layers/masks/gravity/restitution, PS2 UI controls, ADPCM voices, CPU skeletal skinning, animation clips, materials/basic lighting and runtime profiler counters.

## Experimental / requires target verification
- VU1 mesh dispatch: DVP assembly is real, but the shipping renderer still falls back to EE until VIF/DMA microprogram upload/dispatch is verified on PCSX2 and physical PS2 hardware.
- Live reload is rebuild/relaunch, not executable memory patching.
- FBX/glTF/DAE import is an external conversion workflow; the internal PS2 asset representation remains OBJ or .ps2skin/.ps2anim.
- ps2gdb capabilities depend on the PS2-side debug stub and network target.

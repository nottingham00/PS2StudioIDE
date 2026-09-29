# PS2 Studio 0.8 pre-build checklist

Implemented before the first serious local build:

- Persistent window/dock layout and profile/target/video selections via QSettings.
- Per-project `.ps2studio/project.json` with PS2 target host, PCSX2 path, autosave interval and selected build settings.
- Autosave snapshots and newer-autosave recovery.
- Recent project bookkeeping.
- Asset directory/file watching with database refresh and reimport visibility.
- Build output double-click navigation for GCC-style `file.cpp:line:column` diagnostics.
- Game View Play, Pause/Resume, Frame Step and NTSC/PAL virtual resolutions.
- Separate play-state transforms so editor scene values are not overwritten by simulation.
- PCSX2 custom executable selection and real-PS2 host configuration.
- ps2client deployment with `PS2HOST` environment configuration.
- ps2gdb/GDB launch hook.
- Existing texture/OBJ/WAV compiler, scene exporter, prefab, profiler and PS2DEV environment checks retained.

Still requires validation on the developer machine:

1. Qt 6 host IDE compile.
2. PS2DEV/gsKit project compile.
3. PCSX2 ELF launch behavior for the installed PCSX2 version.
4. ps2client/ps2link deployment against real hardware.
5. Experimental VU1/DVP path on PCSX2 and real hardware.

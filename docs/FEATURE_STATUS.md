# Feature Status – PS2 Studio 0.17

## Working IDE paths
- tabbed C/C++ editor, save/save-all, syntax highlight, line numbers
- project search and compiler-error navigation
- clangd definition/references/rename/hover/completion/diagnostics
- Problems dock with clickable diagnostics and source squiggles
- EE build, IOP IRX build path, DVP assembly path
- PCSX2 launch/restart/log capture
- ps2client deployment
- GDB console and visual panes
- asset conversion path through external Blender/Assimp
- live rebuild + relaunch
- dependency/bootstrap/environment doctor

## Working runtime foundations
- scenes/components
- 2D + textured 3D EE rendering
- UI text/panel/button/image
- particles
- AABB physics foundation with layers/masks/restitution
- skeletal CPU skinning + animation clips
- multi-voice ADPCM path where supported
- runtime profiler output

## Implemented but requires validation / still experimental
- VU1 backend interface and counters; real VIF/DMA mesh dispatch remains fail-safe to EE
- richer animation-state interfaces; full blending/root motion/IK need runtime iteration
- richer collider/audio/UI layout interfaces; advanced solvers/streaming/layout designer need hardware/game validation
- hot reload is rebuild/relaunch, not arbitrary live code patching
- disc/burn workflows require optical-drive/media/console validation

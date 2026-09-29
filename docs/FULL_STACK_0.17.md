# PS2 Studio 0.17 Full Stack

## IDE / LSP
- clangd diagnostics feed the Problems dock and source squiggles.
- smart completion, hover/type info, go-to-definition, references, and rename are exposed from the Edit menu.
- project-wide search, line navigation, breakpoints, debugger panes, and compiler-error navigation remain available.

## Debug / profiler
- GDB console + locals/stack/registers/memory/watches.
- clangd Problems dock.
- profiler parser accepts extended counters: physics pairs, skinned vertices, VIF packets, VU1 dispatch count.

## Runtime
0.17 retains 0.15/0.16 component scripting, physics foundation, UI, ADPCM audio, skeletal CPU skinning, animation clips, scene runtime, and PCSX2/ps2client workflows.

Additional runtime interfaces are present for animation state machines, audio buses, UI layout metadata, collider shape selection, and a VU1 backend state/counter layer.

## Important validation boundary
The VU1 backend interface is deliberately fail-safe: `dispatchTransform()` returns false until a real VIF/DMA upload + MSCAL synchronization implementation is verified on PCSX2 and real PS2 hardware. The renderer therefore keeps the EE path authoritative.

Likewise, full continuous rigid-body physics, root-motion/IK, lossless live code patching, and disc/hardware behavior still require iterative runtime validation. They are not represented as already verified merely because interfaces exist.

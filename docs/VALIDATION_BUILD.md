# PS2 Studio 0.10 validation workflow

Use **PS2 > Environment Doctor** before treating a build as validated.

Recommended order:
1. Host IDE prerequisites: CMake, Ninja, Git, compiler.
2. PS2DEV variables and EE/IOP/DVP toolchains.
3. Project structure and scene files.
4. Build Debug, Optimized, and Release targets.
5. Run the ELF in PCSX2.
6. Deploy to a homebrew-enabled real PS2 with ps2client/ps2link.
7. Build the disc staging tree and ISO.
8. Burn only when a writable drive/media is available.
9. Perform SHA-256 read-back verification.
10. Record real-hardware boot verification separately.

PS2 Studio does not implement or bundle a retail-console security bypass. A successful burn/read-back test verifies the authored homebrew data, not retail boot authorization.

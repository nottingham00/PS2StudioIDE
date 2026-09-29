# First release validation

1. Run First-Run Setup and Environment Doctor.
2. Create a Validation sample from Help > Samples.
3. Build Debug, Optimized and Release.
4. Verify Game View, then PCSX2.
5. Deploy through ps2client/ps2link and test on real PS2 hardware.
6. Stage an ISO, burn only if desired, verify SHA-256 read-back, then manually record real-console boot results.
7. Generate a Support Bundle if a failure needs investigation.
8. Build release packages with `packaging/windows/Build-Installer.ps1`.

A read-back-verified disc is not equivalent to a retail-bootable disc. No retail-console security bypass is included.

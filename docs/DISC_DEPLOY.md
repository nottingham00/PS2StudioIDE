# Disc / Deploy workflow

1. Build the game ELF normally.
2. Open **PS2 > Disc / Deploy Manager**.
3. **Build Disc Layout** creates `build/disc_stage/` with `SYSTEM.CNF` and `GAME.ELF`. Files under the project's optional `disc/` folder are copied into the root as an overlay.
4. **Build ISO** uses xorriso, genisoimage/mkisofs, or oscdimg when one is available.
5. **Burn Disc** uses Windows IMAPI2 on Windows. On other platforms PS2 Studio can use xorriso where supported.
6. Insert/remount the burned disc, select its filesystem root, and choose **Verify Disc**. Every staged file is compared with SHA-256.
7. Test the disc on the intended real PS2 setup, then manually mark **Real PS2 boot verified** and save the JSON verification report.

Read-back verification confirms file contents. It is not a substitute for hardware boot testing, optical-drive compatibility testing, media-quality testing, or console boot/security compatibility.

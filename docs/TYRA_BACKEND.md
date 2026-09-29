# Tyra Backend
Tyra is an optional external engine backend. PS2 Studio does not vendor or fork it.

- Install: `BOOTSTRAP.cmd` -> `C:\tyra`, `TYRA=C:\tyra`
- New Project -> Engine -> `Tyra Engine`
- Build uses upstream-style `-ltyra`, `engine/inc`, `engine/bin`, `Makefile.base`
- Output ELF under `bin/` is detected for PCSX2/ps2client.
- Native PS2 Studio projects remain unchanged.

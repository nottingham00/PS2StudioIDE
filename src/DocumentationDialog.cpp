#include "DocumentationDialog.h"
#include <QtWidgets>
DocumentationDialog::DocumentationDialog(QWidget*p):QDialog(p){setWindowTitle("PS2 Studio Documentation");resize(900,700);auto*l=new QVBoxLayout(this);auto*t=new QTextBrowser;t->setOpenExternalLinks(true);t->setMarkdown(R"MD(
# PS2 Studio

## Workflow
1. Create or open a project.
2. Create scenes and import textures, OBJ models and WAV audio.
3. Run **PS2 > Environment Doctor** until required toolchain items are detected.
4. Build using Debug, Optimized or Release.
5. Test in PCSX2, then deploy through ps2client/ps2link on real hardware.
6. For discs, use **PS2 > Disc / Deploy Manager** and separately verify the written disc and real-console boot.

## Game View
Game View is an editor-side simulation preview. PCSX2 and real hardware remain the authoritative PS2 runtime targets.

## VU1
The VU/DVP path is experimental. Projects keep an EE fallback until VU1 upload/dispatch is verified for the project.

## Retail discs
PS2 Studio can stage, author, burn and verify homebrew disc data. It does not provide a retail-console security bypass.

## Useful upstream documentation
- [ps2dev organization](https://github.com/ps2dev)
- [PS2SDK](https://github.com/ps2dev/ps2sdk)
- [gsKit](https://github.com/ps2dev/gsKit)
- [PCSX2](https://github.com/PCSX2/pcsx2)
)MD");l->addWidget(t);auto*b=new QDialogButtonBox(QDialogButtonBox::Close);connect(b,&QDialogButtonBox::rejected,this,&QDialog::reject);connect(b,&QDialogButtonBox::accepted,this,&QDialog::accept);l->addWidget(b);}

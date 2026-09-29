#include "TyraTemplates.h"
QString tyraMainSource(const QString& c){return QString(R"(#include "engine.hpp"
#include "%1.hpp"
int main(){ Tyra::Engine engine; Tyra::%1 game(&engine); engine.run(&game); return 0; }
)").arg(c);}
QString tyraGameHeader(const QString& c){return QString(R"(#pragma once
#include <tyra>
namespace Tyra { class %1 : public Game { public: explicit %1(Engine*); ~%1(); void init(); void loop(); private: Engine* engine; }; }
)").arg(c);}
QString tyraGameSource(const QString& c){return QString(R"(#include "%1.hpp"
namespace Tyra {
%1::%1(Engine* e):engine(e){}
%1::~%1(){}
void %1::init(){ TYRA_LOG("PS2 Studio + Tyra initialized"); }
void %1::loop(){ /* game logic */ }
}
)").arg(c);}
QString tyraMakefile(const QString& n){return QString(R"(TARGET := %1.elf
TYRADIR ?= $(TYRA)
ENGINEDIR := $(TYRADIR)/engine
SRCDIR := src
INCDIR := inc
BUILDDIR := obj
TARGETDIR := bin
RESDIR := res
SRCEXT := cpp
VSMEXT := vsm
VCLEXT := vcl
VCLPPEXT := vclpp
DEPEXT := d
OBJEXT := o
CFLAGS :=
LIB := -ltyra
LIBDIRS := -L$(ENGINEDIR)/bin
INC := -I$(INCDIR) -I$(ENGINEDIR)/inc
INCDEP := -I$(INCDIR) -I$(ENGINEDIR)/inc
include $(TYRADIR)/Makefile.base
clean-engine:
	cd $(ENGINEDIR) && $(MAKE) cleaner
build-engine:
	cd $(ENGINEDIR) && $(MAKE)
build-release-engine:
	cd $(ENGINEDIR) && $(MAKE) release
)").arg(n);}
QString tyraRunScript(){return R"($ErrorActionPreference="Stop"
if(-not $env:TYRA){$env:TYRA=[Environment]::GetEnvironmentVariable("TYRA","User")}
if(-not $env:TYRA){throw "TYRA is not configured."}
& make
)";}

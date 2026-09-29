#include "SetupWizardDialog.h"
#include <QtWidgets>
#include <QProcessEnvironment>
#include <QSettings>
#include <QStandardPaths>
#include "DependencyManagerDialog.h"

SetupWizardDialog::SetupWizardDialog(QWidget* p):QDialog(p){
    setWindowTitle("PS2 Studio First-Run Setup"); resize(650,420);
    auto* root=new QVBoxLayout(this);
    auto* title=new QLabel("<h2>PS2 Studio Setup</h2><p>Configure the host tools used by the IDE. You can change these later in Project Settings.</p>"); title->setWordWrap(true);root->addWidget(title);
    auto* form=new QFormLayout; m_ps2dev=new QLineEdit(QProcessEnvironment::systemEnvironment().value("PS2DEV"));m_pcsx2=new QLineEdit(QSettings("PS2Studio","PS2Studio").value("pcsx2Path").toString());m_host=new QLineEdit(QSettings("PS2Studio","PS2Studio").value("ps2Host","192.168.0.10").toString());
    auto row=[&](QLineEdit* e, const char* text, auto slot){auto*w=new QWidget;auto*l=new QHBoxLayout(w);l->setContentsMargins(0,0,0,0);l->addWidget(e);auto*b=new QPushButton(text);l->addWidget(b);connect(b,&QPushButton::clicked,this,slot);return w;};
    form->addRow("PS2DEV root",row(m_ps2dev,"Browse...",&SetupWizardDialog::browsePs2dev));form->addRow("PCSX2 executable",row(m_pcsx2,"Browse...",&SetupWizardDialog::browsePcsx2));form->addRow("Real PS2 host/IP",m_host);root->addLayout(form);
    m_result=new QLabel;m_result->setWordWrap(true);m_result->setFrameShape(QFrame::StyledPanel);m_result->setMinimumHeight(100);root->addWidget(m_result);
    auto* buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);auto* install=new QPushButton("Install Dependencies...");buttons->addButton(install,QDialogButtonBox::ActionRole);connect(install,&QPushButton::clicked,this,[this]{DependencyManagerDialog d(this);d.exec();runCheck();});auto* check=new QPushButton("Run Quick Check");buttons->addButton(check,QDialogButtonBox::ActionRole);connect(check,&QPushButton::clicked,this,&SetupWizardDialog::runCheck);connect(buttons,&QDialogButtonBox::accepted,this,&SetupWizardDialog::save);connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);root->addWidget(buttons);runCheck();
}
QString SetupWizardDialog::findExe(const QStringList& n)const{for(const auto&s:n){auto p=QStandardPaths::findExecutable(s);if(!p.isEmpty())return p;}QString ps=m_ps2dev?m_ps2dev->text().trimmed():QString();if(ps.isEmpty()&&QFileInfo::exists("C:/ps2dev"))ps="C:/ps2dev";QStringList dirs;if(!ps.isEmpty())dirs<<ps+"/ee/bin"<<ps+"/iop/bin"<<ps+"/dvp/bin"<<ps+"/bin"<<ps+"/ps2sdk/bin";
#ifdef Q_OS_WIN
 dirs<<"C:/msys64/usr/bin"<<"C:/Program Files/PCSX2";QString la=qEnvironmentVariable("LOCALAPPDATA");if(!la.isEmpty())dirs<<la+"/Microsoft/WinGet/Links"<<la+"/Programs/PCSX2";
#endif
 for(const auto&d:dirs)for(auto name:n){
#ifdef Q_OS_WIN
 if(!name.endsWith(".exe",Qt::CaseInsensitive))name+=".exe";
#endif
 QString p=QDir(d).filePath(name);if(QFileInfo::exists(p))return p;}return{};}
void SetupWizardDialog::browsePcsx2(){auto p=QFileDialog::getOpenFileName(this,"PCSX2 executable");if(!p.isEmpty())m_pcsx2->setText(p);}void SetupWizardDialog::browsePs2dev(){auto p=QFileDialog::getExistingDirectory(this,"PS2DEV root");if(!p.isEmpty())m_ps2dev->setText(p);}
void SetupWizardDialog::runCheck(){QStringList r;auto mark=[&](bool ok,const QString&s){r<<QString("%1 %2").arg(ok?"✓":"○",s);};mark(QFileInfo::exists(m_ps2dev->text()),"PS2DEV root");mark(!findExe({"cmake"}).isEmpty(),"CMake");mark(!findExe({"ninja","ninja-build"}).isEmpty(),"Ninja");mark(!findExe({"git"}).isEmpty(),"Git");mark(!findExe({"make","mingw32-make"}).isEmpty(),"GNU make");mark(!findExe({"mips64r5900el-ps2-elf-g++"}).isEmpty(),"EE compiler");mark(!findExe({"mipsel-none-elf-gcc"}).isEmpty(),"IOP compiler");mark(QFileInfo::exists(m_pcsx2->text())||!findExe({"pcsx2-qt","pcsx2"}).isEmpty(),"PCSX2");mark(!findExe({"ps2client"}).isEmpty(),"ps2client");m_result->setText(r.join("<br>"));}
void SetupWizardDialog::save(){QSettings s("PS2Studio","PS2Studio");s.setValue("pcsx2Path",m_pcsx2->text().trimmed());s.setValue("ps2Host",m_host->text().trimmed());s.setValue("setupCompleted",true);if(!m_ps2dev->text().trimmed().isEmpty())s.setValue("ps2devHint",m_ps2dev->text().trimmed());accept();}

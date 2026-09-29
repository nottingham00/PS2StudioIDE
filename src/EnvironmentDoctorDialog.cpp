#include "EnvironmentDoctorDialog.h"
#include <QtWidgets>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QDirIterator>

EnvironmentDoctorDialog::EnvironmentDoctorDialog(const QString& projectRoot,const QString& pcsx2Path,const QString& ps2Host,QWidget* parent)
    :QDialog(parent),m_projectRoot(projectRoot),m_pcsx2Path(pcsx2Path),m_ps2Host(ps2Host){
    setWindowTitle("PS2 Studio Environment Doctor");resize(980,720);
    auto* root=new QVBoxLayout(this);
    auto* intro=new QLabel("Checks the host IDE toolchain, PS2DEV environment, testing/deploy tools, project structure, and disc-authoring prerequisites. Hardware behavior still requires real validation.");intro->setWordWrap(true);root->addWidget(intro);
    m_overall=new QLabel;QFont f=m_overall->font();f.setBold(true);f.setPointSize(f.pointSize()+2);m_overall->setFont(f);root->addWidget(m_overall);
    m_table=new QTableWidget(0,5);m_table->setHorizontalHeaderLabels({"Group","Check","Status","Details","Suggested fix"});for(int i=0;i<3;i++)m_table->horizontalHeader()->setSectionResizeMode(i,QHeaderView::ResizeToContents);m_table->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Stretch);m_table->horizontalHeader()->setSectionResizeMode(4,QHeaderView::Stretch);m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);root->addWidget(m_table,1);
    m_summary=new QPlainTextEdit;m_summary->setReadOnly(true);m_summary->setMaximumHeight(130);root->addWidget(m_summary);
    auto* row=new QHBoxLayout;auto* run=new QPushButton("Run Full Diagnostic");auto* save=new QPushButton("Save JSON Report");auto* close=new QPushButton("Close");row->addWidget(run);row->addWidget(save);row->addStretch();row->addWidget(close);root->addLayout(row);connect(run,&QPushButton::clicked,this,&EnvironmentDoctorDialog::runChecks);connect(save,&QPushButton::clicked,this,&EnvironmentDoctorDialog::saveReport);connect(close,&QPushButton::clicked,this,&QDialog::accept);runChecks();
}
QString EnvironmentDoctorDialog::findTool(const QStringList& names)const{
    for(const auto&n:names){auto p=QStandardPaths::findExecutable(n);if(!p.isEmpty())return p;}
    QString ps2=qEnvironmentVariable("PS2DEV");if(ps2.isEmpty()&&QFileInfo::exists("C:/ps2dev"))ps2="C:/ps2dev";
    QStringList dirs;if(!ps2.isEmpty())dirs<<ps2+"/bin"<<ps2+"/ee/bin"<<ps2+"/iop/bin"<<ps2+"/dvp/bin"<<ps2+"/ps2sdk/bin";
#ifdef Q_OS_WIN
    dirs<<"C:/msys64/usr/bin"<<"C:/msys64/mingw64/bin"<<"C:/Program Files/PCSX2"<<"C:/Program Files/PCSX2 Nightly";
    QString la=qEnvironmentVariable("LOCALAPPDATA");if(!la.isEmpty())dirs<<la+"/Microsoft/WinGet/Links"<<la+"/Programs/PCSX2";
#endif
    for(const auto&d:dirs)for(auto n:names){
#ifdef Q_OS_WIN
        if(!n.endsWith(".exe",Qt::CaseInsensitive))n+=".exe";
#endif
        QString p=QDir(d).filePath(n);if(QFileInfo::exists(p))return QFileInfo(p).absoluteFilePath();
    }
#ifdef Q_OS_WIN
    for(const auto&r:QStringList{"C:/Program Files/PCSX2","C:/Program Files/PCSX2 Nightly",qEnvironmentVariable("LOCALAPPDATA")+"/Programs/PCSX2"}){if(r.isEmpty()||!QFileInfo::exists(r))continue;QDirIterator it(r,{"pcsx2-qt.exe","pcsx2.exe"},QDir::Files,QDirIterator::Subdirectories);if(it.hasNext())return it.next();}
#endif
    return{};
}
void EnvironmentDoctorDialog::addCheck(const QString&group,const QString&name,bool ok,const QString&detail,const QString&fix){int r=m_table->rowCount();m_table->insertRow(r);QStringList vals{group,name,ok?"PASS":"MISSING",detail,fix};for(int c=0;c<vals.size();++c)m_table->setItem(r,c,new QTableWidgetItem(vals[c]));m_results.append(QJsonObject{{"group",group},{"name",name},{"ok",ok},{"detail",detail},{"fix",fix}});}
bool EnvironmentDoctorDialog::pingHost(const QString&host,QString*detail)const{if(host.trimmed().isEmpty()){if(detail)*detail="No PS2 host configured";return false;}QProcess p;
#ifdef Q_OS_WIN
    p.start("ping",{"-n","1","-w","900",host});
#else
    p.start("ping",{"-c","1","-W","1",host});
#endif
    if(!p.waitForFinished(1800)){p.kill();if(detail)*detail="Ping timed out";return false;}bool ok=p.exitCode()==0;if(detail)*detail=ok?"Host responded to ping":"No ping response (this does not prove ps2link is unavailable)";return ok;}
bool EnvironmentDoctorDialog::burnerPresent(QString*detail)const{
#ifdef Q_OS_WIN
    QProcess p;p.start("powershell.exe",{"-NoProfile","-Command","$m=New-Object -ComObject IMAPI2.MsftDiscMaster2; Write-Output $m.Count"});if(!p.waitForFinished(2500)){p.kill();if(detail)*detail="IMAPI2 probe timed out";return false;}bool ok=false;int count=QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed().toInt(&ok);if(detail)*detail=ok?QString("%1 writable optical recorder(s) reported by IMAPI2").arg(count):"Could not query IMAPI2";return ok&&count>0;
#else
    auto x=findTool({"xorriso","wodim","cdrecord"});if(detail)*detail=x.isEmpty()?"No supported burn command found":x;return !x.isEmpty();
#endif
}
void EnvironmentDoctorDialog::runChecks(){m_table->setRowCount(0);m_results=QJsonArray{};auto env=QProcessEnvironment::systemEnvironment();auto tool=[&](QString g,QString n,QStringList names,QString fix){auto p=findTool(names);addCheck(g,n,!p.isEmpty(),p.isEmpty()?"Not found":p,fix);};
    tool("Host","CMake",{"cmake"},"Install CMake.");tool("Host","Ninja",{"ninja"},"Install Ninja.");tool("Host","Git",{"git"},"Install Git.");tool("Host","GNU make",{"make","mingw32-make"},"Install MSYS2 GNU Make (BOOTSTRAP.cmd installs it).");
#ifdef Q_OS_WIN
    tool("Host","MSVC compiler",{"cl"},"Install MSVC Build Tools or Visual Studio C++ workload.");
#else
    tool("Host","C++ compiler",{"clang++","g++"},"Install clang++ or g++.");
#endif
    for(auto key:{"PS2DEV","PS2SDK","GSKIT"}){QString v=env.value(key);if(v.isEmpty()&&QString(key)=="PS2DEV"&&QFileInfo::exists("C:/ps2dev"))v="C:/ps2dev";if(v.isEmpty()&&QString(key)=="PS2SDK"&&QFileInfo::exists("C:/ps2dev/ps2sdk"))v="C:/ps2dev/ps2sdk";if(v.isEmpty()&&QString(key)=="GSKIT"&&QFileInfo::exists("C:/ps2dev/gsKit"))v="C:/ps2dev/gsKit";addCheck("PS2DEV",key,!v.isEmpty()&&QFileInfo::exists(v),v.isEmpty()?"Not configured":v,"Run BOOTSTRAP.cmd or configure this path.");}
    tool("PS2DEV","EE compiler",{"mips64r5900el-ps2-elf-g++"},"Install the EE toolchain.");tool("PS2DEV","IOP compiler",{"mipsel-none-elf-gcc"},"Install the IOP toolchain.");tool("PS2DEV","DVP assembler",{"dvp-as"},"Install the DVP/VU toolchain.");tool("PS2DEV","ps2client",{"ps2client"},"Install ps2client.");tool("PS2DEV","PS2 GDB",{"mips64r5900el-ps2-elf-gdb","ps2gdb"},"Install a PS2-compatible GDB client.");
    QString pcsx=(!m_pcsx2Path.isEmpty()&&QFileInfo::exists(m_pcsx2Path))?m_pcsx2Path:findTool({"pcsx2-qt","pcsx2","pcsx2-qtx64-avx2"});addCheck("Testing","PCSX2",!pcsx.isEmpty(),pcsx.isEmpty()?"Not configured or found":pcsx,"Install PCSX2 or set it in Project Settings.");
    QString ping;bool hostOk=pingHost(m_ps2Host,&ping);addCheck("Testing","Real PS2 host",hostOk,m_ps2Host+" — "+ping,"Check network/ps2link configuration.");
    tool("Disc","ISO authoring",{"xorriso","genisoimage","mkisofs","oscdimg"},"Install an ISO authoring tool if disc output is needed.");QString burn;bool burnOk=burnerPresent(&burn);addCheck("Disc","Writable optical drive",burnOk,burn,"Connect a writable optical drive if physical-disc testing is required.");
    bool haveProject=!m_projectRoot.isEmpty()&&QDir(m_projectRoot).exists();addCheck("Project","Project open",haveProject,haveProject?m_projectRoot:"No project is open","Open or create a project.");if(haveProject){auto f=[&](QString n,QString rel){bool ok=QFileInfo::exists(QDir(m_projectRoot).filePath(rel));addCheck("Project",n,ok,rel,ok?QString():"Restore/regenerate this required file.");};f("Project metadata","ps2studio.json");f("Makefile","Makefile");f("Runtime header","runtime/PS2SceneRuntime.h");f("Runtime source","runtime/PS2SceneRuntime.cpp");bool scene=!QDir(m_projectRoot+"/scenes").entryList({"*.ps2scene"},QDir::Files).isEmpty();addCheck("Project","At least one scene",scene,scene?"Scene file found":"No .ps2scene found","Create and save a scene.");}
    int pass=0;for(const auto&v:m_results)if(v.toObject().value("ok").toBool())pass++;int total=m_results.size();m_overall->setText(QString("%1 / %2 checks passed").arg(pass).arg(total));m_summary->setPlainText(pass==total?"Environment looks ready for a validation build.":QString("%1 check(s) need attention. Hardware behavior remains a separate validation step.").arg(total-pass));}
void EnvironmentDoctorDialog::saveReport(){QJsonObject root{{"format","PS2StudioEnvironmentDoctor"},{"version",2},{"timestamp",QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},{"project",m_projectRoot},{"ps2Host",m_ps2Host},{"results",m_results}};QString base=m_projectRoot.isEmpty()?QDir::homePath():QDir(m_projectRoot).filePath("build");QDir().mkpath(base);QString p=QFileDialog::getSaveFileName(this,"Save Environment Report",QDir(base).filePath("environment-doctor.json"),"JSON (*.json)");if(p.isEmpty())return;QFile f(p);if(f.open(QIODevice::WriteOnly|QIODevice::Truncate))f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));}

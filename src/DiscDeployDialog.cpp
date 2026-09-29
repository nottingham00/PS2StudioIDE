#include "DiscDeployDialog.h"
#include <QtWidgets>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

DiscDeployDialog::DiscDeployDialog(const QString& projectRoot,const QString& elfPath,QWidget* parent)
    :QDialog(parent),m_projectRoot(projectRoot),m_elfPath(elfPath){
    setWindowTitle("PS2 Disc / Deploy Manager");resize(820,680);
    m_stageDir=QDir(projectRoot).filePath("build/disc_stage");
    m_isoPath=QDir(projectRoot).filePath("build/PS2Game.iso");
    auto* root=new QVBoxLayout(this);
    auto* form=new QFormLayout;
    m_stageEdit=new QLineEdit(m_stageDir);m_isoEdit=new QLineEdit(m_isoPath);m_volumeEdit=new QLineEdit("PS2GAME");
    m_videoCombo=new QComboBox;m_videoCombo->addItems({"NTSC","PAL"});m_driveEdit=new QLineEdit;
    m_driveEdit->setPlaceholderText("Verification drive, e.g. D:\\ or /media/ps2disc");
    form->addRow("Staging folder",m_stageEdit);form->addRow("ISO output",m_isoEdit);form->addRow("Volume label",m_volumeEdit);form->addRow("SYSTEM.CNF VMODE",m_videoCombo);form->addRow("Burned disc root",m_driveEdit);root->addLayout(form);
    auto* buttons=new QHBoxLayout;
    auto* layoutBtn=new QPushButton("1. Build Disc Layout");auto* isoBtn=new QPushButton("2. Build ISO");auto* burnBtn=new QPushButton("3. Burn Disc");auto* verifyBtn=new QPushButton("4. Verify Disc");
    buttons->addWidget(layoutBtn);buttons->addWidget(isoBtn);buttons->addWidget(burnBtn);buttons->addWidget(verifyBtn);root->addLayout(buttons);
    auto* statusBox=new QGroupBox("Verification state");auto* status=new QFormLayout(statusBox);
    m_layoutStatus=new QLabel;m_isoStatus=new QLabel;m_burnStatus=new QLabel;m_verifyStatus=new QLabel;m_realHardwareConfirmed=new QCheckBox("Real PS2 boot verified manually");
    status->addRow("Disc layout",m_layoutStatus);status->addRow("ISO image",m_isoStatus);status->addRow("Burn operation",m_burnStatus);status->addRow("Read-back file verification",m_verifyStatus);status->addRow("Hardware",m_realHardwareConfirmed);root->addWidget(statusBox);
    auto* note=new QLabel("Boot compatibility is a separate hardware test. PS2 Studio builds and verifies the homebrew disc contents; it does not bypass retail-console security or claim that a recordable disc will boot on an unmodified PS2.");note->setWordWrap(true);root->addWidget(note);
    m_log=new QPlainTextEdit;m_log->setReadOnly(true);root->addWidget(m_log,1);
    auto* footer=new QHBoxLayout;auto* report=new QPushButton("Save Verification Report");auto* close=new QPushButton("Close");footer->addWidget(report);footer->addStretch();footer->addWidget(close);root->addLayout(footer);
    m_process=new QProcess(this);connect(m_process,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int c,QProcess::ExitStatus){onProcessFinished(c);});connect(m_process,&QProcess::readyReadStandardOutput,this,[this]{log(QString::fromLocal8Bit(m_process->readAllStandardOutput()));});connect(m_process,&QProcess::readyReadStandardError,this,[this]{log(QString::fromLocal8Bit(m_process->readAllStandardError()));});
    connect(layoutBtn,&QPushButton::clicked,this,&DiscDeployDialog::buildLayout);connect(isoBtn,&QPushButton::clicked,this,&DiscDeployDialog::buildIso);connect(burnBtn,&QPushButton::clicked,this,&DiscDeployDialog::burnDisc);connect(verifyBtn,&QPushButton::clicked,this,&DiscDeployDialog::verifyDisc);connect(report,&QPushButton::clicked,this,&DiscDeployDialog::saveVerificationReport);connect(close,&QPushButton::clicked,this,&QDialog::accept);
    updateStatus();
}
QString DiscDeployDialog::findTool(const QStringList& names)const{for(const auto&n:names){auto p=QStandardPaths::findExecutable(n);if(!p.isEmpty())return p;}return{};}
void DiscDeployDialog::log(const QString&t){m_log->appendPlainText(t.trimmed());}
void DiscDeployDialog::updateStatus(){auto s=[](bool ok){return ok?QString("✓ passed"):QString("○ not verified");};m_layoutStatus->setText(s(m_layoutOk));m_isoStatus->setText(s(m_isoOk));m_burnStatus->setText(s(m_burnOk));m_verifyStatus->setText(s(m_verifyOk));}
bool DiscDeployDialog::copyTree(const QString&from,const QString&to,QString*error){QDir src(from);if(!src.exists())return true;QDir().mkpath(to);for(const auto&fi:src.entryInfoList(QDir::Files|QDir::NoDotAndDotDot)){if(!QFile::copy(fi.absoluteFilePath(),QDir(to).filePath(fi.fileName()))){QFile::remove(QDir(to).filePath(fi.fileName()));if(!QFile::copy(fi.absoluteFilePath(),QDir(to).filePath(fi.fileName()))){if(error)*error="Failed to copy "+fi.absoluteFilePath();return false;}}}for(const auto&di:src.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot))if(!copyTree(di.absoluteFilePath(),QDir(to).filePath(di.fileName()),error))return false;return true;}
void DiscDeployDialog::buildLayout(){m_stageDir=m_stageEdit->text().trimmed();if(m_elfPath.isEmpty()||!QFileInfo::exists(m_elfPath)){QMessageBox::warning(this,"Disc Layout","Build the project first; no ELF is available.");return;}QDir stage(m_stageDir);if(stage.exists())stage.removeRecursively();QDir().mkpath(m_stageDir);QString target=stage.filePath("GAME.ELF");if(!QFile::copy(m_elfPath,target)){QMessageBox::warning(this,"Disc Layout","Could not copy ELF into staging folder.");return;}QFile cnf(stage.filePath("SYSTEM.CNF"));if(!cnf.open(QIODevice::WriteOnly|QIODevice::Text)){QMessageBox::warning(this,"Disc Layout","Could not write SYSTEM.CNF.");return;}cnf.write(QString("BOOT2 = cdrom0:\\\\GAME.ELF;1\r\nVER = 1.00\r\nVMODE = %1\r\n").arg(m_videoCombo->currentText()).toLatin1());cnf.close();QString err;copyTree(QDir(m_projectRoot).filePath("disc"),m_stageDir,&err);if(!err.isEmpty()){QMessageBox::warning(this,"Disc Layout",err);return;}m_layoutOk=QFileInfo::exists(stage.filePath("SYSTEM.CNF"))&&QFileInfo::exists(target);log("Disc layout created: "+m_stageDir);log("Boot ELF: GAME.ELF\nSYSTEM.CNF generated.");updateStatus();}
void DiscDeployDialog::buildIso(){if(!m_layoutOk)buildLayout();if(!m_layoutOk)return;m_isoPath=m_isoEdit->text().trimmed();QDir().mkpath(QFileInfo(m_isoPath).absolutePath());auto x=findTool({"xorriso"});auto mk=findTool({"genisoimage","mkisofs"});auto oscd=findTool({"oscdimg"});if(!x.isEmpty()){m_process->setProperty("op","iso");m_process->start(x,{"-as","mkisofs","-iso-level","2","-V",m_volumeEdit->text().left(32),"-o",m_isoPath,m_stageDir});log("> xorriso -as mkisofs ...");return;}if(!mk.isEmpty()){m_process->setProperty("op","iso");m_process->start(mk,{"-iso-level","2","-V",m_volumeEdit->text().left(32),"-o",m_isoPath,m_stageDir});log("> mkisofs/genisoimage ...");return;}if(!oscd.isEmpty()){m_process->setProperty("op","iso");m_process->start(oscd,{"-n","-m","-l" ,QString("-l%1").arg(m_volumeEdit->text().left(32)),m_stageDir,m_isoPath});log("> oscdimg ...");return;}QMessageBox::information(this,"ISO Builder","No ISO authoring tool was found. Install xorriso, genisoimage/mkisofs, or Windows ADK oscdimg.\n\nYou can still burn the staging folder directly on Windows using the native IMAPI2 option.");}
void DiscDeployDialog::burnDisc(){if(!m_layoutOk)buildLayout();if(!m_layoutOk)return;
#ifdef Q_OS_WIN
    // IMAPI2 burns the verified staging directory. This avoids depending on third-party burning software.
    QString ps=QDir::temp().filePath("ps2studio_burn.ps1");QFile f(ps);if(!f.open(QIODevice::WriteOnly|QIODevice::Text)){QMessageBox::warning(this,"Burn","Cannot create temporary IMAPI script.");return;}
    QString script=R"PS($ErrorActionPreference='Stop'
$source=$args[0]
$master=New-Object -ComObject IMAPI2.MsftDiscMaster2
if($master.Count -lt 1){throw 'No writable optical recorder detected.'}
$rec=New-Object -ComObject IMAPI2.MsftDiscRecorder2
$rec.InitializeDiscRecorder($master.Item(0))
$writer=New-Object -ComObject IMAPI2.MsftDiscFormat2Data
$writer.Recorder=$rec
$writer.ClientName='PS2 Studio'
if(-not $writer.IsCurrentMediaSupported($rec)){throw 'Inserted media is not writable/supported.'}
$fsi=New-Object -ComObject IMAPI2FS.MsftFileSystemImage
$fsi.ChooseImageDefaults($rec)
$fsi.FileSystemsToCreate=1
$fsi.VolumeName='PS2GAME'
$fsi.Root.AddTree($source,$false)
$result=$fsi.CreateResultImage()
$writer.Write($result.ImageStream)
Write-Output 'IMAPI2 burn complete.'
)PS";
    f.write(script.toUtf8());f.close();m_process->setProperty("op","burn");m_process->start("powershell.exe",{"-NoProfile","-ExecutionPolicy","Bypass","-File",ps,m_stageDir});log("Starting native Windows IMAPI2 burn using the first writable optical recorder...");
#else
    auto x=findTool({"xorriso"});if(x.isEmpty()||m_isoPath.isEmpty()||!QFileInfo::exists(m_isoPath)){QMessageBox::information(this,"Burn","On this platform configure/use a supported disc-burning tool. PS2 Studio can still build and verify the disc layout/ISO.");return;}QString dev=QInputDialog::getText(this,"Burn device","xorriso device path (for example /dev/sr0):");if(dev.isEmpty())return;m_process->setProperty("op","burn");m_process->start(x,{"-as","cdrecord","dev="+dev,"-v","-eject",m_isoPath});
#endif
}
QString DiscDeployDialog::hashFile(const QString&path)const{QFile f(path);if(!f.open(QIODevice::ReadOnly))return{};QCryptographicHash h(QCryptographicHash::Sha256);while(!f.atEnd())h.addData(f.read(1024*1024));return QString::fromLatin1(h.result().toHex());}
bool DiscDeployDialog::verifyTree(const QString&expected,const QString&actual,QStringList*problems)const{QDir e(expected),a(actual);if(!e.exists()||!a.exists()){problems->append("Expected or actual root does not exist.");return false;}bool ok=true;QDirIterator it(expected,QDir::Files,QDirIterator::Subdirectories);while(it.hasNext()){QString src=it.next();QString rel=e.relativeFilePath(src);QString dst=a.filePath(rel);if(!QFileInfo::exists(dst)){problems->append("Missing: "+rel);ok=false;continue;}auto hs=hashFile(src),hd=hashFile(dst);if(hs.isEmpty()||hd.isEmpty()||hs!=hd){problems->append("Hash mismatch: "+rel);ok=false;}}return ok;}
void DiscDeployDialog::verifyDisc(){QString root=m_driveEdit->text().trimmed();if(root.isEmpty())root=QFileDialog::getExistingDirectory(this,"Select burned disc root");if(root.isEmpty())return;m_driveEdit->setText(root);QStringList p;m_verifyOk=verifyTree(m_stageDir,root,&p);if(m_verifyOk)log("Read-back verification passed: every staged file matched SHA-256.");else{log("Read-back verification failed:");for(const auto&s:p)log("  "+s);}updateStatus();}
void DiscDeployDialog::saveVerificationReport(){QJsonObject o{{"project",m_projectRoot},{"elf",m_elfPath},{"staging",m_stageDir},{"iso",m_isoPath},{"layoutVerified",m_layoutOk},{"isoBuilt",m_isoOk},{"burnCompleted",m_burnOk},{"readBackVerified",m_verifyOk},{"realPs2BootConfirmed",m_realHardwareConfirmed->isChecked()},{"timestamp",QDateTime::currentDateTimeUtc().toString(Qt::ISODate)}};QString p=QFileDialog::getSaveFileName(this,"Save Verification Report",QDir(m_projectRoot).filePath("build/disc-verification.json"),"JSON (*.json)");if(p.isEmpty())return;QFile f(p);if(f.open(QIODevice::WriteOnly))f.write(QJsonDocument(o).toJson(QJsonDocument::Indented));}
void DiscDeployDialog::onProcessFinished(int exitCode){QString op=m_process->property("op").toString();if(op=="iso")m_isoOk=(exitCode==0&&QFileInfo::exists(m_isoPath)&&QFileInfo(m_isoPath).size()>0);else if(op=="burn")m_burnOk=(exitCode==0);log(QString("%1 finished with exit code %2").arg(op).arg(exitCode));updateStatus();}

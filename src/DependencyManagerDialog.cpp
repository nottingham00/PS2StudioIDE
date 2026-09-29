#include "DependencyManagerDialog.h"
#include <QtWidgets>
#include <QCoreApplication>
#include <QProcess>
#include <QFileInfo>
#include <QDir>

DependencyManagerDialog::DependencyManagerDialog(QWidget* parent):QDialog(parent){
    setWindowTitle("PS2 Studio Dependency Manager"); resize(820,560);
    auto* root=new QVBoxLayout(this);
    auto* intro=new QLabel(
        "<h2>Dependencies</h2>"
        "<p>Install or repair the tools PS2 Studio uses. Host tools are needed to compile the IDE itself; PS2DEV is the compiler/SDK used for games. "
        "Administrator approval may be requested by Windows installers.</p>"
        "<p><b>Important:</b> if Qt or Visual Studio is missing before PS2 Studio has been compiled, run <code>BOOTSTRAP.cmd</code> from the source folder first.</p>");
    intro->setWordWrap(true); root->addWidget(intro);
    auto* buttons=new QHBoxLayout;
    m_all=new QPushButton("Install All"); m_host=new QPushButton("Host Build Tools"); m_ps2=new QPushButton("PS2DEV SDK"); m_optional=new QPushButton("Optional Tools"); m_verify=new QPushButton("Verify");
    buttons->addWidget(m_all);buttons->addWidget(m_host);buttons->addWidget(m_ps2);buttons->addWidget(m_optional);buttons->addStretch();buttons->addWidget(m_verify);root->addLayout(buttons);
    m_status=new QLabel("Ready");root->addWidget(m_status);
    m_output=new QPlainTextEdit;m_output->setReadOnly(true);m_output->setLineWrapMode(QPlainTextEdit::NoWrap);root->addWidget(m_output,1);
    auto* close=new QDialogButtonBox(QDialogButtonBox::Close);connect(close,&QDialogButtonBox::rejected,this,&QDialog::reject);root->addWidget(close);
    m_process=new QProcess(this);m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process,&QProcess::readyReadStandardOutput,this,&DependencyManagerDialog::processReady);
    connect(m_process,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,&DependencyManagerDialog::processFinished);
    connect(m_all,&QPushButton::clicked,this,&DependencyManagerDialog::installAll);
    connect(m_host,&QPushButton::clicked,this,&DependencyManagerDialog::installHost);
    connect(m_ps2,&QPushButton::clicked,this,&DependencyManagerDialog::installPs2);
    connect(m_optional,&QPushButton::clicked,this,&DependencyManagerDialog::installOptional);
    connect(m_verify,&QPushButton::clicked,this,&DependencyManagerDialog::verify);
}
QString DependencyManagerDialog::scriptPath(const QString& name) const{
    const QStringList roots={QCoreApplication::applicationDirPath()+"/scripts",QCoreApplication::applicationDirPath()+"/../scripts",QDir::currentPath()+"/scripts"};
    for(const auto&r:roots){QString p=QDir(r).filePath(name);if(QFileInfo::exists(p))return QDir::cleanPath(p);}return{};
}
void DependencyManagerDialog::setBusy(bool b){for(auto*w:{m_all,m_host,m_ps2,m_optional,m_verify})w->setEnabled(!b);m_status->setText(b?"Running installer...":"Ready");}
void DependencyManagerDialog::runScript(const QString& script,const QStringList& args){
    if(script.isEmpty()){QMessageBox::warning(this,"Script missing","The dependency scripts were not found next to PS2 Studio. Reinstall PS2 Studio or run them from the source package.");return;}
    if(m_process->state()!=QProcess::NotRunning)return;
    m_output->appendPlainText(QString("\n> powershell -File \"%1\" %2\n").arg(script,args.join(' ')));setBusy(true);
#ifdef Q_OS_WIN
    QStringList psArgs={"-NoProfile","-ExecutionPolicy","Bypass","-File",QDir::toNativeSeparators(script)};psArgs+=args;m_process->start("powershell.exe",psArgs);
#else
    m_output->appendPlainText("Automatic installation is currently Windows-first. Use the platform instructions in docs/DEPENDENCIES.md.");setBusy(false);
#endif
}
void DependencyManagerDialog::runBootstrap(const QString& mode){runScript(scriptPath("Bootstrap-Windows.ps1"),{"-Mode",mode,"-NonInteractive"});}
void DependencyManagerDialog::installAll(){runBootstrap("All");}void DependencyManagerDialog::installHost(){runBootstrap("Host");}void DependencyManagerDialog::installPs2(){runBootstrap("PS2");}void DependencyManagerDialog::installOptional(){runBootstrap("Optional");}
void DependencyManagerDialog::verify(){runScript(scriptPath("Verify-Dependencies.ps1"));}
void DependencyManagerDialog::processReady(){m_output->moveCursor(QTextCursor::End);m_output->insertPlainText(QString::fromLocal8Bit(m_process->readAllStandardOutput()));m_output->moveCursor(QTextCursor::End);}
void DependencyManagerDialog::processFinished(int code,QProcess::ExitStatus){processReady();setBusy(false);m_status->setText(code==0?"Completed successfully":"Finished with errors");if(code==0)m_output->appendPlainText("\n[OK] Dependency operation completed. Restart PS2 Studio if PATH/environment variables changed.\n");else m_output->appendPlainText(QString("\n[FAILED] Installer exit code: %1\n").arg(code));}

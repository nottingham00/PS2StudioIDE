#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QMutex>
#include "MainWindow.h"

static QString logPath(){QString d=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);QDir().mkpath(d+"/logs");return d+"/logs/ps2studio.log";}
static void logMessage(QtMsgType type,const QMessageLogContext&,const QString& msg){static QMutex m;QMutexLocker lock(&m);QFile f(logPath());if(f.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text)){QTextStream s(&f);const char* t=type==QtDebugMsg?"DEBUG":type==QtInfoMsg?"INFO":type==QtWarningMsg?"WARN":type==QtCriticalMsg?"ERROR":"FATAL";s<<QDateTime::currentDateTimeUtc().toString(Qt::ISODate)<<" ["<<t<<"] "<<msg<<'\n';}if(type==QtFatalMsg)abort();}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("PS2 Studio");
    QApplication::setApplicationVersion("0.14.0");
    QApplication::setOrganizationName("mjf_0.0");
    qInstallMessageHandler(logMessage);
    qInfo()<<"PS2 Studio starting"<<QApplication::applicationVersion();
    MainWindow w;
    w.show();
    int rc=app.exec();
    qInfo()<<"PS2 Studio exiting"<<rc;
    return rc;
}

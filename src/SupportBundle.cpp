#include "SupportBundle.h"
#include <QtWidgets>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
namespace SupportBundle{
bool create(const QString& root,QWidget* parent,QString*out){QString base=QFileDialog::getExistingDirectory(parent,"Choose folder for support bundle");if(base.isEmpty())return false;QString stamp=QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");QString dir=base+"/PS2Studio-Support-"+stamp;QDir().mkpath(dir);QJsonObject j{{"generatedAt",QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},{"project",root}};auto env=QProcessEnvironment::systemEnvironment();for(auto k:{"PS2DEV","PS2SDK","GSKIT","PATH"})j[QString(k)]=env.value(k);QSettings s("PS2Studio","PS2Studio");j["appVersion"]=QCoreApplication::applicationVersion();j["pcsx2Path"]=s.value("pcsx2Path").toString();j["ps2Host"]=s.value("ps2Host").toString();QFile f(dir+"/environment.json");if(f.open(QIODevice::WriteOnly))f.write(QJsonDocument(j).toJson(QJsonDocument::Indented));if(!root.isEmpty()){for(auto rel:{"ps2studio.json",".ps2studio/project.json","validation-report.json"}){QString src=root+"/"+rel;if(QFileInfo::exists(src))QFile::copy(src,dir+"/"+QString(rel).replace('/','_'));}}QString appLog=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/logs/ps2studio.log";if(QFileInfo::exists(appLog))QFile::copy(appLog,dir+"/ps2studio.log");QFile note(dir+"/README.txt");if(note.open(QIODevice::WriteOnly|QIODevice::Text))note.write("PS2 Studio support bundle. Review files before sharing. No game assets are included automatically.\n");if(out)*out=dir;QDesktopServices::openUrl(QUrl::fromLocalFile(dir));return true;}
}

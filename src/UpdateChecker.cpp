#include "UpdateChecker.h"
#include <QtWidgets>
#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QDesktopServices>
void UpdateChecker::check(QWidget* parent){
    QSettings s("PS2Studio","PS2Studio");
    QString feed=s.value("updateFeedUrl").toString().trimmed();
    if(feed.isEmpty()){
        QMessageBox mb(parent);mb.setWindowTitle("Update Check");mb.setText("PS2 Studio update feed is not configured yet.");mb.setInformativeText("The update-check plumbing is installed, but this source package is not tied to a public PS2 Studio release repository. You can configure an API/JSON release URL later without changing the IDE UI.");auto*up=mb.addButton("Open PS2DEV releases",QMessageBox::ActionRole);mb.addButton(QMessageBox::Close);mb.exec();if(mb.clickedButton()==up)QDesktopServices::openUrl(QUrl("https://github.com/ps2dev/ps2dev/releases"));return;
    }
    auto*nam=new QNetworkAccessManager(this);QNetworkRequest req{QUrl(feed)};req.setHeader(QNetworkRequest::UserAgentHeader,"PS2Studio/0.14");auto*r=nam->get(req);connect(r,&QNetworkReply::finished,this,[r,parent]{if(r->error()!=QNetworkReply::NoError){auto e=r->errorString();r->deleteLater();QMessageBox::information(parent,"Update Check","Could not query the configured update feed.\n\n"+e);return;}auto data=r->readAll();r->deleteLater();auto o=QJsonDocument::fromJson(data).object();auto tag=o.value("tag_name").toString(o.value("version").toString());auto url=o.value("html_url").toString(o.value("url").toString());QMessageBox mb(parent);mb.setWindowTitle("Update Check");mb.setText(tag.isEmpty()?"Update feed responded successfully.":"Latest PS2 Studio version: "+tag);auto*open=mb.addButton("Open release page",QMessageBox::ActionRole);mb.addButton(QMessageBox::Close);mb.exec();if(mb.clickedButton()==open&&!url.isEmpty())QDesktopServices::openUrl(QUrl(url));});
}

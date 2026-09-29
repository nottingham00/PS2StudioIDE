#include "ClangdClient.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QCoreApplication>
#include <QUrl>

ClangdClient::ClangdClient(QObject* p):QObject(p){
    connect(&m_proc,&QProcess::readyReadStandardOutput,this,[this]{m_buf+=m_proc.readAllStandardOutput();consume();});
    connect(&m_proc,&QProcess::readyReadStandardError,this,[this]{m_error+=QString::fromLocal8Bit(m_proc.readAllStandardError());});
}
void ClangdClient::send(const QJsonObject& o){
    const QByteArray body=QJsonDocument(o).toJson(QJsonDocument::Compact);
    m_proc.write("Content-Length: "+QByteArray::number(body.size())+"\r\n\r\n"+body);
}
QJsonObject ClangdClient::textPosition(const QString& file,int line,int col) const{
    return QJsonObject{{"textDocument",QJsonObject{{"uri",QUrl::fromLocalFile(file).toString()}}},
                       {"position",QJsonObject{{"line",line},{"character",col}}}};
}
bool ClangdClient::start(const QString& exe,const QString& root){
    if(isRunning())return true;
    m_root=root;m_proc.setWorkingDirectory(root);
    m_proc.start(exe,{"--background-index","--clang-tidy","--header-insertion=never","--completion-style=detailed"});
    if(!m_proc.waitForStarted(2500)){m_error=m_proc.errorString();return false;}
    int id=m_id++;
    m_callbacks.insert(id,[this](QJsonValue){send(QJsonObject{{"jsonrpc","2.0"},{"method","initialized"},{"params",QJsonObject{}}});});
    QJsonObject params{{"processId",(int)QCoreApplication::applicationPid()},
                       {"rootUri",QUrl::fromLocalFile(root).toString()},
                       {"capabilities",QJsonObject{{"textDocument",QJsonObject{{"publishDiagnostics",QJsonObject{}}}}}}};
    send(QJsonObject{{"jsonrpc","2.0"},{"id",id},{"method","initialize"},{"params",params}});
    return true;
}
void ClangdClient::openDocument(const QString& file,const QString& text){
    if(!isRunning())return;
    int v=1;m_docVersions[file]=v;
    QJsonObject doc{{"uri",QUrl::fromLocalFile(file).toString()},{"languageId",file.endsWith(".c")?"c":"cpp"},{"version",v},{"text",text}};
    send(QJsonObject{{"jsonrpc","2.0"},{"method","textDocument/didOpen"},{"params",QJsonObject{{"textDocument",doc}}}});
}
void ClangdClient::changeDocument(const QString& file,const QString& text){
    if(!isRunning())return;
    if(!m_docVersions.contains(file)){openDocument(file,text);return;}
    int v=++m_docVersions[file];
    QJsonObject doc{{"uri",QUrl::fromLocalFile(file).toString()},{"version",v}};
    QJsonArray changes; changes.append(QJsonObject{{"text",text}});
    send(QJsonObject{{"jsonrpc","2.0"},{"method","textDocument/didChange"},{"params",QJsonObject{{"textDocument",doc},{"contentChanges",changes}}}});
}
void ClangdClient::closeDocument(const QString& file){
    if(!isRunning()||!m_docVersions.contains(file))return;
    send(QJsonObject{{"jsonrpc","2.0"},{"method","textDocument/didClose"},{"params",QJsonObject{{"textDocument",QJsonObject{{"uri",QUrl::fromLocalFile(file).toString()}}}}}});
    m_docVersions.remove(file);
}
void ClangdClient::requestDefinition(const QString& file,int line,int col,std::function<void(QString,int,int)> cb){
    if(!isRunning()){cb({},0,0);return;}int id=m_id++;
    m_callbacks.insert(id,[cb=std::move(cb)](QJsonValue r){QJsonObject loc;if(r.isArray()&&!r.toArray().isEmpty())loc=r.toArray().first().toObject();else if(r.isObject())loc=r.toObject();QString uri=loc.value("uri").toString();if(uri.isEmpty())uri=loc.value("targetUri").toString();QJsonObject range=loc.value("range").toObject();if(range.isEmpty())range=loc.value("targetSelectionRange").toObject();auto st=range.value("start").toObject();cb(QUrl(uri).toLocalFile(),st.value("line").toInt()+1,st.value("character").toInt()+1);});
    send(QJsonObject{{"jsonrpc","2.0"},{"id",id},{"method","textDocument/definition"},{"params",textPosition(file,line,col)}});
}
void ClangdClient::requestReferences(const QString& file,int line,int col,std::function<void(QList<QPair<QString,int>>)> cb){
    if(!isRunning()){cb({});return;}int id=m_id++;
    m_callbacks.insert(id,[cb=std::move(cb)](QJsonValue r){QList<QPair<QString,int>> out;for(const auto&v:r.toArray()){auto o=v.toObject();auto st=o.value("range").toObject().value("start").toObject();out.push_back({QUrl(o.value("uri").toString()).toLocalFile(),st.value("line").toInt()+1});}cb(out);});
    QJsonObject p=textPosition(file,line,col);p.insert("context",QJsonObject{{"includeDeclaration",true}});
    send(QJsonObject{{"jsonrpc","2.0"},{"id",id},{"method","textDocument/references"},{"params",p}});
}
void ClangdClient::requestCompletion(const QString& file,int line,int col,std::function<void(QStringList)> cb){
    if(!isRunning()){cb({});return;}int id=m_id++;
    m_callbacks.insert(id,[cb=std::move(cb)](QJsonValue r){QJsonArray items=r.isArray()?r.toArray():r.toObject().value("items").toArray();QStringList out;for(const auto&v:items){QString s=v.toObject().value("label").toString();if(!s.isEmpty()&&!out.contains(s))out<<s;}out.sort(Qt::CaseInsensitive);cb(out.mid(0,200));});
    send(QJsonObject{{"jsonrpc","2.0"},{"id",id},{"method","textDocument/completion"},{"params",textPosition(file,line,col)}});
}
void ClangdClient::requestHover(const QString& file,int line,int col,std::function<void(QString)> cb){
    if(!isRunning()){cb({});return;}int id=m_id++;
    m_callbacks.insert(id,[cb=std::move(cb)](QJsonValue r){auto c=r.toObject().value("contents");QString out;if(c.isString())out=c.toString();else if(c.isObject())out=c.toObject().value("value").toString();else if(c.isArray()){for(const auto&v:c.toArray()){if(v.isString())out+=v.toString()+"\n";else out+=v.toObject().value("value").toString()+"\n";}}cb(out.trimmed());});
    send(QJsonObject{{"jsonrpc","2.0"},{"id",id},{"method","textDocument/hover"},{"params",textPosition(file,line,col)}});
}
void ClangdClient::requestRename(const QString& file,int line,int col,const QString& newName,std::function<void(QJsonObject)> cb){
    if(!isRunning()){cb({});return;}int id=m_id++;
    m_callbacks.insert(id,[cb=std::move(cb)](QJsonValue r){cb(r.toObject());});
    QJsonObject p=textPosition(file,line,col);p.insert("newName",newName);
    send(QJsonObject{{"jsonrpc","2.0"},{"id",id},{"method","textDocument/rename"},{"params",p}});
}
void ClangdClient::consume(){
    while(true){
        int sep=m_buf.indexOf("\r\n\r\n");if(sep<0)return;
        QByteArray header=m_buf.left(sep);int pos=header.indexOf("Content-Length:");
        if(pos<0){m_buf.remove(0,sep+4);continue;}
        int end=header.indexOf("\r\n",pos);QByteArray ls=header.mid(pos+15,(end<0?header.size():end)-(pos+15)).trimmed();
        bool ok=false;int len=ls.toInt(&ok);if(!ok||m_buf.size()<sep+4+len)return;
        QByteArray body=m_buf.mid(sep+4,len);m_buf.remove(0,sep+4+len);
        QJsonDocument d=QJsonDocument::fromJson(body);if(!d.isObject())continue;QJsonObject o=d.object();
        if(o.value("method").toString()=="textDocument/publishDiagnostics"){
            auto p=o.value("params").toObject();QString file=QUrl(p.value("uri").toString()).toLocalFile();QList<ClangdDiagnostic> out;
            for(const auto&v:p.value("diagnostics").toArray()){auto x=v.toObject();auto range=x.value("range").toObject();auto a=range.value("start").toObject();auto b=range.value("end").toObject();ClangdDiagnostic dg;dg.file=file;dg.line=a.value("line").toInt()+1;dg.column=a.value("character").toInt()+1;dg.endLine=b.value("line").toInt()+1;dg.endColumn=b.value("character").toInt()+1;dg.severity=x.value("severity").toInt(2);dg.message=x.value("message").toString();dg.source=x.value("source").toString("clangd");out<<dg;}if(m_diagCallback)m_diagCallback(file,out);continue;
        }
        int id=o.value("id").toInt(-1);if(id<0||!m_callbacks.contains(id))continue;
        auto cb=m_callbacks.take(id);cb(o.value("result"));
    }
}

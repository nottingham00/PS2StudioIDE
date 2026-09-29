#pragma once
#include <QObject>
#include <QProcess>
#include <QJsonObject>
#include <QHash>
#include <functional>

struct ClangdDiagnostic {
    QString file;
    int line=1,column=1,endLine=1,endColumn=1,severity=2;
    QString message;
    QString source;
};

class ClangdClient final : public QObject {
public:
    explicit ClangdClient(QObject* parent=nullptr);
    bool start(const QString& executable,const QString& projectRoot);
    bool isRunning() const{return m_proc.state()!=QProcess::NotRunning;}
    void openDocument(const QString& file,const QString& text);
    void changeDocument(const QString& file,const QString& text);
    void closeDocument(const QString& file);
    void requestDefinition(const QString& file,int lineZeroBased,int columnZeroBased,std::function<void(QString,int,int)> cb);
    void requestReferences(const QString& file,int lineZeroBased,int columnZeroBased,std::function<void(QList<QPair<QString,int>>)> cb);
    void requestCompletion(const QString& file,int lineZeroBased,int columnZeroBased,std::function<void(QStringList)> cb);
    void requestHover(const QString& file,int lineZeroBased,int columnZeroBased,std::function<void(QString)> cb);
    void requestRename(const QString& file,int lineZeroBased,int columnZeroBased,const QString& newName,std::function<void(QJsonObject)> cb);
    void setDiagnosticsCallback(std::function<void(QString,QList<ClangdDiagnostic>)> cb){m_diagCallback=std::move(cb);}
    QString lastError() const{return m_error;}
private:
    void send(const QJsonObject& o); void consume();
    QJsonObject textPosition(const QString& file,int line,int col) const;
    QProcess m_proc; QByteArray m_buf; int m_id=1; QString m_root,m_error;
    QHash<QString,int> m_docVersions;
    QHash<int,std::function<void(QJsonValue)>> m_callbacks;
    std::function<void(QString,QList<ClangdDiagnostic>)> m_diagCallback;
};

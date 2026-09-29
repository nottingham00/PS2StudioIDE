#pragma once
#include <QObject>
#include <QStringList>
class AssetDatabase : public QObject {
    Q_OBJECT
public:
    explicit AssetDatabase(QObject* parent=nullptr):QObject(parent){}
    void setProjectRoot(const QString& root){m_root=root;refresh();}
    QStringList assets() const{return m_assets;}
    QString importAsset(const QString& sourcePath, QString* error=nullptr);
    void refresh();
signals:void changed();
private:QString m_root;QStringList m_assets;
};

#pragma once
#include <QDialog>
#include <QJsonArray>
class QTableWidget;
class QPlainTextEdit;
class QLabel;

class EnvironmentDoctorDialog : public QDialog {
    Q_OBJECT
public:
    EnvironmentDoctorDialog(const QString& projectRoot,
                            const QString& pcsx2Path,
                            const QString& ps2Host,
                            QWidget* parent=nullptr);
private slots:
    void runChecks();
    void saveReport();
private:
    QString findTool(const QStringList& names) const;
    void addCheck(const QString& group,const QString& name,bool ok,const QString& detail,const QString& fix={});
    bool pingHost(const QString& host,QString* detail) const;
    bool burnerPresent(QString* detail) const;
    QString m_projectRoot,m_pcsx2Path,m_ps2Host;
    QTableWidget* m_table{};
    QPlainTextEdit* m_summary{};
    QLabel* m_overall{};
    QJsonArray m_results;
};

#pragma once
#include <QDialog>
#include <QProcess>
class QPlainTextEdit; class QPushButton; class QLabel;
class DependencyManagerDialog : public QDialog {
    Q_OBJECT
public:
    explicit DependencyManagerDialog(QWidget* parent=nullptr);
private slots:
    void installAll();
    void installHost();
    void installPs2();
    void installOptional();
    void verify();
    void processReady();
    void processFinished(int exitCode, QProcess::ExitStatus status);
private:
    QString scriptPath(const QString& name) const;
    void runBootstrap(const QString& mode);
    void runScript(const QString& script, const QStringList& args={});
    void setBusy(bool busy);
    QPlainTextEdit* m_output{};
    QLabel* m_status{};
    QPushButton *m_all{},*m_host{},*m_ps2{},*m_optional{},*m_verify{};
    QProcess* m_process{};
};

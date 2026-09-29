#pragma once
#include <QDialog>
class QLineEdit; class QComboBox; class QPlainTextEdit; class QPushButton; class QLabel; class QCheckBox; class QProcess;

class DiscDeployDialog : public QDialog {
    Q_OBJECT
public:
    explicit DiscDeployDialog(const QString& projectRoot, const QString& elfPath, QWidget* parent=nullptr);
private slots:
    void buildLayout();
    void buildIso();
    void burnDisc();
    void verifyDisc();
    void saveVerificationReport();
    void onProcessFinished(int exitCode);
private:
    QString findTool(const QStringList& names) const;
    bool copyTree(const QString& from,const QString& to,QString* error);
    QString hashFile(const QString& path) const;
    bool verifyTree(const QString& expected,const QString& actual,QStringList* problems) const;
    void log(const QString& text);
    void updateStatus();
    QString m_projectRoot,m_elfPath,m_stageDir,m_isoPath;
    QLineEdit *m_stageEdit{},*m_isoEdit{},*m_volumeEdit{},*m_driveEdit{};
    QComboBox *m_videoCombo{};
    QPlainTextEdit *m_log{};
    QLabel *m_layoutStatus{},*m_isoStatus{},*m_burnStatus{},*m_verifyStatus{};
    QCheckBox *m_realHardwareConfirmed{};
    QProcess *m_process{};
    bool m_layoutOk=false,m_isoOk=false,m_burnOk=false,m_verifyOk=false;
};

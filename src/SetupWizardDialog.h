#pragma once
#include <QDialog>
class QLineEdit; class QLabel;
class SetupWizardDialog : public QDialog {
    Q_OBJECT
public:
    explicit SetupWizardDialog(QWidget* parent=nullptr);
private slots:
    void browsePcsx2(); void browsePs2dev(); void runCheck(); void save();
private:
    QString findExe(const QStringList& names) const;
    QLineEdit *m_ps2dev{}, *m_pcsx2{}, *m_host{}; QLabel *m_result{};
};

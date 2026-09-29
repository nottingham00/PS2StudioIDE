#pragma once
#include <QDialog>

class QLineEdit;
class QComboBox;

class NewProjectDialog : public QDialog {
    Q_OBJECT
public:
    explicit NewProjectDialog(QWidget* parent = nullptr);
    QString projectName() const;
    QString projectPath() const;
    QString projectType() const;
    QString engineBackend() const;
private:
    QLineEdit* m_name;
    QLineEdit* m_path;
    QComboBox* m_type;
    QComboBox* m_engine;
};

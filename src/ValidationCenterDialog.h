#pragma once
#include <QDialog>
class QTreeWidget; class QLabel;
class ValidationCenterDialog: public QDialog{
 Q_OBJECT
public: ValidationCenterDialog(const QString& projectRoot,const QString& pcsx2,const QString& host,QWidget* parent=nullptr);
private slots: void runChecks(); void exportReport();
private: QString m_root,m_pcsx2,m_host; QTreeWidget*m_tree{}; QLabel*m_summary{};
};

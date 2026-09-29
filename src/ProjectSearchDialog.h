#pragma once
#include <QDialog>
#include <functional>
class QLineEdit; class QTreeWidget; class QCheckBox;
class ProjectSearchDialog final : public QDialog {
public:
    explicit ProjectSearchDialog(const QString& root,QWidget* parent=nullptr);
    void setOpenCallback(std::function<void(const QString&,int)> cb){m_open=std::move(cb);}    
private:
    void search();
    QString m_root; QLineEdit* m_query{}; QTreeWidget* m_results{}; QCheckBox* m_case{};
    std::function<void(const QString&,int)> m_open;
};

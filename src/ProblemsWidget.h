#pragma once
#include <QWidget>
#include <functional>

class QTreeWidget;

struct PS2StudioProblem {
    QString file;
    int line = 1;
    int column = 1;
    int severity = 2; // 1 error, 2 warning, 3 info/hint
    QString message;
    QString source;
};

class ProblemsWidget final : public QWidget {
public:
    explicit ProblemsWidget(QWidget* parent=nullptr);
    void setProblems(const QList<PS2StudioProblem>& problems);
    void clear();
    void setOpenCallback(std::function<void(const QString&,int,int)> cb) { m_open = std::move(cb); }
    int errorCount() const;
    int warningCount() const;
private:
    QTreeWidget* m_tree{};
    QList<PS2StudioProblem> m_problems;
    std::function<void(const QString&,int,int)> m_open;
};

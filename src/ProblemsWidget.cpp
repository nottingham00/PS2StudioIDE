#include "ProblemsWidget.h"
#include <QtWidgets>

ProblemsWidget::ProblemsWidget(QWidget* parent):QWidget(parent){
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    m_tree = new QTreeWidget;
    m_tree->setHeaderLabels({"Severity","File","Line","Message","Source"});
    m_tree->setRootIsDecorated(false);
    m_tree->setAlternatingRowColors(true);
    layout->addWidget(m_tree);
    connect(m_tree,&QTreeWidget::itemDoubleClicked,this,[this](QTreeWidgetItem* item,int){
        if(!item || !m_open) return;
        int index = item->data(0,Qt::UserRole).toInt();
        if(index<0 || index>=m_problems.size()) return;
        const auto& p = m_problems[index];
        m_open(p.file,p.line,p.column);
    });
}
void ProblemsWidget::clear(){m_problems.clear();m_tree->clear();}
void ProblemsWidget::setProblems(const QList<PS2StudioProblem>& problems){
    m_problems=problems; m_tree->clear();
    for(int i=0;i<m_problems.size();++i){
        const auto& p=m_problems[i];
        QString sev=p.severity<=1?"Error":(p.severity==2?"Warning":"Info");
        auto* it=new QTreeWidgetItem({sev,QFileInfo(p.file).fileName(),QString::number(p.line),p.message,p.source});
        it->setData(0,Qt::UserRole,i);
        it->setToolTip(1,p.file);
        if(p.severity<=1) it->setForeground(0,QBrush(QColor("#ff5c70")));
        else if(p.severity==2) it->setForeground(0,QBrush(QColor("#ffc857")));
        else it->setForeground(0,QBrush(QColor("#78a9ff")));
        m_tree->addTopLevelItem(it);
    }
    for(int c=0;c<5;++c)m_tree->resizeColumnToContents(c);
}
int ProblemsWidget::errorCount() const {int n=0;for(const auto&p:m_problems)if(p.severity<=1)++n;return n;}
int ProblemsWidget::warningCount() const {int n=0;for(const auto&p:m_problems)if(p.severity==2)++n;return n;}

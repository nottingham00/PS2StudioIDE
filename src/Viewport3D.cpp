#include "Viewport3D.h"
#include "SceneDocument.h"
#include <QPainter>
#include <QtMath>
Viewport3D::Viewport3D(QWidget* p):QWidget(p){setMinimumSize(640,360);setFocusPolicy(Qt::StrongFocus);}
void Viewport3D::setDocument(SceneDocument* d){ if(m_doc) disconnect(m_doc,nullptr,this,nullptr);m_doc=d;if(m_doc)connect(m_doc,&SceneDocument::changed,this,qOverload<>(&Viewport3D::update));update();}
void Viewport3D::setSelectedId(const QString& id){m_selected=id;update();}
void Viewport3D::paintEvent(QPaintEvent*){QPainter p(this);p.fillRect(rect(),QColor("#0f1419"));p.setRenderHint(QPainter::Antialiasing); const QPointF c(width()/2.0,height()*0.62);
    p.setPen(QColor(42,51,60)); for(int i=-12;i<=12;i++){ double dx=i*38.0; p.drawLine(QPointF(c.x()+dx,c.y()),QPointF(c.x()+dx*0.18,height()*0.18)); p.drawLine(QPointF(0,c.y()+i*13),QPointF(width(),c.y()+i*13)); }
    p.setPen(QPen(QColor(85,95,108),2));p.drawLine(QPointF(0,c.y()),QPointF(width(),c.y()));p.drawLine(c,QPointF(c.x(),height()*0.15));
    if(m_doc) for(const auto& o:m_doc->objects()){ QPointF pos=c+QPointF(o.position.x()*2.2-o.position.z()*0.8,-o.position.y()*2.2-o.position.z()*0.45); QRectF r(pos-QPointF(22,22),QSizeF(44,44)); p.setBrush(QColor(45,84,126,150)); p.setPen(QPen(o.id==m_selected?QColor("#58a6ff"):QColor("#91a0b0"),o.id==m_selected?3:1)); p.drawRect(r); p.setPen(QColor("#d7dee8"));p.drawText(r.adjusted(-35,48,35,70),Qt::AlignHCenter,o.name); }
    p.setPen(QColor("#8290a0"));p.drawText(14,24,"PS2 Studio 3D Preview  •  editor visualization (runtime renderer uses PS2SDK/gsKit)"); }

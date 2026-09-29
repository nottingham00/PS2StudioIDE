#include "SceneView2D.h"
#include "SceneDocument.h"
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QMouseEvent>
#include <QFileInfo>
#include <QPixmap>
#include <functional>

class ObjectItem : public QGraphicsRectItem {
public:
    ObjectItem(QString id, QRectF r) : QGraphicsRectItem(r), objectId(std::move(id)) { setFlags(ItemIsSelectable|ItemIsMovable|ItemSendsGeometryChanges); }
    QString objectId;
    std::function<void(const QPointF&)> onMoved;
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override {
        if (change == ItemPositionHasChanged && onMoved) onMoved(value.toPointF());
        return QGraphicsRectItem::itemChange(change, value);
    }
};

SceneView2D::SceneView2D(QWidget* parent):QGraphicsView(parent){ setScene(new QGraphicsScene(this)); setRenderHint(QPainter::Antialiasing); setDragMode(RubberBandDrag); setSceneRect(-640,-360,1280,720); setBackgroundBrush(QColor("#11161b"));
    connect(scene(), &QGraphicsScene::selectionChanged, this, [this]{ const auto items=scene()->selectedItems(); if(items.isEmpty()) return; if(auto* o=dynamic_cast<ObjectItem*>(items.first())) { m_selected=o->objectId; emit objectSelected(o->objectId); } });
}
void SceneView2D::setDocument(SceneDocument* doc){ if(m_doc) disconnect(m_doc,nullptr,this,nullptr); m_doc=doc; if(m_doc) connect(m_doc,&SceneDocument::changed,this,&SceneView2D::rebuild); rebuild(); }
void SceneView2D::setSelectedId(const QString& id){ m_selected=id; for(auto* it:scene()->items()) if(auto* o=dynamic_cast<ObjectItem*>(it)) o->setSelected(o->objectId==id); }
void SceneView2D::rebuild(){ scene()->clear();
    for(int x=-640;x<=640;x+=64) scene()->addLine(x,-360,x,360,QPen(QColor(38,45,53)));
    for(int y=-360;y<=360;y+=64) scene()->addLine(-640,y,640,y,QPen(QColor(38,45,53)));
    scene()->addLine(-640,0,640,0,QPen(QColor(70,80,92))); scene()->addLine(0,-360,0,360,QPen(QColor(70,80,92)));
    if(!m_doc) return; for(const auto& obj:m_doc->objects()){ QSizeF size=obj.type=="Camera"?QSizeF(320,180):obj.type=="Light"?QSizeF(30,30):QSizeF(96,96); auto* item=new ObjectItem(obj.id,QRectF(-size.width()/2,-size.height()/2,size.width(),size.height())); item->setPos(obj.position.x(),obj.position.y()); item->setScale(qMax(0.05f,obj.scale.x())); item->setPen(QPen(obj.id==m_selected?QColor("#58a6ff"):QColor("#8290a0"),2)); if(obj.type=="Sprite"&&QFileInfo::exists(obj.asset)){QPixmap px(obj.asset);if(!px.isNull())item->setBrush(QBrush(px.scaled(size.toSize(),Qt::KeepAspectRatio,Qt::SmoothTransformation)));else item->setBrush(QColor(40,70,105,110));}else item->setBrush(QColor(40,70,105,110)); scene()->addItem(item); if(obj.colliderEnabled){auto* col=scene()->addRect(-obj.colliderSize.x()/2,-obj.colliderSize.y()/2,obj.colliderSize.x(),obj.colliderSize.y(),QPen(QColor("#ff7b72"),1,Qt::DashLine));col->setPos(item->pos());col->setZValue(-0.1);} item->onMoved=[this,id=obj.id](const QPointF& p){ if(!m_doc) return; if(auto* o=m_doc->objectById(id)){ o->position.setX(float(p.x())); o->position.setY(float(p.y())); emit objectMoved(id,p.x(),p.y()); } }; auto* label=scene()->addText(obj.name); label->setDefaultTextColor(QColor("#d7dee8")); label->setPos(item->pos()+QPointF(-label->boundingRect().width()/2,size.height()/2+6)); }
}

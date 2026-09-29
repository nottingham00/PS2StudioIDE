#pragma once
#include <QGraphicsView>
class SceneDocument;
class SceneView2D : public QGraphicsView {
    Q_OBJECT
public:
    explicit SceneView2D(QWidget* parent=nullptr);
    void setDocument(SceneDocument* doc);
    void setSelectedId(const QString& id);
signals:
    void objectSelected(const QString& id);
    void objectMoved(const QString& id, double x, double y);
private:
    void rebuild();
    SceneDocument* m_doc{};
    QString m_selected;
};

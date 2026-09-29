#pragma once
#include <QWidget>
class SceneDocument;
class Viewport3D : public QWidget {
    Q_OBJECT
public:
    explicit Viewport3D(QWidget* parent=nullptr);
    void setDocument(SceneDocument* doc);
    void setSelectedId(const QString& id);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    SceneDocument* m_doc{};
    QString m_selected;
};

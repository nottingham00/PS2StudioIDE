#pragma once
#include <QWidget>
class QLabel; class SceneDocument;
class ProfilerWidget: public QWidget{
    Q_OBJECT
public:
    explicit ProfilerWidget(QWidget* parent=nullptr);
    void setProjectRoot(const QString& root);
    void setDocument(SceneDocument* doc);
public slots:
    void refresh();
    void setFrameTime(float ms);
    void ingestRuntimeOutput(const QString& text);
private:
    QString m_root; SceneDocument* m_doc{}; QLabel* m_text{}; float m_frameMs=0.0f;
    unsigned m_runtimeFrame=0,m_runtimeEntities=0,m_runtimeDrawCalls=0,m_runtimeTriangles=0,m_runtimeCollisions=0,m_runtimeAudioVoices=0,m_runtimePhysicsPairs=0,m_runtimeSkinVertices=0,m_runtimeVifPackets=0,m_runtimeVuDispatches=0; bool m_hasRuntimeStats=false;
};

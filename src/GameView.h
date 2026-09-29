#pragma once
#include <QWidget>
#include <QTimer>
#include <QHash>
#include <QVector3D>
#include <QSet>
class SceneDocument;

class GameView : public QWidget {
    Q_OBJECT
public:
    explicit GameView(QWidget* parent=nullptr);
    void setDocument(SceneDocument* doc);
    void setProjectRoot(const QString& root);
    bool isPlaying() const { return m_playing; }
    bool isPaused() const { return m_paused; }
    QSize virtualResolution() const { return m_virtualResolution; }
public slots:
    void play(); void pause(); void stop(); void frameStep(); void setVideoPreset(const QString& preset);
signals:
    void playStateChanged(bool playing); void pauseStateChanged(bool paused); void frameAdvanced(float frameMs);
protected:
    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;
private slots:
    void tick();
private:
    void advance(float dt);
    SceneDocument* m_doc{}; QString m_root; bool m_playing=false,m_paused=false; QTimer m_timer;
    QHash<QString,QVector3D> m_positions,m_velocities; QSet<int> m_keys; float m_time=0.0f; QSize m_virtualResolution{640,448}; QString m_videoPreset="NTSC 640x448"; int m_uiSelection=0;
};

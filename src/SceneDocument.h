#pragma once
#include <QObject>
#include <QVector3D>
#include <QList>

struct SceneObject {
    QString id;
    QString name;
    QString type = "Empty";
    QString tag;
    QVector3D position{0,0,0};
    QVector3D rotation{0,0,0};
    QVector3D scale{1,1,1};
    QVector3D velocity{0,0,0};

    // Rendering / assets.
    QString asset;
    QString materialTexture;
    QVector3D materialTint{1,1,1};
    float materialAlpha = 1.0f;
    bool materialUnlit = false;
    bool materialTransparent = false;
    float lightIntensity = 1.0f;

    // Animation / gameplay component.
    QString animationAsset;
    float animationSpeed = 1.0f;
    bool animationLoop = true;
    bool animationAutoplay = true;
    QString script;
    QString prefabSource;

    // UI.
    QString uiText;
    QString uiAction;
    QVector3D uiColor{0.18f,0.24f,0.34f};
    float uiWidth = 160.0f;
    float uiHeight = 40.0f;
    bool uiSelectable = false;
    QString nextScene;

    // Physics.
    bool colliderEnabled = false;
    QVector3D colliderSize{32,32,1};
    bool trigger = false;
    bool rigidBody = false;
    bool kinematic = false;
    bool useGravity = false;
    float gravityScale = 1.0f;
    float mass = 1.0f;
    float restitution = 0.0f;
    float friction = 0.2f;
    int collisionLayer = 1;
    int collisionMask = 0xffff;

    // Particles.
    bool particleEmitter = false;
    int particleCount = 32;
    float particleLife = 1.0f;

    // Audio source.
    QString audioAsset;
    bool audioAutoplay = false;
    bool audioLoop = false;
    float audioVolume = 1.0f;
    float audioPan = 0.0f;
};

class SceneDocument : public QObject {
    Q_OBJECT
public:
    explicit SceneDocument(QObject* parent = nullptr);
    const QList<SceneObject>& objects() const { return m_objects; }
    SceneObject* objectById(const QString& id);
    const SceneObject* objectById(const QString& id) const;
    SceneObject& addObject(const QString& type, const QString& name = {});
    SceneObject& duplicateObject(const QString& id);
    bool removeObject(const QString& id);
    void clear();
    bool load(const QString& filePath, QString* error = nullptr);
    bool save(const QString& filePath, QString* error = nullptr) const;
    bool savePrefab(const QString& id,const QString& filePath,QString* error=nullptr) const;
    bool instantiatePrefab(const QString& filePath,QString* error=nullptr);
    QString filePath() const { return m_filePath; }
    void setFilePath(const QString& path) { m_filePath = path; }
    void notifyChanged();
signals:
    void changed();
private:
    QList<SceneObject> m_objects;
    QString m_filePath;
};

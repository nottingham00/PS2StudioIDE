#pragma once
#include <QWidget>
class QLineEdit; class QComboBox; class QDoubleSpinBox; class QSpinBox; class QCheckBox; class SceneDocument;
class InspectorWidget : public QWidget {
    Q_OBJECT
public:
    explicit InspectorWidget(QWidget* parent=nullptr);
    void setDocument(SceneDocument* doc);
    void setObjectId(const QString& id);
signals: void objectEdited();
private:
    void load(); void apply();
    SceneDocument* m_doc{}; QString m_id; bool m_loading=false;
    QLineEdit *m_name{},*m_tag{},*m_asset{},*m_material{},*m_animation{},*m_script{},*m_text{},*m_action{},*m_nextScene{},*m_audio{};
    QComboBox* m_type{};
    QDoubleSpinBox *px{},*py{},*pz{},*rx{},*ry{},*rz{},*sx{},*sy{},*sz{},*vx{},*vy{},*vz{},*cx{},*cy{},*cz{},
                   *m_mass{},*m_particleLife{},*m_gravityScale{},*m_restitution{},*m_friction{},*m_animSpeed{},*m_alpha{},*m_lightIntensity{},
                   *m_tintR{},*m_tintG{},*m_tintB{},*m_uiR{},*m_uiG{},*m_uiB{},*m_uiW{},*m_uiH{},*m_audioVolume{},*m_audioPan{};
    QCheckBox *m_collider{},*m_trigger{},*m_rigid{},*m_kinematic{},*m_gravity{},*m_particles{},*m_unlit{},*m_transparent{},
              *m_animLoop{},*m_animAutoplay{},*m_uiSelectable{},*m_audioAutoplay{},*m_audioLoop{};
    QSpinBox *m_particleCount{},*m_layer{},*m_mask{};
};

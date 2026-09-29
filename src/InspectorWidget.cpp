#include "InspectorWidget.h"
#include "SceneDocument.h"
#include <QtWidgets>

static QDoubleSpinBox* spin(double mn,double mx,double v,QWidget*p,int decimals=3){auto*s=new QDoubleSpinBox(p);s->setRange(mn,mx);s->setDecimals(decimals);s->setValue(v);return s;}

InspectorWidget::InspectorWidget(QWidget*p):QWidget(p){
    auto*outer=new QVBoxLayout(this);auto*scroll=new QScrollArea;scroll->setWidgetResizable(true);auto*body=new QWidget;auto*f=new QFormLayout(body);scroll->setWidget(body);outer->addWidget(scroll);
    m_name=new QLineEdit;m_tag=new QLineEdit;m_type=new QComboBox;m_type->addItems({"Empty","Sprite","Mesh","Camera","Light","Text","Panel","Button","Image","AudioSource"});
    auto row=[&](QDoubleSpinBox*&a,QDoubleSpinBox*&b,QDoubleSpinBox*&c,double mn,double mx,double v){auto*w=new QWidget;auto*l=new QHBoxLayout(w);l->setContentsMargins(0,0,0,0);a=spin(mn,mx,v,this);b=spin(mn,mx,v,this);c=spin(mn,mx,v,this);l->addWidget(a);l->addWidget(b);l->addWidget(c);return w;};
    auto colorRow=[&](QDoubleSpinBox*&a,QDoubleSpinBox*&b,QDoubleSpinBox*&c){return row(a,b,c,0,1,1);};
    m_asset=new QLineEdit;m_material=new QLineEdit;m_animation=new QLineEdit;m_script=new QLineEdit;m_text=new QLineEdit;m_action=new QLineEdit;m_nextScene=new QLineEdit;m_audio=new QLineEdit;
    f->addRow("Name",m_name);f->addRow("Tag",m_tag);f->addRow("Type",m_type);
    f->addRow("Position",row(px,py,pz,-100000,100000,0));f->addRow("Rotation",row(rx,ry,rz,-3600,3600,0));f->addRow("Scale",row(sx,sy,sz,0.001,1000,1));f->addRow("Velocity",row(vx,vy,vz,-100000,100000,0));

    f->addRow(new QLabel("<b>Rendering / Material</b>"));
    f->addRow("Asset",m_asset);f->addRow("Material Texture",m_material);f->addRow("Tint RGB",colorRow(m_tintR,m_tintG,m_tintB));m_alpha=spin(0,1,1,this);m_unlit=new QCheckBox("Unlit");m_transparent=new QCheckBox("Alpha blending");m_lightIntensity=spin(0,16,1,this);f->addRow("Alpha",m_alpha);f->addRow("Unlit",m_unlit);f->addRow("Transparent",m_transparent);f->addRow("Light Intensity",m_lightIntensity);

    f->addRow(new QLabel("<b>Animation / Component</b>"));
    f->addRow("Animation",m_animation);m_animSpeed=spin(-16,16,1,this);m_animLoop=new QCheckBox("Loop");m_animAutoplay=new QCheckBox("Autoplay");f->addRow("Anim Speed",m_animSpeed);f->addRow("Anim Loop",m_animLoop);f->addRow("Anim Autoplay",m_animAutoplay);f->addRow("Script/Component",m_script);

    f->addRow(new QLabel("<b>UI</b>"));
    f->addRow("UI Text",m_text);f->addRow("UI Action",m_action);f->addRow("UI Color RGB",colorRow(m_uiR,m_uiG,m_uiB));m_uiW=spin(1,4096,160,this);m_uiH=spin(1,4096,40,this);m_uiSelectable=new QCheckBox("Controller selectable");f->addRow("UI Width",m_uiW);f->addRow("UI Height",m_uiH);f->addRow("Selectable",m_uiSelectable);f->addRow("Next Scene",m_nextScene);

    f->addRow(new QLabel("<b>Physics</b>"));
    m_collider=new QCheckBox("Enabled");m_trigger=new QCheckBox("Trigger");m_rigid=new QCheckBox("Rigid Body");m_kinematic=new QCheckBox("Kinematic");m_gravity=new QCheckBox("Use Gravity");m_mass=spin(0.001,100000,1,this);m_gravityScale=spin(-32,32,1,this);m_restitution=spin(0,1,0,this);m_friction=spin(0,1,0.2,this);m_layer=new QSpinBox;m_layer->setRange(1,0x7fff);m_layer->setValue(1);m_mask=new QSpinBox;m_mask->setRange(0,0x7fff);m_mask->setValue(0x7fff);
    f->addRow("Collider",m_collider);f->addRow("Collider Size",row(cx,cy,cz,0.001,100000,32));f->addRow("Trigger",m_trigger);f->addRow("Rigid Body",m_rigid);f->addRow("Kinematic",m_kinematic);f->addRow("Gravity",m_gravity);f->addRow("Gravity Scale",m_gravityScale);f->addRow("Mass",m_mass);f->addRow("Restitution",m_restitution);f->addRow("Friction",m_friction);f->addRow("Layer",m_layer);f->addRow("Mask",m_mask);

    f->addRow(new QLabel("<b>Particles</b>"));
    m_particles=new QCheckBox("Emitter");m_particleCount=new QSpinBox;m_particleCount->setRange(1,4096);m_particleCount->setValue(32);m_particleLife=spin(0.01,60,1,this);f->addRow("Particles",m_particles);f->addRow("Particle Count",m_particleCount);f->addRow("Particle Life",m_particleLife);

    f->addRow(new QLabel("<b>Audio Source</b>"));
    m_audioAutoplay=new QCheckBox("Autoplay");m_audioLoop=new QCheckBox("Loop");m_audioVolume=spin(0,1,1,this);m_audioPan=spin(-1,1,0,this);f->addRow("Audio Asset",m_audio);f->addRow("Autoplay",m_audioAutoplay);f->addRow("Loop",m_audioLoop);f->addRow("Volume",m_audioVolume);f->addRow("Pan",m_audioPan);

    auto*applyBtn=new QPushButton("Apply");f->addRow(applyBtn);connect(applyBtn,&QPushButton::clicked,this,&InspectorWidget::apply);connect(m_name,&QLineEdit::editingFinished,this,&InspectorWidget::apply);connect(m_type,&QComboBox::currentTextChanged,this,[this]{apply();});
}
void InspectorWidget::setDocument(SceneDocument*d){m_doc=d;load();}
void InspectorWidget::setObjectId(const QString&id){m_id=id;load();}
void InspectorWidget::load(){
    m_loading=true;auto*o=m_doc?m_doc->objectById(m_id):nullptr;setEnabled(o);
    if(o){
        m_name->setText(o->name);m_tag->setText(o->tag);m_type->setCurrentText(o->type);
        px->setValue(o->position.x());py->setValue(o->position.y());pz->setValue(o->position.z());rx->setValue(o->rotation.x());ry->setValue(o->rotation.y());rz->setValue(o->rotation.z());sx->setValue(o->scale.x());sy->setValue(o->scale.y());sz->setValue(o->scale.z());vx->setValue(o->velocity.x());vy->setValue(o->velocity.y());vz->setValue(o->velocity.z());
        m_asset->setText(o->asset);m_material->setText(o->materialTexture);m_tintR->setValue(o->materialTint.x());m_tintG->setValue(o->materialTint.y());m_tintB->setValue(o->materialTint.z());m_alpha->setValue(o->materialAlpha);m_unlit->setChecked(o->materialUnlit);m_transparent->setChecked(o->materialTransparent);m_lightIntensity->setValue(o->lightIntensity);
        m_animation->setText(o->animationAsset);m_animSpeed->setValue(o->animationSpeed);m_animLoop->setChecked(o->animationLoop);m_animAutoplay->setChecked(o->animationAutoplay);m_script->setText(o->script);
        m_text->setText(o->uiText);m_action->setText(o->uiAction);m_uiR->setValue(o->uiColor.x());m_uiG->setValue(o->uiColor.y());m_uiB->setValue(o->uiColor.z());m_uiW->setValue(o->uiWidth);m_uiH->setValue(o->uiHeight);m_uiSelectable->setChecked(o->uiSelectable);m_nextScene->setText(o->nextScene);
        m_collider->setChecked(o->colliderEnabled);cx->setValue(o->colliderSize.x());cy->setValue(o->colliderSize.y());cz->setValue(o->colliderSize.z());m_trigger->setChecked(o->trigger);m_rigid->setChecked(o->rigidBody);m_kinematic->setChecked(o->kinematic);m_gravity->setChecked(o->useGravity);m_gravityScale->setValue(o->gravityScale);m_mass->setValue(o->mass);m_restitution->setValue(o->restitution);m_friction->setValue(o->friction);m_layer->setValue(o->collisionLayer);m_mask->setValue(o->collisionMask);
        m_particles->setChecked(o->particleEmitter);m_particleCount->setValue(o->particleCount);m_particleLife->setValue(o->particleLife);
        m_audio->setText(o->audioAsset);m_audioAutoplay->setChecked(o->audioAutoplay);m_audioLoop->setChecked(o->audioLoop);m_audioVolume->setValue(o->audioVolume);m_audioPan->setValue(o->audioPan);
    }
    m_loading=false;
}
void InspectorWidget::apply(){
    if(m_loading||!m_doc)return;auto*o=m_doc->objectById(m_id);if(!o)return;
    o->name=m_name->text();o->tag=m_tag->text();o->type=m_type->currentText();o->position={float(px->value()),float(py->value()),float(pz->value())};o->rotation={float(rx->value()),float(ry->value()),float(rz->value())};o->scale={float(sx->value()),float(sy->value()),float(sz->value())};o->velocity={float(vx->value()),float(vy->value()),float(vz->value())};
    o->asset=m_asset->text();o->materialTexture=m_material->text();o->materialTint={float(m_tintR->value()),float(m_tintG->value()),float(m_tintB->value())};o->materialAlpha=float(m_alpha->value());o->materialUnlit=m_unlit->isChecked();o->materialTransparent=m_transparent->isChecked();o->lightIntensity=float(m_lightIntensity->value());
    o->animationAsset=m_animation->text();o->animationSpeed=float(m_animSpeed->value());o->animationLoop=m_animLoop->isChecked();o->animationAutoplay=m_animAutoplay->isChecked();o->script=m_script->text();
    o->uiText=m_text->text();o->uiAction=m_action->text();o->uiColor={float(m_uiR->value()),float(m_uiG->value()),float(m_uiB->value())};o->uiWidth=float(m_uiW->value());o->uiHeight=float(m_uiH->value());o->uiSelectable=m_uiSelectable->isChecked();o->nextScene=m_nextScene->text();
    o->colliderEnabled=m_collider->isChecked();o->colliderSize={float(cx->value()),float(cy->value()),float(cz->value())};o->trigger=m_trigger->isChecked();o->rigidBody=m_rigid->isChecked();o->kinematic=m_kinematic->isChecked();o->useGravity=m_gravity->isChecked();o->gravityScale=float(m_gravityScale->value());o->mass=float(m_mass->value());o->restitution=float(m_restitution->value());o->friction=float(m_friction->value());o->collisionLayer=m_layer->value();o->collisionMask=m_mask->value();
    o->particleEmitter=m_particles->isChecked();o->particleCount=m_particleCount->value();o->particleLife=float(m_particleLife->value());
    o->audioAsset=m_audio->text();o->audioAutoplay=m_audioAutoplay->isChecked();o->audioLoop=m_audioLoop->isChecked();o->audioVolume=float(m_audioVolume->value());o->audioPan=float(m_audioPan->value());
    m_doc->notifyChanged();emit objectEdited();
}

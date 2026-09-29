#include "SceneDocument.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QFileInfo>

static QJsonArray vec(const QVector3D& v){return {v.x(),v.y(),v.z()};}
static QVector3D readVec(const QJsonValue& value,const QVector3D& fallback){auto a=value.toArray();if(a.size()!=3)return fallback;return {float(a[0].toDouble()),float(a[1].toDouble()),float(a[2].toDouble())};}

static QJsonObject toJson(const SceneObject&o){
    return QJsonObject{
        {"id",o.id},{"name",o.name},{"type",o.type},{"tag",o.tag},
        {"position",vec(o.position)},{"rotation",vec(o.rotation)},{"scale",vec(o.scale)},{"velocity",vec(o.velocity)},
        {"asset",o.asset},{"materialTexture",o.materialTexture},{"materialTint",vec(o.materialTint)},
        {"materialAlpha",o.materialAlpha},{"materialUnlit",o.materialUnlit},{"materialTransparent",o.materialTransparent},{"lightIntensity",o.lightIntensity},
        {"animationAsset",o.animationAsset},{"animationSpeed",o.animationSpeed},{"animationLoop",o.animationLoop},{"animationAutoplay",o.animationAutoplay},
        {"script",o.script},{"prefabSource",o.prefabSource},
        {"uiText",o.uiText},{"uiAction",o.uiAction},{"uiColor",vec(o.uiColor)},{"uiWidth",o.uiWidth},{"uiHeight",o.uiHeight},{"uiSelectable",o.uiSelectable},{"nextScene",o.nextScene},
        {"colliderEnabled",o.colliderEnabled},{"colliderSize",vec(o.colliderSize)},{"trigger",o.trigger},{"rigidBody",o.rigidBody},{"kinematic",o.kinematic},
        {"useGravity",o.useGravity},{"gravityScale",o.gravityScale},{"mass",o.mass},{"restitution",o.restitution},{"friction",o.friction},
        {"collisionLayer",o.collisionLayer},{"collisionMask",o.collisionMask},
        {"particleEmitter",o.particleEmitter},{"particleCount",o.particleCount},{"particleLife",o.particleLife},
        {"audioAsset",o.audioAsset},{"audioAutoplay",o.audioAutoplay},{"audioLoop",o.audioLoop},{"audioVolume",o.audioVolume},{"audioPan",o.audioPan}
    };
}

static SceneObject fromJson(const QJsonObject&j){
    SceneObject o;
    o.id=j.value("id").toString(QUuid::createUuid().toString(QUuid::WithoutBraces));o.name=j.value("name").toString("Object");o.type=j.value("type").toString("Empty");o.tag=j.value("tag").toString();
    o.position=readVec(j.value("position"),{});o.rotation=readVec(j.value("rotation"),{});o.scale=readVec(j.value("scale"),{1,1,1});o.velocity=readVec(j.value("velocity"),{});
    o.asset=j.value("asset").toString();o.materialTexture=j.value("materialTexture").toString();o.materialTint=readVec(j.value("materialTint"),{1,1,1});o.materialAlpha=float(j.value("materialAlpha").toDouble(1.0));o.materialUnlit=j.value("materialUnlit").toBool(false);o.materialTransparent=j.value("materialTransparent").toBool(false);o.lightIntensity=float(j.value("lightIntensity").toDouble(1.0));
    o.animationAsset=j.value("animationAsset").toString();o.animationSpeed=float(j.value("animationSpeed").toDouble(1.0));o.animationLoop=j.value("animationLoop").toBool(true);o.animationAutoplay=j.value("animationAutoplay").toBool(true);o.script=j.value("script").toString();o.prefabSource=j.value("prefabSource").toString();
    o.uiText=j.value("uiText").toString();o.uiAction=j.value("uiAction").toString();o.uiColor=readVec(j.value("uiColor"),{0.18f,0.24f,0.34f});o.uiWidth=float(j.value("uiWidth").toDouble(160.0));o.uiHeight=float(j.value("uiHeight").toDouble(40.0));o.uiSelectable=j.value("uiSelectable").toBool(false);o.nextScene=j.value("nextScene").toString();
    o.colliderEnabled=j.value("colliderEnabled").toBool(false);o.colliderSize=readVec(j.value("colliderSize"),{32,32,1});o.trigger=j.value("trigger").toBool(false);o.rigidBody=j.value("rigidBody").toBool(false);o.kinematic=j.value("kinematic").toBool(false);o.useGravity=j.value("useGravity").toBool(false);o.gravityScale=float(j.value("gravityScale").toDouble(1.0));o.mass=float(j.value("mass").toDouble(1.0));o.restitution=float(j.value("restitution").toDouble(0.0));o.friction=float(j.value("friction").toDouble(0.2));o.collisionLayer=j.value("collisionLayer").toInt(1);o.collisionMask=j.value("collisionMask").toInt(0xffff);
    o.particleEmitter=j.value("particleEmitter").toBool(false);o.particleCount=j.value("particleCount").toInt(32);o.particleLife=float(j.value("particleLife").toDouble(1.0));
    o.audioAsset=j.value("audioAsset").toString();o.audioAutoplay=j.value("audioAutoplay").toBool(false);o.audioLoop=j.value("audioLoop").toBool(false);o.audioVolume=float(j.value("audioVolume").toDouble(1.0));o.audioPan=float(j.value("audioPan").toDouble(0.0));
    return o;
}

SceneDocument::SceneDocument(QObject*p):QObject(p){}
SceneObject* SceneDocument::objectById(const QString&id){for(auto&o:m_objects)if(o.id==id)return&o;return nullptr;}
const SceneObject*SceneDocument::objectById(const QString&id)const{for(const auto&o:m_objects)if(o.id==id)return&o;return nullptr;}
SceneObject&SceneDocument::addObject(const QString&type,const QString&name){SceneObject o;o.id=QUuid::createUuid().toString(QUuid::WithoutBraces);o.type=type;o.name=name.isEmpty()?type:name;if(type=="Button"){o.uiSelectable=true;o.uiText="BUTTON";}if(type=="Panel")o.uiWidth=240;if(type=="AudioSource")o.audioAutoplay=true;m_objects.append(o);emit changed();return m_objects.last();}
SceneObject&SceneDocument::duplicateObject(const QString&id){auto*src=objectById(id);if(!src)return addObject("Empty","Object");SceneObject o=*src;o.id=QUuid::createUuid().toString(QUuid::WithoutBraces);o.name+=" Copy";o.position+=QVector3D(16,16,0);m_objects.append(o);emit changed();return m_objects.last();}
bool SceneDocument::removeObject(const QString&id){for(qsizetype i=0;i<m_objects.size();++i)if(m_objects[i].id==id){m_objects.removeAt(i);emit changed();return true;}return false;}
void SceneDocument::clear(){m_objects.clear();emit changed();}
void SceneDocument::notifyChanged(){emit changed();}
bool SceneDocument::load(const QString&filePath,QString*error){QFile f(filePath);if(!f.open(QIODevice::ReadOnly)){if(error)*error=f.errorString();return false;}QJsonParseError pe;auto doc=QJsonDocument::fromJson(f.readAll(),&pe);if(pe.error!=QJsonParseError::NoError){if(error)*error=pe.errorString();return false;}QList<SceneObject> loaded;for(const auto&v:doc.object().value("objects").toArray())loaded.append(fromJson(v.toObject()));m_objects=loaded;m_filePath=filePath;emit changed();return true;}
bool SceneDocument::save(const QString&filePath,QString*error)const{QJsonArray a;for(const auto&o:m_objects)a.append(toJson(o));QJsonObject root{{"format","PS2StudioScene"},{"version",3},{"objects",a}};QFile f(filePath);if(!f.open(QIODevice::WriteOnly|QIODevice::Truncate)){if(error)*error=f.errorString();return false;}f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));return true;}
bool SceneDocument::savePrefab(const QString&id,const QString&filePath,QString*error)const{auto*o=objectById(id);if(!o){if(error)*error="No object selected";return false;}QJsonObject root{{"format","PS2StudioPrefab"},{"version",2},{"object",toJson(*o)}};QFile f(filePath);if(!f.open(QIODevice::WriteOnly|QIODevice::Truncate)){if(error)*error=f.errorString();return false;}f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));return true;}
bool SceneDocument::instantiatePrefab(const QString&filePath,QString*error){QFile f(filePath);if(!f.open(QIODevice::ReadOnly)){if(error)*error=f.errorString();return false;}QJsonParseError pe;auto d=QJsonDocument::fromJson(f.readAll(),&pe);if(pe.error!=QJsonParseError::NoError){if(error)*error=pe.errorString();return false;}SceneObject o=fromJson(d.object().value("object").toObject());o.id=QUuid::createUuid().toString(QUuid::WithoutBraces);o.prefabSource=QFileInfo(filePath).fileName();m_objects.append(o);emit changed();return true;}

#include "AssetDatabase.h"
#include <QDirIterator>
#include <QFileInfo>
#include <QDir>
#include <QFile>

void AssetDatabase::refresh(){
    m_assets.clear();
    if(m_root.isEmpty())return;
    QDirIterator it(m_root+"/assets",QDir::Files,QDirIterator::Subdirectories);
    while(it.hasNext()){auto p=it.next();m_assets<<QDir(m_root).relativeFilePath(p);}
    m_assets.sort();
    emit changed();
}

QString AssetDatabase::importAsset(const QString& src, QString* error){
    if(m_root.isEmpty()){if(error)*error="No project is open";return{};}
    QFileInfo fi(src);
    const QString ext=fi.suffix().toLower();
    QString dir;
    // These are the formats the current PS2 asset compiler can actually turn into runtime data.
    if(QStringList{"png","bmp","jpg","jpeg","tga"}.contains(ext))dir="textures";
    else if(ext=="obj"||ext=="ps2skin")dir="models";
    else if(ext=="wav")dir="audio";
    else if(ext=="ps2anim")dir="animations";
    else{
        if(error)*error=QString("Unsupported runtime asset '%1'. PS2 Studio compiles images, OBJ meshes, .ps2skin skeletal meshes, .ps2anim transform clips, and WAV audio. Convert other formats before importing.").arg(ext);
        return{};
    }
    QDir().mkpath(m_root+"/assets/"+dir);
    QString dst=m_root+"/assets/"+dir+"/"+fi.fileName();
    if(QFile::exists(dst)&&!QFile::remove(dst)){if(error)*error="Could not replace existing asset";return{};}
    if(!QFile::copy(src,dst)){if(error)*error="Could not copy asset";return{};}
    refresh();
    return dst;
}

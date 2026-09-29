#include "AssetCompiler.h"
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QRegularExpression>
#include <QTextStream>
#include <QVector3D>
#include <QVector2D>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QtEndian>
#include <cmath>

static QString ident(QString s){s=s.toLower();s.replace(QRegularExpression("[^a-z0-9_]"),"_");if(s.isEmpty()||s[0].isDigit())s.prepend("a_");return s;}
static bool writeText(const QString&p,const QString&t,QString&err){QFile f(p);if(!f.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text)){err=f.errorString();return false;}f.write(t.toUtf8());return true;}
static QString hexBytes(const QByteArray&b){QString out;out.reserve(b.size()*6);for(int i=0;i<b.size();++i){if(i%16==0)out+="\n    ";out+=QString("0x%1,").arg((quint8)b[i],2,16,QChar('0'));}return out;}
static QString f32(double v){return QString::number(v,'f',7)+"f";}
static QVector3D jsonVec3(const QJsonValue&v,const QVector3D&fallback={}){auto a=v.toArray();if(a.size()!=3)return fallback;return {float(a[0].toDouble()),float(a[1].toDouble()),float(a[2].toDouble())};}
static QJsonValue jsonAt(const QJsonArray&a,int i){return (i>=0&&i<a.size())?a.at(i):QJsonValue();}
static double jsonDoubleAt(const QJsonArray&a,int i,double fallback=0.0){auto v=jsonAt(a,i);return v.isUndefined()?fallback:v.toDouble(fallback);}
static int jsonIntAt(const QJsonArray&a,int i,int fallback=0){auto v=jsonAt(a,i);return v.isUndefined()?fallback:v.toInt(fallback);}
static QString findAdpenc(){
    QString sdk=qEnvironmentVariable("PS2SDK");if(sdk.isEmpty()&&QFileInfo::exists("C:/ps2dev/ps2sdk"))sdk="C:/ps2dev/ps2sdk";
    QStringList candidates;if(!sdk.isEmpty())candidates<<sdk+"/bin/adpenc"<<sdk+"/bin/adpenc.exe";
    auto path=QStandardPaths::findExecutable("adpenc");if(!path.isEmpty())return path;
    for(const auto&p:candidates){QFileInfo fi(p);if((fi.exists()&&fi.isExecutable())||fi.exists())return QDir::cleanPath(p);}return{};
}
static QByteArray encodeAdpcm(const QString&wav,const QString&outPath){
    QString enc=findAdpenc();if(enc.isEmpty())return{};QFile::remove(outPath);QProcess p;p.start(enc,{wav,outPath});if(!p.waitForFinished(30000)||p.exitStatus()!=QProcess::NormalExit||p.exitCode()!=0||!QFileInfo::exists(outPath))return{};QFile f(outPath);if(!f.open(QIODevice::ReadOnly))return{};return f.readAll();
}

AssetCompiler::Result AssetCompiler::compileProject(const QString&root){
    Result r;QDir().mkpath(root+"/generated");
    QString h=R"HDR(#pragma once
#include <tamtypes.h>

struct PS2EmbeddedTexture { const char* name; int width,height; const u32* rgba; };
struct PS2EmbeddedMesh { const char* name; int vertexCount,indexCount; const float* xyz; const float* uv; const float* normal; const u16* indices; };
struct PS2SkinVertex { float x,y,z,u,v; u8 bones[4]; float weights[4]; };
struct PS2SkinBone { const char* name; int parent; float invBind[16]; };
struct PS2EmbeddedSkinMesh { const char* name; int vertexCount,indexCount,boneCount; const PS2SkinVertex* vertices; const u16* indices; const PS2SkinBone* bones; };
struct PS2AnimKey { float time; float px,py,pz,rx,ry,rz,sx,sy,sz; };
struct PS2AnimTrack { int bone; int keyCount; const PS2AnimKey* keys; };
struct PS2EmbeddedAnimation { const char* name; float duration; int loop; int trackCount; const PS2AnimTrack* tracks; };
enum { PS2_AUDIO_PCM16 = 0, PS2_AUDIO_ADPCM = 1 };
struct PS2EmbeddedSound { const char* name; int encoding,rate,channels,bits,bytes; const unsigned char* data; };
extern const PS2EmbeddedTexture g_ps2Textures[]; extern const int g_ps2TextureCount;
extern const PS2EmbeddedMesh g_ps2Meshes[]; extern const int g_ps2MeshCount;
extern const PS2EmbeddedSkinMesh g_ps2SkinMeshes[]; extern const int g_ps2SkinMeshCount;
extern const PS2EmbeddedAnimation g_ps2Animations[]; extern const int g_ps2AnimationCount;
extern const PS2EmbeddedSound g_ps2Sounds[]; extern const int g_ps2SoundCount;
)HDR";
    QString c="#include \"PS2Assets.h\"\n\n";QStringList texRows,meshRows,skinRows,animRows,sndRows;

    // Textures.
    QDirIterator ti(root+"/assets/textures",QDir::Files,QDirIterator::Subdirectories);
    while(ti.hasNext()){
        const QString p=ti.next();QImage img(p);if(img.isNull())continue;img=img.convertToFormat(QImage::Format_RGBA8888);const QString id=ident(QFileInfo(p).completeBaseName());
        c+=QString("alignas(64) static const u32 tex_%1[] = {").arg(id);for(int y=0;y<img.height();++y){const uchar*row=img.constScanLine(y);for(int x=0;x<img.width();++x){const uchar*px=row+x*4;quint32 v=(quint32(px[3])<<24)|(quint32(px[2])<<16)|(quint32(px[1])<<8)|quint32(px[0]);c+=QString("0x%1,").arg(v,8,16,QChar('0'));}c+="\n";}c+="};\n";
        texRows<<QString("{\"%1\",%2,%3,tex_%4}").arg(QFileInfo(p).fileName()).arg(img.width()).arg(img.height()).arg(id);r.textures++;
    }

    // Static OBJ meshes with UVs and normals. Normals are generated if missing.
    QDirIterator mi(root+"/assets/models",QStringList{"*.obj","*.OBJ"},QDir::Files,QDirIterator::Subdirectories);
    while(mi.hasNext()){
        const QString p=mi.next(),id=ident(QFileInfo(p).completeBaseName());QFile f(p);if(!f.open(QIODevice::ReadOnly|QIODevice::Text))continue;
        QVector<QVector3D> srcVerts,srcNorm;QVector<QVector2D> srcUv;QVector<QVector3D> verts,norm;QVector<QVector2D>uv;QVector<quint16>idx;QHash<QString,int>remap;QVector<bool> explicitNorm;QTextStream in(&f);
        auto mapped=[&](int vi,int ti,int ni)->int{QString key=QString::number(vi)+"/"+QString::number(ti)+"/"+QString::number(ni);if(remap.contains(key))return remap[key];if(vi<0||vi>=srcVerts.size()||verts.size()>=65535)return-1;int n=verts.size();verts<<srcVerts[vi];uv<<(ti>=0&&ti<srcUv.size()?srcUv[ti]:QVector2D(0,0));norm<<(ni>=0&&ni<srcNorm.size()?srcNorm[ni]:QVector3D(0,0,0));explicitNorm<<(ni>=0&&ni<srcNorm.size());remap[key]=n;return n;};
        while(!in.atEnd()){
            QString line=in.readLine().trimmed();
            if(line.startsWith("v ")){auto q=line.mid(2).simplified().split(' ');if(q.size()>=3)srcVerts<<QVector3D(q[0].toFloat(),q[1].toFloat(),q[2].toFloat());}
            else if(line.startsWith("vt ")){auto q=line.mid(3).simplified().split(' ');if(q.size()>=2)srcUv<<QVector2D(q[0].toFloat(),1.0f-q[1].toFloat());}
            else if(line.startsWith("vn ")){auto q=line.mid(3).simplified().split(' ');if(q.size()>=3)srcNorm<<QVector3D(q[0].toFloat(),q[1].toFloat(),q[2].toFloat()).normalized();}
            else if(line.startsWith("f ")){auto q=line.mid(2).simplified().split(' ');QVector<int>face;for(const auto&token:q){auto parts=token.split('/');bool ok=false;int vi=parts.value(0).toInt(&ok);if(!ok)continue;if(vi<0)vi=srcVerts.size()+vi+1;int ui=-1,ni=-1;if(parts.size()>1&&!parts[1].isEmpty()){ui=parts[1].toInt();if(ui<0)ui=srcUv.size()+ui+1;ui--;}if(parts.size()>2&&!parts[2].isEmpty()){ni=parts[2].toInt();if(ni<0)ni=srcNorm.size()+ni+1;ni--;}int n=mapped(vi-1,ui,ni);if(n>=0)face<<n;}for(int k=1;k+1<face.size();++k)idx<<face[0]<<face[k]<<face[k+1];}
        }
        if(verts.isEmpty()||idx.isEmpty())continue;
        for(int k=0;k+2<idx.size();k+=3){int a=idx[k],b=idx[k+1],d=idx[k+2];QVector3D fn=QVector3D::crossProduct(verts[b]-verts[a],verts[d]-verts[a]).normalized();for(int v:{a,b,d})if(!explicitNorm[v])norm[v]+=fn;}
        for(int i=0;i<norm.size();++i)if(norm[i].lengthSquared()<0.000001f)norm[i]=QVector3D(0,0,1);else norm[i].normalize();
        c+=QString("alignas(64) static const float mesh_%1_xyz[] = {").arg(id);for(auto&v:verts)c+=f32(v.x())+","+f32(v.y())+","+f32(v.z())+",";c+="};\n";
        c+=QString("alignas(64) static const float mesh_%1_uv[] = {").arg(id);for(auto&t:uv)c+=f32(t.x())+","+f32(t.y())+",";c+="};\n";
        c+=QString("alignas(64) static const float mesh_%1_norm[] = {").arg(id);for(auto&n:norm)c+=f32(n.x())+","+f32(n.y())+","+f32(n.z())+",";c+="};\n";
        c+=QString("alignas(64) static const u16 mesh_%1_idx[] = {").arg(id);for(auto n:idx)c+=QString::number(n)+",";c+="};\n";
        meshRows<<QString("{\"%1\",%2,%3,mesh_%4_xyz,mesh_%4_uv,mesh_%4_norm,mesh_%4_idx}").arg(QFileInfo(p).fileName()).arg(verts.size()).arg(idx.size()).arg(id);r.meshes++;
    }

    // Custom PS2 skinned-mesh interchange. This is intentionally explicit and small enough for homebrew pipelines.
    QDirIterator si(root+"/assets/models",QStringList{"*.ps2skin","*.PS2SKIN"},QDir::Files,QDirIterator::Subdirectories);
    while(si.hasNext()){
        const QString p=si.next(),id=ident(QFileInfo(p).completeBaseName());QFile f(p);if(!f.open(QIODevice::ReadOnly))continue;QJsonParseError pe;auto doc=QJsonDocument::fromJson(f.readAll(),&pe);if(pe.error!=QJsonParseError::NoError||!doc.isObject())continue;auto o=doc.object();auto va=o.value("vertices").toArray();auto ia=o.value("indices").toArray();auto ba=o.value("bones").toArray();if(va.isEmpty()||ia.isEmpty()||ba.isEmpty())continue;
        c+=QString("alignas(64) static const PS2SkinVertex skin_%1_vertices[] = {\n").arg(id);
        for(const auto&vv:va){auto vo=vv.toObject();auto pv=vo.value("position").toArray();auto uvv=vo.value("uv").toArray();auto bones=vo.value("bones").toArray();auto weights=vo.value("weights").toArray();c+="{"+f32(jsonDoubleAt(pv,0))+","+f32(jsonDoubleAt(pv,1))+","+f32(jsonDoubleAt(pv,2))+","+f32(jsonDoubleAt(uvv,0))+","+f32(1.0-jsonDoubleAt(uvv,1))+",{";for(int i=0;i<4;i++){c+=QString::number(qBound(0,jsonIntAt(bones,i,0),255));if(i<3)c+=",";}c+="},{";double sum=0;for(int i=0;i<4;i++)sum+=jsonDoubleAt(weights,i,i==0?1.0:0.0);if(sum<=0)sum=1;for(int i=0;i<4;i++){c+=f32(jsonDoubleAt(weights,i,i==0?1.0:0.0)/sum);if(i<3)c+=",";}c+="}},\n";}c+="};\n";
        c+=QString("alignas(64) static const u16 skin_%1_idx[] = {").arg(id);for(const auto&v:ia)c+=QString::number(qBound(0,v.toInt(),65535))+",";c+="};\n";
        c+=QString("static const PS2SkinBone skin_%1_bones[] = {\n").arg(id);for(const auto&bv:ba){auto bo=bv.toObject();auto inv=bo.value("inverseBind").toArray();c+="{\""+bo.value("name").toString("bone").replace("\"","\\\"")+"\","+QString::number(bo.value("parent").toInt(-1))+",{";for(int i=0;i<16;i++){double def=(i%5==0)?1.0:0.0;c+=f32(jsonDoubleAt(inv,i,def));if(i<15)c+=",";}c+="}},\n";}c+="};\n";
        skinRows<<QString("{\"%1\",%2,%3,%4,skin_%5_vertices,skin_%5_idx,skin_%5_bones}").arg(QFileInfo(p).fileName()).arg(va.size()).arg(ia.size()).arg(ba.size()).arg(id);r.skinnedMeshes++;
    }

    // Transform/skeletal animation clips.
    QDirIterator ani(root+"/assets/animations",QStringList{"*.ps2anim","*.PS2ANIM"},QDir::Files,QDirIterator::Subdirectories);
    while(ani.hasNext()){
        const QString p=ani.next(),id=ident(QFileInfo(p).completeBaseName());QFile f(p);if(!f.open(QIODevice::ReadOnly))continue;QJsonParseError pe;auto doc=QJsonDocument::fromJson(f.readAll(),&pe);if(pe.error!=QJsonParseError::NoError||!doc.isObject())continue;auto o=doc.object();auto tracks=o.value("tracks").toArray();if(tracks.isEmpty())continue;QStringList trackRows;int trackNo=0;for(const auto&tv:tracks){auto to=tv.toObject();auto keys=to.value("keys").toArray();if(keys.isEmpty())continue;QString kn=QString("anim_%1_t%2_keys").arg(id).arg(trackNo);c+="static const PS2AnimKey "+kn+"[] = {\n";for(const auto&kv:keys){auto ko=kv.toObject();auto P=jsonVec3(ko.value("position"),{}),R=jsonVec3(ko.value("rotation"),{}),S=jsonVec3(ko.value("scale"),{1,1,1});c+="{"+f32(ko.value("time").toDouble())+","+f32(P.x())+","+f32(P.y())+","+f32(P.z())+","+f32(R.x())+","+f32(R.y())+","+f32(R.z())+","+f32(S.x())+","+f32(S.y())+","+f32(S.z())+"},\n";}c+="};\n";trackRows<<QString("{%1,%2,%3}").arg(to.contains("bone")?to.value("bone").toInt():-1).arg(keys.size()).arg(kn);trackNo++;}if(trackRows.isEmpty())continue;QString tn="anim_"+id+"_tracks";c+="static const PS2AnimTrack "+tn+"[] = {"+trackRows.join(",")+"};\n";double duration=o.value("duration").toDouble(1.0);animRows<<QString("{\"%1\",%2,%3,%4,%5}").arg(QFileInfo(p).fileName()).arg(f32(duration)).arg(o.value("loop").toBool(true)?1:0).arg(trackRows.size()).arg(tn);r.animations++;
    }

    // Audio: preserve PCM fallback, but prefer PS2 ADPCM so SFX can use concurrent SPU2 voices.
    QDirIterator ai(root+"/assets/audio",QStringList{"*.wav","*.WAV"},QDir::Files,QDirIterator::Subdirectories);
    while(ai.hasNext()){
        const QString p=ai.next(),id=ident(QFileInfo(p).completeBaseName());QFile f(p);if(!f.open(QIODevice::ReadOnly))continue;QByteArray b=f.readAll();if(b.size()<44||b.left(4)!="RIFF"||b.mid(8,4)!="WAVE")continue;int pos=12,rate=0,channels=0,bits=0;QByteArray pcm;while(pos+8<=b.size()){QByteArray tag=b.mid(pos,4);quint32 len=qFromLittleEndian<quint32>((const uchar*)b.constData()+pos+4);pos+=8;if(pos+(int)len>b.size())break;if(tag=="fmt "&&len>=16){channels=qFromLittleEndian<quint16>((const uchar*)b.constData()+pos+2);rate=qFromLittleEndian<quint32>((const uchar*)b.constData()+pos+4);bits=qFromLittleEndian<quint16>((const uchar*)b.constData()+pos+14);}if(tag=="data"){pcm=b.mid(pos,len);break;}pos+=len+(len&1);}if(pcm.isEmpty()||rate<=0||channels<=0||bits!=16)continue;
        QByteArray data=encodeAdpcm(p,root+"/generated/"+id+".adp");int encoding=0;
        if(!data.isEmpty()){encoding=1;r.adpcmSounds++;}else data=pcm;
        c+=QString("alignas(64) static const unsigned char snd_%1[] = {").arg(id)+hexBytes(data)+"\n};\n";sndRows<<QString("{\"%1\",%2,%3,%4,%5,%6,snd_%7}").arg(QFileInfo(p).fileName()).arg(encoding).arg(rate).arg(channels).arg(bits).arg(data.size()).arg(id);r.sounds++;
    }

    c+="\nconst PS2EmbeddedTexture g_ps2Textures[] = {"+(texRows.isEmpty()?QString("{nullptr,0,0,nullptr}"):texRows.join(",\n"))+"};\nconst int g_ps2TextureCount = "+QString::number(texRows.size())+";\n";
    c+="const PS2EmbeddedMesh g_ps2Meshes[] = {"+(meshRows.isEmpty()?QString("{nullptr,0,0,nullptr,nullptr,nullptr,nullptr}"):meshRows.join(",\n"))+"};\nconst int g_ps2MeshCount = "+QString::number(meshRows.size())+";\n";
    c+="const PS2EmbeddedSkinMesh g_ps2SkinMeshes[] = {"+(skinRows.isEmpty()?QString("{nullptr,0,0,0,nullptr,nullptr,nullptr}"):skinRows.join(",\n"))+"};\nconst int g_ps2SkinMeshCount = "+QString::number(skinRows.size())+";\n";
    c+="const PS2EmbeddedAnimation g_ps2Animations[] = {"+(animRows.isEmpty()?QString("{nullptr,0,0,0,nullptr}"):animRows.join(",\n"))+"};\nconst int g_ps2AnimationCount = "+QString::number(animRows.size())+";\n";
    c+="const PS2EmbeddedSound g_ps2Sounds[] = {"+(sndRows.isEmpty()?QString("{nullptr,0,0,0,0,0,nullptr}"):sndRows.join(",\n"))+"};\nconst int g_ps2SoundCount = "+QString::number(sndRows.size())+";\n";
    if(!writeText(root+"/generated/PS2Assets.h",h,r.error)||!writeText(root+"/generated/PS2Assets.cpp",c,r.error))return r;
    r.generated={root+"/generated/PS2Assets.h",root+"/generated/PS2Assets.cpp"};r.ok=true;return r;
}

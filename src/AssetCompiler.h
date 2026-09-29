#pragma once
#include <QString>
#include <QStringList>

class AssetCompiler {
public:
    struct Result {
        bool ok=false;
        QString error;
        QStringList generated;
        int textures=0;
        int meshes=0;
        int skinnedMeshes=0;
        int animations=0;
        int sounds=0;
        int adpcmSounds=0;
    };
    static Result compileProject(const QString& projectRoot);
};

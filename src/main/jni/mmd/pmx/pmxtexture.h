//
// Created by wjy50 on 18-3-17.
//

#ifndef MMDVIEWER_PMXTEXTURE_H
#define MMDVIEWER_PMXTEXTURE_H

#include "pmxcommon.h"
#include "../../utils/mstring.h"

class PMXTexture
{
private:
    const char *path;
    GLuint textureId;
    bool mHasAlpha;
public:
    PMXTexture();

    void read(std::ifstream &file, MStringEncoding encoding, const char *pmxPath, int pathLength);

    GLuint getTextureId();

    void initGLTexture();

    bool hasAlpha();

    ~PMXTexture();
};

#endif //MMDVIEWER_PMXTEXTURE_H

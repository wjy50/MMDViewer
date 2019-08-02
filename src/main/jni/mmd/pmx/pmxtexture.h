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
    std::string path;
    GLuint textureId;
    bool mHasAlpha;
public:
    PMXTexture();

    void read(std::ifstream &file, MStringEncoding encoding, const std::string &parentPath);

    GLuint getTextureId();

    void initGLTexture();

    bool hasAlpha();

    ~PMXTexture();
};

#endif //MMDVIEWER_PMXTEXTURE_H

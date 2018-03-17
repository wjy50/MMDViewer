//
// Created by wjy50 on 18-3-17.
//

#ifndef MMDVIEWER_PMXCOMMON_H
#define MMDVIEWER_PMXCOMMON_H

#include <GLES3/gl3.h>

#define PMX_MODEL_SCALE 0.1f;

typedef struct PMX_INFO{
    unsigned char encoding;
    unsigned char UVACount;
    unsigned char vertexSize;
    unsigned char texSize;
    unsigned char materialSize;
    unsigned char boneSize;
    unsigned char morphSize;
    unsigned char bodySize;
}PMXInfo;

#endif //MMDVIEWER_PMXCOMMON_H

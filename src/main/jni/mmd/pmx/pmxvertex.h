//
// Created by wjy50 on 18-3-17.
//

#ifndef MMDVIEWER_PMXVERTEX_H
#define MMDVIEWER_PMXVERTEX_H

#include <fstream>
#include "pmxcommon.h"

class PMXVertex
{
private:
    float *coordinate;
    float *initialCoordinate;
    float *normal;
    float *uv;
    float *initialUV;
    //something here (UVA) is ignored while reading
    char deform;
    char boneCount;
    int *bones;
    float *weights;
    float *sDefVec;
    //something here (4 bytes) is also ignored
public:
    PMXVertex();

    void read(std::ifstream &file, PMXInfo *info, float *coordinate, float *normal, float *uv);

    char getBoneCount();

    int getBoneAt(int index);

    float getWeightAt(int index);

    void setBoneAt(int index, int bone);

    const float *getInitialCoordinate();

    void setPosition(float x, float y, float z);

    const float *getInitialUV();

    void setUV(float u, float v);

    ~PMXVertex();
};

#endif //MMDVIEWER_PMXVERTEX_H

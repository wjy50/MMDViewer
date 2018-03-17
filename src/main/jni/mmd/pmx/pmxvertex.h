//
// Created by wjy50 on 18-3-17.
//

#ifndef MMDVIEWER_PMXVERTEX_H
#define MMDVIEWER_PMXVERTEX_H

#include <stdio.h>
#include "pmxcommon.h"

class PMXVertex{
private:
    float * coordinate;
    float * normal;
    float * uv;
    //something here (UVA) is ignored while reading
    char deform;
    char boneCount;
    unsigned int *bones;
    float *weights;
    float *sDefVec;
    //something here (4 bytes) is also ignored
public:
    PMXVertex();
    void read(FILE* file,PMXInfo* info,float* coordinate,float* normal,float* uv);
    char getBoneCount();
    unsigned int getBoneAt(int index);
    float getWeightAt(int index);
    void setBoneAt(int index, unsigned int bone);
    ~PMXVertex();
};

#endif //MMDVIEWER_PMXVERTEX_H
//
// Created by wjy50 on 18-3-17.
//

#include "pmxvertex.h"
#include "../../utils/mathutils.h"

PMXVertex::PMXVertex() {
    bones=0;
    weights=0;
    sDefVec=0;
}

void PMXVertex::read(FILE *file, PMXInfo *info,float* coordinate,float* normal,float* uv) {
    this->coordinate=coordinate;
    this->normal=normal;
    this->uv=uv;
    fread(coordinate, sizeof(float),3,file);
    fread(normal, sizeof(float),3,file);
    fread(uv, sizeof(float),2,file);
    int sk=MIN(info->UVACount,4);
    fseek(file,sk*16,SEEK_CUR);
    fread(&deform, sizeof(char),1,file);
    switch (deform)
    {
        case 0:
            bones=new unsigned int[1];
            weights=new float[1];
            boneCount=1;
            bones[0]=0;
            weights[0]=1;
            fread(bones,info->boneSize,1,file);
            break;
        case 1:
            bones=new unsigned int[2];
            weights=new float[2];
            boneCount=2;
            bones[0]=bones[1]=0;
            fread(bones,info->boneSize,1,file);
            fread(bones+1,info->boneSize,1,file);
            fread(weights, sizeof(float),1,file);
            weights[1]=1-weights[0];
            break;
        case 2:
        case 4:
            bones=new unsigned int[4];
            weights=new float[4];
            boneCount=4;
            bones[0]=bones[1]=bones[2]=bones[3]=0;
            fread(bones,info->boneSize,1,file);
            fread(bones+1,info->boneSize,1,file);
            fread(bones+2,info->boneSize,1,file);
            fread(bones+3,info->boneSize,1,file);
            fread(weights, sizeof(float),4,file);
            break;
        case 3:
            bones=new unsigned int[2];
            weights=new float[2];
            boneCount=2;
            bones[0]=bones[1]=0;
            fread(bones,info->boneSize,1,file);
            fread(bones+1,info->boneSize,1,file);
            fread(weights, sizeof(float),1,file);
            weights[1]=1-weights[0];
            float sDefVec[9];
            fread(sDefVec, sizeof(float),9,file);
            break;
        default:
            break;
    }
    fseek(file,4,SEEK_CUR);
}

char PMXVertex::getBoneCount() {
    return boneCount;
}

unsigned int PMXVertex::getBoneAt(int index) {
    return bones[index];
}

float PMXVertex::getWeightAt(int index) {
    return weights[index];
}

void PMXVertex::setBoneAt(int index, unsigned int bone) {
    bones[index]=bone;
}

PMXVertex::~PMXVertex() {
    if(bones)delete [] bones;
    if(weights)delete [] weights;
    if(sDefVec)delete [] sDefVec;
}
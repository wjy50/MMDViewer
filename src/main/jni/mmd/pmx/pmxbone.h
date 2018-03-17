//
// Created by wjy50 on 18-3-17.
//

#ifndef MMDVIEWER_PMXBONE_H
#define MMDVIEWER_PMXBONE_H

#include <stdio.h>
#include "../../utils/mstring.h"

typedef enum BONE_FLAG{
    TO_BONE=1,
    ROTATION=2,
    TRANSLATION=4,
    VISIBLE=8,
    ENABLE=0x10,
    IK=0x20,
    APPEND_LOCAL=0x80,
    APPEND_ROTATION=0x100,
    APPEND_TRANSLATION=0x200,
    FIX_AXIS=0x400,
    LOCAL_FRAME=0x800,
    AFTER_PHYSICS=0x1000,
    EXTRA_PARENT=0x2000
}BoneFlag;

class PMXBoneIKChainNode{
private:
    unsigned int ikBone;
    bool limited;
    float * low;
    float * high;
    float * ikMat;
public:
    PMXBoneIKChainNode();
    void read(FILE *file, size_t boneSize);

    unsigned int getBoneIndex();
    bool isLimited();
    const float * getLowLimit();
    const float * getHighLimit();

    float * getIKMat();

    ~PMXBoneIKChainNode();
};

class PMXBoneIK{
private:
    unsigned int target;
    int loopCount;
    float loopAngleLimit;
    int ikChainLength;
    PMXBoneIKChainNode* ikChain;
public:
    PMXBoneIK(FILE *file, size_t boneSize);
    unsigned int getTarget();
    int getIkChainLength();
    PMXBoneIKChainNode* getIkChainNodeAt(int index);
    float getLoopAngleLimit();
    int getLoopCount();
    ~PMXBoneIK();
};

class PMXBone{
private:
    MString * name,*nameE;
    float * position;
    unsigned int parent;
    int level;
    unsigned short flag;
    int child;
    float * offset;
    unsigned int appendParent;
    float appendRatio;
    float * axis;
    float * localX;
    float * localZ;
    float * localY;
    int extraKey;
    PMXBoneIK* boneIK;

    unsigned int actualIndex;

    float * localMat;
    float * localMatWithAppend;

    float * ikMat;

    int childCount;
    int childrenCapacity;
    unsigned int * children;

    bool appendFromSelf;
public:
    PMXBone();
    void
    read(FILE *file, size_t boneSize, MStringEncoding encoding, float *localMat, float *position);
    void normalizeLocal();

    PMXBoneIK* getBoneIK();
    void setActualIndex(unsigned int actualIndex);
    unsigned int getActualIndex();
    float * getPosition();
    unsigned int getParent();
    unsigned int getAppendParent();
    float getAppendRatio();
    int getChildCount();
    const float * getLocalMat();
    void setIKMat(float * ikMat);
    void addChild(unsigned int child);
    unsigned int getChildAt(int index);
    void setAppendFromSelf(bool appendFromSelf);
    bool isAppendFromSelf();
    const float * getCurrentMat();
    float * getLocalMatWithAppend();
    float * getIKMat();

    const char * getName();

    void rotateBy(float a, float x, float y, float z);
    void translationBy(float x, float y, float z);
    void resetLocal();

    ~PMXBone();
};

#endif //MMDVIEWER_PMXBONE_H

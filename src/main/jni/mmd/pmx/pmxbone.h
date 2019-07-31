//
// Created by wjy50 on 18-3-17.
//

#ifndef MMDVIEWER_PMXBONE_H
#define MMDVIEWER_PMXBONE_H

#include <vector>

#include "../../utils/mstring.h"

typedef enum BONE_FLAG
{
    TO_BONE = 1,
    ROTATION = 2,
    TRANSLATION = 4,
    VISIBLE = 8,
    ENABLE = 0x10,
    IK = 0x20,
    APPEND_LOCAL = 0x80,
    APPEND_ROTATION = 0x100,
    APPEND_TRANSLATION = 0x200,
    FIX_AXIS = 0x400,
    LOCAL_FRAME = 0x800,
    AFTER_PHYSICS = 0x1000,
    EXTRA_PARENT = 0x2000
} BoneFlag;

class PMXBoneIKChainNode
{
private:
    int ikBone;
    bool limited;
    float *low;
    float *high;
public:
    PMXBoneIKChainNode();

    void read(std::ifstream &file, size_t boneSize);

    int getBoneIndex() const;

    bool isLimited() const;

    const float *getLowLimit() const;

    const float *getHighLimit() const;

    ~PMXBoneIKChainNode();
};

class PMXBoneIK
{
private:
    int target;
    int loopCount;
    float loopAngleLimit;
    int ikChainLength;
    PMXBoneIKChainNode *ikChain;
public:
    PMXBoneIK(std::ifstream &file, size_t boneSize);

    int getTarget() const;

    int getIkChainLength() const;

    PMXBoneIKChainNode *getIkChainNodeAt(int index) const;

    float getLoopAngleLimit() const;

    int getLoopCount() const;

    ~PMXBoneIK();
};

class PMXBone
{
private:
    MString name, nameE;
    float *position;
    int parent;
    int level;
    unsigned short flag;
    int child;
    float *offset;
    int appendParent;
    float appendRatio;
    float *axis;
    float *localX;
    float *localY;
    float *localZ;
    int extraKey;
    PMXBoneIK *boneIK;

    int actualIndex;

    float *localMat;
    float *localMatWithAppend;

    std::vector<int> children;

    bool appendFromSelf;

    float *ikMat;
    bool ikMatEnabled;
public:
    PMXBone();

    void
    read(std::ifstream &file, size_t boneSize, MStringEncoding encoding, float *localMat, float *position);

    void normalizeLocal();

    PMXBoneIK *getBoneIK() const;

    void setActualIndex(int actualIndex);

    int getActualIndex() const;

    float *getPosition() const;

    int getParent() const;

    int getAppendParent() const;

    float getAppendRatio() const;

    int getChildCount() const;

    const float *getLocalMat() const;

    void addChild(int child);

    int getChildAt(int index) const;

    void setAppendFromSelf(bool appendFromSelf);

    bool isAppendFromSelf() const;

    const float *getCurrentMat() const;

    float *getLocalMatWithAppend() const;

    const char *getName() const;

    float *getIKMat() const;

    void enableIKMat();

    void disableIKMat();

    void rotateBy(float a, float x, float y, float z);

    void translationBy(float x, float y, float z);

    ~PMXBone();
};

#endif //MMDVIEWER_PMXBONE_H

//
// Created by wjy50 on 18-3-17.
//

#include "pmxbone.h"
#include "pmxcommon.h"
#include "../../vector/vector.h"
#include "../../matrix/matrix.h"
#include "../../utils/mathutils.h"

/*implementation of PMXBone*/

PMXBone::PMXBone() {
    name=nameE=0;
    offset=localX=localY=localZ=axis=0;
    boneIK=0;
    ikMat=0;
    childCount=0;
    childrenCapacity=0;
    children=0;
    localMatWithAppend=0;
    appendFromSelf= false;
}

void PMXBone::read(FILE *file, size_t boneSize, MStringEncoding encoding, float *localMat, float *position) {
    this->localMat=localMat;
    setIdentityM(localMat);
    name=MString::readString(file,encoding);
    nameE=MString::readString(file,encoding);
    this->position=position;
    fread(position, sizeof(float),3,file);
    position[0]*=PMX_MODEL_SCALE;
    position[1]*=PMX_MODEL_SCALE;
    position[2]*=-PMX_MODEL_SCALE;
    position[3]=1;
    parent=0;
    fread(&parent,boneSize,1,file);
    fread(&level, sizeof(int),1,file);
    fread(&flag, sizeof(unsigned short),1,file);
    if(CHECK_FLAG(flag,TO_BONE))//FLAG_TO_BONE
    {
        child=0;
        fread(&child,boneSize,1,file);
        offset=0;
    }
    else
    {
        offset=new float[3];
        fread(offset, sizeof(float),3,file);
        offset[0]*=0.1f;
        offset[1]*=0.1f;
        offset[2]=-offset[2]*0.1f;
    }
    if(CHECK_FLAG(flag,APPEND_ROTATION) || CHECK_FLAG(flag,APPEND_TRANSLATION))
    {
        appendParent=0;
        fread(&appendParent,boneSize,1,file);
        fread(&appendRatio, sizeof(float),1,file);
        if(appendRatio == 0)
        {
            appendParent=0xffffffff;
        }
        else
        {
            localMatWithAppend=new float[16];
        }
    }
    else appendParent=0xffffffff;
    if(CHECK_FLAG(flag,FIX_AXIS))
    {
        axis=new float[3];
        fread(axis, sizeof(float),3,file);
        axis[2]=-axis[2];
    }
    else axis=0;
    if(CHECK_FLAG(flag,LOCAL_FRAME))
    {
        localX=new float[3];
        localZ=new float[3];
        fread(localX, sizeof(float),3,file);
        fread(localZ, sizeof(float),3,file);
        localX[2]=-localX[2];
        localZ[2]=-localZ[2];
        //normalize local
        normalizeLocal();
    }
    else
    {
        localX=0;
        localY=0;
        localZ=0;
    }
    if(CHECK_FLAG(flag,EXTRA_PARENT))
    {
        fread(&extraKey, sizeof(int),1,file);
    }
    if(CHECK_FLAG(flag,IK))
    {
        boneIK=new PMXBoneIK(file,boneSize);
    }
    else boneIK=0;
}

void PMXBone::normalizeLocal() {
    if(localX && localZ)
    {
        if(!localY)localY=new float[3];
        crossProduct(localY,localZ,localX);
        crossProduct(localZ,localX,localY);
        normalize3Into(localX);
        normalize3Into(localY);
        normalize3Into(localZ);
    }
}

void PMXBone::setActualIndex(unsigned int actualIndex) {
    this->actualIndex=actualIndex;
}

unsigned int PMXBone::getActualIndex() {
    return actualIndex;
}

float* PMXBone::getPosition() {
    return position;
}

unsigned int PMXBone::getParent() {
    return parent;
}

unsigned int PMXBone::getAppendParent() {
    return appendParent;
}

float PMXBone::getAppendRatio() {
    return appendRatio;
}

int PMXBone::getChildCount() {
    return childCount;
}

const float* PMXBone::getLocalMat() {
    return localMat;
}

void PMXBone::setIKMat(float *ikMat) {
    this->ikMat=ikMat;
}

void PMXBone::addChild(unsigned int child) {
    if(!children)
    {
        children=new unsigned int[1];
        children[0]=child;
        childrenCapacity=childCount=1;
    }
    else
    {
        if(childCount == childrenCapacity)
        {
            childrenCapacity<<=1;
            unsigned int * newArray=new unsigned int[childrenCapacity];
            for (int i = 0; i < childCount; ++i) {
                newArray[i]=children[i];
            }
            delete [] children;
            children=newArray;
        }
        children[childCount++]=child;
    }
}

unsigned int PMXBone::getChildAt(int index) {
    return children[index];
}

void PMXBone::setAppendFromSelf(bool appendFromSelf) {
    this->appendFromSelf=appendFromSelf;
    if(appendFromSelf)setIdentityM(localMatWithAppend);
}

bool PMXBone::isAppendFromSelf() {
    return appendFromSelf;
}

const float* PMXBone::getCurrentMat() {
    return ikMat ? ikMat : localMat;
}

float* PMXBone::getLocalMatWithAppend() {
    return localMatWithAppend;
}

float* PMXBone::getIKMat() {
    return ikMat;
}

const char* PMXBone::getName() {
    return name->getData();
}

void PMXBone::rotateBy(float a, float x, float y, float z) {
    rotateM(localMat,a,x,y,z);
}

void PMXBone::translationBy(float x, float y, float z) {
    translateM(localMat,x,y,z);
}

PMXBone::~PMXBone() {
    delete name;
    delete nameE;
    if(offset)delete [] offset;
    if(axis)delete [] axis;
    if(localX)delete [] localX;
    if(localY)delete [] localY;
    if(localZ)delete [] localZ;
    if(boneIK)delete boneIK;
    if(children)delete [] children;
    if(localMatWithAppend)delete [] localMatWithAppend;
}

/*end of PMXBone*/

/*implementation of PMXBoneIKChainNode*/

PMXBoneIKChainNode::PMXBoneIKChainNode() {
    low=high=0;
    ikMat=0;
}

void PMXBoneIKChainNode::read(FILE *file, size_t boneSize) {
    ikBone=0;
    fread(&(ikBone),boneSize,1,file);
    char lim;
    fread(&lim, sizeof(char),1,file);
    limited=(lim != 0);
    if(limited)
    {
        low=new float[3];
        high=new float[3];
        fread(low, sizeof(float),3,file);
        fread(high, sizeof(float),3,file);
    }
    else
    {
        low=0;
        high=0;
    }
}

unsigned int PMXBoneIKChainNode::getBoneIndex() {
    return ikBone;
}

bool PMXBoneIKChainNode::isLimited() {
    return limited;
}

const float* PMXBoneIKChainNode::getLowLimit() {
    return low;
}

const float* PMXBoneIKChainNode::getHighLimit() {
    return high;
}

float* PMXBoneIKChainNode::getIKMat() {
    if(!ikMat)ikMat=new float[16];
    return ikMat;
}

/*end of PMXBoneIKChainNode*/

/*implementation of PMXBoneIK*/

PMXBoneIK::PMXBoneIK(FILE *file, size_t boneSize) {
    target=0;
    fread(&(target),boneSize,1,file);
    fread(&(loopCount), sizeof(int),1,file);
    fread(&(loopAngleLimit), sizeof(float),1,file);
    fread(&(ikChainLength), sizeof(int),1,file);
    if(ikChainLength != 0)
    {
        ikChain=new PMXBoneIKChainNode[ikChainLength];
        for (int i = 0; i < ikChainLength; ++i) {
            ikChain[i].read(file,boneSize);
        }
    }
    else
    {
        ikChain=0;
    }
}

unsigned int PMXBoneIK::getTarget() {
    return target;
}

int PMXBoneIK::getIkChainLength() {
    return ikChainLength;
}

PMXBoneIKChainNode* PMXBoneIK::getIkChainNodeAt(int index) {
    return ikChain+index;
}

PMXBoneIK* PMXBone::getBoneIK() {
    return boneIK;
}

PMXBoneIKChainNode::~PMXBoneIKChainNode() {
    if(limited)
    {
        delete [] low;
        delete [] high;
    }
    if(ikMat)delete [] ikMat;
}

float PMXBoneIK::getLoopAngleLimit() {
    return loopAngleLimit;
}

int PMXBoneIK::getLoopCount() {
    return loopCount;
}

PMXBoneIK::~PMXBoneIK() {
    if(ikChain)
    {
        delete [] ikChain;
    }
}

/*end of PMXBoneIK*/
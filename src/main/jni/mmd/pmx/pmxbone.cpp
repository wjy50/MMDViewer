//
// Created by wjy50 on 18-3-17.
//

#include "pmxbone.h"
#include "pmxcommon.h"
#include "../../vector/vector.h"
#include "../../matrix/matrix.h"
#include "../../utils/mathutils.h"
#include "../mmdcommon.h"

using namespace std;

/*implementation of PMXBone*/

PMXBone::PMXBone()
: offset(nullptr), axis(nullptr), localX(nullptr), localY(nullptr), localZ(nullptr),
  boneIK(nullptr), localMatWithAppend(nullptr), appendFromSelf(false),
  ikMat(nullptr), ikMatEnabled(false)
{}

void PMXBone::read(ifstream &file, size_t boneSize, MStringEncoding encoding, float *localMat,
                   float *position)
{
    this->localMat = localMat;
    setIdentityM(localMat);
    name.readString(file, encoding, UTF_8);
    nameE.readString(file, encoding, UTF_8);
    this->position = position;
    file.read(reinterpret_cast<char *>(position), 3 * sizeof(float));
    position[0] *= MMD_COORDINATE_SCALE;
    position[1] *= MMD_COORDINATE_SCALE;
    position[2] *= -MMD_COORDINATE_SCALE;
    position[3] = 1;
    parent = 0;
    file.read(reinterpret_cast<char *>(&parent), boneSize);
    if (IS_NEGATIVE_ONE(parent, boneSize))
        parent = -1;
    file.read(reinterpret_cast<char *>(&level), sizeof(int));
    file.read(reinterpret_cast<char *>(&flag), sizeof(unsigned short));
    if (CHECK_FLAG(flag, TO_BONE))  // FLAG_TO_BONE
    {
        child = 0;
        file.read(reinterpret_cast<char *>(&child), boneSize);
        offset = nullptr;
    } else {
        offset = new float[3];
        file.read(reinterpret_cast<char *>(offset), 3 * sizeof(float));
        offset[0] *= 0.1f;
        offset[1] *= 0.1f;
        offset[2] = -offset[2] * 0.1f;
    }
    if (CHECK_FLAG(flag, APPEND_ROTATION) || CHECK_FLAG(flag, APPEND_TRANSLATION)) {
        appendParent = 0;
        file.read(reinterpret_cast<char *>(&appendParent), boneSize);
        file.read(reinterpret_cast<char *>(&appendRatio), sizeof(float));
        if (appendRatio == 0 || IS_NEGATIVE_ONE(appendParent, boneSize)) {
            appendParent = -1;
        } else {
            localMatWithAppend = new float[16];
        }
    } else
        appendParent = -1;
    if (CHECK_FLAG(flag, FIX_AXIS)) {
        axis = new float[3];
        file.read(reinterpret_cast<char *>(axis), 3 * sizeof(float));
        axis[2] = -axis[2];
    } else
        axis = nullptr;
    if (CHECK_FLAG(flag, LOCAL_FRAME)) {
        localX = new float[3];
        localZ = new float[3];
        file.read(reinterpret_cast<char *>(localX), 3 * sizeof(float));
        file.read(reinterpret_cast<char *>(localZ), 3 * sizeof(float));
        localX[2] = -localX[2];
        localZ[2] = -localZ[2];
        localY = new float[3];
        normalizeLocal();
    }
    if (CHECK_FLAG(flag, EXTRA_PARENT)) {
        file.read(reinterpret_cast<char *>(&extraKey), sizeof(int));
    }
    if (CHECK_FLAG(flag, IK)) {
        boneIK = new PMXBoneIK(file, boneSize);
    }
}

void PMXBone::normalizeLocal()
{
    if (localX && localZ) {
        crossProduct(localY, localZ, localX);
        crossProduct(localZ, localX, localY);
        normalize3Into(localX);
        normalize3Into(localY);
        normalize3Into(localZ);
    }
}

void PMXBone::setActualIndex(int actualIndex)
{
    this->actualIndex = actualIndex;
}

int PMXBone::getActualIndex() const
{
    return actualIndex;
}

float *PMXBone::getPosition() const
{
    return position;
}

int PMXBone::getParent() const
{
    return parent;
}

int PMXBone::getAppendParent() const
{
    return appendParent;
}

float PMXBone::getAppendRatio() const
{
    return appendRatio;
}

int PMXBone::getChildCount() const
{
    return static_cast<int>(children.size());
}

const float *PMXBone::getLocalMat() const
{
    return localMat;
}

void PMXBone::addChild(int child)
{
    children.push_back(child);
}

int PMXBone::getChildAt(int index) const
{
    return children[index];
}

void PMXBone::setAppendFromSelf(bool appendFromSelf)
{
    this->appendFromSelf = appendFromSelf;
    if (appendFromSelf)
        setIdentityM(localMatWithAppend);
}

bool PMXBone::isAppendFromSelf() const
{
    return appendFromSelf;
}

const float *PMXBone::getCurrentMat() const
{
    return (ikMat && ikMatEnabled) ? ikMat : localMat;
}

float *PMXBone::getLocalMatWithAppend() const
{
    return localMatWithAppend;
}

float *PMXBone::getIKMat() const
{
    return (ikMat && ikMatEnabled) ? ikMat : nullptr;
}

void PMXBone::enableIKMat()
{
    ikMatEnabled = true;
    if (!ikMat)
        ikMat = new float[16];
}

void PMXBone::disableIKMat()
{
    ikMatEnabled = false;
}

const char *PMXBone::getName() const
{
    return name.getData();
}

void PMXBone::rotateBy(float a, float x, float y, float z)
{
    rotateM(localMat, a, x, y, z);
}

void PMXBone::translationBy(float x, float y, float z)
{
    translateM(localMat, x, y, z);
}

PMXBone::~PMXBone()
{
    delete[] offset;
    delete[] axis;
    delete[] localX;
    delete[] localY;
    delete[] localZ;
    delete boneIK;
    delete[] localMatWithAppend;
    delete[] ikMat;
}

/*end of PMXBone*/

/*implementation of PMXBoneIKChainNode*/

PMXBoneIKChainNode::PMXBoneIKChainNode()
: low(nullptr), high(nullptr)
{}

void PMXBoneIKChainNode::read(ifstream &file, size_t boneSize)
{
    ikBone = 0;
    file.read(reinterpret_cast<char *>(&ikBone), boneSize);
    if (IS_NEGATIVE_ONE(ikBone, boneSize)) ikBone = -1;
    char lim;
    file.read(&lim, sizeof(char));
    limited = (lim != 0);
    if (limited) {
        low = new float[3];
        high = new float[3];
        file.read(reinterpret_cast<char *>(low), 3 * sizeof(float));
        file.read(reinterpret_cast<char *>(high), 3 * sizeof(float));
    }
}

int PMXBoneIKChainNode::getBoneIndex() const
{
    return ikBone;
}

bool PMXBoneIKChainNode::isLimited() const
{
    return limited;
}

const float *PMXBoneIKChainNode::getLowLimit() const
{
    return low;
}

const float *PMXBoneIKChainNode::getHighLimit() const
{
    return high;
}

PMXBoneIKChainNode::~PMXBoneIKChainNode()
{
    delete[] low;
    delete[] high;
}

/*end of PMXBoneIKChainNode*/

/*implementation of PMXBoneIK*/

PMXBoneIK::PMXBoneIK(ifstream &file, size_t boneSize)
{
    target = 0;
    file.read(reinterpret_cast<char *>(&target), boneSize);
    if (IS_NEGATIVE_ONE(target, boneSize))
        target = -1;
    file.read(reinterpret_cast<char *>(&loopCount), sizeof(int));
    file.read(reinterpret_cast<char *>(&loopAngleLimit), sizeof(float));
    file.read(reinterpret_cast<char *>(&ikChainLength), sizeof(int));
    if (ikChainLength != 0) {
        ikChain = new PMXBoneIKChainNode[ikChainLength];
        for (int i = 0; i < ikChainLength; ++i) {
            ikChain[i].read(file, boneSize);
        }
    } else {
        ikChain = 0;
    }
}

int PMXBoneIK::getTarget() const
{
    return target;
}

int PMXBoneIK::getIkChainLength() const
{
    return ikChainLength;
}

PMXBoneIKChainNode *PMXBoneIK::getIkChainNodeAt(int index) const
{
    return ikChain + index;
}

PMXBoneIK *PMXBone::getBoneIK() const
{
    return boneIK;
}

float PMXBoneIK::getLoopAngleLimit() const
{
    return loopAngleLimit;
}

int PMXBoneIK::getLoopCount() const
{
    return loopCount;
}

PMXBoneIK::~PMXBoneIK()
{
    delete[] ikChain;
}

/*end of PMXBoneIK*/
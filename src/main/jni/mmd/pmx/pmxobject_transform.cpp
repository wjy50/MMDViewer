//
// Created by wjy50 on 19-8-1.
//

#include <cmath>

#include "pmxobject.h"
#include "../../matrix/matrix.h"
#include "../../quaternion/quaternion.h"
#include "../../utils/mathutils.h"
#include "../../utils/debugutils.h"

using namespace std;

void PMXObject::invalidateChildren(int index)
{
    boneStateIds[index] = static_cast<char>(currentPassId ^ 1);
    int childCount = bones[index].getChildCount();
    for (int i = 0; i < childCount; ++i) {
        int child = bones[index].getChildAt(i);
        if (boneStateIds[child] == currentPassId)
            invalidateChildren(child);
    }
}

void PMXObject::calculateBone(int index)
{
    if (boneStateIds[index] != currentPassId) {
        bones[index].disableIKMat();
        int appendParent = bones[index].getAppendParent();
        float *position = bones[index].getPosition();
        int parent = bones[index].getParent();
        // LOG_PRINTF("append parent = %d", appendParent);
        if (appendParent >= 0 && appendParent != index) {
            calculateBone(appendParent);
            const float *appendParentLocal = bones[appendParent].getLocalMatWithAppend()
                                             ? bones[appendParent].getLocalMatWithAppend()
                                             : bones[appendParent].getLocalMat();
            float ratio = bones[index].getAppendRatio();
            if (ratio == 1) {
                multiplyMM(bones[index].getLocalMatWithAppend(), bones[index].getLocalMat(),
                           appendParentLocal);
            } else {
                matrixToQuaternion(vecTmp, appendParentLocal);
                translateM2(bones[index].getLocalMatWithAppend(), bones[index].getLocalMat(),
                            appendParentLocal[12] * ratio, appendParentLocal[13] * ratio,
                            appendParentLocal[14] * ratio);
                if (fabsf(vecTmp[0]) < 1 - 1e-6f) {
                    float angle = acosf(vecTmp[0]) * 2 * ratio;
                    rotateM(bones[index].getLocalMatWithAppend(), (float) (angle * RAD_TO_DEG),
                            vecTmp[1], vecTmp[2], vecTmp[3]);
                } else {
                    floatArrayCopy(bones[index].getLocalMat(), bones[index].getLocalMatWithAppend(),
                                   16);
                }
            }
            if (parent >= 0 && parent != index) {
                calculateBone(parent);
                translateM2(matrixTmp, bones[index].getLocalMatWithAppend(), -position[0],
                            -position[1], -position[2]);
                translateMPre(matrixTmp, position[0], position[1], position[2]);
                multiplyMM(finalBoneMats + (bones[index].getActualIndex() * 16),
                           finalBoneMats + (bones[parent].getActualIndex() * 16), matrixTmp);
            } else {
                translateM2(finalBoneMats + (bones[index].getActualIndex() * 16),
                            bones[index].getLocalMatWithAppend(), -position[0], -position[1],
                            -position[2]);
                translateMPre(finalBoneMats + (bones[index].getActualIndex() * 16), position[0],
                              position[1], position[2]);
            }
            boneStateIds[index] = currentPassId;
        } else {
            if (parent >= 0 && parent != index) {
                calculateBone(parent);
                translateM2(matrixTmp, bones[index].getLocalMat(), -position[0], -position[1],
                            -position[2]);
                translateMPre(matrixTmp, position[0], position[1], position[2]);
                multiplyMM(finalBoneMats + (bones[index].getActualIndex() * 16),
                           finalBoneMats + (bones[parent].getActualIndex() * 16), matrixTmp);
            } else {
                translateM2(finalBoneMats + (bones[index].getActualIndex() * 16),
                            bones[index].getLocalMat(), -position[0], -position[1], -position[2]);
                translateMPre(finalBoneMats + (bones[index].getActualIndex() * 16), position[0],
                              position[1], position[2]);
            }
            boneStateIds[index] = currentPassId;
        }
    }
}

void PMXObject::updateSelfAppend()
{
    for (int i = 0; i < selfAppendBoneCount; ++i) {
        invalidateChildren(selfAppendBones[i]);
    }
    for (int i = 0; i < selfAppendBoneCount; ++i) {
        int index = selfAppendBones[i];
        float ratio = bones[index].getAppendRatio();
        const float *localMat = bones[index].getCurrentMat();
        float *localAppend = bones[index].getLocalMatWithAppend();

        translateM2(matrixTmp, localAppend, localMat[12] * ratio, localMat[13] * ratio,
                    localMat[14] * ratio);
        matrixToQuaternion(vecTmp, localMat);
        if (fabsf(vecTmp[0]) < 1 - 1e-6f) {
            float angle = acos(vecTmp[0]) * 2 * ratio;
            rotateM(matrixTmp, (float) (angle * RAD_TO_DEG), vecTmp[1], vecTmp[2], vecTmp[3]);
        }

        floatArrayCopy(matrixTmp, localAppend, 16);

        int parent = bones[index].getParent();
        const float *position = bones[index].getPosition();
        if (parent >= 0 && parent != index) {
            translateM2(matrixTmp, localAppend, -position[0], -position[1], -position[2]);
            translateMPre(matrixTmp, position[0], position[1], position[2]);
            multiplyMM(finalBoneMats + (bones[index].getActualIndex() * 16),
                       finalBoneMats + (bones[parent].getActualIndex() * 16), matrixTmp);
        } else {
            translateM2(finalBoneMats + (bones[index].getActualIndex() * 16), localAppend,
                        -position[0], -position[1], -position[2]);
            translateMPre(finalBoneMats + (bones[index].getActualIndex() * 16), position[0],
                          position[1], position[2]);
        }
        boneStateIds[index] = currentPassId;
    }
    for (int i = 0; i < boneCount; ++i) {
        if (boneStateIds[i] != currentPassId)
            updateIKMatrix(i);
    }
}

void PMXObject::updateBoneMats()
{
    currentPassId ^= 1;
    for (int i = 0; i < boneCount; ++i) {
        calculateBone(i);
    }
    calculateIK();
}

void PMXObject::rotateBone(int index, float a, float x, float y, float z)
{
    bones[index].rotateBy(a, x, y, z);
    newBoneTransform = true;
}

void PMXObject::translateBone(int index, float x, float y, float z)
{
    bones[index].translationBy(x, y, z);
    LOG_PRINTF("bone name = %s", bones[index].getName());
    newBoneTransform = true;
}
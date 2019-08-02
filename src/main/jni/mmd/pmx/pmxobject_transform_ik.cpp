//
// Created by wjy50 on 19-8-1.
//

#include <cmath>

#include "pmxobject.h"
#include "../../matrix/matrix.h"
#include "../../vector/vector.h"
#include "../../quaternion/quaternion.h"
#include "../../utils/mathutils.h"

using namespace std;

void PMXObject::calculateIK()
{
    float ikPosition[4];
    float targetPosition[4];
    float linkNodePosition[4];
    float toIk[3];
    float toTarget[3];
    float ikDir[3];
    float targetDir[3];
    float rotateAxis[4];
    float rotateAxisInWorld[4];
    float rotateQuaternion[4];
    float nodeRotateQuaternion[4];
    float euler[3];
    for (int i = 0; i < ikCount; ++i) {
        bool ok = false;
        bool needUpdate = false;
        PMXBone &bone = bones[ikIndices[i]];
        PMXBoneIK &boneIK = *(bone.getBoneIK());
        PMXBone &target = bones[boneIK.getTarget()];
        multiplyMV(ikPosition, finalBoneMats + (bone.getActualIndex() * 16), bone.getPosition());
        for (int currentIndex = 0; currentIndex < boneIK.getIkChainLength(); ++currentIndex) {
            PMXBoneIKChainNode &ikNode = *boneIK.getIkChainNodeAt(currentIndex);
            PMXBone &linkNode = bones[ikNode.getBoneIndex()];
            // LOG_PRINTF("%d %d", info.boneSize, ikNode->getBoneIndex());
            multiplyMV(targetPosition, finalBoneMats + (target.getActualIndex() * 16),
                       target.getPosition());
            multiplyMV(linkNodePosition, finalBoneMats + (linkNode.getActualIndex() * 16),
                       linkNode.getPosition());

            if (distance3(ikPosition, targetPosition) < 1e-6f) {
                ok = boneIK.getIkChainLength() == 1;
                break;
            }

            subtractVector3(toIk, ikPosition, linkNodePosition);
            subtractVector3(toTarget, targetPosition, linkNodePosition);

            normalize3(ikDir, toIk);
            normalize3(targetDir, toTarget);

            float p = dotProduct3(targetDir, ikDir);
            if (abs(p) >= 1 - 1e-6f) {
                if (boneIK.getIkChainLength() > 1)
                    continue;
                else {
                    ok = true;
                    break;
                }
            }
            float angle = acos(p);
            crossProduct(rotateAxis, targetDir, ikDir);
            normalize3(rotateAxis, rotateAxis);
            rotateAxis[3] = 0;
            invertM(matrixTmp, finalBoneMats + linkNode.getActualIndex() * 16);
            multiplyMV(rotateAxisInWorld, matrixTmp, rotateAxis);
            float loopAngleLimit = boneIK.getLoopAngleLimit();
            angle = clamp(-loopAngleLimit, loopAngleLimit, angle);
            setRotateM(matrixTmp, static_cast<float>(angle * RAD_TO_DEG), rotateAxisInWorld[0],
                       rotateAxisInWorld[1], rotateAxisInWorld[2]);
            if (ikNode.isLimited()) {
                matrixToQuaternion(rotateQuaternion, matrixTmp);
                matrixToQuaternion(nodeRotateQuaternion, linkNode.getLocalMat());
                multiplyQuaternionWXYZ(nodeRotateQuaternion, nodeRotateQuaternion, rotateQuaternion);
                quaternionToEuler(euler, nodeRotateQuaternion);
                euler[0] = -clamp(ikNode.getLowLimit()[0], ikNode.getHighLimit()[0], -euler[0]);
                euler[1] = clamp(ikNode.getLowLimit()[1], ikNode.getHighLimit()[1], euler[1]);
                euler[2] = clamp(ikNode.getLowLimit()[2], ikNode.getHighLimit()[2], euler[2]);
                eulerToQuaternion(nodeRotateQuaternion, euler);
                if (abs(nodeRotateQuaternion[0]) >= 1 - 1e-6f)
                    continue;
                float a = acos(nodeRotateQuaternion[0]) * 2;
                linkNode.enableIKMat();
                setRotateM(linkNode.getIKMat(), static_cast<float>(a * RAD_TO_DEG),
                        nodeRotateQuaternion[1], nodeRotateQuaternion[2], nodeRotateQuaternion[3]);
            } else {
                linkNode.enableIKMat();
                multiplyMM(linkNode.getIKMat(), linkNode.getLocalMat(), matrixTmp);
            }
            needUpdate = true;
            boneStateIds[boneIK.getTarget()] = static_cast<char>(currentPassId ^ 1);
            for (int j = 0; j <= currentIndex; ++j)
                boneStateIds[boneIK.getIkChainNodeAt(j)->getBoneIndex()] = static_cast<char>(currentPassId ^ 1);
            updateIKMatrix(boneIK.getTarget());
            for (int j = 0; j <= currentIndex; ++j) {
                if (boneStateIds[boneIK.getIkChainNodeAt(j)->getBoneIndex()] != currentPassId)
                    updateIKMatrix(boneIK.getIkChainNodeAt(j)->getBoneIndex());
            }
        }
        if (!ok) {
            int loopCount = boneIK.getLoopCount();
            for (int j = 1; j < loopCount; ++j) {
                for (int currentIndex = 0; currentIndex < boneIK.getIkChainLength(); ++currentIndex) {
                    PMXBoneIKChainNode &ikNode = *boneIK.getIkChainNodeAt(currentIndex);
                    PMXBone &linkNode = bones[ikNode.getBoneIndex()];
                    multiplyMV(targetPosition, finalBoneMats + (target.getActualIndex() * 16),
                               target.getPosition());
                    multiplyMV(linkNodePosition, finalBoneMats + (linkNode.getActualIndex() * 16),
                               linkNode.getPosition());

                    if (j > 1 && distance3(ikPosition, targetPosition) < 1e-6f)
                        break;

                    subtractVector3(toIk, ikPosition, linkNodePosition);
                    subtractVector3(toTarget, targetPosition, linkNodePosition);

                    normalize3(ikDir, toIk);
                    normalize3(targetDir, toTarget);

                    float p = dotProduct3(targetDir, ikDir);
                    float angle;
                    int ikChainLen = boneIK.getIkChainLength();
                    if (ikChainLen > 1 && j < 2 && currentIndex + 1 == ikChainLen) {
                        angle = 0.2f;
                        rotateAxisInWorld[0] = 1;
                        rotateAxisInWorld[1] = rotateAxisInWorld[2] = 0;
                    } else {
                        if (abs(p) > 1 - 1e-6f)
                            continue;
                        angle = acos(p);
                        crossProduct(rotateAxis, targetDir, ikDir);
                        normalize3(rotateAxis, rotateAxis);
                        invertM(matrixTmp, finalBoneMats + (linkNode.getActualIndex() * 16));
                        multiplyMV(rotateAxisInWorld, matrixTmp, rotateAxis);
                    }
                    float loopAngleLimit = boneIK.getLoopAngleLimit();
                    angle = clamp(-loopAngleLimit, loopAngleLimit, angle);
                    if (ikNode.isLimited()) {
                        setRotateM(matrixTmp, (float) (angle * RAD_TO_DEG), rotateAxisInWorld[0],
                                   rotateAxisInWorld[1], rotateAxisInWorld[2]);
                        matrixToQuaternion(rotateQuaternion, matrixTmp);
                        matrixToQuaternion(nodeRotateQuaternion, linkNode.getCurrentMat());
                        multiplyQuaternionWXYZ(nodeRotateQuaternion, nodeRotateQuaternion,
                                               rotateQuaternion);
                        quaternionToEuler(euler, nodeRotateQuaternion);
                        euler[0] = -clamp(ikNode.getLowLimit()[0], ikNode.getHighLimit()[0],
                                          -euler[0]);
                        euler[1] = clamp(ikNode.getLowLimit()[1], ikNode.getHighLimit()[1],
                                         euler[1]);
                        euler[2] = clamp(ikNode.getLowLimit()[2], ikNode.getHighLimit()[2],
                                         euler[2]);
                        eulerToQuaternion(nodeRotateQuaternion, euler);
                        if (abs(nodeRotateQuaternion[0]) >= 1 - 1e-6f)
                            continue;
                        float a = acos(nodeRotateQuaternion[0]) * 2;
                        linkNode.enableIKMat();
                        setRotateM(linkNode.getIKMat(), static_cast<float>(a * RAD_TO_DEG),
                                   nodeRotateQuaternion[1], nodeRotateQuaternion[2],
                                   nodeRotateQuaternion[3]);
                    } else {
                        if (linkNode.getIKMat())
                            rotateM(linkNode.getIKMat(), static_cast<float>(angle * RAD_TO_DEG),
                                    rotateAxisInWorld[0], rotateAxisInWorld[1],
                                    rotateAxisInWorld[2]);
                        else {
                            linkNode.enableIKMat();
                            rotateM2(linkNode.getIKMat(), linkNode.getLocalMat(),
                                     static_cast<float>(angle * RAD_TO_DEG), rotateAxisInWorld[0],
                                     rotateAxisInWorld[1], rotateAxisInWorld[2]);
                        }
                    }
                    needUpdate = true;
                    boneStateIds[boneIK.getTarget()] = static_cast<char>(currentPassId ^ 1);
                    for (int k = 0; k <= currentIndex; ++k)
                        boneStateIds[boneIK.getIkChainNodeAt(k)->getBoneIndex()] = static_cast<char>(currentPassId ^ 1);
                    updateIKMatrix(boneIK.getTarget());
                    for (int k = 0; k <= currentIndex; ++k) {
                        if (boneStateIds[boneIK.getIkChainNodeAt(k)->getBoneIndex()] != currentPassId)
                            updateIKMatrix(boneIK.getIkChainNodeAt(k)->getBoneIndex());
                    }
                }
            }
        }
        if (needUpdate) {
            invalidateChildren(boneIK.getTarget());
            for (int j = 0; j < boneIK.getIkChainLength(); ++j) {
                if (boneStateIds[boneIK.getIkChainNodeAt(j)->getBoneIndex()] == currentPassId)
                    invalidateChildren(boneIK.getIkChainNodeAt(j)->getBoneIndex());
            }
            for (int j = 0; j < boneCount; ++j) {
                int appendParent = (bones + j)->getAppendParent();
                if (appendParent >= 0 && boneStateIds[appendParent] != currentPassId) {
                    invalidateChildren(j);
                }
            }
            for (int j = 0; j < boneCount; ++j) {
                if (boneStateIds[j] != currentPassId) {
                    updateIKMatrix(j);
                }
            }
        }
    }
}

void PMXObject::updateIKMatrix(int index)
{
    if (boneStateIds[index] != currentPassId) {
        int appendParent = bones[index].getAppendParent();
        float *position = bones[index].getPosition();
        int parent = bones[index].getParent();
        if (appendParent >= 0 && appendParent != index) {
            updateIKMatrix(appendParent);
            const float *appendParentLocal = bones[appendParent].getLocalMatWithAppend()
                                             ? bones[appendParent].getLocalMatWithAppend()
                                             : bones[appendParent].getCurrentMat();
            if (bones[index].getAppendRatio() == 1) {
                multiplyMM(bones[index].getLocalMatWithAppend(), bones[index].getCurrentMat(),
                           appendParentLocal);
            } else {
                matrixToQuaternion(vecTmp, appendParentLocal);
                translateM2(bones[index].getLocalMatWithAppend(), bones[index].getCurrentMat(),
                            appendParentLocal[12], appendParentLocal[13], appendParentLocal[14]);
                if (fabsf(vecTmp[0]) < 1 - 1e-6f) {
                    float angle = acosf(vecTmp[0]) * 2;
                    rotateM(bones[index].getLocalMatWithAppend(), (float) (angle * RAD_TO_DEG),
                            vecTmp[1], vecTmp[2], vecTmp[3]);
                } else {
                    floatArrayCopy(bones[index].getCurrentMat(),
                                   bones[index].getLocalMatWithAppend(), 16);
                }
            }
            if (parent >= 0 && parent != index) {
                updateIKMatrix(parent);
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
                updateIKMatrix(parent);
                translateM2(matrixTmp, bones[index].getCurrentMat(), -position[0], -position[1],
                            -position[2]);
                translateMPre(matrixTmp, position[0], position[1], position[2]);
                multiplyMM(finalBoneMats + (bones[index].getActualIndex() * 16),
                           finalBoneMats + (bones[parent].getActualIndex() * 16), matrixTmp);
            } else {
                translateM2(finalBoneMats + (bones[index].getActualIndex() * 16),
                            bones[index].getCurrentMat(), -position[0], -position[1], -position[2]);
                translateMPre(finalBoneMats + (bones[index].getActualIndex() * 16), position[0],
                              position[1], position[2]);
            }
            boneStateIds[index] = currentPassId;
        }
    }
}
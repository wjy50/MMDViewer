//
// Created by wjy50 on 2018/2/7.
//

#include "pmxreader.h"
#include "../../matrix/matrix.h"
#include "../../gl/shaderloader.h"
#include "../../vector/vector.h"
#include "../../quaternion/quaternion.h"
#include "../../utils/mathutils.h"
#include "../../utils/debugutils.h"
#include <cmath>
#include <cstring>

using namespace std;

PMXReader::PMXReader(const char *filePath)
: vertices(nullptr), textures(nullptr), materials(nullptr), bones(nullptr), morphs(nullptr),
  materialIndices(nullptr), materialDiffuses(nullptr), materialSpecular(nullptr),
  materialAmbient(nullptr), materialEdgeColors(nullptr), bonePositions(nullptr),
  localBoneMats(nullptr), finalBoneMats(nullptr), boneStateIds(nullptr),
  ikIndices(nullptr), selfAppendBones(nullptr)
{
    ifstream file(filePath, ios::binary);
    PMXHeader header;
    file.read(reinterpret_cast<char *>(&header), sizeof(PMXHeader));
    switch (header.magic & 0xffffff) {
        case 0x584d50://"PMX "
            float version;
            file.read(reinterpret_cast<char *>(&version), sizeof(float));
            LOG_PRINTF("version=%f", version);
            if (version > 2.0) {
                //not supported
                break;
            } else {
                readInfo(file);

                readNameAndDescription(file);

                readVerticesAndIndices(file);

                readTextures(file, filePath);

                readMaterials(file);

                readBones(file);

                readMorphs(file);

                file.close();

                LOG_PRINTLN("load from file finished");

                //end reading
                hasVertexBuffers = false;
                hasBoneBuffers = false;
                newBoneTransform = true;

                directBoneCount = 0;
                glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxVertexShaderVecCount);
                LOG_PRINTF("max vec4=%d", maxVertexShaderVecCount);
                if (boneCount > 0) {
                    unsigned int *boneRecord = new unsigned int[boneCount];
                    for (int i = 0; i < boneCount; ++i) {
                        boneRecord[i] = 0;
                    }
                    for (int i = 0; i < vertexCount; ++i) {
                        int boneCount = vertices[i].getBoneCount();
                        for (int j = 0; j < boneCount; ++j) {
                            int bone = vertices[i].getBoneAt(j);
                            if (!boneRecord[bone]) {
                                boneRecord[bone] = 1;
                                bones[bone].setActualIndex(directBoneCount++);
                            }
                            vertices[i].setBoneAt(j, bones[bone].getActualIndex());
                        }
                    }
                    LOG_PRINTF("direct bone count=%d", directBoneCount);
                    int k = directBoneCount;
                    selfAppendBoneCount = 0;
                    for (int i = 0; i < boneCount; ++i) {
                        if (!boneRecord[i])
                            bones[i].setActualIndex(k++);
                        if (bones[i].getAppendParent() == i) {
                            selfAppendBoneCount++;
                            bones[i].setAppendFromSelf(true);
                        }
                    }
                    delete[] boneRecord;
                    if (selfAppendBoneCount > 0) {
                        selfAppendBones = new int[selfAppendBoneCount];
                        selfAppendBoneCount = 0;
                        for (int i = 0; i < boneCount; ++i) {
                            if (bones[i].isAppendFromSelf())
                                selfAppendBones[selfAppendBoneCount++] = i;
                        }
                    }

                    for (int i = 0; i < boneCount; ++i) {
                        int parent = bones[i].getParent();
                        if (parent >= 0 && parent != i)
                            bones[parent].addChild(i);
                    }

                    if (ikCount > 0) {
                        k = 0;
                        ikIndices = new int[ikCount];
                        for (int i = 0; i < boneCount; ++i) {
                            if (bones[i].getBoneIK())
                                ikIndices[k++] = i;
                        }
                    }
                }

                genVertexBuffers();
                genBoneBuffers();
                initShader();

                vertexChangeStart = vertexChangeEnd = -1;
                uvChangeStart = uvChangeEnd = -1;
            }
            break;
        case 0x786d50://"Pmx " pmx v1
            file.close();
            break;
        default:
            LOG_PRINTF("magic=%d", header.magic);
            file.close();
            throw runtime_error("Not a PMX file");
    }
}

void PMXReader::calculateIK()
{
    float ikPosition[4];
    float targetPosition[4];
    float linkNodePosition[4];
    float toIk[4];
    float toTarget[4];
    float ikDir[4];
    float targetDir[4];
    float rotateAxis[4];
    float rotateAxisInWorld[4];
    float rotateQuaternion[4];
    float nodeRotateQuaternion[4];
    float euler[3];
    for (int i = 0; i < ikCount; ++i) {
        bool ok = false;
        bool needUpdate = false;
        PMXBone *bone = bones + ikIndices[i];
        PMXBoneIK *boneIK = bone->getBoneIK();
        PMXBone *target = bones + boneIK->getTarget();
        multiplyMV(ikPosition, finalBoneMats + (bone->getActualIndex() * 16), bone->getPosition());
        for (int currentIndex = 0; currentIndex < boneIK->getIkChainLength(); ++currentIndex) {
            PMXBoneIKChainNode *ikNode = boneIK->getIkChainNodeAt(currentIndex);
            PMXBone *linkNode = bones + ikNode->getBoneIndex();
            // LOG_PRINTF("%d %d", info.boneSize, ikNode->getBoneIndex());
            multiplyMV(targetPosition, finalBoneMats + (target->getActualIndex() * 16),
                       target->getPosition());
            multiplyMV(linkNodePosition, finalBoneMats + (linkNode->getActualIndex() * 16),
                       linkNode->getPosition());

            subtractVector3(toIk, ikPosition, linkNodePosition);
            subtractVector3(toTarget, targetPosition, linkNodePosition);

            if (distance3(toIk, toTarget) < 1e-6f) {
                ok = boneIK->getIkChainLength() == 1;
                break;
            }

            normalize3(ikDir, toIk);
            normalize3(targetDir, toTarget);

            float p = dotProduct3(targetDir, ikDir);
            if (abs(p) >= 1 - 1e-6f) {
                if (boneIK->getIkChainLength() > 1)
                    continue;
                else {
                    ok = true;
                    break;
                }
            }
            float angle = cos(p);
            crossProduct(rotateAxis, targetDir, ikDir);
            rotateAxis[3] = 0;
            invertM(matrixTmp, finalBoneMats + (linkNode->getActualIndex() * 16));
            multiplyMV(rotateAxisInWorld, matrixTmp, rotateAxis);
            float loopAngleLimit = boneIK->getLoopAngleLimit();
            angle = clamp(-loopAngleLimit, loopAngleLimit, angle);
            setRotateM(matrixTmp, static_cast<float>(angle * RAD_TO_DEG), rotateAxisInWorld[0],
                       rotateAxisInWorld[1], rotateAxisInWorld[2]);
            if (ikNode->isLimited()) {
                matrixToQuaternion(rotateQuaternion, matrixTmp);
                matrixToQuaternion(nodeRotateQuaternion, linkNode->getLocalMat());
                multiplyQuaternionWXYZ(nodeRotateQuaternion, nodeRotateQuaternion, rotateQuaternion);
                quaternionToEuler(euler, nodeRotateQuaternion);
                euler[0] = -clamp(ikNode->getLowLimit()[0], ikNode->getHighLimit()[0], -euler[0]);
                euler[1] = clamp(ikNode->getLowLimit()[1], ikNode->getHighLimit()[1], euler[1]);
                euler[2] = clamp(ikNode->getLowLimit()[2], ikNode->getHighLimit()[2], euler[2]);
                eulerToQuaternion(nodeRotateQuaternion, euler);
                if (abs(nodeRotateQuaternion[0]) >= 1 - 1e-6f)
                    continue;
                float a = acos(nodeRotateQuaternion[0]) * 2;
                linkNode->enableIKMat();
                setRotateM(linkNode->getIKMat(), static_cast<float>(a * RAD_TO_DEG), nodeRotateQuaternion[1],
                           nodeRotateQuaternion[2], nodeRotateQuaternion[3]);
            } else {
                linkNode->enableIKMat();
                multiplyMM(linkNode->getIKMat(), linkNode->getLocalMat(), matrixTmp);
            }
            needUpdate = true;
            boneStateIds[boneIK->getTarget()] = static_cast<char>(currentPassId ^ 1);
            for (int j = 0; j <= currentIndex; ++j)
                boneStateIds[boneIK->getIkChainNodeAt(j)->getBoneIndex()] = static_cast<char>(currentPassId ^ 1);
            updateIKMatrix(boneIK->getTarget());
            for (int j = 0; j <= currentIndex; ++j) {
                if (boneStateIds[boneIK->getIkChainNodeAt(j)->getBoneIndex()] != currentPassId)
                    updateIKMatrix(boneIK->getIkChainNodeAt(j)->getBoneIndex());
            }
        }
        if (!ok) {
            int loopCount = boneIK->getLoopCount();
            for (int j = 1; j < loopCount; ++j) {
                for (int currentIndex = 0; currentIndex < boneIK->getIkChainLength(); ++currentIndex) {
                    PMXBoneIKChainNode *ikNode = boneIK->getIkChainNodeAt(currentIndex);
                    PMXBone *linkNode = bones + ikNode->getBoneIndex();
                    multiplyMV(targetPosition, finalBoneMats + (target->getActualIndex() * 16),
                               target->getPosition());
                    multiplyMV(linkNodePosition, finalBoneMats + (linkNode->getActualIndex() * 16),
                               linkNode->getPosition());

                    subtractVector3(toIk, ikPosition, linkNodePosition);
                    subtractVector3(toTarget, targetPosition, linkNodePosition);

                    if (j > 1 && distance3(toIk, toTarget) < 1e-6f)
                        break;

                    normalize3(ikDir, toIk);
                    normalize3(targetDir, toTarget);

                    float p = dotProduct3(targetDir, ikDir);
                    float angle;
                    if (boneIK->getIkChainLength() > 1) {
                        if (j < 2 && currentIndex + 1 == boneIK->getIkChainLength()) {
                            angle = 0.2f;
                            rotateAxisInWorld[0] = 1;
                            rotateAxisInWorld[1] = rotateAxisInWorld[2] = 0;
                        } else {
                            if (abs(p) > 1 - 1e-6f)
                                continue;
                            angle = acos(p);
                            crossProduct(rotateAxis, targetDir, ikDir);
                            invertM(matrixTmp, finalBoneMats + (linkNode->getActualIndex() * 16));
                            multiplyMV(rotateAxisInWorld, matrixTmp, rotateAxis);
                        }
                    } else {
                        if (abs(p) > 1 - 1e-6f)continue;
                        angle = acos(p);
                        crossProduct(rotateAxis, targetDir, ikDir);
                        invertM(matrixTmp, finalBoneMats + (linkNode->getActualIndex() * 16));
                        multiplyMV(rotateAxisInWorld, matrixTmp, rotateAxis);
                    }
                    float loopAngleLimit = boneIK->getLoopAngleLimit();
                    angle = clamp(-loopAngleLimit, loopAngleLimit, angle);
                    if (ikNode->isLimited()) {
                        setRotateM(matrixTmp, (float) (angle * RAD_TO_DEG), rotateAxisInWorld[0],
                                   rotateAxisInWorld[1], rotateAxisInWorld[2]);
                        matrixToQuaternion(rotateQuaternion, matrixTmp);
                        matrixToQuaternion(nodeRotateQuaternion, linkNode->getCurrentMat());
                        multiplyQuaternionWXYZ(nodeRotateQuaternion, nodeRotateQuaternion,
                                               rotateQuaternion);
                        quaternionToEuler(euler, nodeRotateQuaternion);
                        euler[0] = -clamp(ikNode->getLowLimit()[0], ikNode->getHighLimit()[0],
                                          -euler[0]);
                        euler[1] = clamp(ikNode->getLowLimit()[1], ikNode->getHighLimit()[1],
                                         euler[1]);
                        euler[2] = clamp(ikNode->getLowLimit()[2], ikNode->getHighLimit()[2],
                                         euler[2]);
                        eulerToQuaternion(nodeRotateQuaternion, euler);
                        if (abs(nodeRotateQuaternion[0]) >= 1 - 1e-6f)
                            continue;
                        float a = acosf(nodeRotateQuaternion[0]) * 2;
                        linkNode->enableIKMat();
                        setRotateM(linkNode->getIKMat(), static_cast<float>(a * RAD_TO_DEG),
                                   nodeRotateQuaternion[1], nodeRotateQuaternion[2],
                                   nodeRotateQuaternion[3]);
                    } else {
                        if (linkNode->getIKMat())
                            rotateM(linkNode->getIKMat(), static_cast<float>(angle * RAD_TO_DEG),
                                    rotateAxisInWorld[0], rotateAxisInWorld[1],
                                    rotateAxisInWorld[2]);
                        else {
                            linkNode->enableIKMat();
                            rotateM2(linkNode->getIKMat(), linkNode->getLocalMat(),
                                     static_cast<float>(angle * RAD_TO_DEG), rotateAxisInWorld[0],
                                     rotateAxisInWorld[1], rotateAxisInWorld[2]);
                        }
                    }
                    needUpdate = true;
                    boneStateIds[boneIK->getTarget()] = static_cast<char>(currentPassId ^ 1);
                    for (int k = 0; k <= currentIndex; ++k)
                        boneStateIds[boneIK->getIkChainNodeAt(k)->getBoneIndex()] = static_cast<char>(currentPassId ^ 1);
                    updateIKMatrix(boneIK->getTarget());
                    for (int k = 0; k <= currentIndex; ++k) {
                        if (boneStateIds[boneIK->getIkChainNodeAt(k)->getBoneIndex()] != currentPassId)
                            updateIKMatrix(boneIK->getIkChainNodeAt(k)->getBoneIndex());
                    }
                }
            }
        }
        if (needUpdate) {
            invalidateChildren(boneIK->getTarget());
            for (int j = 0; j < boneIK->getIkChainLength(); ++j) {
                if (boneStateIds[boneIK->getIkChainNodeAt(j)->getBoneIndex()] == currentPassId)
                    invalidateChildren(boneIK->getIkChainNodeAt(j)->getBoneIndex());
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

void PMXReader::invalidateChildren(int index)
{
    boneStateIds[index] = (char) (currentPassId ^ 1);
    int childCount = bones[index].getChildCount();
    for (int i = 0; i < childCount; ++i) {
        int child = bones[index].getChildAt(i);
        if (boneStateIds[child] == currentPassId)invalidateChildren(child);
    }
}

void PMXReader::updateIKMatrix(int index)
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

void PMXReader::calculateBone(int index)
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

void PMXReader::updateSelfAppend()
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

void PMXReader::updateBoneMats()
{
    currentPassId ^= 1;
    for (int i = 0; i < boneCount; ++i) {
        calculateBone(i);
    }
    calculateIK();
}

void PMXReader::updateModelState()
{
    if (newBoneTransform) {
        newBoneTransform = false;
        updateBoneMats();
    }
    if (selfAppendBoneCount > 0)updateSelfAppend();
    if (vertexChangeStart != vertexChangeEnd) {
        vertexChangeStart = (vertexChangeStart * 4) - vertexChangeStart;
        vertexChangeEnd = (vertexChangeEnd * 4) - vertexChangeEnd;
        glBindBuffer(GL_ARRAY_BUFFER, bufferIds[0]);
        int size = vertexChangeEnd - vertexChangeStart;
        glBufferSubData(GL_ARRAY_BUFFER, vertexChangeStart * 4, size * 4,
                        vertexCoordinates + vertexChangeStart);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        vertexChangeStart = vertexChangeEnd = -1;
    }
    if (uvChangeStart != uvChangeEnd) {
        uvChangeStart <<= 1;
        uvChangeEnd = uvChangeEnd * 2;
        glBindBuffer(GL_ARRAY_BUFFER, bufferIds[3]);
        int size = uvChangeEnd - uvChangeStart;
        glBufferSubData(GL_ARRAY_BUFFER, uvChangeStart * 4, size * 4, uvs + uvChangeStart);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        uvChangeStart = uvChangeEnd = 0xffffffff;
    }
}

void PMXReader::draw(const float *viewMat, const float *projectionMat,
                     EnvironmentLight &environmentLight)
{
    glUseProgram(mProgram);

    glEnableVertexAttribArray(mPositionHandle);
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds[0]);
    glVertexAttribPointer(mPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(mNormalHandle);
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds[1]);
    glVertexAttribPointer(mNormalHandle, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(mUVHandle);
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds[3]);
    glVertexAttribPointer(mUVHandle, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(mBonesHandle);
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds[4]);
    glVertexAttribPointer(mBonesHandle, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(mWeightsHandle);
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds[5]);
    glVertexAttribPointer(mWeightsHandle, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUniformMatrix4fv(mViewMatHandle, 1, GL_FALSE, viewMat);
    glUniformMatrix4fv(mProjectionMatHandle, 1, GL_FALSE, projectionMat);
    glUniformMatrix4fv(mBoneMatsHandle, directBoneCount, GL_FALSE, finalBoneMats);

    glUniform3fv(mSunPositionHandle, 1, environmentLight.getSunPosition());
    glUniform3fv(mSunLightStrengthHandle, 1, environmentLight.getSunLightStrength());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds[2]);
    samplers[2] = environmentLight.getShadowMapTextureUnit();
    if (samplers[2] >= 0) {
        glUniform1iv(mSamplersHandle, 3, samplers);
        glUniformMatrix4fv(mSunMatHandle, 1, GL_FALSE, environmentLight.getSunMatForDraw());
    } else {
        glUniform1iv(mSamplersHandle, 2, samplers);
    }
    for (int i = 0; i < materialCount; ++i) {
        if (materials[materialIndices[i]].getDiffuse()[3] != 0) {
            glUniform3fv(mAmbientHandle, 1, materials[materialIndices[i]].getAmbient());
            glUniform4fv(mDiffuseHandle, 1, materials[materialIndices[i]].getDiffuse());
            glUniform4fv(mSpecularHandle, 1, materials[materialIndices[i]].getSpecular());
            if (materials[materialIndices[i]].getTextureState()) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D,
                              textures[materials[materialIndices[i]].getTextureIndex()].getTextureId());
            }
            if (materials[materialIndices[i]].getSphereState()) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D,
                              textures[materials[materialIndices[i]].getSphereIndex()].getTextureId());
            }
            glUniform3i(mTextureModesHandle, materials[materialIndices[i]].getTextureState(),
                        materials[materialIndices[i]].getSphereState(),
                        samplers[2] >= 0 && materials[materialIndices[i]].acceptShadow());
            glUniform4fv(mTextureCoefficientHandle, 1,
                         materials[materialIndices[i]].getTextureCoefficient());
            glUniform4fv(mSphereCoefficientHandle, 1,
                         materials[materialIndices[i]].getSphereCoefficient());
            if (materials[materialIndices[i]].isDoubleSided())
                glDisable(GL_CULL_FACE);
            else
                glEnable(GL_CULL_FACE);
            glDrawElements(materials[materialIndices[i]].getDrawMode(),
                           materials[materialIndices[i]].getIndexCount(), GL_UNSIGNED_INT,
                           (const void *) materials[materialIndices[i]].getOffset());
        }
    }
    glEnable(GL_CULL_FACE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDisableVertexAttribArray(mWeightsHandle);
    glDisableVertexAttribArray(mBonesHandle);
    glDisableVertexAttribArray(mUVHandle);
    glDisableVertexAttribArray(mNormalHandle);
    glDisableVertexAttribArray(mPositionHandle);
}

void PMXReader::drawShadowMap(EnvironmentLight &environmentLight)
{
    glUseProgram(mShadowProgram);

    glEnableVertexAttribArray(mShadowPositionHandle);
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds[0]);
    glVertexAttribPointer(mShadowPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(mShadowBonesHandle);
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds[4]);
    glVertexAttribPointer(mShadowBonesHandle, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(mShadowWeightHandle);
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds[5]);
    glVertexAttribPointer(mShadowWeightHandle, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUniformMatrix4fv(mShadowSunMatHandle, 1, GL_FALSE, environmentLight.getSunMat());
    glUniformMatrix4fv(mShadowBoneMatsHandle, directBoneCount, GL_FALSE, finalBoneMats);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds[2]);
    for (int i = 0; i < materialCount; ++i) {
        if (materials[materialIndices[i]].castShadow() &&
            materials[materialIndices[i]].getDiffuse()[3] != 0) {
            if (materials[materialIndices[i]].isDoubleSided())glDisable(GL_CULL_FACE);
            else glEnable(GL_CULL_FACE);
            glDrawElements(materials[materialIndices[i]].getDrawMode(),
                           materials[materialIndices[i]].getIndexCount(), GL_UNSIGNED_INT,
                           (const void *) materials[materialIndices[i]].getOffset());
        }
    }
    glEnable(GL_CULL_FACE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDisableVertexAttribArray(mShadowPositionHandle);
    glDisableVertexAttribArray(mShadowBonesHandle);
    glDisableVertexAttribArray(mShadowWeightHandle);
}

void PMXReader::genBoneBuffers()
{
    if (!hasBoneBuffers && vertices && bones) {
        auto boneIndices = make_unique_array<float[]>(vertexCount * 4);
        auto weights = make_unique_array<float[]>(vertexCount * 4);
        for (int i = 0; i < vertexCount; ++i) {
            int offset = i * 4;
            int boneCount = vertices[i].getBoneCount();
            for (int j = 0; j < boneCount; ++j) {
                boneIndices[offset + j] = vertices[i].getBoneAt(j);
                weights[offset + j] = vertices[i].getWeightAt(j);
            }
            if (boneCount != 4)
                boneIndices[offset + boneCount] = -1;
        }
        glGenBuffers(2, bufferIds + 4);
        glBindBuffer(GL_ARRAY_BUFFER, bufferIds[4]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 4, boneIndices.get(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, bufferIds[5]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 4, weights.get(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        hasBoneBuffers = true;
    }
}

void PMXReader::genVertexBuffers()
{
    if (!hasVertexBuffers && vertexCoordinates && normals && indices && uvs) {
        glGenBuffers(4, bufferIds);
        glBindBuffer(GL_ARRAY_BUFFER, bufferIds[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 3, vertexCoordinates,
                     GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, bufferIds[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 3, normals, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds[2]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indexCount, indices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, bufferIds[3]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 2, uvs, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        hasVertexBuffers = true;
    }
}

void PMXReader::initShader()
{
    mProgram = glCreateProgram();
    mVertexShader = glCreateShader(GL_VERTEX_SHADER);
    mFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    int v;
    glGetShaderiv(mVertexShader, GL_SHADING_LANGUAGE_VERSION, &v);
    LOG_PRINTF("-----------------glsl ver = %d", v);
    int length;
    unique_ptr<char[]> s(nullptr);
    loadShader("/data/data/com.wjy50.app.mmdviewer/files/pmxVertexShader.fs", &length, s);
    int ind = -1;
    int lim = length - 4;
    for (int i = 0; i < lim; ++i) {
        if (s[i] == '-' && s[i + 1] == '*' && s[i + 2] == 'd' && s[i + 3] == '-') {
            ind = i;
            break;
        }
    }
    int b = directBoneCount;
    for (int i = 3; i >= 0; --i) {
        if (b != 0) {
            s[ind + i] = (char) ((b % 10) + '0');
            b /= 10;
        } else
            s[ind + i] = ' ';
    }
    const char *ptrS = s.get();
    glShaderSource(mVertexShader, 1, &ptrS, &length);
    glCompileShader(mVertexShader);
    LOG_PRINTLN(ptrS);
    /*glGetShaderInfoLog(mVertexShader, length, &length, s);
    LOG_PRINTLN(s);*/
    loadShader("/data/data/com.wjy50.app.mmdviewer/files/pmxFragmentShader.fs", &length, s);
    ptrS = s.get();
    glShaderSource(mFragmentShader, 1, &ptrS, &length);
    glCompileShader(mFragmentShader);
    glAttachShader(mProgram, mVertexShader);
    glAttachShader(mProgram, mFragmentShader);
    glLinkProgram(mProgram);

    mPositionHandle = static_cast<GLuint>(glGetAttribLocation(mProgram, "aPosition"));
    LOG_PRINTF("err=%d", glGetError());
    mNormalHandle = static_cast<GLuint>(glGetAttribLocation(mProgram, "aNormal"));
    mUVHandle = static_cast<GLuint>(glGetAttribLocation(mProgram, "aUV"));
    mBonesHandle = static_cast<GLuint>(glGetAttribLocation(mProgram, "aBones"));
    mWeightsHandle = static_cast<GLuint>(glGetAttribLocation(mProgram, "aWeights"));

    mSunPositionHandle = glGetUniformLocation(mProgram, "uSunPosition");
    mViewMatHandle = glGetUniformLocation(mProgram, "uViewMat");
    mProjectionMatHandle = glGetUniformLocation(mProgram, "uProjectionMat");
    mBoneMatsHandle = glGetUniformLocation(mProgram, "uBoneMats");
    mSunMatHandle = glGetUniformLocation(mProgram, "uSunMat");

    mSunLightStrengthHandle = glGetUniformLocation(mProgram, "uSunLightStrength");
    mAmbientHandle = glGetUniformLocation(mProgram, "uAmbient");
    mDiffuseHandle = glGetUniformLocation(mProgram, "uDiffuse");
    mSpecularHandle = glGetUniformLocation(mProgram, "uSpecular");
    mSamplersHandle = glGetUniformLocation(mProgram, "uSamplers");
    mTextureModesHandle = glGetUniformLocation(mProgram, "uTextureModes");
    mTextureCoefficientHandle = glGetUniformLocation(mProgram, "uTextureCoefficient");
    mSphereCoefficientHandle = glGetUniformLocation(mProgram, "uSphereCoefficient");
}

void PMXReader::initShadowMapShader()
{
    mShadowProgram = glCreateProgram();
    mShadowVertexShader = glCreateShader(GL_VERTEX_SHADER);
    mShadowFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    int length;
    unique_ptr<char[]> s(nullptr);
    loadShader("/data/data/com.wjy50.app.mmdviewer/files/pmxShadowVertexShader.fs", &length, s);
    int ind = -1;
    int lim = length - 4;
    for (int i = 0; i < lim; ++i) {
        if (s[i] == '-' && s[i + 1] == '*' && s[i + 2] == 'd' && s[i + 3] == '-') {
            ind = i;
            break;
        }
    }
    int b = directBoneCount;
    for (int i = 3; i >= 0; --i) {
        if (b != 0) {
            s[ind + i] = static_cast<char>((b % 10) + '0');
            b /= 10;
        } else s[ind + i] = ' ';
    }
    const char *ptrS = s.get();
    glShaderSource(mShadowVertexShader, 1, &ptrS, &length);
    glCompileShader(mShadowVertexShader);
    LOG_PRINTLN(s.get());
    loadShader("/data/data/com.wjy50.app.mmdviewer/files/pmxShadowFragmentShader.fs", &length, s);
    ptrS = s.get();
    glShaderSource(mShadowFragmentShader, 1, &ptrS, &length);
    glCompileShader(mShadowFragmentShader);
    glAttachShader(mShadowProgram, mShadowVertexShader);
    glAttachShader(mShadowProgram, mShadowFragmentShader);
    glLinkProgram(mShadowProgram);

    mShadowPositionHandle = (GLuint) glGetAttribLocation(mShadowProgram, "aPosition");
    LOG_PRINTF("err=%d", glGetError());
    mShadowBonesHandle = (GLuint) glGetAttribLocation(mShadowProgram, "aBones");
    mShadowWeightHandle = (GLuint) glGetAttribLocation(mShadowProgram, "aWeights");

    mShadowSunMatHandle = glGetUniformLocation(mShadowProgram, "uSunMat");
    mShadowBoneMatsHandle = glGetUniformLocation(mShadowProgram, "uBoneMats");
}

void PMXReader::readInfo(ifstream &file)
{
    char rSize;
    file.read(&rSize, sizeof(char));
    file.read(reinterpret_cast<char *>(&info), sizeof(info));
    if (rSize > 8) {
        file.seekg(rSize - 8, ios::cur);
    }
    encoding = (MStringEncoding) info.encoding;
}

void PMXReader::readNameAndDescription(ifstream &file)
{
    name.readString(file, encoding, UTF_8);
    nameE.readString(file, encoding, UTF_8);
    desc.readString(file, encoding, UTF_8);
    descE.readString(file, encoding, UTF_8);
}

void PMXReader::readVerticesAndIndices(ifstream &file)
{
    file.read(reinterpret_cast<char *>(&vertexCount), sizeof(int));
    if (vertexCount > 0) {
        vertexCoordinates = new float[vertexCount * 3];
        normals = new float[vertexCount * 3];
        uvs = new float[vertexCount * 2];
        vertices = new PMXVertex[vertexCount];
        for (int i = 0; i < vertexCount; ++i) {
            vertices[i].read(file, &info, vertexCoordinates + (i * 4) - i, normals + (i * 4) - i,
                             uvs + (i * 2));
        }
    }
    file.read(reinterpret_cast<char *>(&indexCount), sizeof(int));
    if (indexCount > 0) {
        faceCount = indexCount / 3;
        indices = new unsigned int[indexCount];
        for (int i = 0; i < indexCount; ++i) {
            indices[i] = 0;
            file.read(reinterpret_cast<char *>(indices + i), info.vertexSize);
            if (i % 3 == 2) {
                indices[i] ^= indices[i - 1];
                indices[i - 1] = indices[i] ^ indices[i - 1];
                indices[i] ^= indices[i - 1];
            }
        }
    }
}

void PMXReader::readTextures(ifstream &file, const char *filePath)
{
    file.read(reinterpret_cast<char *>(&textureCount), sizeof(int));
    if (textureCount > 0) {
        textures = new PMXTexture[textureCount];
        int pathLength = -1;
        for (int i = 0;; ++i) {
            if (filePath[i] == '/')pathLength = i + 1;
            else if (filePath[i] == 0)break;
        }
        for (int i = 0; i < textureCount; ++i) {
            textures[i].read(file, encoding, filePath, pathLength);
        }
    }
}

void PMXReader::readMaterials(ifstream &file)
{
    file.read(reinterpret_cast<char *>(&materialCount), sizeof(int));
    if (materialCount > 0) {
        materials = new PMXMaterial[materialCount];
        materialIndices = new int[materialCount];
        materialDiffuses = new float[materialCount * 4];
        materialSpecular = new float[materialCount * 4];
        materialAmbient = new float[(materialCount * 4) - materialCount];
        materialEdgeColors = new float[materialCount * 4];
        int lastNoAlphaIndex = 0;
        int lastAlphaIndex = materialCount - 1;
        long offset = 0;
        for (int i = 0; i < materialCount; ++i) {
            materials[i].read(file, info.texSize, encoding, materialDiffuses + (i * 4),
                              materialSpecular + (i * 4),
                              materialAmbient + (i * 4) - i,
                              materialEdgeColors + (i * 4));
            bool hasAlpha = materials[i].getDiffuse()[3] != 1;
            GLsizei textureIndex = materials[i].getTextureIndex();
            if (textureIndex < textureCount) {
                textures[textureIndex].initGLTexture();
                hasAlpha |= textures[textureIndex].hasAlpha();
            }
            GLsizei sphereIndex = materials[i].getSphereIndex();
            if (sphereIndex < textureCount)textures[sphereIndex].initGLTexture();
            materials[i].onTextureLoaded(
                    textureIndex < textureCount && textures[textureIndex].getTextureId() != 0,
                    sphereIndex < textureCount && textures[sphereIndex].getTextureId() != 0);
            materials[i].setOffset(offset * 4);
            offset += materials[i].getIndexCount();
            if (hasAlpha) materialIndices[lastAlphaIndex--] = i;
            else materialIndices[lastNoAlphaIndex++] = i;
        }
        int halfAlphaCount = (materialCount - lastNoAlphaIndex) / 2;
        for (int i = 0; i < halfAlphaCount; ++i) {
            materialIndices[materialCount - i - 1] ^= materialIndices[lastNoAlphaIndex + i];
            materialIndices[lastNoAlphaIndex + i] ^= materialIndices[materialCount - i - 1];
            materialIndices[materialCount - i - 1] ^= materialIndices[lastNoAlphaIndex + i];
        }
    }
}

void PMXReader::readBones(ifstream &file)
{
    file.read(reinterpret_cast<char *>(&boneCount), sizeof(int));
    ikIndices = 0;
    if (boneCount > 0) {
        bones = new PMXBone[boneCount];
        bonePositions = new float[boneCount * 4];
        localBoneMats = new float[boneCount * 16];
        finalBoneMats = new float[boneCount * 16];
        boneStateIds = new char[boneCount];
        currentPassId = 0;
        ikCount = 0;
        for (int i = 0; i < boneCount; ++i) {
            bones[i].read(file, info.boneSize, encoding, localBoneMats + (i * 16),
                          bonePositions + (i * 4));
            if (bones[i].getBoneIK())ikCount++;
            boneStateIds[i] = 0;
        }
    }
}

void PMXReader::readMorphs(ifstream &file)
{
    file.read(reinterpret_cast<char *>(&morphCount), sizeof(int));
    if (morphCount > 0) {
        morphs = new PMXMorph[morphCount];
        for (int i = 0; i < morphCount; ++i) {
            morphs[i].read(file, &info);
        }
    }
}

void PMXReader::performMaterialAddOperation(int index, PMXMaterialMorphData *data, float f)
{
    PMXMaterial *material = materials + index;

    const float *initialDiffuse = material->getInitialDiffuse();
    const float *diffuse = data->getDiffuse();
    material->setDiffuse(initialDiffuse[0] + diffuse[0] * f, initialDiffuse[1] + diffuse[1] * f,
                         initialDiffuse[2] + diffuse[2] * f, initialDiffuse[3] + diffuse[3] * f);

    const float *initialSpecular = material->getInitialSpecular();
    const float *specular = data->getSpecular();
    material->setSpecular(initialSpecular[0] + specular[0] * f,
                          initialSpecular[1] + specular[1] * f,
                          initialSpecular[2] + specular[2] * f,
                          initialSpecular[3] + specular[3] * f);

    const float *initialAmbient = material->getInitialAmbient();
    const float *ambient = data->getAmbient();
    material->setAmbient(initialAmbient[0] + ambient[0] * f, initialAmbient[1] + ambient[1] * f,
                         initialAmbient[2] + ambient[2] * f);

    const float *initialEdgeColor = material->getInitialEdgeColor();
    const float *edgeColor = data->getEdgeColor();
    material->setEdgeColor(initialEdgeColor[0] + edgeColor[0] * f,
                           initialEdgeColor[1] + edgeColor[1] * f,
                           initialEdgeColor[2] + edgeColor[2] * f,
                           initialEdgeColor[3] + edgeColor[3] * f);

    material->setEdgeSize(material->getInitialEdgeSize() + data->getEdgeSize() * f);

    const float *textureCoefficient = data->getTextureCoefficient();
    material->setTextureCoefficient(1 + textureCoefficient[0] * f, 1 + textureCoefficient[1] * f,
                                    1 + textureCoefficient[2] * f, 1 + textureCoefficient[3] * f);

    const float *sphereCoefficient = data->getSphereCoefficient();
    material->setSphereCoefficient(1 + sphereCoefficient[0] * f, 1 + sphereCoefficient[1] * f,
                                   1 + sphereCoefficient[2] * f, 1 + sphereCoefficient[3] * f);
}

void PMXReader::performMaterialMulOperation(int index, PMXMaterialMorphData *data, float f)
{
    PMXMaterial *material = materials + index;

    const float *initialDiffuse = material->getInitialDiffuse();
    const float *diffuse = data->getDiffuse();
    material->setDiffuse(initialDiffuse[0] * (1 + (diffuse[0] - 1) * f),
                         initialDiffuse[1] * (1 + (diffuse[1] - 1) * f),
                         initialDiffuse[2] * (1 + (diffuse[2] - 1) * f),
                         initialDiffuse[3] * (1 + (diffuse[3] - 1) * f));

    const float *initialSpecular = material->getInitialSpecular();
    const float *specular = data->getSpecular();
    material->setSpecular(initialSpecular[0] * (1 + (specular[0] - 1) * f),
                          initialSpecular[1] * (1 + (specular[1] - 1) * f),
                          initialSpecular[2] * (1 + (specular[2] - 1) * f),
                          initialSpecular[3] * (1 + (specular[3] - 1) * f));

    const float *initialAmbient = material->getInitialAmbient();
    const float *ambient = data->getAmbient();
    material->setAmbient(initialAmbient[0] * (1 + (ambient[0] - 1) * f),
                         initialAmbient[1] * (1 + (ambient[1] - 1) * f),
                         initialAmbient[2] * (1 + (ambient[2] - 1) * f));

    const float *initialEdgeColor = material->getInitialEdgeColor();
    const float *edgeColor = data->getEdgeColor();
    material->setEdgeColor(initialEdgeColor[0] * (1 + (edgeColor[0] - 1) * f),
                           initialEdgeColor[1] * (1 + (edgeColor[1] - 1) * f),
                           initialEdgeColor[2] * (1 + (edgeColor[2] - 1) * f),
                           initialEdgeColor[3] * (1 + (edgeColor[3] - 1) * f));

    material->setEdgeSize(material->getInitialEdgeSize() * (1 + (data->getEdgeSize() - 1) * f));

    const float *textureCoefficient = data->getSphereCoefficient();
    material->setTextureCoefficient(1 + (textureCoefficient[0] - 1) * f,
                                    1 + (textureCoefficient[1] - 1) * f,
                                    1 + (textureCoefficient[2] - 1) * f,
                                    1 + (textureCoefficient[3] - 1) * f);

    const float *sphereCoefficient = data->getSphereCoefficient();
    material->setSphereCoefficient(1 + (sphereCoefficient[0] - 1) * f,
                                   1 + (sphereCoefficient[1] - 1) * f,
                                   1 + (sphereCoefficient[2] - 1) * f,
                                   1 + (sphereCoefficient[3] - 1) * f);
}

void PMXReader::rotateBone(int index, float a, float x, float y, float z)
{
    bones[index].rotateBy(a, x, y, z);
    newBoneTransform = true;
}

void PMXReader::translateBone(int index, float x, float y, float z)
{
    bones[index].translationBy(x, y, z);
    LOG_PRINTF("bone name = %s", bones[index].getName());
    newBoneTransform = true;
}

int PMXReader::getMorphCount() const
{
    return morphCount;
}

const char *PMXReader::getMorphName(int index) const
{
    return morphs[index].getName();
}

float PMXReader::getMorphFraction(int index) const
{
    return morphs[index].getFraction();
}

void PMXReader::setMorphFraction(int index, float f)
{
    f = clamp(0, 1, f);
    float delta = morphs[index].setFraction(f);
    int count = morphs[index].getMorphDataCount();
    switch (morphs[index].getKind()) {
        case 0:
        case 9:
            for (int i = 0; i < count; ++i) {
                PMXGroupMorphData *data = (PMXGroupMorphData *) morphs[index].getDataAt(i);
                setMorphFraction(data->getIndex(), f * data->getRatio());
            }
            break;
        case 1:
            for (int i = 0; i < count; ++i) {
                PMXVertexMorphData *data = (PMXVertexMorphData *) morphs[index].getDataAt(i);
                const float *offset = data->getOffset();
                int vertexIndex = data->getIndex();
                const float *initialPosition = vertices[vertexIndex].getInitialCoordinate();
                vertices[vertexIndex].setPosition(initialPosition[0] + offset[0] * f,
                                                  initialPosition[1] + offset[1] * f,
                                                  initialPosition[2] + offset[2] * f);
                if (vertexChangeStart < 0 ||
                    vertexIndex < vertexChangeStart)
                    vertexChangeStart = vertexIndex;
                if (vertexChangeEnd < 0 ||
                    vertexIndex + 1 > vertexChangeEnd)
                    vertexChangeEnd = vertexIndex + 1;
            }
            break;
        case 2:
            for (int i = 0; i < count; ++i) {
                PMXBoneMorphData *data = (PMXBoneMorphData *) morphs[index].getDataAt(i);
                const float *translation = data->getTranslation();
                translateBone(data->getIndex(), translation[0] * delta, translation[1] * delta,
                              translation[2] * delta);
                const float *rotation = data->getRotation();
                if (fabsf(rotation[3]) > 1e-6f)
                    rotateBone(data->getIndex(), rotation[3] * delta, rotation[0], rotation[1],
                               rotation[2]);
            }
            break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            for (int i = 0; i < count; ++i) {
                PMXUVMorphData *data = (PMXUVMorphData *) morphs[index].getDataAt(i);
                const float *offset = data->getOffset();
                int vertexIndex = data->getIndex();
                const float *initialUV = vertices[vertexIndex].getInitialUV();
                vertices[vertexIndex].setUV(initialUV[0] + offset[0] * f,
                                            initialUV[1] + offset[1] * f);
                if (uvChangeStart < 0 || vertexIndex < uvChangeStart)
                    uvChangeStart = vertexIndex;
                if (uvChangeEnd < 0 || vertexIndex + 1 > uvChangeEnd)
                    uvChangeEnd = vertexIndex + 1;
            }
            break;
        case 8:
            for (int i = 0; i < count; ++i) {
                PMXMaterialMorphData *data = (PMXMaterialMorphData *) morphs[index].getDataAt(i);
                int materialIndex = data->getIndex();
                if (materialIndex < materialCount) {
                    if (data->getOperation() == 1) {
                        performMaterialAddOperation(materialIndex, data, f);
                    } else {
                        performMaterialMulOperation(materialIndex, data, f);
                    }
                } else {
                    if (data->getOperation() == 1) {
                        for (int j = 0; j < materialCount; ++j) {
                            performMaterialAddOperation(j, data, f);
                        }
                    } else {
                        for (int j = 0; j < materialCount; ++j) {
                            performMaterialMulOperation(j, data, f);
                        }
                    }
                }
            }
            break;
        default:
            break;
    }
}

PMXReader::~PMXReader()
{
    delete[] vertexCoordinates;
    delete[] normals;
    delete[] indices;
    delete[] uvs;
    delete[] vertices;
    delete[] textures;
    delete[] materials;
    delete[] materialIndices;
    delete[] materialDiffuses;
    delete[] materialSpecular;
    delete[] materialAmbient;
    delete[] materialEdgeColors;
    delete[] bones;
    delete[] bonePositions;
    delete[] localBoneMats;
    delete[] finalBoneMats;
    delete[] ikIndices;
    delete[] boneStateIds;
    delete[] morphs;
    delete[] selfAppendBones;
}
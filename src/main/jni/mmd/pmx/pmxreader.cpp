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
#include <math.h>

PMXReader::PMXReader(const char* filePath) {
    FILE* file=fopen(filePath,"rb");
    LOG_PRINTF("file=%p",file);
    PMXHeader header;
    fread(&header, sizeof(PMXHeader),1,file);
    switch (header.magic&0xffffff)
    {
        case 0x584d50://"PMX "
            float version;
            fread(&version, sizeof(float),1,file);
            LOG_PRINTF("version=%f",version);
            if(version > 2.0)
            {
                //not supported
                break;
            }
            else
            {
                readInfo(file);

                readNameAndDescription(file);

                readVerticesAndIndices(file);

                readTextures(file, filePath);

                readMaterials(file);

                readBones(file);

                readMorphs(file);

                fclose(file);

                //end reading
                hasVertexBuffers = false;
                hasBoneBuffers= false;
                newBoneTransform= true;

                directBoneCount=0;
                glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS,&maxVertexShaderVecCount);
                LOG_PRINTF("max vec4=%d",maxVertexShaderVecCount);
                if(boneCount > 0)
                {
                    unsigned int * boneRecord=new unsigned int[boneCount];
                    for (int i = 0; i < boneCount; ++i) {
                        boneRecord[i]=0;
                    }
                    for (int i = 0; i < vertexCount; ++i) {
                        int boneCount=vertices[i].getBoneCount();
                        for (int j = 0; j < boneCount; ++j) {
                            unsigned int bone=vertices[i].getBoneAt(j);
                            if(!boneRecord[bone])
                            {
                                boneRecord[bone]=1;
                                bones[bone].setActualIndex(directBoneCount++);
                            }
                            vertices[i].setBoneAt(j,bones[bone].getActualIndex());
                        }
                    }
                    LOG_PRINTF("direct bone count=%d",directBoneCount);
                    unsigned int k=directBoneCount;
                    selfAppendBoneCount=0;
                    for (int i = 0; i < boneCount; ++i) {
                        if (!boneRecord[i])bones[i].setActualIndex(k++);
                        if(bones[i].getAppendParent() == i)
                        {
                            selfAppendBoneCount++;
                            bones[i].setAppendFromSelf(true);
                        }
                    }
                    delete [] boneRecord;
                    if(selfAppendBoneCount > 0)
                    {
                        selfAppendBones=new unsigned int[selfAppendBoneCount];
                        selfAppendBoneCount=0;
                        for (unsigned int i = 0; i < boneCount; ++i) {
                            if(bones[i].isAppendFromSelf())selfAppendBones[selfAppendBoneCount++]=i;
                        }
                    }

                    for (unsigned int i = 0; i < boneCount; ++i) {
                        unsigned int parent=bones[i].getParent();
                        if(parent < boneCount && parent != i)bones[parent].addChild(i);
                    }

                    if(ikCount > 0)
                    {
                        k=0;
                        ikIndices=new unsigned int[ikCount];
                        for (unsigned int i = 0; i < boneCount; ++i) {
                            if(bones[i].getBoneIK())ikIndices[k++]=i;
                        }
                    }
                }

                genVertexBuffers();
                genBoneBuffers();
                initShader();

                vertexChangeStart=vertexChangeEnd=0xffffffff;
                uvChangeStart=uvChangeEnd=0xffffffff;
            }
            break;
        case 0x786d50://"Pmx " pmx v1
            fclose(file);
            break;
        default:
            LOG_PRINTF("magic=%d",header.magic);
            fclose(file);
            break;
    }
}
void PMXReader::calculateIK() {
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
        bool ok= false;
        bool needUpdate= false;
        PMXBone* bone=bones+ikIndices[i];
        PMXBoneIK* boneIK=bone->getBoneIK();
        PMXBone* target=bones+boneIK->getTarget();
        multiplyMV(ikPosition,finalBoneMats+(bone->getActualIndex()<<4),bone->getPosition());
        for (int currentIndex = 0; currentIndex < boneIK->getIkChainLength(); ++currentIndex) {
            PMXBoneIKChainNode* ikNode=boneIK->getIkChainNodeAt(currentIndex);
            PMXBone* linkNode=bones+ikNode->getBoneIndex();
            multiplyMV(targetPosition,finalBoneMats+(target->getActualIndex()<<4),target->getPosition());
            multiplyMV(linkNodePosition,finalBoneMats+(linkNode->getActualIndex()<<4),linkNode->getPosition());

            subtractVector3(toIk,ikPosition,linkNodePosition);
            subtractVector3(toTarget,targetPosition,linkNodePosition);

            if(distance3(toIk,toTarget) < 1e-6f)
            {
                ok=boneIK->getIkChainLength() == 1;
                break;
            }

            normalize3(ikDir,toIk);
            normalize3(targetDir,toTarget);

            float p=dotProduct3(targetDir,ikDir);
            if(fabsf(p) >= 1-1e-6f)
            {
                if(boneIK->getIkChainLength() > 1)continue;
                else
                {
                    ok=true;
                    break;
                }
            }
            float angle=(float)acos(p);
            crossProduct(rotateAxis,targetDir,ikDir);
            rotateAxis[3]=0;
            invertM(matrixTmp,finalBoneMats+(linkNode->getActualIndex()<<4));
            multiplyMV(rotateAxisInWorld,matrixTmp,rotateAxis);
            float loopAngleLimit=boneIK->getLoopAngleLimit();
            if(angle > loopAngleLimit)angle=loopAngleLimit;
            else if(angle < -loopAngleLimit)angle=-loopAngleLimit;
            setRotateM(matrixTmp,(float)(angle*RAD_TO_DEG),rotateAxisInWorld[0],rotateAxisInWorld[1],rotateAxisInWorld[2]);
            if(ikNode->isLimited())
            {
                matrixToQuaternion(rotateQuaternion,matrixTmp);
                matrixToQuaternion(nodeRotateQuaternion,linkNode->getLocalMat());
                multiplyQuaternionWXYZ(nodeRotateQuaternion,nodeRotateQuaternion,rotateQuaternion);
                quaternionToEuler(euler, nodeRotateQuaternion);
                euler[0]=-clamp(ikNode->getLowLimit()[0],ikNode->getHighLimit()[0],-euler[0]);
                euler[1]=clamp(ikNode->getLowLimit()[1],ikNode->getHighLimit()[1],euler[1]);
                euler[2]=clamp(ikNode->getLowLimit()[2],ikNode->getHighLimit()[2],euler[2]);
                eulerToQuaternion(nodeRotateQuaternion,euler);
                if(fabs(nodeRotateQuaternion[0]) >= 1-1e-6f)continue;
                float a= acosf(nodeRotateQuaternion[0])*2;
                linkNode->setIKMat(ikNode->getIKMat());
                setRotateM(linkNode->getIKMat(),(float)(a*RAD_TO_DEG),nodeRotateQuaternion[1],nodeRotateQuaternion[2],nodeRotateQuaternion[3]);
            }
            else
            {
                linkNode->setIKMat(ikNode->getIKMat());
                multiplyMM(linkNode->getIKMat(),linkNode->getLocalMat(),matrixTmp);
            }
            needUpdate=true;
            boneStateIds[boneIK->getTarget()]= (char) (currentPassId ^ 1);
            for (int j = 0; j <= currentIndex; ++j)boneStateIds[boneIK->getIkChainNodeAt(j)->getBoneIndex()]= (char) (currentPassId ^ 1);
            updateIKMatrix(boneIK->getTarget());
            for (int j = 0; j <= currentIndex; ++j) {
                if(boneStateIds[boneIK->getIkChainNodeAt(j)->getBoneIndex()] != currentPassId)updateIKMatrix(boneIK->getIkChainNodeAt(j)->getBoneIndex());
            }
        }
        if(!ok)
        {
            int loopCount=boneIK->getLoopCount();
            for (int j = 1; j < loopCount; ++j) {
                for (int currentIndex = 0; currentIndex < boneIK->getIkChainLength(); ++currentIndex) {
                    PMXBoneIKChainNode *ikNode = boneIK->getIkChainNodeAt(currentIndex);
                    PMXBone *linkNode = bones + ikNode->getBoneIndex();
                    multiplyMV(targetPosition, finalBoneMats + (target->getActualIndex() << 4), target->getPosition());
                    multiplyMV(linkNodePosition, finalBoneMats + (linkNode->getActualIndex() << 4), linkNode->getPosition());

                    subtractVector3(toIk, ikPosition, linkNodePosition);
                    subtractVector3(toTarget, targetPosition, linkNodePosition);

                    if(j > 1 && distance3(toIk,toTarget) < 1e-6f)break;

                    normalize3(ikDir,toIk);
                    normalize3(targetDir,toTarget);

                    float p=dotProduct3(targetDir,ikDir);
                    float angle;
                    if(boneIK->getIkChainLength() > 1)
                    {
                        if(j < 2 && currentIndex+1 == boneIK->getIkChainLength())
                        {
                            angle=0.2f;
                            rotateAxisInWorld[0]=1;
                            rotateAxisInWorld[1]=rotateAxisInWorld[2]=0;
                        }
                        else
                        {
                            if(fabsf(p) > 1-1e-6f)continue;
                            angle=acosf(p);
                            crossProduct(rotateAxis,targetDir,ikDir);
                            invertM(matrixTmp,finalBoneMats+(linkNode->getActualIndex()<<4));
                            multiplyMV(rotateAxisInWorld,matrixTmp,rotateAxis);
                        }
                    }
                    else
                    {
                        if(fabsf(p) > 1-1e-6f)continue;
                        angle=acosf(p);
                        crossProduct(rotateAxis,targetDir,ikDir);
                        invertM(matrixTmp,finalBoneMats+(linkNode->getActualIndex()<<4));
                        multiplyMV(rotateAxisInWorld,matrixTmp,rotateAxis);
                    }
                    float loopAngleLimit=boneIK->getLoopAngleLimit();
                    if(angle > loopAngleLimit)angle=loopAngleLimit;
                    else if(angle < -loopAngleLimit)angle=-loopAngleLimit;
                    if(ikNode->isLimited())
                    {
                        setRotateM(matrixTmp,(float)(angle*RAD_TO_DEG),rotateAxisInWorld[0],rotateAxisInWorld[1],rotateAxisInWorld[2]);
                        matrixToQuaternion(rotateQuaternion,matrixTmp);
                        matrixToQuaternion(nodeRotateQuaternion,linkNode->getCurrentMat());
                        multiplyQuaternionWXYZ(nodeRotateQuaternion,nodeRotateQuaternion,rotateQuaternion);
                        quaternionToEuler(euler,nodeRotateQuaternion);
                        euler[0]=-clamp(ikNode->getLowLimit()[0],ikNode->getHighLimit()[0],-euler[0]);
                        euler[1]=clamp(ikNode->getLowLimit()[1],ikNode->getHighLimit()[1],euler[1]);
                        euler[2]=clamp(ikNode->getLowLimit()[2],ikNode->getHighLimit()[2],euler[2]);
                        eulerToQuaternion(nodeRotateQuaternion,euler);
                        if(fabs(nodeRotateQuaternion[0]) >= 1-1e-6f)continue;
                        float a = acosf(nodeRotateQuaternion[0])*2;
                        if(!linkNode->getIKMat())linkNode->setIKMat(ikNode->getIKMat());
                        setRotateM(linkNode->getIKMat(),(float)(a*RAD_TO_DEG),nodeRotateQuaternion[1],nodeRotateQuaternion[2],nodeRotateQuaternion[3]);
                    }
                    else
                    {
                        if(linkNode->getIKMat())rotateM(linkNode->getIKMat(),(float)(angle*RAD_TO_DEG),rotateAxisInWorld[0],rotateAxisInWorld[1],rotateAxisInWorld[2]);
                        else
                        {
                            linkNode->setIKMat(ikNode->getIKMat());
                            rotateM2(linkNode->getIKMat(),linkNode->getLocalMat(),(float)(angle*RAD_TO_DEG),rotateAxisInWorld[0],rotateAxisInWorld[1],rotateAxisInWorld[2]);
                        }
                    }
                    needUpdate=true;
                    boneStateIds[boneIK->getTarget()]= (char) (currentPassId ^ 1);
                    for (int k = 0; k <= currentIndex; ++k)boneStateIds[boneIK->getIkChainNodeAt(k)->getBoneIndex()]= (char) (currentPassId ^ 1);
                    updateIKMatrix(boneIK->getTarget());
                    for (int k = 0; k <= currentIndex; ++k) {
                        if(boneStateIds[boneIK->getIkChainNodeAt(k)->getBoneIndex()] != currentPassId)updateIKMatrix(boneIK->getIkChainNodeAt(k)->getBoneIndex());
                    }
                }
            }
        }
        if (needUpdate)
        {
            invalidateChildren(boneIK->getTarget());
            for (int j = 0; j < boneIK->getIkChainLength(); ++j) {
                if(boneStateIds[boneIK->getIkChainNodeAt(j)->getBoneIndex()] == currentPassId)invalidateChildren(boneIK->getIkChainNodeAt(j)->getBoneIndex());
            }
            for (unsigned int j = 0; j < boneCount; ++j) {
                unsigned int appendParent=(bones+j)->getAppendParent();
                if(appendParent < boneCount && boneStateIds[appendParent] != currentPassId)
                {
                    invalidateChildren(j);
                }
            }
            for (unsigned int j = 0; j < boneCount; ++j) {
                if(boneStateIds[j] != currentPassId)
                {
                    updateIKMatrix(j);
                }
            }
        }
    }
}
void PMXReader::invalidateChildren(unsigned int index) {
    boneStateIds[index]= (char) (currentPassId ^ 1);
    int childCount=bones[index].getChildCount();
    for (int i = 0; i < childCount; ++i) {
        unsigned int child=bones[index].getChildAt(i);
        if(boneStateIds[child] == currentPassId)invalidateChildren(child);
    }
}
void PMXReader::updateIKMatrix(unsigned int index) {
    if(boneStateIds[index] != currentPassId)
    {
        unsigned int appendParent=bones[index].getAppendParent();
        float * position=bones[index].getPosition();
        unsigned int parent=bones[index].getParent();
        if(appendParent < boneCount && appendParent != index)
        {
            updateIKMatrix(appendParent);
            const float * appendParentLocal=bones[appendParent].getLocalMatWithAppend() ? bones[appendParent].getLocalMatWithAppend() : bones[appendParent].getCurrentMat();
            if(bones[index].getAppendRatio() == 1)
            {
                multiplyMM(bones[index].getLocalMatWithAppend(),bones[index].getCurrentMat(),appendParentLocal);
            }
            else
            {
                matrixToQuaternion(vecTmp,appendParentLocal);
                translateM2(bones[index].getLocalMatWithAppend(),bones[index].getCurrentMat(),appendParentLocal[12],appendParentLocal[13],appendParentLocal[14]);
                if(fabsf(vecTmp[0]) < 1-1e-6f)
                {
                    float angle=acosf(vecTmp[0])*2;
                    rotateM(bones[index].getLocalMatWithAppend(), (float) (angle * RAD_TO_DEG), vecTmp[1], vecTmp[2], vecTmp[3]);
                }
                else
                {
                    floatArrayCopy(bones[index].getCurrentMat(),bones[index].getLocalMatWithAppend(),16);
                }
            }
            if(parent < boneCount && parent != index)
            {
                updateIKMatrix(parent);
                translateM2(matrixTmp,bones[index].getLocalMatWithAppend(),-position[0],-position[1],-position[2]);
                translateMPre(matrixTmp,position[0],position[1],position[2]);
                multiplyMM(finalBoneMats+(bones[index].getActualIndex()<<4),finalBoneMats+(bones[parent].getActualIndex()<<4),matrixTmp);
            }
            else
            {
                translateM2(finalBoneMats+(bones[index].getActualIndex()<<4),bones[index].getLocalMatWithAppend(),-position[0],-position[1],-position[2]);
                translateMPre(finalBoneMats+(bones[index].getActualIndex()<<4),position[0],position[1],position[2]);
            }
            boneStateIds[index]=currentPassId;
        }
        else
        {
            if(parent < boneCount && parent != index)
            {
                updateIKMatrix(parent);
                translateM2(matrixTmp,bones[index].getCurrentMat(),-position[0],-position[1],-position[2]);
                translateMPre(matrixTmp,position[0],position[1],position[2]);
                multiplyMM(finalBoneMats+(bones[index].getActualIndex()<<4),finalBoneMats+(bones[parent].getActualIndex()<<4),matrixTmp);
            }
            else
            {
                translateM2(finalBoneMats+(bones[index].getActualIndex()<<4),bones[index].getCurrentMat(),-position[0],-position[1],-position[2]);
                translateMPre(finalBoneMats+(bones[index].getActualIndex()<<4),position[0],position[1],position[2]);
            }
            boneStateIds[index]=currentPassId;
        }
    }
}
void PMXReader::calculateBone(unsigned int index) {
    if(boneStateIds[index] != currentPassId)
    {
        bones[index].setIKMat(0);
        unsigned int appendParent=bones[index].getAppendParent();
        float * position=bones[index].getPosition();
        unsigned int parent=bones[index].getParent();
        if(appendParent < boneCount && appendParent != index)
        {
            calculateBone(appendParent);
            const float * appendParentLocal=bones[appendParent].getLocalMatWithAppend() ? bones[appendParent].getLocalMatWithAppend() : bones[appendParent].getLocalMat();
            float ratio=bones[index].getAppendRatio();
            if(ratio == 1)
            {
                multiplyMM(bones[index].getLocalMatWithAppend(),bones[index].getLocalMat(),appendParentLocal);
            }
            else
            {
                matrixToQuaternion(vecTmp,appendParentLocal);
                translateM2(bones[index].getLocalMatWithAppend(),bones[index].getLocalMat(),appendParentLocal[12]*ratio,appendParentLocal[13]*ratio,appendParentLocal[14]*ratio);
                if(fabsf(vecTmp[0]) < 1-1e-6f)
                {
                    float angle=acosf(vecTmp[0])*2*ratio;
                    rotateM(bones[index].getLocalMatWithAppend(), (float) (angle * RAD_TO_DEG), vecTmp[1], vecTmp[2], vecTmp[3]);
                }
                else
                {
                    floatArrayCopy(bones[index].getLocalMat(),bones[index].getLocalMatWithAppend(),16);
                }
            }
            if(parent < boneCount && parent != index)
            {
                calculateBone(parent);
                translateM2(matrixTmp,bones[index].getLocalMatWithAppend(),-position[0],-position[1],-position[2]);
                translateMPre(matrixTmp,position[0],position[1],position[2]);
                multiplyMM(finalBoneMats+(bones[index].getActualIndex()<<4),finalBoneMats+(bones[parent].getActualIndex()<<4),matrixTmp);
            }
            else
            {
                translateM2(finalBoneMats+(bones[index].getActualIndex()<<4),bones[index].getLocalMatWithAppend(),-position[0],-position[1],-position[2]);
                translateMPre(finalBoneMats+(bones[index].getActualIndex()<<4),position[0],position[1],position[2]);
            }
            boneStateIds[index]=currentPassId;
        }
        else
        {
            if(parent < boneCount && parent != index)
            {
                calculateBone(parent);
                translateM2(matrixTmp,bones[index].getLocalMat(),-position[0],-position[1],-position[2]);
                translateMPre(matrixTmp,position[0],position[1],position[2]);
                multiplyMM(finalBoneMats+(bones[index].getActualIndex()<<4),finalBoneMats+(bones[parent].getActualIndex()<<4),matrixTmp);
            }
            else
            {
                translateM2(finalBoneMats+(bones[index].getActualIndex()<<4),bones[index].getLocalMat(),-position[0],-position[1],-position[2]);
                translateMPre(finalBoneMats+(bones[index].getActualIndex()<<4),position[0],position[1],position[2]);
            }
            boneStateIds[index]=currentPassId;
        }
    }
}
void PMXReader::updateSelfAppend() {
    for (unsigned int i = 0; i < selfAppendBoneCount; ++i) {
        invalidateChildren(selfAppendBones[i]);
    }
    for (int i = 0; i < selfAppendBoneCount; ++i) {
        unsigned int index=selfAppendBones[i];
        float ratio=bones[index].getAppendRatio();
        const float * localMat=bones[index].getCurrentMat();
        float * localAppend=bones[index].getLocalMatWithAppend();

        translateM2(matrixTmp,localAppend,localMat[12]*ratio,localMat[13]*ratio,localMat[14]*ratio);
        matrixToQuaternion(vecTmp,localMat);
        if(fabsf(vecTmp[0]) < 1-1e-6f)
        {
            float angle=acosf(vecTmp[0])*2*ratio;
            rotateM(matrixTmp, (float) (angle * RAD_TO_DEG), vecTmp[1], vecTmp[2], vecTmp[3]);
        }

        floatArrayCopy(matrixTmp,localAppend,16);

        unsigned int parent=bones[index].getParent();
        const float * position=bones[index].getPosition();
        if(parent < boneCount && parent != index)
        {
            translateM2(matrixTmp,localAppend,-position[0],-position[1],-position[2]);
            translateMPre(matrixTmp,position[0],position[1],position[2]);
            multiplyMM(finalBoneMats+(bones[index].getActualIndex()<<4),finalBoneMats+(bones[parent].getActualIndex()<<4),matrixTmp);
        }
        else
        {
            translateM2(finalBoneMats+(bones[index].getActualIndex()<<4),localAppend,-position[0],-position[1],-position[2]);
            translateMPre(finalBoneMats+(bones[index].getActualIndex()<<4),position[0],position[1],position[2]);
        }
        boneStateIds[index]=currentPassId;
    }
    for (unsigned int i = 0; i < boneCount; ++i) {
        if(boneStateIds[i] != currentPassId)updateIKMatrix(i);
    }
}
void PMXReader::updateBoneMats() {
    currentPassId^=1;
    for (unsigned int i = 0; i < boneCount; ++i) {
        calculateBone(i);
    }
    calculateIK();
}
void PMXReader::updateModelState() {
    if(newBoneTransform)
    {
        newBoneTransform= false;
        updateBoneMats();
    }
    if(selfAppendBoneCount > 0)updateSelfAppend();
    if(vertexChangeStart != vertexChangeEnd)
    {
        vertexChangeStart=(vertexChangeStart<<2)-vertexChangeStart;
        vertexChangeEnd=(vertexChangeEnd<<2)-vertexChangeEnd;
        glBindBuffer(GL_ARRAY_BUFFER,bufferIds[0]);
        unsigned int size=vertexChangeEnd-vertexChangeStart;
        glBufferSubData(GL_ARRAY_BUFFER,vertexChangeStart<<2,size<<2,vertexCoordinates+vertexChangeStart);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        vertexChangeStart=vertexChangeEnd=0xffffffff;
    }
    if(uvChangeStart != uvChangeEnd)
    {
        uvChangeStart<<=1;
        uvChangeEnd=uvChangeEnd<<1;
        glBindBuffer(GL_ARRAY_BUFFER,bufferIds[3]);
        unsigned int size=uvChangeEnd-uvChangeStart;
        glBufferSubData(GL_ARRAY_BUFFER,uvChangeStart<<2,size<<2,uvs+uvChangeStart);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        uvChangeStart=uvChangeEnd=0xffffffff;
    }
}
void PMXReader::draw(const float *viewMat, const float *projectionMat, EnvironmentLight* environmentLight) {
    glUseProgram(mProgram);

    glEnableVertexAttribArray(mPositionHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[0]);
    glVertexAttribPointer(mPositionHandle,3,GL_FLOAT, GL_FALSE,0,0);

    glEnableVertexAttribArray(mNormalHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[1]);
    glVertexAttribPointer(mNormalHandle,3,GL_FLOAT, GL_FALSE,0,0);

    glEnableVertexAttribArray(mUVHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[3]);
    glVertexAttribPointer(mUVHandle,2,GL_FLOAT, GL_FALSE,0,0);

    glEnableVertexAttribArray(mBonesHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[4]);
    glVertexAttribPointer(mBonesHandle,4,GL_FLOAT,GL_FALSE,0,0);

    glEnableVertexAttribArray(mWeightsHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[5]);
    glVertexAttribPointer(mWeightsHandle,4,GL_FLOAT,GL_FALSE,0,0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    glUniformMatrix4fv(mViewMatHandle,1,GL_FALSE,viewMat);
    glUniformMatrix4fv(mProjectionMatHandle,1,GL_FALSE,projectionMat);
    glUniformMatrix4fv(mBoneMatsHandle,directBoneCount,GL_FALSE,finalBoneMats);

    glUniform3fv(mSunPositionHandle,1,environmentLight->getSunPosition());
    glUniform3fv(mSunLightStrengthHandle,1,environmentLight->getSunLightStrength());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bufferIds[2]);
    samplers[2]=environmentLight->getShadowMapTextureUnit();
    if(samplers[2] >= 0)
    {
        glUniform1iv(mSamplersHandle,3,samplers);
        glUniformMatrix4fv(mSunMatHandle,1,GL_FALSE,environmentLight->getSunMatForDraw());
    }
    else
    {
        glUniform1iv(mSamplersHandle,2,samplers);
    }
    for (int i = 0; i < materialCount; ++i) {
        if(materials[materialIndices[i]].getDiffuse()[3] != 0)
        {
            glUniform3fv(mAmbientHandle,1,materials[materialIndices[i]].getAmbient());
            glUniform4fv(mDiffuseHandle,1,materials[materialIndices[i]].getDiffuse());
            glUniform4fv(mSpecularHandle,1,materials[materialIndices[i]].getSpecular());
            if(materials[materialIndices[i]].getTextureState())
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D,textures[materials[materialIndices[i]].getTextureIndex()].getTextureId());
            }
            if(materials[materialIndices[i]].getSphereState())
            {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D,textures[materials[materialIndices[i]].getSphereIndex()].getTextureId());
            }
            glUniform3i(mTextureModesHandle,materials[materialIndices[i]].getTextureState(),materials[materialIndices[i]].getSphereState(),samplers[2] >= 0 && materials[materialIndices[i]].acceptShadow());
            glUniform4fv(mTextureCoefficientHandle,1,materials[materialIndices[i]].getTextureCoefficient());
            glUniform4fv(mSphereCoefficientHandle,1,materials[materialIndices[i]].getSphereCoefficient());
            if(materials[materialIndices[i]].isDoubleSided())glDisable(GL_CULL_FACE);
            else glEnable(GL_CULL_FACE);
            glDrawElements(materials[materialIndices[i]].getDrawMode(),materials[materialIndices[i]].getIndexCount(),GL_UNSIGNED_INT,(const void*)materials[materialIndices[i]].getOffset());
        }
    }
    glEnable(GL_CULL_FACE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    glDisableVertexAttribArray(mWeightsHandle);
    glDisableVertexAttribArray(mBonesHandle);
    glDisableVertexAttribArray(mUVHandle);
    glDisableVertexAttribArray(mNormalHandle);
    glDisableVertexAttribArray(mPositionHandle);
}
void PMXReader::drawShadowMap(EnvironmentLight* environmentLight) {
    glUseProgram(mShadowProgram);

    glEnableVertexAttribArray(mShadowPositionHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[0]);
    glVertexAttribPointer(mShadowPositionHandle,3,GL_FLOAT, GL_FALSE,0,0);

    glEnableVertexAttribArray(mShadowBonesHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[4]);
    glVertexAttribPointer(mShadowBonesHandle,4,GL_FLOAT,GL_FALSE,0,0);

    glEnableVertexAttribArray(mShadowWeightHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[5]);
    glVertexAttribPointer(mShadowWeightHandle,4,GL_FLOAT,GL_FALSE,0,0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    glUniformMatrix4fv(mShadowSunMatHandle,1,GL_FALSE,environmentLight->getSunMat());
    glUniformMatrix4fv(mShadowBoneMatsHandle,directBoneCount,GL_FALSE,finalBoneMats);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bufferIds[2]);
    for (int i = 0; i < materialCount; ++i) {
        if (materials[materialIndices[i]].castShadow() && materials[materialIndices[i]].getDiffuse()[3] != 0)
        {
            if(materials[materialIndices[i]].isDoubleSided())glDisable(GL_CULL_FACE);
            else glEnable(GL_CULL_FACE);
            glDrawElements(materials[materialIndices[i]].getDrawMode(),materials[materialIndices[i]].getIndexCount(),GL_UNSIGNED_INT,(const void*)materials[materialIndices[i]].getOffset());
        }
    }
    glEnable(GL_CULL_FACE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    glDisableVertexAttribArray(mShadowPositionHandle);
    glDisableVertexAttribArray(mShadowBonesHandle);
    glDisableVertexAttribArray(mShadowWeightHandle);
}
void PMXReader::genBoneBuffers() {
    if(!hasBoneBuffers && vertices && bones)
    {
        float * boneIndices=new float[vertexCount<<2];
        float * weights=new float[vertexCount<<2];
        for (int i = 0; i < vertexCount; ++i) {
            int offset=i<<2;
            int boneCount=vertices[i].getBoneCount();
            for (int j = 0; j < boneCount; ++j) {
                boneIndices[offset+j]=vertices[i].getBoneAt(j);
                weights[offset+j]=vertices[i].getWeightAt(j);
            }
            if(boneCount != 4)boneIndices[offset+boneCount]=-1;
        }
        glGenBuffers(2,bufferIds+4);
        glBindBuffer(GL_ARRAY_BUFFER,bufferIds[4]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertexCount<<2,boneIndices,GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,bufferIds[5]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertexCount<<2,weights,GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        delete [] boneIndices;
        delete [] weights;
        hasBoneBuffers= true;
    }
}
void PMXReader::genVertexBuffers() {
    if(!hasVertexBuffers && vertexCoordinates && normals && indices && uvs)
    {
        glGenBuffers(4,bufferIds);
        glBindBuffer(GL_ARRAY_BUFFER,bufferIds[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertexCount*3,vertexCoordinates,GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,bufferIds[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertexCount*3,normals,GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bufferIds[2]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*indexCount,indices,GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
        glBindBuffer(GL_ARRAY_BUFFER,bufferIds[3]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertexCount<<1,uvs,GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        hasVertexBuffers= true;
    }
}
void PMXReader::initShader() {
    mProgram=glCreateProgram();
    mVertexShader=glCreateShader(GL_VERTEX_SHADER);
    mFragmentShader=glCreateShader(GL_FRAGMENT_SHADER);
    int length;
    char *s;
    loadShader("/data/data/com.wjy50.app.mmdviewer/files/pmxVertexShader.fs",&length,&s);
    int ind=-1;
    int lim=length-4;
    for (int i = 0; i < lim; ++i) {
        if(s[i] == '-' && s[i+1] == '*' && s[i+2] == 'd' && s[i+3] == '-')
        {
            ind=i;
            break;
        }
    }
    int b=directBoneCount;
    for (int i = 3; i >= 0; --i) {
        if(b != 0)
        {
            s[ind+i]=(char)((b%10)+'0');
            b/=10;
        }
        else s[ind+i]=' ';
    }
    glShaderSource(mVertexShader,1,(const char**)&s,&length);
    glCompileShader(mVertexShader);
    LOG_PRINTLN(s);
    delete[] s;
    loadShader("/data/data/com.wjy50.app.mmdviewer/files/pmxFragmentShader.fs",&length,&s);
    glShaderSource(mFragmentShader,1,(const char**)&s,&length);
    glCompileShader(mFragmentShader);
    delete [] s;
    glAttachShader(mProgram,mVertexShader);
    glAttachShader(mProgram,mFragmentShader);
    glLinkProgram(mProgram);

    mPositionHandle= (GLuint) glGetAttribLocation(mProgram, "aPosition");
    LOG_PRINTF("err=%d",glGetError());
    mNormalHandle= (GLuint) glGetAttribLocation(mProgram, "aNormal");
    mUVHandle= (GLuint) glGetAttribLocation(mProgram, "aUV");
    mBonesHandle= (GLuint) glGetAttribLocation(mProgram, "aBones");
    mWeightsHandle= (GLuint) glGetAttribLocation(mProgram, "aWeights");

    mSunPositionHandle= glGetUniformLocation(mProgram, "uSunPosition");
    mViewMatHandle= glGetUniformLocation(mProgram, "uViewMat");
    mProjectionMatHandle= glGetUniformLocation(mProgram, "uProjectionMat");
    mBoneMatsHandle= glGetUniformLocation(mProgram, "uBoneMats");
    mSunMatHandle= glGetUniformLocation(mProgram, "uSunMat");

    mSunLightStrengthHandle= glGetUniformLocation(mProgram, "uSunLightStrength");
    mAmbientHandle= glGetUniformLocation(mProgram, "uAmbient");
    mDiffuseHandle= glGetUniformLocation(mProgram, "uDiffuse");
    mSpecularHandle= glGetUniformLocation(mProgram, "uSpecular");
    mSamplersHandle= glGetUniformLocation(mProgram, "uSamplers");
    mTextureModesHandle= glGetUniformLocation(mProgram, "uTextureModes");
    mTextureCoefficientHandle= glGetUniformLocation(mProgram, "uTextureCoefficient");
    mSphereCoefficientHandle= glGetUniformLocation(mProgram, "uSphereCoefficient");
}
void PMXReader::initShadowMapShader() {
    mShadowProgram=glCreateProgram();
    mShadowVertexShader=glCreateShader(GL_VERTEX_SHADER);
    mShadowFragmentShader=glCreateShader(GL_FRAGMENT_SHADER);
    int length;
    char *s;
    loadShader("/data/data/com.wjy50.app.mmdviewer/files/pmxShadowVertexShader.fs",&length,&s);
    int ind=-1;
    int lim=length-4;
    for (int i = 0; i < lim; ++i) {
        if(s[i] == '-' && s[i+1] == '*' && s[i+2] == 'd' && s[i+3] == '-')
        {
            ind=i;
            break;
        }
    }
    int b=directBoneCount;
    for (int i = 3; i >= 0; --i) {
        if(b != 0)
        {
            s[ind+i]=(char)((b%10)+'0');
            b/=10;
        }
        else s[ind+i]=' ';
    }
    glShaderSource(mShadowVertexShader,1,(const char**)&s,&length);
    glCompileShader(mShadowVertexShader);
    LOG_PRINTLN(s);
    delete[] s;
    loadShader("/data/data/com.wjy50.app.mmdviewer/files/pmxShadowFragmentShader.fs",&length,&s);
    glShaderSource(mShadowFragmentShader,1,(const char**)&s,&length);
    glCompileShader(mShadowFragmentShader);
    delete [] s;
    glAttachShader(mShadowProgram,mShadowVertexShader);
    glAttachShader(mShadowProgram,mShadowFragmentShader);
    glLinkProgram(mShadowProgram);

    mShadowPositionHandle= (GLuint) glGetAttribLocation(mShadowProgram, "aPosition");
    LOG_PRINTF("err=%d",glGetError());
    mShadowBonesHandle= (GLuint) glGetAttribLocation(mShadowProgram, "aBones");
    mShadowWeightHandle= (GLuint) glGetAttribLocation(mShadowProgram, "aWeights");

    mShadowSunMatHandle= glGetUniformLocation(mShadowProgram, "uSunMat");
    mShadowBoneMatsHandle= glGetUniformLocation(mShadowProgram, "uBoneMats");
}

void PMXReader::readInfo(FILE *file) {
    char rSize;
    fread(&rSize, sizeof(char),1,file);
    fread(&info, sizeof(info),1,file);
    if(rSize > 8)
    {
        fseek(file,rSize-8,SEEK_CUR);
    }
    encoding= (MStringEncoding) info.encoding;
}

void PMXReader::readNameAndDescription(FILE *file) {
    name= MString::readString(file, encoding, UTF_8);
    nameE= MString::readString(file, encoding, UTF_8);
    desc= MString::readString(file, encoding, UTF_8);
    descE= MString::readString(file, encoding, UTF_8);
}

void PMXReader::readVerticesAndIndices(FILE *file) {
    fread(&vertexCount, sizeof(int),1,file);
    if(vertexCount > 0)
    {
        vertexCoordinates=new float[vertexCount*3];
        normals=new float[vertexCount*3];
        uvs=new float[vertexCount<<1];
        vertices=new PMXVertex[vertexCount];
        for (int i = 0; i < vertexCount; ++i) {
            vertices[i].read(file,&info,vertexCoordinates+(i<<2)-i,normals+(i<<2)-i,uvs+(i<<1));
        }
    }
    fread(&indexCount, sizeof(int),1,file);
    faceCount=indexCount/3;
    indices=new unsigned int[indexCount];
    for (int i = 0; i < indexCount; ++i) {
        indices[i]=0;
        fread(indices+i,info.vertexSize,1,file);
        if(i%3 == 2)
        {
            indices[i]^=indices[i-1];
            indices[i-1]=indices[i]^indices[i-1];
            indices[i]^=indices[i-1];
        }
    }
}

void PMXReader::readTextures(FILE *file, const char *filePath) {
    fread(&textureCount, sizeof(int),1,file);
    if(textureCount > 0)
    {
        textures=new PMXTexture[textureCount];
        int pathLength=-1;
        for (int i = 0; ; ++i) {
            if(filePath[i] == '/')pathLength=i+1;
            else if(filePath[i] == 0)break;
        }
        for (int i = 0; i < textureCount; ++i) {
            textures[i].read(file,encoding,filePath,pathLength);
        }
    }
    else textures=0;
}

void PMXReader::readMaterials(FILE *file) {
    fread(&materialCount, sizeof(int),1,file);
    if(materialCount > 0)
    {
        materials=new PMXMaterial[materialCount];
        materialIndices=new unsigned int[materialCount];
        materialDiffuses=new float[materialCount<<2];
        materialSpecular=new float[materialCount<<2];
        materialAmbient=new float[(materialCount<<2)-materialCount];
        materialEdgeColors=new float[materialCount<<2];
        int lastNoAlphaIndex=0;
        int lastAlphaIndex=materialCount-1;
        long offset=0;
        for (unsigned int i = 0; i < materialCount; ++i) {
            materials[i].read(file, info.texSize, encoding, materialDiffuses + (i << 2),
                              materialSpecular + (i << 2),
                              materialAmbient + (i << 2) - i,
                              materialEdgeColors + (i << 2));
            bool hasAlpha=materials[i].getDiffuse()[3] != 1;
            GLsizei textureIndex=materials[i].getTextureIndex();
            if(textureIndex < textureCount)
            {
                textures[textureIndex].initGLTexture();
                hasAlpha|=textures[textureIndex].hasAlpha();
            }
            GLsizei sphereIndex=materials[i].getSphereIndex();
            if(sphereIndex < textureCount)textures[sphereIndex].initGLTexture();
            materials[i].onTextureLoaded(textureIndex < textureCount && textures[textureIndex].getTextureId() != 0,sphereIndex < textureCount && textures[sphereIndex].getTextureId() != 0);
            materials[i].setOffset(offset<<2);
            offset+=materials[i].getIndexCount();
            if(hasAlpha)materialIndices[lastAlphaIndex--]=i;
            else materialIndices[lastNoAlphaIndex++]=i;
        }
        int halfAlphaCount=(materialCount-lastNoAlphaIndex)>>1;
        for (int i = 0; i < halfAlphaCount; ++i) {
            materialIndices[materialCount-i-1]^=materialIndices[lastNoAlphaIndex+i];
            materialIndices[lastNoAlphaIndex+i]=materialIndices[materialCount-i-1]^materialIndices[lastNoAlphaIndex+i];
            materialIndices[materialCount-i-1]^=materialIndices[lastNoAlphaIndex+i];
        }
    }
    else
    {
        materials=0;
        materialDiffuses=0;
        materialSpecular=0;
        materialAmbient=0;
        materialEdgeColors=0;
    }
}

void PMXReader::readBones(FILE *file) {
    fread(&boneCount, sizeof(int),1,file);
    ikIndices=0;
    if(boneCount > 0)
    {
        bones=new PMXBone[boneCount];
        bonePositions=new float[boneCount<<2];
        localBoneMats=new float[boneCount<<4];
        finalBoneMats=new float[boneCount<<4];
        boneStateIds =new char[boneCount];
        currentPassId=0;
        ikCount=0;
        for (int i = 0; i < boneCount; ++i) {
            bones[i].read(file, info.boneSize, encoding, localBoneMats + (i << 4),
                          bonePositions + (i << 2));
            if(bones[i].getBoneIK())ikCount++;
            boneStateIds[i]=0;
        }
    }
    else
    {
        bones=0;
        bonePositions=0;
        localBoneMats=0;
        finalBoneMats=0;
        boneStateIds=0;
        selfAppendBones=0;
    }
}

void PMXReader::readMorphs(FILE *file) {
    fread(&morphCount, sizeof(int),1,file);
    if(morphCount > 0)
    {
        morphs=new PMXMorph[morphCount];
        for (int i = 0; i < morphCount; ++i) {
            morphs[i].read(file,&info);
        }
    }
    else morphs=0;
}

void PMXReader::performMaterialAddOperation(unsigned int index, PMXMaterialMorphData *data, float f) {
    PMXMaterial* material=materials+index;
    
    const float * initialDiffuse=material->getInitialDiffuse();
    const float * diffuse=data->getDiffuse();
    material->setDiffuse(initialDiffuse[0]+diffuse[0]*f,initialDiffuse[1]+diffuse[1]*f,initialDiffuse[2]+diffuse[2]*f,initialDiffuse[3]+diffuse[3]*f);

    const float * initialSpecular=material->getInitialSpecular();
    const float * specular=data->getSpecular();
    material->setSpecular(initialSpecular[0]+specular[0]*f,initialSpecular[1]+specular[1]*f,initialSpecular[2]+specular[2]*f,initialSpecular[3]+specular[3]*f);

    const float * initialAmbient=material->getInitialAmbient();
    const float * ambient=data->getAmbient();
    material->setAmbient(initialAmbient[0]+ambient[0]*f,initialAmbient[1]+ambient[1]*f,initialAmbient[2]+ambient[2]*f);

    const float * initialEdgeColor=material->getInitialEdgeColor();
    const float * edgeColor=data->getEdgeColor();
    material->setEdgeColor(initialEdgeColor[0]+edgeColor[0]*f,initialEdgeColor[1]+edgeColor[1]*f,initialEdgeColor[2]+edgeColor[2]*f,initialEdgeColor[3]+edgeColor[3]*f);

    material->setEdgeSize(material->getInitialEdgeSize()+data->getEdgeSize()*f);

    const float * textureCoefficient=data->getTextureCoefficient();
    material->setTextureCoefficient(1+textureCoefficient[0]*f,1+textureCoefficient[1]*f,1+textureCoefficient[2]*f,1+textureCoefficient[3]*f);

    const float * sphereCoefficient=data->getSphereCoefficient();
    material->setSphereCoefficient(1+sphereCoefficient[0]*f,1+sphereCoefficient[1]*f,1+sphereCoefficient[2]*f,1+sphereCoefficient[3]*f);
}

void PMXReader::performMaterialMulOperation(unsigned int index, PMXMaterialMorphData *data, float f) {
    PMXMaterial* material=materials+index;

    const float * initialDiffuse=material->getInitialDiffuse();
    const float * diffuse=data->getDiffuse();
    material->setDiffuse(initialDiffuse[0]*(1+(diffuse[0]-1)*f),initialDiffuse[1]*(1+(diffuse[1]-1)*f),initialDiffuse[2]*(1+(diffuse[2]-1)*f),initialDiffuse[3]*(1+(diffuse[3]-1)*f));

    const float * initialSpecular=material->getInitialSpecular();
    const float * specular=data->getSpecular();
    material->setSpecular(initialSpecular[0]*(1+(specular[0]-1)*f),initialSpecular[1]*(1+(specular[1]-1)*f),initialSpecular[2]*(1+(specular[2]-1)*f),initialSpecular[3]*(1+(specular[3]-1)*f));

    const float * initialAmbient=material->getInitialAmbient();
    const float * ambient=data->getAmbient();
    material->setAmbient(initialAmbient[0]*(1+(ambient[0]-1)*f),initialAmbient[1]*(1+(ambient[1]-1)*f),initialAmbient[2]*(1+(ambient[2]-1)*f));

    const float * initialEdgeColor=material->getInitialEdgeColor();
    const float * edgeColor=data->getEdgeColor();
    material->setEdgeColor(initialEdgeColor[0]*(1+(edgeColor[0]-1)*f),initialEdgeColor[1]*(1+(edgeColor[1]-1)*f),initialEdgeColor[2]*(1+(edgeColor[2]-1)*f),initialEdgeColor[3]*(1+(edgeColor[3]-1)*f));

    material->setEdgeSize(material->getInitialEdgeSize()*(1+(data->getEdgeSize()-1)*f));

    const float * textureCoefficient=data->getSphereCoefficient();
    material->setTextureCoefficient(1+(textureCoefficient[0]-1)*f,1+(textureCoefficient[1]-1)*f,1+(textureCoefficient[2]-1)*f,1+(textureCoefficient[3]-1)*f);

    const float * sphereCoefficient=data->getSphereCoefficient();
    material->setSphereCoefficient(1+(sphereCoefficient[0]-1)*f,1+(sphereCoefficient[1]-1)*f,1+(sphereCoefficient[2]-1)*f,1+(sphereCoefficient[3]-1)*f);
}

void PMXReader::rotateBone(unsigned int index, float a, float x, float y, float z) {
    bones[index].rotateBy(a,x,y,z);
    newBoneTransform=true;
}

void PMXReader::translateBone(unsigned int index, float x, float y, float z) {
    bones[index].translationBy(x,y,z);
    newBoneTransform=true;
}

unsigned int PMXReader::getMorphCount() {
    return morphCount;
}

const char* PMXReader::getMorphName(int index) {
    return morphs[index].getName();
}

float PMXReader::getMorphFraction(int index) {
    return morphs[index].getFraction();
}

void PMXReader::setMorphFraction(int index, float f) {
    f=clamp(0,1,f);
    float delta=morphs[index].setFraction(f);
    int count=morphs[index].getMorphDataCount();
    switch (morphs[index].getKind())
    {
        case 0:
        case 9:
            for (int i = 0; i < count; ++i) {
                PMXGroupMorphData* data= (PMXGroupMorphData *) morphs[index].getDataAt(i);
                setMorphFraction(data->getIndex(),f*data->getRatio());
            }
            break;
        case 1:
            for (int i = 0; i < count; ++i) {
                PMXVertexMorphData* data= (PMXVertexMorphData *) morphs[index].getDataAt(i);
                const float * offset=data->getOffset();
                unsigned int vertexIndex=data->getIndex();
                const float * initialPosition=vertices[vertexIndex].getInitialCoordinate();
                vertices[vertexIndex].setPosition(initialPosition[0]+offset[0]*f,initialPosition[1]+offset[1]*f,initialPosition[2]+offset[2]*f);
                if(vertexChangeStart >= vertexCount || vertexIndex < vertexChangeStart)vertexChangeStart=vertexIndex;
                if(vertexChangeEnd >= vertexCount || vertexIndex+1 > vertexChangeEnd)vertexChangeEnd=vertexIndex+1;
            }
            break;
        case 2:
            for (int i = 0; i < count; ++i) {
                PMXBoneMorphData* data= (PMXBoneMorphData *) morphs[index].getDataAt(i);
                const float * translation=data->getTranslation();
                translateBone(data->getIndex(),translation[0]*delta,translation[1]*delta,translation[2]*delta);
                const float * rotation=data->getRotation();
                if(fabsf(rotation[3]) > 1e-6f)rotateBone(data->getIndex(),rotation[3]*delta,rotation[0],rotation[1],rotation[2]);
            }
            break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            for (int i = 0; i < count; ++i) {
                PMXUVMorphData* data= (PMXUVMorphData *) morphs[index].getDataAt(i);
                const float * offset=data->getOffset();
                unsigned int vertexIndex=data->getIndex();
                const float * initialUV=vertices[vertexIndex].getInitialUV();
                vertices[vertexIndex].setUV(initialUV[0]+offset[0]*f,initialUV[1]+offset[1]*f);
                if(uvChangeStart >= vertexCount || vertexIndex < uvChangeStart)uvChangeStart=vertexIndex;
                if(uvChangeEnd >= vertexCount || vertexIndex+1 > uvChangeEnd)uvChangeEnd=vertexIndex+1;
            }
            break;
        case 8:
            for (int i = 0; i < count; ++i) {
                PMXMaterialMorphData* data= (PMXMaterialMorphData *) morphs[index].getDataAt(i);
                unsigned int materialIndex=data->getIndex();
                if(materialIndex < materialCount)
                {
                    if(data->getOperation() == 1)
                    {
                        performMaterialAddOperation(materialIndex,data,f);
                    }
                    else
                    {
                        performMaterialMulOperation(materialIndex,data,f);
                    }
                }
                else
                {
                    if(data->getOperation() == 1)
                    {
                        for (unsigned int j = 0; j < materialCount; ++j) {
                            performMaterialAddOperation(j,data,f);
                        }
                    }
                    else
                    {
                        for (unsigned int j = 0; j < materialCount; ++j) {
                            performMaterialMulOperation(j,data,f);
                        }
                    }
                }
            }
            break;
        default:
            break;
    }
}

PMXReader::~PMXReader() {
    delete name;
    delete nameE;
    delete desc;
    delete descE;
    if(vertexCoordinates)delete [] vertexCoordinates;
    if(normals)delete [] normals;
    if(indices)delete [] indices;
    if(uvs)delete [] uvs;
    if(vertices)delete [] vertices;
    if(textures)delete [] textures;
    if(materials)delete [] materials;
    if(materialIndices)delete [] materialIndices;
    if(materialDiffuses)delete [] materialDiffuses;
    if(materialSpecular)delete [] materialSpecular;
    if(materialAmbient)delete [] materialAmbient;
    if(materialEdgeColors)delete [] materialEdgeColors;
    if(bones)delete [] bones;
    if(bonePositions)delete [] bonePositions;
    if(localBoneMats)delete [] localBoneMats;
    if(finalBoneMats)delete [] finalBoneMats;
    if(ikIndices)delete [] ikIndices;
    if(boneStateIds)delete [] boneStateIds;
    if(morphs)delete [] morphs;
    if(selfAppendBones)delete [] selfAppendBones;
}
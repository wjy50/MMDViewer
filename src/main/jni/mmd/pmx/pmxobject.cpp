//
// Created by wjy50 on 19-8-1.
//

#include "pmxobject.h"

using namespace std;

PMXObject::PMXObject()
        : vertexCount(0), vertices(nullptr), vertexCoordinates(nullptr), normals(nullptr),
          uvs(nullptr),
          indexCount(0), indices(nullptr), textureCount(0), textures(nullptr),
          materialCount(0), materials(nullptr), boneCount(0), directBoneCount(0), bones(nullptr),
          morphCount(0), morphs(nullptr), materialIndices(nullptr), materialDiffuses(nullptr),
          materialSpecular(nullptr), materialAmbient(nullptr), materialEdgeColors(nullptr),
          bonePositions(nullptr), localBoneMats(nullptr), finalBoneMats(nullptr),
          currentPassId(0), boneStateIds(nullptr), newBoneTransform(true),
          ikCount(0), ikIndices(nullptr), vertexChangeStart(0), vertexChangeEnd(0),
          uvChangeStart(0), uvChangeEnd(0), selfAppendBones(nullptr), selfAppendBoneCount(0),
          shaderHolder()
{}

PMXObject::PMXObject(PMXObject &&other) noexcept
        : vertexCount(other.vertexCount), vertices(other.vertices),
          vertexCoordinates(other.vertexCoordinates),
          normals(other.normals), uvs(other.uvs),
          indexCount(other.indexCount), indices(other.indices), textureCount(other.textureCount),
          textures(other.textures),
          materialCount(other.materialCount), materials(other.materials),
          boneCount(other.boneCount), directBoneCount(other.directBoneCount),
          bones(other.bones),
          morphCount(other.morphCount), morphs(other.morphs),
          materialIndices(other.materialIndices),
          materialDiffuses(other.materialDiffuses), materialSpecular(other.materialSpecular),
          materialAmbient(other.materialAmbient), materialEdgeColors(other.materialEdgeColors),
          bonePositions(other.bonePositions), localBoneMats(other.localBoneMats),
          finalBoneMats(other.finalBoneMats),
          currentPassId(other.currentPassId), boneStateIds(other.boneStateIds),
          newBoneTransform(other.newBoneTransform),
          ikCount(other.ikCount), ikIndices(other.ikIndices),
          vertexChangeStart(other.vertexChangeStart), vertexChangeEnd(other.vertexChangeEnd),
          uvChangeStart(other.uvChangeStart), uvChangeEnd(other.uvChangeEnd),
          selfAppendBones(other.selfAppendBones),
          selfAppendBoneCount(other.selfAppendBoneCount),
          shaderHolder(move(other.shaderHolder))
{
    other.nullPointers();

    memcpy(matrixTmp, other.matrixTmp, sizeof(matrixTmp));
    memcpy(vecTmp, other.vecTmp, sizeof(vecTmp));
}

PMXObject &PMXObject::operator=(PMXObject &&other) noexcept
{
    releasePointers();

    vertexCount = other.vertexCount;
    vertices = other.vertices;
    vertexCoordinates = other.vertexCoordinates;
    normals = other.normals;
    uvs = other.uvs;
    indexCount = other.indexCount;
    indices = other.indices;
    textureCount = other.textureCount;
    textures = other.textures;
    materialCount = other.materialCount;
    materials = other.materials;
    boneCount = other.boneCount;
    directBoneCount = other.directBoneCount;
    bones = other.bones;
    morphCount = other.morphCount;
    morphs = other.morphs;
    materialIndices = other.materialIndices;
    materialDiffuses = other.materialDiffuses;
    materialSpecular = other.materialSpecular;
    materialAmbient = other.materialAmbient;
    materialEdgeColors = other.materialEdgeColors;
    bonePositions = other.bonePositions;
    localBoneMats = other.localBoneMats;
    finalBoneMats = other.finalBoneMats;
    currentPassId = other.currentPassId;
    boneStateIds = other.boneStateIds;
    newBoneTransform = other.newBoneTransform;
    ikCount = other.ikCount;
    ikIndices = other.ikIndices;
    vertexChangeStart = other.vertexChangeStart;
    vertexChangeEnd = other.vertexChangeEnd;
    uvChangeStart = other.uvChangeStart;
    uvChangeEnd = other.uvChangeEnd;
    selfAppendBones = other.selfAppendBones;
    selfAppendBoneCount = other.selfAppendBoneCount;
    shaderHolder = move(other.shaderHolder);

    other.nullPointers();

    memcpy(matrixTmp, other.matrixTmp, sizeof(matrixTmp));
    memcpy(vecTmp, other.vecTmp, sizeof(vecTmp));

    return *this;
}

void PMXObject::updateModelState()
{
    if (newBoneTransform) {
        newBoneTransform = false;
        updateBoneMats();
    }
    if (selfAppendBoneCount > 0)
        updateSelfAppend();
    if (vertexChangeStart != vertexChangeEnd) {
        vertexChangeStart = (vertexChangeStart * 4) - vertexChangeStart;
        vertexChangeEnd = (vertexChangeEnd * 4) - vertexChangeEnd;
        glBindBuffer(GL_ARRAY_BUFFER, shaderHolder.getVertexBuffer());
        int size = vertexChangeEnd - vertexChangeStart;
        glBufferSubData(GL_ARRAY_BUFFER, vertexChangeStart * 4, size * 4,
                        vertexCoordinates + vertexChangeStart);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        vertexChangeStart = vertexChangeEnd = -1;
    }
    if (uvChangeStart != uvChangeEnd) {
        uvChangeStart <<= 1;
        uvChangeEnd = uvChangeEnd * 2;
        glBindBuffer(GL_ARRAY_BUFFER, shaderHolder.getUVBuffer());
        int size = uvChangeEnd - uvChangeStart;
        glBufferSubData(GL_ARRAY_BUFFER, uvChangeStart * 4, size * 4, uvs + uvChangeStart);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        uvChangeStart = uvChangeEnd = 0xffffffff;
    }
}

void PMXObject::draw(const float *viewMat, const float *projectionMat,
                     EnvironmentLight &environmentLight)
{
    shaderHolder.prepareProgramAndVertexAttr();

    glUniformMatrix4fv(shaderHolder.mViewMatHandle, 1, GL_FALSE, viewMat);
    glUniformMatrix4fv(shaderHolder.mProjectionMatHandle, 1, GL_FALSE, projectionMat);
    glUniformMatrix4fv(shaderHolder.mBoneMatsHandle, directBoneCount, GL_FALSE, finalBoneMats);

    glUniform3fv(shaderHolder.mSunPositionHandle, 1, environmentLight.getSunPosition());
    glUniform3fv(shaderHolder.mSunLightStrengthHandle, 1, environmentLight.getSunLightStrength());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shaderHolder.getIndexBuffer());
    shaderHolder.getShadowSampler() = environmentLight.getShadowMapTextureUnit();
    if (shaderHolder.getShadowSampler() >= 0) {
        glUniform1iv(shaderHolder.mSamplersHandle, 3, shaderHolder.samplers);
        glUniformMatrix4fv(shaderHolder.mSunMatHandle, 1, GL_FALSE,
                           environmentLight.getSunMatForDraw());
    } else {
        glUniform1iv(shaderHolder.mSamplersHandle, 2, shaderHolder.samplers);
    }
    for (int i = 0; i < materialCount; ++i) {
        if (materials[materialIndices[i]].getDiffuse()[3] != 0) {
            glUniform3fv(shaderHolder.mAmbientHandle, 1,
                         materials[materialIndices[i]].getAmbient());
            glUniform4fv(shaderHolder.mDiffuseHandle, 1,
                         materials[materialIndices[i]].getDiffuse());
            glUniform4fv(shaderHolder.mSpecularHandle, 1,
                         materials[materialIndices[i]].getSpecular());
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
            glUniform3i(shaderHolder.mTextureModesHandle,
                        materials[materialIndices[i]].getTextureState(),
                        materials[materialIndices[i]].getSphereState(),
                        shaderHolder.getShadowSampler() >= 0 &&
                        materials[materialIndices[i]].acceptShadow());
            glUniform4fv(shaderHolder.mTextureCoefficientHandle, 1,
                         materials[materialIndices[i]].getTextureCoefficient());
            glUniform4fv(shaderHolder.mSphereCoefficientHandle, 1,
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

    shaderHolder.disableVertexAttr();
}

void PMXObject::drawShadowMap(EnvironmentLight &environmentLight)
{
    shaderHolder.prepareShadowProgram();

    glUniformMatrix4fv(shaderHolder.mShadowSunMatHandle, 1, GL_FALSE, environmentLight.getSunMat());
    glUniformMatrix4fv(shaderHolder.mShadowBoneMatsHandle, directBoneCount, GL_FALSE,
                       finalBoneMats);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shaderHolder.getIndexBuffer());
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

    shaderHolder.disableShadowProgram();
}

void PMXObject::nullPointers()
{
    vertices = nullptr;
    vertexCoordinates = nullptr;
    normals = nullptr;
    uvs = nullptr;
    indices = nullptr;
    textures = nullptr;
    materials = nullptr;
    bones = nullptr;
    morphs = nullptr;
    materialIndices = nullptr;
    materialDiffuses = nullptr;
    materialSpecular = nullptr;
    materialAmbient = nullptr;
    materialEdgeColors = nullptr;
    bonePositions = nullptr;
    localBoneMats = nullptr;
    finalBoneMats = nullptr;
    boneStateIds = nullptr;
    newBoneTransform = true;
    ikIndices = nullptr;
    selfAppendBones = nullptr;
}

void PMXObject::releasePointers()
{
    delete[] vertices;
    delete[] vertexCoordinates;
    delete[] normals;
    delete[] uvs;
    delete[] indices;
    delete[] textures;
    delete[] materials;
    delete[] bones;
    delete[] morphs;
    delete[] materialIndices;
    delete[] materialDiffuses;
    delete[] materialSpecular;
    delete[] materialAmbient;
    delete[] materialEdgeColors;
    delete[] bonePositions;
    delete[] localBoneMats;
    delete[] finalBoneMats;
    delete[] boneStateIds;
    delete[] ikIndices;
    delete[] selfAppendBones;
}

PMXObject::~PMXObject()
{
    releasePointers();
}
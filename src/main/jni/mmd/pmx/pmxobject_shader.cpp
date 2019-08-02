//
// Created by wjy50 on 19-8-1.
//

#include "pmxobject.h"
#include "../../utils/UniquePointerExt.h"
#include "../../utils/debugutils.h"
#include "../../gl/shaderloader.h"

using namespace std;

void PMXObject::postRead()
{
    directBoneCount = 0;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &shaderHolder.maxVertexShaderVecCount);
    LOG_PRINTF("max vec4=%d", shaderHolder.maxVertexShaderVecCount);
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

void PMXObject::genBoneBuffers()
{
    if (!shaderHolder.hasBoneBuffers && vertices && bones) {
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
        glGenBuffers(2, shaderHolder.bufferIds + 4);
        glBindBuffer(GL_ARRAY_BUFFER, shaderHolder.getBoneBuffer());
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 4, boneIndices.get(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, shaderHolder.getWeightBuffer());
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 4, weights.get(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        shaderHolder.hasBoneBuffers = true;
    }
}

void PMXObject::genVertexBuffers()
{
    if (!shaderHolder.hasVertexBuffers && vertexCoordinates && normals && indices && uvs) {
        glGenBuffers(4, shaderHolder.bufferIds);
        glBindBuffer(GL_ARRAY_BUFFER, shaderHolder.getVertexBuffer());
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 3, vertexCoordinates,
                     GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, shaderHolder.getNormalBuffer());
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 3, normals, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shaderHolder.getIndexBuffer());
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indexCount, indices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, shaderHolder.getUVBuffer());
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 2, uvs, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        shaderHolder.hasVertexBuffers = true;
    }
}

void PMXObject::initShader()
{
    shaderHolder.createShader();
    int v;
    glGetShaderiv(shaderHolder.mVertexShader, GL_SHADING_LANGUAGE_VERSION, &v);
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
    glShaderSource(shaderHolder.mVertexShader, 1, &ptrS, &length);
    LOG_PRINTLN(ptrS);
    /*glGetShaderInfoLog(mVertexShader, length, &length, s);
    LOG_PRINTLN(s);*/
    loadShader("/data/data/com.wjy50.app.mmdviewer/files/pmxFragmentShader.fs", &length, s);
    ptrS = s.get();
    glShaderSource(shaderHolder.mFragmentShader, 1, &ptrS, &length);
    shaderHolder.buildShader();

    shaderHolder.bindShader();
}

void PMXObject::initShadowMapShader()
{
    shaderHolder.createShadowShader();
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
    glShaderSource(shaderHolder.mShadowVertexShader, 1, &ptrS, &length);
    LOG_PRINTLN(s.get());
    loadShader("/data/data/com.wjy50.app.mmdviewer/files/pmxShadowFragmentShader.fs", &length, s);
    ptrS = s.get();
    glShaderSource(shaderHolder.mShadowFragmentShader, 1, &ptrS, &length);
    shaderHolder.buildShadowShader();

    shaderHolder.bindShadowShader();
}
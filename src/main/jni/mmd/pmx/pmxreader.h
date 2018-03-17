//
// Created by wjy50 on 2018/2/7.
//

#ifndef MMDVIEWER_PMXREADER_H
#define MMDVIEWER_PMXREADER_H

#include <stdio.h>
#include "../../gl/objects/globject.h"
#include "pmxbone.h"
#include "pmxcommon.h"
#include "pmxmaterial.h"
#include "pmxtexture.h"
#include "pmxvertex.h"
#include "pmxmorph.h"

typedef struct PMX_HEADER{
    int magic;
}PMXHeader;

class PMXReader:public AbsGLObject{
private:
    PMXInfo info;
    MString *name,*nameE,*desc,*descE;
    PMXVertex* vertices;
    PMXTexture* textures;
    PMXMaterial* materials;
    PMXBone* bones;
    PMXMorph* morphs;

    bool hasVertexBuffers;
    bool hasBoneBuffers;

    GLuint bufferIds[6];

    GLuint mProgram;
    GLuint mVertexShader,mFragmentShader;

    GLint mPositionHandle,mNormalHandle,mUVHandle,mBonesHandle,mWeightsHandle;
    GLint mSunPositionHandle;
    GLint mViewMatHandle,mProjectionMatHandle,mBoneMatsHandle,mSunMatHandle;

    GLint mSunLightStrengthHandle;
    GLint mAmbientHandle,mDiffuseHandle,mSpecularHandle;
    GLint mSamplersHandle,mTextureModesHandle;

    GLint samplers[3]={0,1,0};

    int vertexCount;
    int indexCount;
    int faceCount;
    int textureCount;
    int materialCount;
    unsigned int boneCount;
    unsigned int directBoneCount;
    int morphCount;
    float * vertexCoordinates;
    float * normals;
    float * uvs;
    unsigned int * indices;

    unsigned int * materialIndices;
    float * materialDiffuses;
    float * materialSpecular;
    float * materialAmbient;
    float * materialEdgeColors;

    float * bonePositions;
    float * localBoneMats;
    float * finalBoneMats;

    char currentPassId;
    char*boneStateIds;
    void calculateIK();
    void calculateBone(unsigned int index);
    void updateIKMatrix(unsigned int index);
    float matrixTmp[16];
    float vecTmp[4];
    bool newBoneTransform;

    GLint maxVertexShaderVecCount;

    GLuint mShadowProgram;
    GLuint mShadowVertexShader,mShadowFragmentShader;
    GLint mShadowPositionHandle,mShadowBonesHandle,mShadowWeightHandle;
    GLint mShadowSunMatHandle,mShadowBoneMatsHandle;

    unsigned int ikCount;
    unsigned int * ikIndices;

    void updateBoneMats();
    void invalidateChildren(unsigned int index);
public:
    PMXReader(const char* filePath);
    void genVertexBuffers();
    void genBoneBuffers();
    void initShader();
    void initShadowMapShader();
    void draw(const float*, const float*, EnvironmentLight*);
    void drawShadowMap(EnvironmentLight*);
    ~PMXReader();
};

#endif //MMDVIEWER_PMXREADER_H

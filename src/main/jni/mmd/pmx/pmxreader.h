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
    MStringEncoding encoding;
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

    GLuint mPositionHandle,mNormalHandle,mUVHandle,mBonesHandle,mWeightsHandle;
    GLint mSunPositionHandle;
    GLint mViewMatHandle,mProjectionMatHandle,mBoneMatsHandle,mSunMatHandle;

    GLint mSunLightStrengthHandle;
    GLint mAmbientHandle,mDiffuseHandle,mSpecularHandle;
    GLint mSamplersHandle,mTextureModesHandle;
    GLint mTextureCoefficientHandle,mSphereCoefficientHandle;

    GLint samplers[3]={0,1,0};

    int vertexCount;
    int indexCount;
    int faceCount;
    int textureCount;
    int materialCount;
    unsigned int boneCount;
    unsigned int directBoneCount;
    unsigned int morphCount;
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
    GLuint mShadowPositionHandle,mShadowBonesHandle,mShadowWeightHandle;
    GLint mShadowSunMatHandle,mShadowBoneMatsHandle;

    unsigned int ikCount;
    unsigned int * ikIndices;

    void updateBoneMats();
    void invalidateChildren(unsigned int index);
    void updateSelfAppend();

    void readInfo(FILE *file);
    void readNameAndDescription(FILE *file);
    void readVerticesAndIndices(FILE *file);
    void readTextures(FILE *file, const char *filePath);
    void readMaterials(FILE *file);
    void readBones(FILE *file);
    void readMorphs(FILE *file);

    void performMaterialAddOperation(unsigned int index, PMXMaterialMorphData *data, float f);
    void performMaterialMulOperation(unsigned int index, PMXMaterialMorphData *data, float f);

    unsigned int vertexChangeStart,vertexChangeEnd;
    unsigned int uvChangeStart,uvChangeEnd;

    unsigned int * selfAppendBones;
    int selfAppendBoneCount;
public:
    PMXReader(const char* filePath);
    void genVertexBuffers();
    void genBoneBuffers();
    void initShader();
    void initShadowMapShader();
    void updateModelState();
    void draw(const float*, const float*, EnvironmentLight*);
    void drawShadowMap(EnvironmentLight*);

    void setMorphFraction(int index, float f);
    float getMorphFraction(int index);

    void rotateBone(unsigned int index, float a, float x, float y, float z);
    void translateBone(unsigned int index, float x, float y, float z);

    unsigned int getMorphCount();
    const char * getMorphName(int index);

    ~PMXReader();
};

#endif //MMDVIEWER_PMXREADER_H

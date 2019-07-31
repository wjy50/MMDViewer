//
// Created by wjy50 on 2018/2/7.
//

#ifndef MMDVIEWER_PMXREADER_H
#define MMDVIEWER_PMXREADER_H

#include <cstdio>
#include "../../gl/objects/globject.h"
#include "pmxbone.h"
#include "pmxcommon.h"
#include "pmxmaterial.h"
#include "pmxtexture.h"
#include "pmxvertex.h"
#include "pmxmorph.h"

typedef struct PMX_HEADER
{
    int magic;
} PMXHeader;

class PMXReader : public AbsGLObject
{
private:
    PMXInfo info;
    MStringEncoding encoding;
    MString name, nameE, desc, descE;
    PMXVertex *vertices;
    PMXTexture *textures;
    PMXMaterial *materials;
    PMXBone *bones;
    PMXMorph *morphs;

    bool hasVertexBuffers;
    bool hasBoneBuffers;

    GLuint bufferIds[6];

    GLuint mProgram;
    GLuint mVertexShader, mFragmentShader;

    GLuint mPositionHandle, mNormalHandle, mUVHandle, mBonesHandle, mWeightsHandle;
    GLint mSunPositionHandle;
    GLint mViewMatHandle, mProjectionMatHandle, mBoneMatsHandle, mSunMatHandle;

    GLint mSunLightStrengthHandle;
    GLint mAmbientHandle, mDiffuseHandle, mSpecularHandle;
    GLint mSamplersHandle, mTextureModesHandle;
    GLint mTextureCoefficientHandle, mSphereCoefficientHandle;

    GLint samplers[3] = {0, 1, 0};

    int vertexCount;
    int indexCount;
    int faceCount;
    int textureCount;
    int materialCount;
    int boneCount;
    int directBoneCount;
    int morphCount;
    float *vertexCoordinates;
    float *normals;
    float *uvs;
    unsigned int *indices;

    int *materialIndices;
    float *materialDiffuses;
    float *materialSpecular;
    float *materialAmbient;
    float *materialEdgeColors;

    float *bonePositions;
    float *localBoneMats;
    float *finalBoneMats;

    char currentPassId;
    char *boneStateIds;

    void calculateIK();

    void calculateBone(int index);

    void updateIKMatrix(int index);

    float matrixTmp[16];
    float vecTmp[4];
    bool newBoneTransform;

    GLint maxVertexShaderVecCount;

    GLuint mShadowProgram;
    GLuint mShadowVertexShader, mShadowFragmentShader;
    GLuint mShadowPositionHandle, mShadowBonesHandle, mShadowWeightHandle;
    GLint mShadowSunMatHandle, mShadowBoneMatsHandle;

    int ikCount;
    int *ikIndices;

    void updateBoneMats();

    void invalidateChildren(int index);

    void updateSelfAppend();

    void readInfo(std::ifstream &file);

    void readNameAndDescription(std::ifstream &file);

    void readVerticesAndIndices(std::ifstream &file);

    void readTextures(std::ifstream &file, const char *filePath);

    void readMaterials(std::ifstream &file);

    void readBones(std::ifstream &file);

    void readMorphs(std::ifstream &file);

    void performMaterialAddOperation(int index, PMXMaterialMorphData *data, float f);

    void performMaterialMulOperation(int index, PMXMaterialMorphData *data, float f);

    int vertexChangeStart, vertexChangeEnd;
    int uvChangeStart, uvChangeEnd;

    int *selfAppendBones;
    int selfAppendBoneCount;
public:
    PMXReader(const char *filePath);

    void genVertexBuffers();

    void genBoneBuffers();

    void initShader();

    void initShadowMapShader();

    void updateModelState();

    void draw(const float *, const float *, EnvironmentLight &);

    void drawShadowMap(EnvironmentLight &environmentLight);

    void setMorphFraction(int index, float f);

    float getMorphFraction(int index) const;

    void rotateBone(int index, float a, float x, float y, float z);

    void translateBone(int index, float x, float y, float z);

    int getMorphCount() const;

    const char *getMorphName(int index) const;

    ~PMXReader();
};

#endif //MMDVIEWER_PMXREADER_H

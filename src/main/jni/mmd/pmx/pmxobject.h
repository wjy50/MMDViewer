//
// Created by wjy50 on 19-8-1.
//

#ifndef MMDVIEWER_PMXOBJECT_H
#define MMDVIEWER_PMXOBJECT_H

#include "../../gl/objects/absglobject.h"
#include "../../utils/mstring.h"
#include "pmxvertex.h"
#include "pmxtexture.h"
#include "pmxmaterial.h"
#include "pmxbone.h"
#include "pmxmorph.h"
#include "pmxobject_shader_holder.h"

class PMXObject : public AbsGLObject
{
private:
    MString name, nameE, desc, descE;

    int vertexCount;
    PMXVertex *vertices;
    float *vertexCoordinates;
    float *normals;
    float *uvs;

    int indexCount;
    unsigned int *indices;

    int textureCount;
    PMXTexture *textures;

    int materialCount;
    PMXMaterial *materials;

    int boneCount;
    int directBoneCount;
    PMXBone *bones;

    int morphCount;
    PMXMorph *morphs;

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

    int ikCount;
    int *ikIndices;

    void updateBoneMats();

    void invalidateChildren(int index);

    void updateSelfAppend();

    void performMaterialAddOperation(int index, PMXMaterialMorphData &data, float f);

    void performMaterialMulOperation(int index, PMXMaterialMorphData &data, float f);

    int vertexChangeStart, vertexChangeEnd;
    int uvChangeStart, uvChangeEnd;

    int *selfAppendBones;
    int selfAppendBoneCount;

    PMXObjectShaderHolder shaderHolder;

    PMXObject();

    void postRead();

    void genVertexBuffers();

    void genBoneBuffers();

    void nullPointers();

    void releasePointers();

    friend class PMXReader;
public:
    PMXObject(PMXObject &&other) noexcept;

    PMXObject &operator = (PMXObject &&other) noexcept;

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

    ~PMXObject();
};

#endif //MMDVIEWER_PMXOBJECT_H

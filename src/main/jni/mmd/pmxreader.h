//
// Created by wjy50 on 2018/2/7.
//

#ifndef MMDVIEWER_PMXREADER_H
#define MMDVIEWER_PMXREADER_H

#include <stdio.h>
#include "../gl/objects/globject.h"
#include "../utils/mstring.h"

#define CHECK_FLAG(a,b) (((a)&(b)) == (b))

typedef enum PMX_ENCODING{
    UTF16=0,
    UTF8
}PMXEncoding;

typedef enum PMX_SPHERE_MAP_MODE{
    SPHERE_MODE_NONE,
    SPHERE_MODE_MUL,
    SPHERE_MODE_ADD,
    SPHERE_MODE_SUB_TEX
}PMXSphereMapMode;

typedef enum BONE_FLAG{
    TO_BONE=1,
    ROTATION=2,
    TRANSLATION=4,
    VISIBLE=8,
    ENABLE=0x10,
    IK=0x20,
    APPEND_LOCAL=0x80,
    APPEND_ROTATION=0x100,
    APPEND_TRANSLATION=0x200,
    FIX_AXIS=0x400,
    LOCAL_FRAME=0x800,
    AFTER_PHYSICS=0x1000,
    EXTRA_PARENT=0x2000
}BoneFlag;

typedef struct PMX_HEADER{
    int magic;
}PMXHeader;

typedef struct PMX_INFO{
    unsigned char encoding;
    unsigned char UVACount;
    unsigned char vertexSize;
    unsigned char texSize;
    unsigned char materialSize;
    unsigned char boneSize;
    unsigned char morphSize;
    unsigned char bodySize;
}PMXInfo;

class PMXVertex{
private:
    float * coordinate;
    float * normal;
    float * uv;
    //something here (UVA) is ignored while reading
    char deform;
    char boneCount;
    unsigned int *bones;
    float *weights;
    float *sDefVec;
    //something here (4 bytes) is also ignored
    friend class PMXReader;
public:
    PMXVertex();
    void read(FILE* file,PMXInfo* info,float* coordinate,float* normal,float* uv);
    ~PMXVertex();
};

class PMXTexture{
private:
    const char * path;
    GLuint textureId;
    bool hasAlpha;
    friend class PMXReader;
public:
    PMXTexture();
    void read(FILE* file,PMXInfo* info, const char* pmxPath,int pathLength);
    GLuint getTextureId();
    void initGLTexture();
    ~PMXTexture();
};

typedef enum PMX_MATERIAL_FLAGS{
    DOUBLE_SIDED=1,
    SHADOW=2,
    SELF_SHADOW_MAP=4,
    SELF_SHADOW=8,
    DRAW_EDGE=0x10,
    DRAW_MODE_POINT=0x40,
    DRAW_MODE_LINE=0x80,
}PMXMaterialFlags;

class PMXMaterial{
private:
    MString * name,*nameE;
    float * diffuse;
    float * specular;//specular[3] == shininess
    float * ambient;
    char flags;
    float * edgeColor;
    float edgeSize;
    int textureIndex, sphereIndex;
    char sphereMode,sharedToon;
    int toonI;
    MString * memo;
    friend class PMXReader;

    long offset;
    GLsizei indexCount;

    bool doubleSided;
    GLenum drawMode;

    bool shadow,selfShadowMap,selfShadow;

    int hasTexture,hasSphere;
public:
    PMXMaterial();
    void read(FILE* file,PMXInfo* info,float* diffuse,float* specular,float* ambient,float* edgeColor);
    ~PMXMaterial();
};

class PMXBoneIKChainNode{
private:
    unsigned int ikBone;
    bool limited;
    float * low;
    float * high;
    float * ikMat;
public:
    PMXBoneIKChainNode();
    void read(FILE* file,PMXInfo* info);

    unsigned int getBoneIndex();
    bool isLimited();
    const float * getLowLimit();
    const float * getHighLimit();

    float * getIKMat();

    ~PMXBoneIKChainNode();
};

class PMXBoneIK{
private:
    unsigned int target;
    int loopCount;
    float loopAngleLimit;
    int ikChainLength;
    PMXBoneIKChainNode* ikChain;
public:
    PMXBoneIK(FILE* file,PMXInfo* info);
    unsigned int getTarget();
    int getIkChainLength();
    PMXBoneIKChainNode* getIkChainNodeAt(int index);
    float getLoopAngleLimit();
    int getLoopCount();
    ~PMXBoneIK();
};

class PMXBone{
private:
    MString * name,*nameE;
    float * position;
    unsigned int parent;
    int level;
    unsigned short flag;
    int child;
    float * offset;
    unsigned int appendParent;
    float appendRatio;
    float * axis;
    float * localX;
    float * localZ;
    float * localY;
    int extraKey;
    PMXBoneIK* boneIK;

    unsigned int actualIndex;

    float * localMat;
    float * localMatWithAppend;

    float * ikMat;

    int childCount;
    int childrenCapacity;
    unsigned int * children;

    bool appendFromSelf;
public:
    PMXBone();
    void read(FILE* file,PMXInfo* info, float* localMat, float* position);
    void normalizeLocal();

    PMXBoneIK* getBoneIK();
    void setActualIndex(unsigned int actualIndex);
    unsigned int getActualIndex();
    float * getPosition();
    unsigned int getParent();
    unsigned int getAppendParent();
    float getAppendRatio();
    int getChildCount();
    const float * getLocalMat();
    void setIKMat(float * ikMat);
    void addChild(unsigned int child);
    unsigned int getChildAt(int index);
    void setAppendFromSelf(bool appendFromSelf);
    bool isAppendFromSelf();
    const float * getCurrentMat();
    float * getLocalMatWithAppend();
    float * getIKMat();

    const char * getName();

    void rotateBy(float a, float x, float y, float z);
    void translationBy(float x, float y, float z);
    void resetLocal();

    ~PMXBone();
};

class PMXMorphData{
public:
    virtual void read(FILE* file,PMXInfo* info)=0;
    virtual ~PMXMorphData();
};

class PMXGroupMorphData:public PMXMorphData{
private:
    int index;
    float ratio;
public:
    void read(FILE* file,PMXInfo* info);
};

class PMXVertexMorphData:public PMXMorphData{
private:
    int index;
    float offset[3];
public:
    void read(FILE* file,PMXInfo* info);
};

class PMXBoneMorphData:public PMXMorphData{
private:
    int index;
    float translation[3];
    float rotation[4];
public:
    void read(FILE* file,PMXInfo* info);
};

class PMXUVMorphData:public PMXMorphData{
private:
    int index;
    float offset[4];
public:
    void read(FILE* file,PMXInfo* info);
};

class PMXMaterialMorphData:public PMXMorphData{
private:
    int index;
    char operation;
    float diffuse[4];
    float specular[4];//specular[3] == shininess
    float ambient[3];
    float edgeColor[4];
    float edgeSize;
    float textureCoefficient[4];
    float sphereCoefficient[4];
    //something here (16 bytes) is ignored while reading
public:
    void read(FILE* file,PMXInfo* info);
};

class PMXMorph{
private:
    MString * name,*nameE;
    char panel,kind;
    int count;
    PMXMorphData** data;
public:
    PMXMorph();
    void read(FILE* file,PMXInfo* info);
    ~PMXMorph();
};

class PMXReader:public AbsGLObject{
private:
    static MString * readPMXString(FILE* file,char encoding);
    PMXInfo info;
    MString *name,*nameE,*desc,*descE;
    friend class PMXBone;
    friend class PMXBoneIK;
    friend class PMXMaterial;
    friend class PMXTexture;
    friend class PMXMorph;
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

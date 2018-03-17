//
// Created by wjy50 on 18-3-17.
//

#ifndef MMDVIEWER_PMXMATERIAL_H
#define MMDVIEWER_PMXMATERIAL_H

#include "../../utils/mstring.h"
#include "pmxcommon.h"

typedef enum PMX_SPHERE_MAP_MODE{
    SPHERE_MODE_NONE,
    SPHERE_MODE_MUL,
    SPHERE_MODE_ADD,
    SPHERE_MODE_SUB_TEX
}PMXSphereMapMode;

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

    long offset;

    GLsizei indexCount;

    bool doubleSided;
    GLenum drawMode;

    bool shadow,mThrowsShadow,mAcceptShadow;

    int textureState,sphereState;
public:
    PMXMaterial();
    void read(FILE *file, size_t textureSize, MStringEncoding encoding, float *diffuse, float *specular,
              float *ambient, float *edgeColor);

    GLsizei getTextureIndex();
    GLsizei getSphereIndex();
    char getSphereMode();
    const float * getAmbient();
    const float * getDiffuse();
    const float * getSpecular();
    GLsizei getIndexCount();
    GLenum getDrawMode();
    bool isDoubleSided();
    long getOffset();
    void setOffset(long offset);
    int getTextureState();
    int getSphereState();
    void onTextureLoaded(bool isTextureSuccessful, bool isSphereSuccessful);
    bool throwsShadow();
    bool acceptShadow();

    ~PMXMaterial();
};

#endif //MMDVIEWER_PMXMATERIAL_H

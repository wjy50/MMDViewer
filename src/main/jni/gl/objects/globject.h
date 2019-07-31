//
// Created by wjy50 on 2018/2/5.
//
#include "absglobject.h"

#ifndef MMDVIEWER_GLOBJECT_H
#define MMDVIEWER_GLOBJECT_H

class GLObject:AbsGLObject{
private:
    int vertexCount;
    float * vertices;
    float * normals;
    int indexCount;
    unsigned int * indices;
    //float * uvs;
    GLuint bufferIds[4];
    bool hasVertexBuffers;
    //bool hasUVBuffers;

    float modelMat[16];

    GLuint mProgram;
    GLuint mVertexShader,mFragmentShader;

    GLuint mPositionHandle,mNormalHandle;
    GLint mSunPositionHandle;
    GLint mModelMatHandle,mViewMatHandle,mProjectionMatHandle;

    GLint mSunLightStrengthHandle;
    GLint mAmbientHandle,mDiffuseHandle,mSpecularHandle;
    GLint mShininessHandle;

    float ambient[3],diffuse[4],specular[3],shininess;
public:
    GLObject();
    void setVertices(int vertexCount, float* vertices, float* normals);
    void setIndices(int indexCount, unsigned int * indices);
    //void setUVs(float * uvs);
    void genVertexBuffers();
    void initShader();
    //void genUVBuffer()

    void setAmbient(float r, float g, float b);
    void setDiffuse(float r, float g, float b, float a);
    void setSpecular(float r, float g, float b);
    void setShininess(float s);

    void draw(const float *viewMat, const float *projMat, EnvironmentLight &env) override;
    void updateModelState() override;
    ~GLObject();
};

#endif //MMDVIEWER_GLOBJECT_H

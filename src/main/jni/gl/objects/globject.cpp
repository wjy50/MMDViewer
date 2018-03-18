//
// Created by wjy50 on 2018/2/5.
//

#include "globject.h"
#include "../shaderloader.h"
#include "../../matrix/matrix.h"

GLObject::GLObject()
{
    vertices=normals/*=uvs*/=0;
    indices=0;
    hasVertexBuffers/*=hasUVBuffers*/= false;
    setIdentityM(modelMat);
    setAmbient(0,0,0);
    setDiffuse(1,1,1,1);
    setSpecular(0,0,0);
    setShininess(1);
}
GLObject::~GLObject() {
    if(vertices)delete [] vertices;
    if(normals)delete [] normals;
    if(indices)delete [] indices;
    //if(uvs)delete [] uvs;
}
void GLObject::setVertices(int vertexCount, float *vertices, float *normals) {
    this->vertexCount=vertexCount;
    this->vertices=vertices;
    this->normals=normals;
}
void GLObject::setIndices(int indexCount, unsigned int *indices) {
    this->indexCount=indexCount;
    this->indices=indices;
}
/*void GLObject::setUVs(float *uvs) {
    this->uvs=uvs;
}*/
void GLObject::genVertexBuffers() {
    if(!hasVertexBuffers && vertices && normals && indices)
    {
        glGenBuffers(3,bufferIds);
        glBindBuffer(GL_ARRAY_BUFFER,bufferIds[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertexCount*3,vertices,GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,bufferIds[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertexCount*3,normals,GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bufferIds[2]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*indexCount,indices,GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
        hasVertexBuffers= true;
    }
}
void GLObject::initShader() {
    mProgram=glCreateProgram();
    mVertexShader=glCreateShader(GL_VERTEX_SHADER);
    mFragmentShader=glCreateShader(GL_FRAGMENT_SHADER);
    size_t length;
    char *s;
    loadShader("/data/data/com.wjy50.app.mmdviewer/files/simpleVertexShader.vs",&length,&s);
    glShaderSource(mVertexShader,1,(const char**)&s,(int*)&length);
    glCompileShader(mVertexShader);
    delete[] s;
    loadShader("/data/data/com.wjy50.app.mmdviewer/files/simpleFragmentShader.fs",&length,&s);
    glShaderSource(mFragmentShader,1,(const char**)&s,(int*)&length);
    glCompileShader(mFragmentShader);
    delete [] s;
    glAttachShader(mProgram,mVertexShader);
    glAttachShader(mProgram,mFragmentShader);
    glLinkProgram(mProgram);
    mPositionHandle= (GLuint) glGetAttribLocation(mProgram, "aPosition");
    mNormalHandle= (GLuint) glGetAttribLocation(mProgram, "aNormal");
    mSunPositionHandle=glGetUniformLocation(mProgram,"uSunPosition");
    mModelMatHandle=glGetUniformLocation(mProgram,"uModelMat");
    mViewMatHandle=glGetUniformLocation(mProgram,"uViewMat");
    mProjectionMatHandle=glGetUniformLocation(mProgram,"uProjectionMat");

    mSunLightStrengthHandle=glGetUniformLocation(mProgram,"uSunLightStrength");
    mAmbientHandle=glGetUniformLocation(mProgram,"uAmbient");
    mDiffuseHandle=glGetUniformLocation(mProgram,"uDiffuse");
    mSpecularHandle=glGetUniformLocation(mProgram,"uSpecular");
    mShininessHandle=glGetUniformLocation(mProgram,"uShininess");
}
void GLObject::updateModelState() {

}
void GLObject::draw(const float * viewMat, const float * projectionMat, EnvironmentLight* environmentLight) {
    glUseProgram(mProgram);

    glEnableVertexAttribArray(mPositionHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[0]);
    glVertexAttribPointer(mPositionHandle,3,GL_FLOAT, 0/*false*/,0,0);

    glEnableVertexAttribArray(mNormalHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[1]);
    glVertexAttribPointer(mNormalHandle,3,GL_FLOAT, 0/*false*/,0,0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    glUniformMatrix4fv(mModelMatHandle,1, 0/*false*/,modelMat);
    glUniformMatrix4fv(mViewMatHandle,1,0/*false*/,viewMat);
    glUniformMatrix4fv(mProjectionMatHandle,1,0/*false*/,projectionMat);

    glUniform3fv(mSunPositionHandle,1,environmentLight->getSunPosition());
    glUniform3fv(mSunLightStrengthHandle,1,environmentLight->getSunLightStrength());

    glUniform3fv(mAmbientHandle,1,ambient);
    glUniform4fv(mDiffuseHandle,1,diffuse);
    glUniform3fv(mSpecularHandle,1,specular);
    glUniform1f(mShininessHandle,shininess);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bufferIds[2]);
    glDrawElements(GL_TRIANGLES,indexCount,GL_UNSIGNED_INT,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    glDisableVertexAttribArray(mNormalHandle);
    glDisableVertexAttribArray(mPositionHandle);
}
void GLObject::setAmbient(float r, float g, float b) {
    ambient[0]=r;
    ambient[1]=g;
    ambient[2]=b;
}
void GLObject::setDiffuse(float r, float g, float b, float a) {
    diffuse[0]=r;
    diffuse[1]=g;
    diffuse[2]=b;
    diffuse[3]=a;
}
void GLObject::setSpecular(float r, float g, float b) {
    specular[0]=r;
    specular[1]=g;
    specular[2]=b;
}
void GLObject::setShininess(float shininess)
{
    this->shininess=shininess;
}
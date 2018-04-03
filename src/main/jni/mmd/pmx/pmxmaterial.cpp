//
// Created by wjy50 on 18-3-17.
//

#include "pmxmaterial.h"
#include "../../utils/mathutils.h"

PMXMaterial::PMXMaterial() {
    name=nameE=memo=0;
    initialDiffuse=0;
    initialSpecular=0;
    initialAmbient=0;
    initialEdgeColor=0;
    for (int i = 0; i < 4; ++i) {
        textureCoefficient[i]=1;
    }
    for (int i = 0; i < 4; ++i) {
        sphereCoefficient[i]=1;
    }
}

void
PMXMaterial::read(FILE *file, size_t textureSize, MStringEncoding encoding, float *diffuse, float *specular,
                  float *ambient, float *edgeColor)
{
    this->diffuse=diffuse;
    this->specular=specular;
    this->ambient=ambient;
    this->edgeColor=edgeColor;
    name= MString::readString(file, encoding, UTF_8);
    nameE= MString::readString(file, encoding, UTF_8);
    fread(diffuse, sizeof(float),4,file);
    fread(specular, sizeof(float),4,file);
    fread(ambient, sizeof(float),3,file);
    fread(&(flags), sizeof(char),1,file);
    fread(edgeColor, sizeof(float),4,file);
    fread(&(edgeSize), sizeof(float),1,file);
    initialEdgeSize=edgeSize;
    textureIndex=0;
    sphereIndex =0;
    textureState=0;
    sphereState=0;
    fread(&textureIndex,textureSize,1,file);
    fread(&sphereIndex, textureSize, 1, file);
    fread(&sphereMode, sizeof(char),1,file);
    fread(&sharedToon, sizeof(char),1,file);
    toonI=0;
    if(sharedToon == 0)fread(&toonI,textureSize,1,file);
    else fread(&toonI, sizeof(char),1,file);
    memo= MString::readString(file, encoding, UTF_8);
    fread(&indexCount, sizeof(GLsizei),1,file);
    doubleSided=CHECK_FLAG(flags,DOUBLE_SIDED);
    groundShadow=CHECK_FLAG(flags,GROUND_SHADOW);
    mCastShadow=CHECK_FLAG(flags,SELF_SHADOW_MAP);
    mAcceptShadow=CHECK_FLAG(flags,SELF_SHADOW);
    if(CHECK_FLAG(flags,DRAW_MODE_LINE))drawMode=GL_LINES;
    else if(CHECK_FLAG(flags,DRAW_MODE_POINT))drawMode=GL_POINTS;
    else drawMode=GL_TRIANGLES;
}

GLsizei PMXMaterial::getTextureIndex() {
    return textureIndex;
}

GLsizei PMXMaterial::getSphereIndex() {
    return sphereIndex;
}

char PMXMaterial::getSphereMode() {
    return sphereMode;
}

const float* PMXMaterial::getAmbient() {
    return ambient;
}

const float* PMXMaterial::getDiffuse() {
    return diffuse;
}

const float* PMXMaterial::getSpecular() {
    return specular;
}

GLsizei PMXMaterial::getIndexCount() {
    return indexCount;
}

GLenum PMXMaterial::getDrawMode() {
    return drawMode;
}

bool PMXMaterial::isDoubleSided() {
    return doubleSided;
}

long PMXMaterial::getOffset() {
    return offset;
}

void PMXMaterial::setOffset(long offset) {
    this->offset = offset;
}

int PMXMaterial::getTextureState() {
    return textureState;
}

int PMXMaterial::getSphereState() {
    return sphereState;
}

void PMXMaterial::onTextureLoaded(bool isTextureSuccessful, bool isSphereSuccessful) {
    if(isTextureSuccessful)textureState=1;
    if(isSphereSuccessful)
    {
        if(sphereMode == 1)sphereState=2;
        else if(sphereMode == 2)sphereState=1;
    }
}

bool PMXMaterial::castShadow() {
    return mCastShadow;
}

bool PMXMaterial::acceptShadow() {
    return mAcceptShadow;
}

const float PMXMaterial::getInitialEdgeSize() {
    return initialEdgeSize;
}

void PMXMaterial::setEdgeSize(float size) {
    edgeSize=size;
}

const float* PMXMaterial::getInitialDiffuse() {
    if(!initialDiffuse)
    {
        initialDiffuse=new float[4];
        for (int i = 0; i < 4; ++i) {
            initialDiffuse[i]=diffuse[i];
        }
    }
    return initialDiffuse;
}

void PMXMaterial::setDiffuse(float r, float g, float b, float a) {
    if(!initialDiffuse)
    {
        initialDiffuse=new float[4];
        for (int i = 0; i < 4; ++i) {
            initialDiffuse[i]=diffuse[i];
        }
    }
    diffuse[0]=r;
    diffuse[1]=g;
    diffuse[2]=b;
    diffuse[3]=a;
}

const float* PMXMaterial::getInitialSpecular(){
    if(!initialSpecular)
    {
        initialSpecular=new float[4];
        for (int i = 0; i < 4; ++i) {
            initialSpecular[i]=specular[i];
        }
    }
    return initialSpecular;
}

void PMXMaterial::setSpecular(float r, float g, float b, float s) {
    if(!initialSpecular)
    {
        initialSpecular=new float[4];
        for (int i = 0; i < 4; ++i) {
            initialSpecular[i]=specular[i];
        }
    }
    specular[0]=r;
    specular[1]=g;
    specular[2]=b;
    specular[3]=s;
}

const float* PMXMaterial::getInitialAmbient(){
    if(!initialAmbient)
    {
        initialAmbient=new float[3];
        for (int i = 0; i < 3; ++i) {
            initialAmbient[i]=ambient[i];
        }
    }
    return initialAmbient;
}

void PMXMaterial::setAmbient(float r, float g, float b) {
    if(!initialAmbient)
    {
        initialAmbient=new float[3];
        for (int i = 0; i < 3; ++i) {
            initialAmbient[i]=ambient[i];
        }
    }
    ambient[0]=r;
    ambient[1]=g;
    ambient[2]=b;
}

const float* PMXMaterial::getInitialEdgeColor(){
    if(!initialEdgeColor)
    {
        initialEdgeColor=new float[4];
        for (int i = 0; i < 4; ++i) {
            initialEdgeColor[i]=edgeColor[i];
        }
    }
    return initialEdgeColor;
}

void PMXMaterial::setEdgeColor(float r, float g, float b, float a) {
    if(!initialEdgeColor)
    {
        initialEdgeColor=new float[4];
        for (int i = 0; i < 4; ++i) {
            initialEdgeColor[i]=edgeColor[i];
        }
    }
    edgeColor[0]=r;
    edgeColor[1]=g;
    edgeColor[2]=b;
    edgeColor[3]=a;
}

void PMXMaterial::setTextureCoefficient(float r, float g, float b, float a) {
    textureCoefficient[0]=r;
    textureCoefficient[1]=g;
    textureCoefficient[2]=b;
    textureCoefficient[3]=a;
}

const float* PMXMaterial::getTextureCoefficient() {
    return textureCoefficient;
}

void PMXMaterial::setSphereCoefficient(float r, float g, float b, float a) {
    sphereCoefficient[0]=r;
    sphereCoefficient[1]=g;
    sphereCoefficient[2]=b;
    sphereCoefficient[3]=a;
}

const float* PMXMaterial::getSphereCoefficient() {
    return sphereCoefficient;
}

PMXMaterial::~PMXMaterial() {
    delete name;
    delete nameE;
    delete memo;
    if(initialDiffuse)delete [] initialDiffuse;
    if(initialSpecular)delete [] initialSpecular;
    if(initialAmbient)delete [] initialAmbient;
    if(initialEdgeColor)delete [] initialEdgeColor;
}
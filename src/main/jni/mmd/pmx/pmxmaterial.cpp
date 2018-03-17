//
// Created by wjy50 on 18-3-17.
//

#include "pmxmaterial.h"
#include "../../utils/mathutils.h"

PMXMaterial::PMXMaterial() {
    name=nameE=memo=0;
}

void
PMXMaterial::read(FILE *file, size_t textureSize, MStringEncoding encoding, float *diffuse, float *specular,
                  float *ambient, float *edgeColor)
{
    this->diffuse=diffuse;
    this->specular=specular;
    this->ambient=ambient;
    this->edgeColor=edgeColor;
    name= MString::readString(file,encoding);
    nameE= MString::readString(file,encoding);
    fread(diffuse, sizeof(float),4,file);
    fread(specular, sizeof(float),4,file);
    fread(ambient, sizeof(float),3,file);
    fread(&(flags), sizeof(char),1,file);
    fread(edgeColor, sizeof(float),4,file);
    fread(&(edgeSize), sizeof(float),1,file);
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
    memo=MString::readString(file,encoding);
    fread(&indexCount, sizeof(GLsizei),1,file);
    doubleSided=CHECK_FLAG(flags,DOUBLE_SIDED);
    shadow=CHECK_FLAG(flags,SHADOW);
    mThrowsShadow=CHECK_FLAG(flags,SELF_SHADOW_MAP);
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

bool PMXMaterial::throwsShadow() {
    return mThrowsShadow;
}

bool PMXMaterial::acceptShadow() {
    return mAcceptShadow;
}

PMXMaterial::~PMXMaterial() {
    delete name;
    delete nameE;
    delete memo;
}
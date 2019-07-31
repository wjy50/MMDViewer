//
// Created by wjy50 on 18-3-17.
//

#include "pmxmaterial.h"
#include "../../utils/mathutils.h"

PMXMaterial::PMXMaterial()
: initialAmbient(nullptr), initialDiffuse(nullptr),
  initialSpecular(nullptr), initialEdgeColor(nullptr)
{
    for (int i = 0; i < 4; ++i) {
        textureCoefficient[i] = 1;
    }
    for (int i = 0; i < 4; ++i) {
        sphereCoefficient[i] = 1;
    }
}

void
PMXMaterial::read(std::ifstream &file, size_t textureSize, MStringEncoding encoding, float *diffuse,
                  float *specular,
                  float *ambient, float *edgeColor)
{
    this->diffuse = diffuse;
    this->specular = specular;
    this->ambient = ambient;
    this->edgeColor = edgeColor;
    name.readString(file, encoding, UTF_8);
    nameE.readString(file, encoding, UTF_8);
    file.read(reinterpret_cast<char *>(diffuse), 4 * sizeof(float));
    file.read(reinterpret_cast<char *>(specular), 4 * sizeof(float));
    file.read(reinterpret_cast<char *>(ambient), 3 * sizeof(float));
    file.read(&flags, sizeof(char));
    file.read(reinterpret_cast<char *>(edgeColor), 4 * sizeof(float));
    file.read(reinterpret_cast<char *>(&edgeSize), sizeof(float));
    initialEdgeSize = edgeSize;
    textureIndex = 0;
    sphereIndex = 0;
    textureState = 0;
    sphereState = 0;
    file.read(reinterpret_cast<char *>(&textureIndex), textureSize);
    file.read(reinterpret_cast<char *>(&sphereIndex), textureSize);
    file.read(&sphereMode, sizeof(char));
    file.read(&sharedToon, sizeof(char));
    toonI = 0;
    if (sharedToon == 0)file.read(reinterpret_cast<char *>(&toonI), textureSize);
    else file.read(reinterpret_cast<char *>(&toonI), sizeof(char));
    memo.readString(file, encoding, UTF_8);
    file.read(reinterpret_cast<char *>(&indexCount), sizeof(GLsizei));
    doubleSided = CHECK_FLAG(flags, DOUBLE_SIDED);
    groundShadow = CHECK_FLAG(flags, GROUND_SHADOW);
    mCastShadow = CHECK_FLAG(flags, SELF_SHADOW_MAP);
    mAcceptShadow = CHECK_FLAG(flags, SELF_SHADOW);
    if (CHECK_FLAG(flags, DRAW_MODE_LINE))drawMode = GL_LINES;
    else if (CHECK_FLAG(flags, DRAW_MODE_POINT))drawMode = GL_POINTS;
    else drawMode = GL_TRIANGLES;
}

GLsizei PMXMaterial::getTextureIndex() const
{
    return textureIndex;
}

GLsizei PMXMaterial::getSphereIndex() const
{
    return sphereIndex;
}

char PMXMaterial::getSphereMode() const
{
    return sphereMode;
}

const float *PMXMaterial::getAmbient() const
{
    return ambient;
}

const float *PMXMaterial::getDiffuse() const
{
    return diffuse;
}

const float *PMXMaterial::getSpecular() const
{
    return specular;
}

GLsizei PMXMaterial::getIndexCount() const
{
    return indexCount;
}

GLenum PMXMaterial::getDrawMode() const
{
    return drawMode;
}

bool PMXMaterial::isDoubleSided() const
{
    return doubleSided;
}

long PMXMaterial::getOffset() const
{
    return offset;
}

void PMXMaterial::setOffset(long offset)
{
    this->offset = offset;
}

int PMXMaterial::getTextureState() const
{
    return textureState;
}

int PMXMaterial::getSphereState() const
{
    return sphereState;
}

void PMXMaterial::onTextureLoaded(bool isTextureSuccessful, bool isSphereSuccessful)
{
    if (isTextureSuccessful)
        textureState = 1;
    if (isSphereSuccessful) {
        if (sphereMode == 1)
            sphereState = 2;
        else if (sphereMode == 2)
            sphereState = 1;
    }
}

bool PMXMaterial::castShadow() const
{
    return mCastShadow;
}

bool PMXMaterial::acceptShadow() const
{
    return mAcceptShadow;
}

float PMXMaterial::getInitialEdgeSize() const
{
    return initialEdgeSize;
}

void PMXMaterial::setEdgeSize(float size)
{
    edgeSize = size;
}

const float *PMXMaterial::getInitialDiffuse() const
{
    return initialDiffuse ? initialDiffuse : diffuse;
}

void PMXMaterial::setDiffuse(float r, float g, float b, float a)
{
    if (!initialDiffuse) {
        initialDiffuse = new float[4];
        for (int i = 0; i < 4; ++i) {
            initialDiffuse[i] = diffuse[i];
        }
    }
    diffuse[0] = r;
    diffuse[1] = g;
    diffuse[2] = b;
    diffuse[3] = a;
}

const float *PMXMaterial::getInitialSpecular() const
{
    return initialSpecular ? initialSpecular : specular;
}

void PMXMaterial::setSpecular(float r, float g, float b, float s)
{
    if (!initialSpecular) {
        initialSpecular = new float[4];
        for (int i = 0; i < 4; ++i) {
            initialSpecular[i] = specular[i];
        }
    }
    specular[0] = r;
    specular[1] = g;
    specular[2] = b;
    specular[3] = s;
}

const float *PMXMaterial::getInitialAmbient() const
{
    return initialAmbient ? initialAmbient : ambient;
}

void PMXMaterial::setAmbient(float r, float g, float b)
{
    if (!initialAmbient) {
        initialAmbient = new float[3];
        for (int i = 0; i < 3; ++i) {
            initialAmbient[i] = ambient[i];
        }
    }
    ambient[0] = r;
    ambient[1] = g;
    ambient[2] = b;
}

const float *PMXMaterial::getInitialEdgeColor() const
{
    return initialEdgeColor ? initialEdgeColor : edgeColor;
}

void PMXMaterial::setEdgeColor(float r, float g, float b, float a)
{
    if (!initialEdgeColor) {
        initialEdgeColor = new float[4];
        for (int i = 0; i < 4; ++i) {
            initialEdgeColor[i] = edgeColor[i];
        }
    }
    edgeColor[0] = r;
    edgeColor[1] = g;
    edgeColor[2] = b;
    edgeColor[3] = a;
}

void PMXMaterial::setTextureCoefficient(float r, float g, float b, float a)
{
    textureCoefficient[0] = r;
    textureCoefficient[1] = g;
    textureCoefficient[2] = b;
    textureCoefficient[3] = a;
}

const float *PMXMaterial::getTextureCoefficient() const
{
    return textureCoefficient;
}

void PMXMaterial::setSphereCoefficient(float r, float g, float b, float a)
{
    sphereCoefficient[0] = r;
    sphereCoefficient[1] = g;
    sphereCoefficient[2] = b;
    sphereCoefficient[3] = a;
}

const float *PMXMaterial::getSphereCoefficient() const
{
    return sphereCoefficient;
}

PMXMaterial::~PMXMaterial()
{
    delete[] initialDiffuse;
    delete[] initialSpecular;
    delete[] initialAmbient;
    delete[] initialEdgeColor;
}
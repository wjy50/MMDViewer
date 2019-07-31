//
// Created by wjy50 on 18-3-17.
//

#ifndef MMDVIEWER_PMXMORPH_H
#define MMDVIEWER_PMXMORPH_H

#include "pmxcommon.h"
#include "../../utils/mstring.h"

class PMXMorphData
{
protected:
    int index;
public:
    virtual void read(std::ifstream &file, PMXInfo *info)=0;

    virtual int getIndex() const;

    virtual ~PMXMorphData();
};

class PMXGroupMorphData : public PMXMorphData
{
private:
    float ratio;
public:
    void read(std::ifstream &file, PMXInfo *info);

    float getRatio() const;
};

class PMXVertexMorphData : public PMXMorphData
{
private:
    float offset[3];
public:
    void read(std::ifstream &file, PMXInfo *info);

    const float *getOffset() const;
};

class PMXBoneMorphData : public PMXMorphData
{
private:
    float translation[3];
    float rotation[4];
public:
    void read(std::ifstream &file, PMXInfo *info);

    const float *getTranslation() const;

    const float *getRotation() const;
};

class PMXUVMorphData : public PMXMorphData
{
private:
    float offset[4];
public:
    void read(std::ifstream &file, PMXInfo *info);

    const float *getOffset() const;
};

class PMXMaterialMorphData : public PMXMorphData
{
private:
    char operation;
    float diffuse[4];
    float specular[4];  // specular[3] == shininess
    float ambient[3];
    float edgeColor[4];
    float edgeSize;
    float textureCoefficient[4];
    float sphereCoefficient[4];
    // something here (16 bytes) is ignored while reading
public:
    void read(std::ifstream &file, PMXInfo *info);

    char getOperation() const;

    const float *getDiffuse() const;

    const float *getSpecular() const;

    const float *getAmbient() const;

    const float *getEdgeColor() const;

    const float *getTextureCoefficient() const;

    const float *getSphereCoefficient() const;

    float getEdgeSize() const;
};

class PMXMorph
{
private:
    MString name, nameE;
    char panel, kind;
    int count;
    PMXMorphData **data;

    float fraction;
public:
    PMXMorph();

    void read(std::ifstream &file, PMXInfo *info);

    int getKind() const;

    int getMorphDataCount() const;

    PMXMorphData *getDataAt(int index) const;

    float setFraction(float fraction);

    float getFraction() const;

    const char *getName() const;

    ~PMXMorph();
};

#endif //MMDVIEWER_PMXMORPH_H

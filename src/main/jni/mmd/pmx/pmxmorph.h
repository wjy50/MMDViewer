//
// Created by wjy50 on 18-3-17.
//

#ifndef MMDVIEWER_PMXMORPH_H
#define MMDVIEWER_PMXMORPH_H

#include <stdio.h>
#include "pmxcommon.h"
#include "../../utils/mstring.h"

class PMXMorphData{
protected:
    unsigned int index;
public:
    virtual void read(FILE* file,PMXInfo* info)=0;
    virtual unsigned int getIndex();
    virtual ~PMXMorphData();
};

class PMXGroupMorphData:public PMXMorphData{
private:
    float ratio;
public:
    void read(FILE* file,PMXInfo* info);

    float getRatio();
};

class PMXVertexMorphData:public PMXMorphData{
private:
    float offset[3];
public:
    void read(FILE* file,PMXInfo* info);

    const float * getOffset();
};

class PMXBoneMorphData:public PMXMorphData{
private:
    float translation[3];
    float rotation[4];
public:
    void read(FILE* file,PMXInfo* info);

    const float * getTranslation();
    const float * getRotation();
};

class PMXUVMorphData:public PMXMorphData{
private:
    float offset[4];
public:
    void read(FILE* file,PMXInfo* info);

    const float * getOffset();
};

class PMXMaterialMorphData:public PMXMorphData{
private:
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

    char getOperation();
    const float * getDiffuse();
    const float * getSpecular();
    const float * getAmbient();
    const float * getEdgeColor();
    const float * getTextureCoefficient();
    const float * getSphereCoefficient();
    const float getEdgeSize();
};

class PMXMorph{
private:
    MString * name,*nameE;
    char panel,kind;
    int count;
    PMXMorphData** data;

    float fraction;
public:
    PMXMorph();
    void read(FILE* file,PMXInfo* info);

    int getKind();
    int getMorphDataCount();
    PMXMorphData* getDataAt(int index);

    float setFraction(float fraction);
    float getFraction();
    const char* getName();

    ~PMXMorph();
};

#endif //MMDVIEWER_PMXMORPH_H

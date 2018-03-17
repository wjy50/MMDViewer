//
// Created by wjy50 on 18-3-17.
//

#ifndef MMDVIEWER_PMXMORPH_H
#define MMDVIEWER_PMXMORPH_H

#include <stdio.h>
#include "pmxcommon.h"
#include "../../utils/mstring.h"

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

#endif //MMDVIEWER_PMXMORPH_H

//
// Created by wjy50 on 18-3-17.
//

#include "pmxmorph.h"

void PMXGroupMorphData::read(FILE *file, PMXInfo *info) {
    index=0;
    fread(&index,info->morphSize,1,file);
    fread(&ratio, sizeof(float),1,file);
}

void PMXVertexMorphData::read(FILE *file, PMXInfo *info) {
    index=0;
    fread(&index,info->vertexSize,1,file);
    fread(offset, sizeof(float),3,file);
}

void PMXBoneMorphData::read(FILE *file, PMXInfo *info) {
    index=0;
    fread(&index,info->boneSize,1,file);
    fread(translation, sizeof(float),3,file);
    fread(rotation, sizeof(float),4,file);
}

void PMXUVMorphData::read(FILE *file, PMXInfo *info) {
    index=0;
    fread(&index,info->vertexSize,1,file);
    fread(offset, sizeof(float),4,file);
}

void PMXMaterialMorphData::read(FILE *file, PMXInfo *info) {
    index=0;
    fread(&index,info->materialSize,1,file);
    fread(&operation, sizeof(char),1,file);
    fread(diffuse, sizeof(float),4,file);
    fread(specular, sizeof(float),4,file);
    fread(ambient, sizeof(float),3,file);
    fread(edgeColor, sizeof(float),4,file);
    fread(&edgeSize, sizeof(float),1,file);
    fread(textureCoefficient, sizeof(float),4,file);
    fread(sphereCoefficient, sizeof(float),4,file);
    fseek(file,16,SEEK_CUR);
}

PMXMorphData::~PMXMorphData() {

}

/*implementation of PMXMorph*/

PMXMorph::PMXMorph() {
    name=nameE=0;
    data=0;
}

void PMXMorph::read(FILE *file, PMXInfo *info) {
    name=MString::readString(file, (MStringEncoding) info->encoding);
    nameE=MString::readString(file, (MStringEncoding) info->encoding);
    fread(&panel, sizeof(char),1,file);
    fread(&kind, sizeof(char),1,file);
    fread(&count, sizeof(int),1,file);
    if(count > 0)
    {
        data=new PMXMorphData*[count];
        switch (kind)
        {
            case 0:
            case 9:
                for (int i = 0; i < count; ++i) {
                    data[i]=new PMXGroupMorphData;
                    data[i]->read(file,info);
                }
                break;
            case 1:
                for (int i = 0; i < count; ++i) {
                    data[i]=new PMXVertexMorphData;
                    data[i]->read(file,info);
                }
                break;
            case 2:
                for (int i = 0; i < count; ++i) {
                    data[i]=new PMXBoneMorphData;
                    data[i]->read(file,info);
                }
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                for (int i = 0; i < count; ++i) {
                    data[i]=new PMXUVMorphData;
                    data[i]->read(file,info);
                }
                break;
            case 8:
                for (int i = 0; i < count; ++i) {
                    data[i]=new PMXMaterialMorphData;
                    data[i]->read(file,info);
                }
                break;
            case 10:
                delete [] data;
                data=0;
                fseek(file,count*(info->bodySize+25),SEEK_CUR);
                return;
            default:
                delete [] data;
                data=0;
                return;
        }
    }
}

PMXMorph::~PMXMorph() {
    delete name;
    delete nameE;
    if(data)
    {
        for (int i = 0; i < count; ++i) {
            delete data[i];
        }
        delete [] data;
    }
}

/*end of PMXMorph*/
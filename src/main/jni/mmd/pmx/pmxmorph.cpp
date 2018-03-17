//
// Created by wjy50 on 18-3-17.
//

#include <math.h>
#include "pmxmorph.h"
#include "../../vector/vector.h"
#include "../../utils/mathutils.h"

/*implementation of PMXGroupMorphData*/

void PMXGroupMorphData::read(FILE *file, PMXInfo *info) {
    index=0;
    fread(&index,info->morphSize,1,file);
    fread(&ratio, sizeof(float),1,file);
}

float PMXGroupMorphData::getRatio() {
    return ratio;
}

/*end of PMXGroupMorphData*/

/*implementation of PMXVertexMorphData*/

void PMXVertexMorphData::read(FILE *file, PMXInfo *info) {
    index=0;
    fread(&index,info->vertexSize,1,file);
    fread(offset, sizeof(float),3,file);
    offset[0]*=PMX_MODEL_SCALE;
    offset[1]*=PMX_MODEL_SCALE;
    offset[2]*=-PMX_MODEL_SCALE;
}

const float* PMXVertexMorphData::getOffset() {
    return offset;
}

/*end of PMXVertexMorphData*/

/*implementation of PMXBoneMorphData*/

void PMXBoneMorphData::read(FILE *file, PMXInfo *info) {
    index=0;
    fread(&index,info->boneSize,1,file);
    fread(translation, sizeof(float),3,file);
    translation[0]*=PMX_MODEL_SCALE;
    translation[1]*=PMX_MODEL_SCALE;
    translation[2]*=-PMX_MODEL_SCALE;
    fread(rotation, sizeof(float),4,file);
    rotation[2]=-rotation[2];
    if(fabsf(rotation[3]) >= 1-1e-6f)rotation[3]=0;
    else
    {
        float a=acosf(rotation[3])*2;
        normalize3Into(rotation);
        rotation[3]= (float) (-a * RAD_TO_DEG);
    }
}

const float* PMXBoneMorphData::getRotation() {
    return rotation;
}

const float* PMXBoneMorphData::getTranslation() {
    return translation;
}

/*end of PMXBoneMorphData*/

/*implementation of PMXUVMorphData*/

void PMXUVMorphData::read(FILE *file, PMXInfo *info) {
    index=0;
    fread(&index,info->vertexSize,1,file);
    fread(offset, sizeof(float),4,file);
}

const float* PMXUVMorphData::getOffset() {
    return offset;
}

/*end of PMXUVMorphData*/

/*implementation of PMXMaterialMorphData*/

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

const float* PMXMaterialMorphData::getDiffuse() {
    return diffuse;
}

const float* PMXMaterialMorphData::getSpecular() {
    return specular;
}

const float* PMXMaterialMorphData::getAmbient() {
    return ambient;
}

const float* PMXMaterialMorphData::getEdgeColor() {
    return edgeColor;
}

const float* PMXMaterialMorphData::getTextureCoefficient() {
    return textureCoefficient;
}

const float* PMXMaterialMorphData::getSphereCoefficient() {
    return sphereCoefficient;
}

char PMXMaterialMorphData::getOperation() {
    return operation;
}

const float PMXMaterialMorphData::getEdgeSize() {
    return edgeSize;
}

/*end of PMXMaterialMorphData*/

/*implementation of PMXMorphData*/

unsigned int PMXMorphData::getIndex() {
    return index;
}

PMXMorphData::~PMXMorphData() {

}

/*end of PMXMorphData*/

/*implementation of PMXMorph*/

PMXMorph::PMXMorph() {
    name=nameE=0;
    data=0;
    fraction=0;
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

int PMXMorph::getKind() {
    return kind;
}

int PMXMorph::getMorphDataCount() {
    return count;
}

PMXMorphData* PMXMorph::getDataAt(int index) {
    return data[index];
}

float PMXMorph::setFraction(float fraction) {
    float delta=fraction-this->fraction;
    this->fraction=fraction;
    return delta;
}

float PMXMorph::getFraction() {
    return fraction;
}

const char* PMXMorph::getName() {
    return name->getData();
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
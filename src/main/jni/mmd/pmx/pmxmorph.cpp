//
// Created by wjy50 on 18-3-17.
//

#include <cmath>
#include "pmxmorph.h"
#include "../../vector/vector.h"
#include "../../utils/mathutils.h"
#include "../mmdcommon.h"
#include "../../utils/UniquePointerExt.h"

using namespace std;

/*implementation of PMXGroupMorphData*/

void PMXGroupMorphData::read(ifstream &file, PMXInfo *info)
{
    index = 0;
    file.read(reinterpret_cast<char *>(&index), info->morphSize);
    file.read(reinterpret_cast<char *>(&ratio), sizeof(float));
}

float PMXGroupMorphData::getRatio() const
{
    return ratio;
}

/*end of PMXGroupMorphData*/

/*implementation of PMXVertexMorphData*/

void PMXVertexMorphData::read(ifstream &file, PMXInfo *info)
{
    index = 0;
    file.read(reinterpret_cast<char *>(&index), info->vertexSize);
    file.read(reinterpret_cast<char *>(offset), 3 * sizeof(float));
    offset[0] *= MMD_COORDINATE_SCALE;
    offset[1] *= MMD_COORDINATE_SCALE;
    offset[2] *= -MMD_COORDINATE_SCALE;
}

const float *PMXVertexMorphData::getOffset() const
{
    return offset;
}

/*end of PMXVertexMorphData*/

/*implementation of PMXBoneMorphData*/

void PMXBoneMorphData::read(ifstream &file, PMXInfo *info)
{
    index = 0;
    file.read(reinterpret_cast<char *>(&index), info->boneSize);
    file.read(reinterpret_cast<char *>(translation), 3 * sizeof(float));
    file.read(reinterpret_cast<char *>(rotation), 4 * sizeof(float));
    translation[0] *= MMD_COORDINATE_SCALE;
    translation[1] *= MMD_COORDINATE_SCALE;
    translation[2] *= -MMD_COORDINATE_SCALE;
    rotation[2] = -rotation[2];
    if (fabsf(rotation[3]) >= 1 - 1e-6f)
        rotation[3] = 0;
    else {
        float a = acosf(rotation[3]) * 2;
        normalize3Into(rotation);
        rotation[3] = (float) (-a * RAD_TO_DEG);
    }
}

const float *PMXBoneMorphData::getRotation() const
{
    return rotation;
}

const float *PMXBoneMorphData::getTranslation() const
{
    return translation;
}

/*end of PMXBoneMorphData*/

/*implementation of PMXUVMorphData*/

void PMXUVMorphData::read(ifstream &file, PMXInfo *info)
{
    index = 0;
    file.read(reinterpret_cast<char *>(&index), info->vertexSize);
    file.read(reinterpret_cast<char *>(offset), 4 * sizeof(float));
}

const float *PMXUVMorphData::getOffset() const
{
    return offset;
}

/*end of PMXUVMorphData*/

/*implementation of PMXMaterialMorphData*/

void PMXMaterialMorphData::read(ifstream &file, PMXInfo *info)
{
    index = 0;
    file.read(reinterpret_cast<char *>(&index), info->materialSize);
    file.read(&operation, sizeof(char));
    file.read(reinterpret_cast<char *>(diffuse), 4 * sizeof(float));
    file.read(reinterpret_cast<char *>(specular), 4 * sizeof(float));
    file.read(reinterpret_cast<char *>(ambient), 3 * sizeof(float));
    file.read(reinterpret_cast<char *>(edgeColor), 4 * sizeof(float));
    file.read(reinterpret_cast<char *>(&edgeSize), sizeof(float));
    file.read(reinterpret_cast<char *>(textureCoefficient), 4 * sizeof(float));
    file.read(reinterpret_cast<char *>(sphereCoefficient), 4 * sizeof(float));
    file.seekg(16, ios::cur);
}

const float *PMXMaterialMorphData::getDiffuse() const
{
    return diffuse;
}

const float *PMXMaterialMorphData::getSpecular() const
{
    return specular;
}

const float *PMXMaterialMorphData::getAmbient() const
{
    return ambient;
}

const float *PMXMaterialMorphData::getEdgeColor() const
{
    return edgeColor;
}

const float *PMXMaterialMorphData::getTextureCoefficient() const
{
    return textureCoefficient;
}

const float *PMXMaterialMorphData::getSphereCoefficient() const
{
    return sphereCoefficient;
}

char PMXMaterialMorphData::getOperation() const
{
    return operation;
}

float PMXMaterialMorphData::getEdgeSize() const
{
    return edgeSize;
}

/*end of PMXMaterialMorphData*/

/*implementation of PMXMorphData*/

int PMXMorphData::getIndex() const
{
    return index;
}

PMXMorphData::~PMXMorphData()
{

}

/*end of PMXMorphData*/

/*implementation of PMXMorph*/

PMXMorph::PMXMorph()
: data(nullptr), fraction(0)
{}

void PMXMorph::read(ifstream &file, PMXInfo *info)
{
    name.readString(file, static_cast<MStringEncoding>(info->encoding), UTF_8);
    nameE.readString(file, static_cast<MStringEncoding>(info->encoding), UTF_8);
    file.read(&panel, sizeof(panel));
    file.read(&kind, sizeof(kind));
    file.read(reinterpret_cast<char *>(&count), sizeof(count));
    if (count > 0) {
        auto data = make_unique_array<PMXMorphData *[]>(count);
        memset(data.get(), 0, sizeof(PMXMorphData*) * count);
        switch (kind) {
            case 0:
            case 9:
                for (int i = 0; i < count; ++i) {
                    data[i] = new PMXGroupMorphData;
                    data[i]->read(file, info);
                }
                break;
            case 1:
                for (int i = 0; i < count; ++i) {
                    data[i] = new PMXVertexMorphData;
                    data[i]->read(file, info);
                }
                break;
            case 2:
                for (int i = 0; i < count; ++i) {
                    data[i] = new PMXBoneMorphData;
                    data[i]->read(file, info);
                }
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                for (int i = 0; i < count; ++i) {
                    data[i] = new PMXUVMorphData;
                    data[i]->read(file, info);
                }
                break;
            case 8:
                for (int i = 0; i < count; ++i) {
                    data[i] = new PMXMaterialMorphData;
                    data[i]->read(file, info);
                }
                break;
            case 10:
                file.seekg(count * (info->bodySize + 25), ios::cur);
                return;
            default:
                return;
        }
        this->data = data.release();
    }
}

int PMXMorph::getKind() const
{
    return kind;
}

int PMXMorph::getMorphDataCount() const
{
    return count;
}

PMXMorphData *PMXMorph::getDataAt(int index) const
{
    return data[index];
}

float PMXMorph::setFraction(float fraction)
{
    float delta = fraction - this->fraction;
    this->fraction = fraction;
    return delta;
}

float PMXMorph::getFraction() const
{
    return fraction;
}

const char *PMXMorph::getName() const
{
    return name.getData();
}

PMXMorph::~PMXMorph()
{
    if (data) {
        for (int i = 0; i < count; ++i) {
            delete data[i];
            data[i] = nullptr;
        }
        delete[] data;
    }
}

/*end of PMXMorph*/
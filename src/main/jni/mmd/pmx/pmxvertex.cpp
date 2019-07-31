//
// Created by wjy50 on 18-3-17.
//

#include "pmxvertex.h"
#include "../../utils/mathutils.h"
#include "../mmdcommon.h"

using namespace std;

PMXVertex::PMXVertex()
: initialCoordinate(nullptr), initialUV(nullptr), bones(nullptr),
  weights(nullptr), sDefVec(nullptr)
{}

void PMXVertex::read(ifstream &file, PMXInfo *info, float *coordinate, float *normal, float *uv)
{
    this->coordinate = coordinate;
    this->normal = normal;
    this->uv = uv;
    file.read(reinterpret_cast<char *>(coordinate), 3 * sizeof(float));
    coordinate[2] = -coordinate[2];
    for (int i = 0; i < 3; ++i)
        coordinate[i] *= MMD_COORDINATE_SCALE;
    file.read(reinterpret_cast<char *>(normal), 3 * sizeof(float));
    normal[2] = -normal[2];
    file.read(reinterpret_cast<char *>(uv), 2 * sizeof(float));
    int sk = MIN(info->UVACount, 4);
    file.seekg(sk * 16, ios::cur);
    file.read(&deform, sizeof(char));
    switch (deform) {
        case 0:
            bones = new int[1];
            weights = new float[1];
            boneCount = 1;
            bones[0] = 0;
            weights[0] = 1;
            file.read(reinterpret_cast<char *>(bones), info->boneSize);
            break;
        case 1:
            bones = new int[2];
            weights = new float[2];
            boneCount = 2;
            bones[0] = bones[1] = 0;
            file.read(reinterpret_cast<char *>(bones), info->boneSize);
            file.read(reinterpret_cast<char *>(bones + 1), info->boneSize);
            file.read(reinterpret_cast<char *>(weights), sizeof(float));
            weights[1] = 1 - weights[0];
            break;
        case 2:
        case 4:
            bones = new int[4];
            weights = new float[4];
            boneCount = 4;
            bones[0] = bones[1] = bones[2] = bones[3] = 0;
            file.read(reinterpret_cast<char *>(bones), info->boneSize);
            file.read(reinterpret_cast<char *>(bones + 1), info->boneSize);
            file.read(reinterpret_cast<char *>(bones + 2), info->boneSize);
            file.read(reinterpret_cast<char *>(bones + 3), info->boneSize);
            file.read(reinterpret_cast<char *>(weights), 4 * sizeof(float));
            break;
        case 3:
            bones = new int[2];
            weights = new float[2];
            boneCount = 2;
            bones[0] = bones[1] = 0;
            file.read(reinterpret_cast<char *>(bones), info->boneSize);
            file.read(reinterpret_cast<char *>(bones + 1), info->boneSize);
            file.read(reinterpret_cast<char *>(weights), sizeof(float));
            weights[1] = 1 - weights[0];
            float sDefVec[9];
            file.read(reinterpret_cast<char *>(sDefVec), 9 * sizeof(float));
            break;
        default:
            break;
    }
    file.seekg(4, ios::cur);
}

char PMXVertex::getBoneCount()
{
    return boneCount;
}

int PMXVertex::getBoneAt(int index)
{
    return bones[index];
}

float PMXVertex::getWeightAt(int index)
{
    return weights[index];
}

void PMXVertex::setBoneAt(int index, int bone)
{
    bones[index] = bone;
}

const float *PMXVertex::getInitialCoordinate()
{
    return initialCoordinate ? initialCoordinate : coordinate;
}

void PMXVertex::setPosition(float x, float y, float z)
{
    if (!initialCoordinate) {
        initialCoordinate = new float[3];
        for (int i = 0; i < 3; ++i) {
            initialCoordinate[i] = coordinate[i];
        }
    }
    coordinate[0] = x;
    coordinate[1] = y;
    coordinate[2] = z;
}

const float *PMXVertex::getInitialUV()
{
    if (!initialUV) {
        initialUV = new float[2];
        for (int i = 0; i < 2; ++i) {
            initialUV[i] = uv[i];
        }
    }
    return initialUV;
}

void PMXVertex::setUV(float u, float v)
{
    if (!initialUV) {
        initialUV = new float[2];
        for (int i = 0; i < 2; ++i) {
            initialUV[i] = uv[i];
        }
    }
    uv[0] = u;
    uv[1] = v;
}

PMXVertex::~PMXVertex()
{
    delete[] bones;
    delete[] weights;
    delete[] sDefVec;
    delete[] initialCoordinate;
    delete[] initialUV;
}
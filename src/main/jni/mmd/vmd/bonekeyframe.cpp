//
// Created by wjy50 on 18-3-18.
//

#include "bonekeyframe.h"
#include "../mmdcommon.h"

using namespace std;

BoneKeyFrame::BoneKeyFrame() : VMDKeyFrame(VMD_BONE_KEYFRAME)
{}

void BoneKeyFrame::read(ifstream &file)
{
    name.readStringAndTrim(file, 15, SHIFT_JIS, UTF_8);
    file.read(reinterpret_cast<char *>(&index), sizeof(unsigned int));
    file.read(reinterpret_cast<char *>(translation), 3 * sizeof(float));
    file.read(reinterpret_cast<char *>(rotation), 4 * sizeof(float));
    char buffer[16];
    float uBuffer[16];
    file.read(buffer, 16 * sizeof(char));
    for (int i = 0; i < 4; ++i) {
        int o = i<<2;
        uBuffer[     i] = (float)buffer[o    ] / 0x7f;
        uBuffer[4  + i] = (float)buffer[o + 1] / 0x7f;
        uBuffer[8  + i] = (float)buffer[o + 2] / 0x7f;
        uBuffer[12 + i] = (float)buffer[o + 3] / 0x7f;
    }
    file.seekg(48, ios::cur);

    translation[2] = -translation[2];
    for (int i = 0; i < 3; ++i) translation[i] *= MMD_COORDINATE_SCALE;

    rotation[2] = -rotation[2];
}

const MString &BoneKeyFrame::getName() const
{
    return name;
}
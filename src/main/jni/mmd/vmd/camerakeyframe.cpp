//
// Created by wjy50 on 18-3-18.
//

#include "camerakeyframe.h"
#include "../mmdcommon.h"

using namespace std;

CameraKeyFrame::CameraKeyFrame() : VMDKeyFrame(VMD_CAMERA_KEYFRAME)
{}

void CameraKeyFrame::read(ifstream &file)
{
    file.read(reinterpret_cast<char *>(&index), sizeof(int));
    file.read(reinterpret_cast<char *>(&distance), sizeof(float));
    file.read(reinterpret_cast<char *>(position), 3 * sizeof(float));
    file.read(reinterpret_cast<char *>(euler), 3 * sizeof(float));
    char buffer[6 * 4];
    float uBuffer[6 * 4];
    file.read(buffer, 6 * 4 * sizeof(char));
    for (int i = 0; i < 6; ++i) {
        int o = i * 6;
        uBuffer[o    ] = (float)buffer[o    ] / 0x7f;
        uBuffer[o + 2] = (float)buffer[o + 1] / 0x7f;
        uBuffer[o + 1] = (float)buffer[o + 2] / 0x7f;
        uBuffer[o + 3] = (float)buffer[o + 3] / 0x7f;
    }
    file.read(reinterpret_cast<char *>(&aspect), sizeof(int));
    file.seekg(1, ios::cur);

    distance *= MMD_COORDINATE_SCALE;

    position[2] = -position[2];
    for (int i = 0; i < 3; ++i) position[i] *= MMD_COORDINATE_SCALE;
}
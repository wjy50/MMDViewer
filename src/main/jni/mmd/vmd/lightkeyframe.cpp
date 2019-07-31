//
// Created by wjy50 on 18-3-18.
//

#include "lightkeyframe.h"

LightKeyFrame::LightKeyFrame() : VMDKeyFrame(VMD_LIGHT_KEYFRAME)
{}

void LightKeyFrame::read(std::ifstream &file)
{
    file.read(reinterpret_cast<char *>(&index), sizeof(int));
    file.read(reinterpret_cast<char *>(&strength), 3 * sizeof(float));
    file.read(reinterpret_cast<char *>(&position), 3 * sizeof(float));
}
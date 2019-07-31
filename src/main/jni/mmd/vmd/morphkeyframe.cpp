//
// Created by wjy50 on 18-3-18.
//

#include "morphkeyframe.h"

MorphKeyFrame::MorphKeyFrame() : VMDKeyFrame(VMD_MORPH_KEYFRAME)
{}

void MorphKeyFrame::read(std::ifstream &file)
{
    name.readStringAndTrim(file, 15, SHIFT_JIS, UTF_8);
    file.read(reinterpret_cast<char *>(&index), sizeof(int));
    file.read(reinterpret_cast<char *>(&fraction), sizeof(float));
}

const MString &MorphKeyFrame::getName() const
{
    return name;
}
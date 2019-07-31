//
// Created by wjy50 on 18-3-18.
//

#include "ikkeyframe.h"

using namespace std;

IKKeyFrame::IKKeyFrame()
: VMDKeyFrame(VMD_IK_KEYFRAME), show(false), switches(nullptr)
{}

void IKKeyFrame::read(ifstream &file)
{
    file.read(reinterpret_cast<char *>(&index), sizeof(int));
    file.read(reinterpret_cast<char *>(&show), sizeof(char));
    file.read(reinterpret_cast<char *>(&ikCount), sizeof(int));
    if (ikCount > 0) {
        switches = new IKSwitch[ikCount];
        for (int i = 0; i < ikCount; ++i) {
            switches[i].read(file);
        }
    }
}

IKKeyFrame::~IKKeyFrame()
{
    delete[] switches;
}

IKKeyFrame::IKSwitch::IKSwitch()
: on(false)
{}

void IKKeyFrame::IKSwitch::read(ifstream &file)
{
    name.readStringAndTrim(file, 20, SHIFT_JIS, UTF_8);
    file.read(reinterpret_cast<char *>(&on), sizeof(char));
}
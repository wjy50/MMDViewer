//
// Created by wjy50 on 18-3-18.
//

#ifndef MMDVIEWER_LIGHTKEYFRAME_H
#define MMDVIEWER_LIGHTKEYFRAME_H

#include <fstream>
#include "vmdkeyframe.h"

class LightKeyFrame : public VMDKeyFrame
{
private:
    float strength[3];
    float position[3];
public:
    LightKeyFrame();

    void read(std::ifstream &file);
};

#endif //MMDVIEWER_LIGHTKEYFRAME_H

//
// Created by wjy50 on 18-3-18.
//

#ifndef MMDVIEWER_CAMERAKEYFRAME_H
#define MMDVIEWER_CAMERAKEYFRAME_H

#include <fstream>
#include "vmdkeyframe.h"

class CameraKeyFrame : public VMDKeyFrame
{
private:
    float distance;
    float position[3];
    float euler[3];
    int aspect;
public:
    CameraKeyFrame();

    void read(std::ifstream &file);
};

#endif //MMDVIEWER_CAMERAKEYFRAME_H

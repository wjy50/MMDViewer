//
// Created by wjy50 on 18-3-18.
//

#ifndef MMDVIEWER_BONEKEYFRAME_H
#define MMDVIEWER_BONEKEYFRAME_H

#include "vmdkeyframe.h"
#include "../../utils/mstring.h"

class BoneKeyFrame : public VMDKeyFrame
{
protected:
    MString name;

    float translation[3];
    float rotation[4];
public:
    BoneKeyFrame();

    void read(std::ifstream &file);

    const MString &getName() const;
};

#endif //MMDVIEWER_BONEKEYFRAME_H

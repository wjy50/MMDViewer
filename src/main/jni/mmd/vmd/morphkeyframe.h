//
// Created by wjy50 on 18-3-18.
//

#ifndef MMDVIEWER_MORPHKEYFRAME_H
#define MMDVIEWER_MORPHKEYFRAME_H

#include "vmdkeyframe.h"
#include "../../utils/mstring.h"

class MorphKeyFrame : public VMDKeyFrame
{
private:
    MString name;
    float fraction;
public:
    MorphKeyFrame();

    void read(std::ifstream &file);

    const MString &getName() const;
};

#endif //MMDVIEWER_MORPHKEYFRAME_H

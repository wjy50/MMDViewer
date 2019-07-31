//
// Created by wjy50 on 18-3-18.
//

#ifndef MMDVIEWER_IKKEYFRAME_H
#define MMDVIEWER_IKKEYFRAME_H

#include "vmdkeyframe.h"
#include "../../utils/mstring.h"

class IKKeyFrame : public VMDKeyFrame
{
public:
    class IKSwitch
    {
    private:
        MString name;
        bool on;

    public:
        IKSwitch();

        void read(std::ifstream &file);
    };
private:
    bool show;
    int ikCount;
    IKSwitch *switches;
public:
    IKKeyFrame();

    void read(std::ifstream &file);

    ~IKKeyFrame();
};

#endif //MMDVIEWER_IKKEYFRAME_H

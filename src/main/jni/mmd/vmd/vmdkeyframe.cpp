//
// Created by wjy50 on 18-4-4.
//

#include "vmdkeyframe.h"

VMDKeyFrame::VMDKeyFrame(VMDKeyFrameType type)
: mType(type), index(0)
{}

int VMDKeyFrame::getIndex() const
{
    return index;
}

void VMDKeyFrame::setIndex(int index)
{
    this->index = index;
}

VMDKeyFrameType VMDKeyFrame::getType() const
{
    return mType;
}
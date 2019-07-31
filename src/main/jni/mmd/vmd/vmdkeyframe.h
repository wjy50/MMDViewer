//
// Created by wjy50 on 18-4-4.
//

#ifndef MMDVIEWER_VMDKEYFRAME_H
#define MMDVIEWER_VMDKEYFRAME_H

typedef enum VMD_KEYFRAME_TYPE
{
    VMD_BONE_KEYFRAME,
    VMD_MORPH_KEYFRAME,
    VMD_CAMERA_KEYFRAME,
    VMD_LIGHT_KEYFRAME,
    VMD_IK_KEYFRAME
}VMDKeyFrameType;

class VMDKeyFrame
{
protected:
    VMDKeyFrameType mType;

    int index;

public:
    VMDKeyFrame(VMDKeyFrameType type);

    VMDKeyFrameType getType() const;

    int getIndex() const;

    void setIndex(int index);
};


#endif //MMDVIEWER_VMDKEYFRAME_H

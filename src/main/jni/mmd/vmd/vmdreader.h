//
// Created by wjy50 on 18-3-18.
//

#ifndef MMDVIEWER_VMDREADER_H
#define MMDVIEWER_VMDREADER_H

#include <fstream>
#include "../../utils/mstring.h"
#include "bonekeyframe.h"
#include "morphkeyframe.h"
#include "camerakeyframe.h"
#include "lightkeyframe.h"
#include "ikkeyframe.h"

class VMDReader
{
private:
    MString modelName;

    int boneKeyFrameCount;
    BoneKeyFrame *boneKeyFrames;

    int morphKeyFrameCount;
    MorphKeyFrame *morphKeyFrames;

    int cameraKeyFrameCount;
    CameraKeyFrame *cameraKeyFrames;

    int lightKeyFrameCount;
    LightKeyFrame *lightKeyFrames;

    int ikKeyFrameCount;
    IKKeyFrame *ikKeyFrames;

    void readBoneKeyFrames(std::ifstream &file);

    void readMorphKeyFrames(std::ifstream &file);

    void readCameraKeyFrames(std::ifstream &file);

    void readLightKeyFrames(std::ifstream &file);

    bool readSelfShadowKeyFramesIfAvailable(std::ifstream &file);

    void readIKKeyFramesIfAvailable(std::ifstream &file);

public:
    VMDReader(const char *filePath);

    ~VMDReader();
};

#endif //MMDVIEWER_VMDREADER_H

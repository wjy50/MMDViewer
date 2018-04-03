//
// Created by wjy50 on 18-3-18.
//

#ifndef MMDVIEWER_VMDREADER_H
#define MMDVIEWER_VMDREADER_H

#include <stdio.h>
#include "../../utils/mstring.h"
#include "bonekeyframe.h"
#include "morphkeyframe.h"
#include "camerakeyframe.h"
#include "lightkeyframe.h"
#include "ikkeyframe.h"

class VMDReader{
private:
    MString* modelName;

    unsigned int boneKeyFrameCount;
    BoneKeyFrame* boneKeyFrames;

    unsigned int morphKeyFrameCount;
    MorphKeyFrame* morphKeyFrames;

    unsigned int cameraKeyFrameCount;
    CameraKeyFrame* cameraKeyFrames;

    unsigned int lightKeyFrameCount;
    LightKeyFrame* lightKeyFrames;

    unsigned int ikKeyFrameCount;
    IKKeyFrame* ikKeyFrames;

    void readBoneKeyFrames(FILE* file);
    void readMorphKeyFrames(FILE* file);
    void readCameraKeyFrames(FILE* file);
    void readLightKeyFrames(FILE* file);
    bool readSelfShadowKeyFramesIfAvailable(FILE *file);
    void readIKKeyFramesIfAvailable(FILE* file);
public:
    VMDReader(const char * filePath);

    ~VMDReader();
};

#endif //MMDVIEWER_VMDREADER_H

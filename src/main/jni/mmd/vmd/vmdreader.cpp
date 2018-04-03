//
// Created by wjy50 on 18-3-18.
//

#include "vmdreader.h"
#include "../../utils/debugutils.h"

VMDReader::VMDReader(const char *filePath) {
    FILE* file=fopen(filePath,"rb");
    if(file)
    {
        MString* header= MString::readStringAndTrim(file, 30, UTF_16_LE, UTF_16_LE);
        MString* verifier=new MString("Vocaloid Motion Data 0002", true);
        bool b=verifier->equals(*header);
        delete header;
        delete verifier;
        if(!b)
        {
            fclose(file);
            LOG_PRINTLN("Not a VMD file.");
            return;
        }
        modelName= MString::readStringAndTrim(file, 20, UTF_16_LE, UTF_16_LE);

        readBoneKeyFrames(file);

        readMorphKeyFrames(file);

        readCameraKeyFrames(file);

        readLightKeyFrames(file);

        if(readSelfShadowKeyFramesIfAvailable(file))
        {
            readIKKeyFramesIfAvailable(file);
        }
    }
}

void VMDReader::readBoneKeyFrames(FILE *file) {
    fread(&boneKeyFrameCount, sizeof(unsigned int),1,file);
    if(boneKeyFrameCount > 0)
    {
        boneKeyFrames=new BoneKeyFrame[boneKeyFrameCount];
        for (int i = 0; i < boneKeyFrameCount; ++i) {
            boneKeyFrames[i].read(file);
        }
    }
    else boneKeyFrames=0;
}

void VMDReader::readMorphKeyFrames(FILE *file) {
    fread(&morphKeyFrameCount, sizeof(unsigned int),1,file);
    if(morphKeyFrameCount > 0)
    {
        morphKeyFrames=new MorphKeyFrame[morphKeyFrameCount];
        for (int i = 0; i < morphKeyFrameCount; ++i) {
            morphKeyFrames[i].read(file);
        }
    }
    else morphKeyFrames=0;
}

void VMDReader::readCameraKeyFrames(FILE *file) {
    fread(&cameraKeyFrameCount, sizeof(unsigned int),1,file);
    if(cameraKeyFrameCount > 0)
    {
        cameraKeyFrames=new CameraKeyFrame[cameraKeyFrameCount];
        for (int i = 0; i < cameraKeyFrameCount; ++i) {
            cameraKeyFrames[i].read(file);
        }
    }
    else cameraKeyFrames=0;
}

void VMDReader::readLightKeyFrames(FILE *file) {
    fread(&lightKeyFrameCount, sizeof(unsigned int),1,file);
    if(lightKeyFrameCount > 0)
    {
        lightKeyFrames=new LightKeyFrame[lightKeyFrameCount];
        for (int i = 0; i < lightKeyFrameCount; ++i) {
            lightKeyFrames[i].read(file);
        }
    }
    else lightKeyFrames=0;
}

bool VMDReader::readSelfShadowKeyFramesIfAvailable(FILE *file) {
    long currentPosition=ftell(file);
    fseek(file,0,SEEK_END);
    if(ftell(file) > currentPosition)
    {
        fseek(file,currentPosition,SEEK_SET);
        unsigned int selfShadowKeyFrameCount;
        fread(&selfShadowKeyFrameCount, sizeof(unsigned int),1,file);
        fseek(file,selfShadowKeyFrameCount*9,SEEK_CUR);
        return true;
    }
    //else selfShadowKeyFrames=0;
    return false;
}

void VMDReader::readIKKeyFramesIfAvailable(FILE *file) {
    long currentPosition=ftell(file);
    fseek(file,0,SEEK_END);
    if(ftell(file) > currentPosition)
    {
        fseek(file,currentPosition,SEEK_SET);
        fread(&ikKeyFrameCount, sizeof(unsigned int),1,file);
        if(ikKeyFrameCount > 0)
        {
            ikKeyFrames=new IKKeyFrame[ikKeyFrameCount];
            for (int i = 0; i < ikKeyFrameCount; ++i) {
                ikKeyFrames[i].read(file);
            }
        }
        else ikKeyFrames=0;
    }
    else ikKeyFrames=0;
}

VMDReader::~VMDReader() {
    delete modelName;
    if(boneKeyFrames)delete [] boneKeyFrames;
    if(morphKeyFrames)delete [] morphKeyFrames;
    if(cameraKeyFrames)delete [] cameraKeyFrames;
    if(lightKeyFrames)delete [] lightKeyFrames;
    if(ikKeyFrames)delete [] ikKeyFrames;
}
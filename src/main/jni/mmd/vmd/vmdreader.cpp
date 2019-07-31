//
// Created by wjy50 on 18-3-18.
//

#include "vmdreader.h"
#include "../../utils/debugutils.h"

using namespace std;

VMDReader::VMDReader(const char *filePath)
: boneKeyFrames(nullptr), morphKeyFrames(nullptr), cameraKeyFrames(nullptr),
  lightKeyFrames(nullptr), ikKeyFrames(nullptr)
{
    ifstream file(filePath, ios::binary);
    if (file) {
        MString header;
        header.readStringAndTrim(file, 30, SHIFT_JIS, UTF_8);
        MString verifier("Vocaloid Motion Data 0002");
        bool b = verifier.equals(header);
        if (!b) {
            file.close();
            LOG_PRINTLN("Not a VMD file.");
            return;
        }
        modelName.readStringAndTrim(file, 20, SHIFT_JIS, UTF_8);

        readBoneKeyFrames(file);

        readMorphKeyFrames(file);

        readCameraKeyFrames(file);

        readLightKeyFrames(file);

        if (readSelfShadowKeyFramesIfAvailable(file)) {
            readIKKeyFramesIfAvailable(file);
        }

        file.close();
    }
}

void VMDReader::readBoneKeyFrames(ifstream &file)
{
    file.read(reinterpret_cast<char *>(&boneKeyFrameCount), sizeof(int));
    if (boneKeyFrameCount != 0) {
        boneKeyFrames = new BoneKeyFrame[boneKeyFrameCount];
        for (int i = 0; i < boneKeyFrameCount; ++i) {
            boneKeyFrames[i].read(file);
        }
    }
}

void VMDReader::readMorphKeyFrames(ifstream &file)
{
    file.read(reinterpret_cast<char *>(&morphKeyFrameCount), sizeof(int));
    if (morphKeyFrameCount > 0) {
        morphKeyFrames = new MorphKeyFrame[morphKeyFrameCount];
        for (int i = 0; i < morphKeyFrameCount; ++i) {
            morphKeyFrames[i].read(file);
        }
    }
}

void VMDReader::readCameraKeyFrames(ifstream &file)
{
    file.read(reinterpret_cast<char *>(&cameraKeyFrameCount), sizeof(int));
    if (cameraKeyFrameCount > 0) {
        cameraKeyFrames = new CameraKeyFrame[cameraKeyFrameCount];
        for (int i = 0; i < cameraKeyFrameCount; ++i) {
            cameraKeyFrames[i].read(file);
        }
    }
}

void VMDReader::readLightKeyFrames(ifstream &file)
{
    file.read(reinterpret_cast<char *>(&lightKeyFrameCount), sizeof(int));
    if (lightKeyFrameCount > 0) {
        lightKeyFrames = new LightKeyFrame[lightKeyFrameCount];
        for (int i = 0; i < lightKeyFrameCount; ++i) {
            lightKeyFrames[i].read(file);
        }
    }
}

bool VMDReader::readSelfShadowKeyFramesIfAvailable(ifstream &file)
{
    long currentPosition = file.tellg();
    file.seekg(0, ios::end);
    if (file.tellg() > currentPosition) {
        file.seekg(currentPosition, ios::beg);
        unsigned int selfShadowKeyFrameCount;
        file.read(reinterpret_cast<char *>(&selfShadowKeyFrameCount), sizeof(int));
        file.seekg(selfShadowKeyFrameCount * 9, ios::cur);
        return true;
    }
    //selfShadowKeyFrames=nullptr;
    return false;
}

void VMDReader::readIKKeyFramesIfAvailable(ifstream &file)
{
    long currentPosition = file.tellg();
    file.seekg(0, ios::end);
    if (file.tellg() > currentPosition) {
        file.seekg(currentPosition, ios::beg);
        file.read(reinterpret_cast<char *>(&ikKeyFrameCount), sizeof(int));
        if (ikKeyFrameCount > 0) {
            ikKeyFrames = new IKKeyFrame[ikKeyFrameCount];
            for (int i = 0; i < ikKeyFrameCount; ++i) {
                ikKeyFrames[i].read(file);
            }
        }
    }
}

VMDReader::~VMDReader()
{
    delete[] boneKeyFrames;
    delete[] morphKeyFrames;
    delete[] cameraKeyFrames;
    delete[] lightKeyFrames;
    delete[] ikKeyFrames;
}
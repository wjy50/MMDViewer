//
// Created by wjy50 on 2018/2/5.
//

#ifndef MMDVIEWER_CAMERA_H
#define MMDVIEWER_CAMERA_H

#include "abscamera.h"

class SceneCamera : public AbsCamera
{
protected:
    float transMat[32];
    float viewMat[16];
    float distance;
    bool newTrans;
    float rx, rz;
public:
    SceneCamera(float, float, float, float);

    void set(float, float, float, float);

    void translate(float, float, float);

    void rotate(float, float, float);

    const float *getViewMat();

    ~SceneCamera();
};

#endif //MMDVIEWER_CAMERA_H

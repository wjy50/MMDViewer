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

    float ax, ay, az;

    float speedX, speedY, speedZ;
    float curSpeedX, curSpeedY, curSpeedZ;
    long lastTime;
public:
    SceneCamera(float, float, float, float);

    void set(float, float, float, float);

    void translate(float, float, float);

    void setMoveSpeed(float x, float y, float z);

    void addMoveSpeed(float x, float y, float z);

    void rotate(float, float, float);

    const float *getViewMat();

    ~SceneCamera();
};

#endif //MMDVIEWER_CAMERA_H

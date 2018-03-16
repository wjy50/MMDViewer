//
// Created by wjy50 on 2018/2/8.
//

#ifndef MMDVIEWER_ABSCAMERA_H
#define MMDVIEWER_ABSCAMERA_H

#include "../matrix/matrix.h"

class AbsCamera{
private:
    float projectionM[16];
public:
    virtual void translate(float, float, float)=0;
    virtual void rotate(float, float, float)=0;
    virtual const float * getViewMat()=0;
    void setPerspectiveM(float, float, float, float);
    const float * getProjectionMat();
    virtual ~AbsCamera();
};

#endif //MMDVIEWER_ABSCAMERA_H

//
// Created by wjy50 on 2018/2/8.
//

#include "abscamera.h"

void AbsCamera::setPerspectiveM(float fovy, float aspect, float zNear, float zFar)
{
    perspectiveM(projectionM, fovy, aspect, zNear, zFar);
}

const float *AbsCamera::getProjectionMat() const
{
    return projectionM;
}

AbsCamera::~AbsCamera()
{}
//
// Created by wjy50 on 2018/2/7.
//

#include "scenecamera.h"
#include "../matrix/matrix.h"

SceneCamera::SceneCamera(float centerX, float centerY, float centerZ, float distance) {
    set(centerX,centerY,centerZ,distance);
}
void SceneCamera::set(float centerX, float centerY, float centerZ, float distance) {
    setIdentityM(transMat);
    translateMPre(transMat, -centerX, -centerY, -centerZ);
    this->distance=distance;
    rx=rz=0;
    newTrans = true;
}
void SceneCamera::translate(float x, float y, float z) {
    translateMPre(transMat, x, y, z);
    newTrans= true;
}
void SceneCamera::rotate(float x, float y, float z) {
    if(rx+x >= 89.99f)
    {
        x=89.99f-rx;
    }
    else if(rx+x <= -89.99f)
    {
        x=-89.99f-rx;
    }
    rotateMPre(transMat, -rz, 0, 0, 1);
    rotateMPre(transMat, -rx, 1, 0, 0);
    rx+=x;
    rz+=z;
    rotateMPre(transMat, y, 0, 1, 0);
    rotateMPre(transMat, rx, 1, 0, 0);
    rotateMPre(transMat, rz, 0, 0, 1);
    newTrans= true;
}
const float* SceneCamera::getViewMat() {
    if(newTrans)
    {
        translateMPre2(viewMat, transMat, 0, 0, -distance);
        newTrans= false;
    }
    return viewMat;
}
SceneCamera::~SceneCamera() {
    AbsCamera::~AbsCamera();
}
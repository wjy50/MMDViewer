//
// Created by wjy50 on 2018/2/7.
//

#include <cmath>
#include <ctime>

#include "scenecamera.h"
#include "../matrix/matrix.h"

using namespace std;

SceneCamera::SceneCamera(float centerX, float centerY, float centerZ, float distance)
: distance(0), newTrans(true), rx(0), rz(0), ax(400), ay(400), az(400),
  speedX(0), speedY(0), speedZ(0), curSpeedX(0), curSpeedY(0), curSpeedZ(0),
  lastTime(-1)
{
    set(centerX, centerY, centerZ, distance);
}

void SceneCamera::set(float centerX, float centerY, float centerZ, float distance)
{
    setIdentityM(transMat);
    translateMPre(transMat, -centerX, -centerY, -centerZ);
    this->distance = distance;
    rx = rz = 0;
    newTrans = true;
}

void SceneCamera::translate(float x, float y, float z)
{
    translateMPre(transMat, x, y, z);
    newTrans = true;
}

void SceneCamera::rotate(float x, float y, float z)
{
    if (rx + x >= 89.99f) {
        x = 89.99f - rx;
    } else if (rx + x <= -89.99f) {
        x = -89.99f - rx;
    }
    rotateMPre(transMat, -rz, 0, 0, 1);
    rotateMPre(transMat, -rx, 1, 0, 0);
    rx += x;
    rz += z;
    rotateMPre(transMat, y, 0, 1, 0);
    rotateMPre(transMat, rx, 1, 0, 0);
    rotateMPre(transMat, rz, 0, 0, 1);
    newTrans = true;
}

float computeTranslation(float &curSpeed, float speed, float a, int millis)
{
    if ((speed < curSpeed && a > 0) || (speed > curSpeed && a < 0))
        a = -a;
    int ta = static_cast<int>((speed - curSpeed) * 1000 / a);
    if (ta > millis) {
        float trans = (curSpeed * millis + a * millis * millis / 2000) / 1000;
        curSpeed += a * millis / 1000;
        return trans;
    } else {
        float trans = (curSpeed * ta + a * ta * ta / 2000 + speed * (millis - ta)) / 1000;
        curSpeed = speed;
        return trans;
    }
}

const float *SceneCamera::getViewMat()
{
    if (lastTime != -1) {
        long t = clock();
        int interval = static_cast<int>((t - lastTime) * 1000 / CLOCKS_PER_SEC);
        lastTime = t;
        float xTrans = computeTranslation(curSpeedX, speedX, ax, interval);
        float yTrans = computeTranslation(curSpeedY, speedY, ay, interval);
        float zTrans = computeTranslation(curSpeedZ, speedZ, az, interval);
        if (xTrans != 0 || yTrans != 0 || zTrans != 0)
            translate(xTrans, yTrans, zTrans);
    } else
        lastTime = clock();
    if (newTrans) {
        translateMPre2(viewMat, transMat, 0, 0, -distance);
        newTrans = false;
    }
    return viewMat;
}

void SceneCamera::setMoveSpeed(float x, float y, float z)
{
    speedX = x;
    speedY = y;
    speedZ = z;
}

void SceneCamera::addMoveSpeed(float x, float y, float z)
{
    speedX += x;
    speedY += y;
    speedZ += z;
}

SceneCamera::~SceneCamera()
{
    AbsCamera::~AbsCamera();
}
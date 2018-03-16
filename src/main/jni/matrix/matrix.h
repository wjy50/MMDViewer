//
// Created by wjy50 on 2018/2/5.
//

#ifndef MMDVIEWER_MATRIX_H
#define MMDVIEWER_MATRIX_H

#define PI 3.14159265358979323846

#ifdef __cplusplus
extern "C" {
#endif

static float sTemp[32];

void floatArrayCopy(float * src, float * dest, int count);
void multiplyMM(float*, float*, float*);
void multiplyMV(float*, float*, float*);
void perspectiveM(float* m, float fovy, float aspect, float zNear, float zFar);
void orthoM(float* m,float left, float right, float bottom, float top, float near, float far);
void setIdentityM(float* m);
void scaleM2(float*, float*, float, float, float);
void scaleM(float*, float, float, float);
void translateM2(float*, float*, float, float, float);
void translateM(float*, float, float, float);
void translateMPre2(float *, float *, float, float, float);
void translateMPre(float *, float, float, float);
void setRotateM(float*, float, float, float, float);
void rotateM2(float*, float*, float, float, float, float);
void rotateM(float*, float, float, float, float);
void rotateMPre2(float*, float*, float, float, float, float);
void rotateMPre(float*, float, float, float, float);
void setLookAtM(float* rm, float, float, float, float, float, float, float, float, float);

#ifdef __cplusplus
}
#endif

#endif //MMDVIEWER_MATRIX_H

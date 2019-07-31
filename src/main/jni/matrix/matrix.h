//
// Created by wjy50 on 2018/2/5.
//

#ifndef MMDVIEWER_MATRIX_H
#define MMDVIEWER_MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

static float sTemp[32];

#define floatArrayCopy(src, dest, count) memcpy(dest,src,sizeof(float)*count)

//void floatArrayCopy(const float * src, float * dest, int count);
void multiplyMM(float *, const float *, const float *);

void multiplyMV(float *, const float *, const float *);

void perspectiveM(float *m, float fovy, float aspect, float zNear, float zFar);

void orthoM(float *m, float left, float right, float bottom, float top, float near, float far);

void setIdentityM(float *m);

void scaleM2(float *, const float *, float, float, float);

void scaleM(float *, float, float, float);

void translateM2(float *, const float *, float, float, float);

void translateM(float *, float, float, float);

void translateMPre2(float *, const float *, float, float, float);

void translateMPre(float *, float, float, float);

void setRotateM(float *, float, float, float, float);

void rotateM2(float *, const float *, float, float, float, float);

void rotateM(float *, float, float, float, float);

void rotateMPre2(float *, const float *, float, float, float, float);

void rotateMPre(float *, float, float, float, float);

void setLookAtM(float *rm, float, float, float, float, float, float, float, float, float);

int invertM(float *rm, const float *m);

#ifdef __cplusplus
}
#endif

#endif //MMDVIEWER_MATRIX_H

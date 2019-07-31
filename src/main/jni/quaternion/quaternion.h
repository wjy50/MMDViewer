//
// Created by wjy50 on 18-3-16.
//

#ifndef MMDVIEWER_QUATERNION_H
#define MMDVIEWER_QUATERNION_H

#ifdef __cplusplus
extern "C" {
#endif

void matrixToQuaternion(float *q, const float *m);

void multiplyQuaternionWXYZ(float *rq, float *lhs, float *rhs);

void quaternionToEuler(float *euler, float *q);

void eulerToQuaternion(float *q, float *euler);

#ifdef __cplusplus
};
#endif

#endif //MMDVIEWER_QUATERNION_H

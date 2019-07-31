//
// Created by wjy50 on 2018/2/5.
//
#include "matrix.h"
#include "../vector/vector.h"
#include <math.h>
#include <string.h>

#define MULTIPLY_MATRIX_ONE

void multiplyMM(float *rm, const float *lhs, const float *rhs)
{
#ifdef MULTIPLY_MATRIX_ONE
    for (int i = 0; i < 16; i += 4) {
        rm[i] = rm[i + 1] = rm[i + 2] = rm[i + 3] = 0;
        for (int j = 0; j < 4; j++) {
            if (rhs[i + j] != 0)
                for (int k = 0; k < 4; k++) {
                    rm[i + k] += lhs[(j << 2) + k] * rhs[i + j];
                }
        }
    }
#else

    for (int i = 0; i < 4; ++i) {
        rm[i]=rm[4+i]=rm[8+i]=rm[12+i]=0;
        for (int k= 0; k < 4; ++k) {
            if(lhs[(k<<2)+i] != 0)for (int j = 0; j < 4; ++j) {
                rm[(j<<2)+i]+=lhs[(k<<2)+i]*rhs[(j<<2)+k];
            }
        }
    }
#endif
}

void multiplyMV(float *rv, const float *lhsMat, const float *rhsVec)
{
    rv[0] = rv[1] = rv[2] = rv[3] = 0;
    for (int i = 0; i < 4; ++i) {
        if (rhsVec[i] != 0)
            for (int j = 0; j < 4; ++j) {
                rv[j] += lhsMat[(i << 2) + j] * rhsVec[i];
            }
    }
}

void perspectiveM(float *m, float fovy, float aspect, float zNear, float zFar)
{
    float f = 1.0f / (float) tan(fovy * (M_PI / 360.0));
    float rangeReciprocal = 1.0f / (zNear - zFar);

    m[0] = f / aspect;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;

    m[4] = 0.0f;
    m[5] = f;
    m[6] = 0.0f;
    m[7] = 0.0f;

    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = (zFar + zNear) * rangeReciprocal;
    m[11] = -1.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 2.0f * zFar * zNear * rangeReciprocal;
    m[15] = 0.0f;
}

void orthoM(float *m, float left, float right, float bottom, float top, float near, float far)
{
    float r_width = 1.0f / (right - left);
    float r_height = 1.0f / (top - bottom);
    float r_depth = 1.0f / (far - near);
    float x = 2.0f * (r_width);
    float y = 2.0f * (r_height);
    float z = -2.0f * (r_depth);
    float tx = -(right + left) * r_width;
    float ty = -(top + bottom) * r_height;
    float tz = -(far + near) * r_depth;
    m[0] = x;
    m[5] = y;
    m[10] = z;
    m[12] = tx;
    m[13] = ty;
    m[14] = tz;
    m[15] = 1.0f;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;
    m[4] = 0.0f;
    m[6] = 0.0f;
    m[7] = 0.0f;
    m[8] = 0.0f;
    m[9] = 0.0f;
    m[11] = 0.0f;
}

void setIdentityM(float *m)
{
    for (int i = 0; i < 16; i++) {
        m[i] = 0;
    }
    for (int i = 0; i < 16; i += 5) {
        m[i] = 1.0f;
    }
}

void scaleM2(float *sm, const float *m, float x, float y, float z)
{
    for (int i = 0; i < 4; i++) {
        sm[i] = m[i] * x;
        sm[4 + i] = m[4 + i] * y;
        sm[8 + i] = m[8 + i] * z;
        sm[12 + i] = m[12 + i];
    }
}

void scaleM(float *m, float x, float y, float z)
{
    for (int i = 0; i < 4; i++) {
        m[i] *= x;
        m[4 + i] *= y;
        m[8 + i] *= z;
    }
}

void translateM2(float *tm, const float *m, float x, float y, float z)
{
    for (int i = 0; i < 12; i++) {
        tm[i] = m[i];
    }
    for (int i = 0; i < 4; i++) {
        tm[12 + i] = m[i] * x + m[4 + i] * y + m[8 + i] * z +
                     m[12 + i];
    }
}

void translateM(float *m, float x, float y, float z)
{
    for (int i = 0; i < 4; i++) {
        m[12 + i] += m[i] * x + m[4 + i] * y + m[8 + i] * z;
    }
}

void translateMPre2(float *tm, const float *m, float x, float y, float z)
{
    floatArrayCopy(m, tm, 16);
    translateMPre(tm, x, y, z);
}

void translateMPre(float *m, float x, float y, float z)
{
    m[12] += x;
    m[13] += y;
    m[14] += z;
}

void setRotateM(float *rm, float a, float x, float y, float z)
{
    rm[3] = 0;
    rm[7] = 0;
    rm[11] = 0;
    rm[12] = 0;
    rm[13] = 0;
    rm[14] = 0;
    rm[15] = 1;
    a *= (float) (M_PI / 180.0f);
    float s = (float) sin(a);
    float c = (float) cos(a);
    if (1.0f == x && 0.0f == y && 0.0f == z) {
        rm[5] = c;
        rm[10] = c;
        rm[6] = s;
        rm[9] = -s;
        rm[1] = 0;
        rm[2] = 0;
        rm[4] = 0;
        rm[8] = 0;
        rm[0] = 1;
    } else if (0.0f == x && 1.0f == y && 0.0f == z) {
        rm[0] = c;
        rm[10] = c;
        rm[8] = s;
        rm[2] = -s;
        rm[1] = 0;
        rm[4] = 0;
        rm[6] = 0;
        rm[9] = 0;
        rm[5] = 1;
    } else if (0.0f == x && 0.0f == y && 1.0f == z) {
        rm[0] = c;
        rm[5] = c;
        rm[1] = s;
        rm[4] = -s;
        rm[2] = 0;
        rm[6] = 0;
        rm[8] = 0;
        rm[9] = 0;
        rm[10] = 1;
    } else {
        float len = vectorLength3(x, y, z);
        if (1.0f != len) {
            float recipLen = 1.0f / len;
            x *= recipLen;
            y *= recipLen;
            z *= recipLen;
        }
        float nc = 1.0f - c;
        float xy = x * y;
        float yz = y * z;
        float zx = z * x;
        float xs = x * s;
        float ys = y * s;
        float zs = z * s;
        rm[0] = x * x * nc + c;
        rm[4] = xy * nc - zs;
        rm[8] = zx * nc + ys;
        rm[1] = xy * nc + zs;
        rm[5] = y * y * nc + c;
        rm[9] = yz * nc - xs;
        rm[2] = zx * nc - ys;
        rm[6] = yz * nc + xs;
        rm[10] = z * z * nc + c;
    }
}

void rotateMPre2(float *rm, const float *m, float a, float x, float y, float z)
{
    setRotateM(sTemp, a, x, y, z);
    multiplyMM(rm, sTemp, m);
}

void rotateMPre(float *m, float a, float x, float y, float z)
{
    setRotateM(sTemp, a, x, y, z);
    multiplyMM(sTemp + 16, sTemp, m);
    floatArrayCopy(sTemp + 16, m, 16);
}

void rotateM2(float *rm, const float *m, float a, float x, float y, float z)
{
    setRotateM(sTemp, a, x, y, z);
    multiplyMM(rm, m, sTemp);
}

void rotateM(float *m, float a, float x, float y, float z)
{
    setRotateM(sTemp, a, x, y, z);
    multiplyMM(sTemp + 16, m, sTemp);
    floatArrayCopy(sTemp + 16, m, 16);
}

void setLookAtM(float *rm, float eyeX, float eyeY, float eyeZ,
                float centerX, float centerY, float centerZ,
                float upX, float upY, float upZ)
{
    // See the OpenGL GLUT documentation for gluLookAt for a description
    // of the algorithm. We implement it in a straightforward way:

    float fx = centerX - eyeX;
    float fy = centerY - eyeY;
    float fz = centerZ - eyeZ;

    // Normalize f
    float rlf = 1.0f / vectorLength3(fx, fy, fz);
    fx *= rlf;
    fy *= rlf;
    fz *= rlf;

    // compute s = f x up (x means "cross product")
    float sx = fy * upZ - fz * upY;
    float sy = fz * upX - fx * upZ;
    float sz = fx * upY - fy * upX;

    // and normalize s
    float rls = 1.0f / vectorLength3(sx, sy, sz);
    sx *= rls;
    sy *= rls;
    sz *= rls;

    // compute u = s x f
    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    rm[0] = sx;
    rm[1] = ux;
    rm[2] = -fx;
    rm[3] = 0.0f;

    rm[4] = sy;
    rm[5] = uy;
    rm[6] = -fy;
    rm[7] = 0.0f;

    rm[8] = sz;
    rm[9] = uz;
    rm[10] = -fz;
    rm[11] = 0.0f;

    rm[12] = 0.0f;
    rm[13] = 0.0f;
    rm[14] = 0.0f;
    rm[15] = 1.0f;

    translateM(rm, -eyeX, -eyeY, -eyeZ);
}

int invertM(float *rm, const float *m)
{
    // Invert a 4 x 4 matrix using Cramer's Rule

    // transpose matrix
    const float src0 = m[0];
    const float src4 = m[1];
    const float src8 = m[2];
    const float src12 = m[3];

    const float src1 = m[4];
    const float src5 = m[5];
    const float src9 = m[6];
    const float src13 = m[7];

    const float src2 = m[8];
    const float src6 = m[9];
    const float src10 = m[10];
    const float src14 = m[11];

    const float src3 = m[12];
    const float src7 = m[13];
    const float src11 = m[14];
    const float src15 = m[15];

    // calculate pairs for first 8 elements (cofactors)
    const float atmp0 = src10 * src15;
    const float atmp1 = src11 * src14;
    const float atmp2 = src9 * src15;
    const float atmp3 = src11 * src13;
    const float atmp4 = src9 * src14;
    const float atmp5 = src10 * src13;
    const float atmp6 = src8 * src15;
    const float atmp7 = src11 * src12;
    const float atmp8 = src8 * src14;
    const float atmp9 = src10 * src12;
    const float atmp10 = src8 * src13;
    const float atmp11 = src9 * src12;

    // calculate first 8 elements (cofactors)
    const float dst0 = (atmp0 * src5 + atmp3 * src6 + atmp4 * src7)
                       - (atmp1 * src5 + atmp2 * src6 + atmp5 * src7);
    const float dst1 = (atmp1 * src4 + atmp6 * src6 + atmp9 * src7)
                       - (atmp0 * src4 + atmp7 * src6 + atmp8 * src7);
    const float dst2 = (atmp2 * src4 + atmp7 * src5 + atmp10 * src7)
                       - (atmp3 * src4 + atmp6 * src5 + atmp11 * src7);
    const float dst3 = (atmp5 * src4 + atmp8 * src5 + atmp11 * src6)
                       - (atmp4 * src4 + atmp9 * src5 + atmp10 * src6);
    const float dst4 = (atmp1 * src1 + atmp2 * src2 + atmp5 * src3)
                       - (atmp0 * src1 + atmp3 * src2 + atmp4 * src3);
    const float dst5 = (atmp0 * src0 + atmp7 * src2 + atmp8 * src3)
                       - (atmp1 * src0 + atmp6 * src2 + atmp9 * src3);
    const float dst6 = (atmp3 * src0 + atmp6 * src1 + atmp11 * src3)
                       - (atmp2 * src0 + atmp7 * src1 + atmp10 * src3);
    const float dst7 = (atmp4 * src0 + atmp9 * src1 + atmp10 * src2)
                       - (atmp5 * src0 + atmp8 * src1 + atmp11 * src2);

    // calculate pairs for second 8 elements (cofactors)
    const float btmp0 = src2 * src7;
    const float btmp1 = src3 * src6;
    const float btmp2 = src1 * src7;
    const float btmp3 = src3 * src5;
    const float btmp4 = src1 * src6;
    const float btmp5 = src2 * src5;
    const float btmp6 = src0 * src7;
    const float btmp7 = src3 * src4;
    const float btmp8 = src0 * src6;
    const float btmp9 = src2 * src4;
    const float btmp10 = src0 * src5;
    const float btmp11 = src1 * src4;

    // calculate second 8 elements (cofactors)
    const float dst8 = (btmp0 * src13 + btmp3 * src14 + btmp4 * src15)
                       - (btmp1 * src13 + btmp2 * src14 + btmp5 * src15);
    const float dst9 = (btmp1 * src12 + btmp6 * src14 + btmp9 * src15)
                       - (btmp0 * src12 + btmp7 * src14 + btmp8 * src15);
    const float dst10 = (btmp2 * src12 + btmp7 * src13 + btmp10 * src15)
                        - (btmp3 * src12 + btmp6 * src13 + btmp11 * src15);
    const float dst11 = (btmp5 * src12 + btmp8 * src13 + btmp11 * src14)
                        - (btmp4 * src12 + btmp9 * src13 + btmp10 * src14);
    const float dst12 = (btmp2 * src10 + btmp5 * src11 + btmp1 * src9)
                        - (btmp4 * src11 + btmp0 * src9 + btmp3 * src10);
    const float dst13 = (btmp8 * src11 + btmp0 * src8 + btmp7 * src10)
                        - (btmp6 * src10 + btmp9 * src11 + btmp1 * src8);
    const float dst14 = (btmp6 * src9 + btmp11 * src11 + btmp3 * src8)
                        - (btmp10 * src11 + btmp2 * src8 + btmp7 * src9);
    const float dst15 = (btmp10 * src10 + btmp4 * src8 + btmp9 * src9)
                        - (btmp8 * src9 + btmp11 * src10 + btmp5 * src8);

    // calculate determinant
    const float det =
            src0 * dst0 + src1 * dst1 + src2 * dst2 + src3 * dst3;

    if (det == 0.0f) {
        return 0;
    }

    // calculate matrix inverse
    const float invdet = 1.0f / det;
    rm[0] = dst0 * invdet;
    rm[1] = dst1 * invdet;
    rm[2] = dst2 * invdet;
    rm[3] = dst3 * invdet;

    rm[4] = dst4 * invdet;
    rm[5] = dst5 * invdet;
    rm[6] = dst6 * invdet;
    rm[7] = dst7 * invdet;

    rm[8] = dst8 * invdet;
    rm[9] = dst9 * invdet;
    rm[10] = dst10 * invdet;
    rm[11] = dst11 * invdet;

    rm[12] = dst12 * invdet;
    rm[13] = dst13 * invdet;
    rm[14] = dst14 * invdet;
    rm[15] = dst15 * invdet;

    return 1;
}
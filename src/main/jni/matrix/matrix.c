//
// Created by wjy50 on 2018/2/5.
//
#include "matrix.h"
#include "../vector/vector.h"
#include <math.h>
#include <string.h>

void floatArrayCopy(float * src, float * dest,int count)
{
    memcpy(dest,src, sizeof(float)*count);
}
#define MULTIPLY_MATRIX_ONE
void multiplyMM(float* rm, float* lhs, float* rhs)
{
#ifdef MULTIPLY_MATRIX_ONE
    for (int i = 0; i < 16; i+=4) {
        rm[i]=rm[i+1]=rm[i+2]=rm[i+3]=0;
        for (int j = 0; j < 4; j++) {
            if(rhs[i+j] != 0)for (int k = 0; k < 4; k++) {
                rm[i+k]+=lhs[(j<<2)+k]*rhs[i+j];
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
void multiplyMV(float* rv, float* lhsMat, float* rhsVec)
{
    rv[0]=rv[1]=rv[2]=rv[3]=0;
    for (int i = 0; i < 4; ++i) {
        if(rhsVec[i] != 0)for (int j = 0; j < 4; ++j) {
            rv[j]+=lhsMat[(i<<2)+j]*rhsVec[i];
        }
    }
}
void perspectiveM(float* m, float fovy, float aspect, float zNear, float zFar)
{
    float f = 1.0f / (float) tan(fovy * (PI / 360.0));
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
void orthoM(float* m,float left, float right, float bottom, float top, float near, float far)
{
    float r_width  = 1.0f / (right - left);
    float r_height = 1.0f / (top - bottom);
    float r_depth  = 1.0f / (far - near);
    float x =  2.0f * (r_width);
    float y =  2.0f * (r_height);
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
void setIdentityM(float* m)
{
    for (int i=0 ; i<16 ; i++) {
        m[i] = 0;
    }
    for(int i = 0; i < 16; i += 5) {
        m[i] = 1.0f;
    }
}
void scaleM2(float* sm, float* m, float x, float y, float z)
{
    for (int i=0 ; i<4 ; i++) {
        sm[     i] = m[     i] * x;
        sm[ 4 + i] = m[ 4 + i] * y;
        sm[ 8 + i] = m[ 8 + i] * z;
        sm[12 + i] = m[12 + i];
    }
}
void scaleM(float* m, float x, float y, float z)
{
    for (int i=0 ; i<4 ; i++) {
        m[     i] *= x;
        m[ 4 + i] *= y;
        m[ 8 + i] *= z;
    }
}
void translateM2(float* tm, float* m, float x, float y, float z)
{
    for (int i=0 ; i<12 ; i++) {
        tm[i] = m[i];
    }
    for (int i=0 ; i<4 ; i++) {
        tm[12 + i] = m[i] * x + m[4 + i] * y + m[8 + i] * z +
                       m[12 + i];
    }
}
void translateM(float* m, float x, float y, float z)
{
    for (int i=0 ; i<4 ; i++) {
        m[12 + i] += m[i] * x + m[4 + i] * y + m[8 + i] * z;
    }
}
void translateMPre2(float *tm, float *m, float x, float y, float z)
{
    floatArrayCopy(m,tm,16);
    translateMPre(tm, x, y, z);
}
void translateMPre(float *m, float x, float y, float z)
{
    m[12]+=x;
    m[13]+=y;
    m[14]+=z;
}
void setRotateM(float* rm, float a, float x, float y, float z)
{
    rm[3] = 0;
    rm[7] = 0;
    rm[11]= 0;
    rm[12]= 0;
    rm[13]= 0;
    rm[14]= 0;
    rm[15]= 1;
    a *= (float) (PI / 180.0f);
    float s = (float) sin(a);
    float c = (float) cos(a);
    if (1.0f == x && 0.0f == y && 0.0f == z) {
        rm[5] = c;   rm[10]= c;
        rm[6] = s;   rm[9] = -s;
        rm[1] = 0;   rm[2] = 0;
        rm[4] = 0;   rm[8] = 0;
        rm[0] = 1;
    } else if (0.0f == x && 1.0f == y && 0.0f == z) {
        rm[0] = c;   rm[10]= c;
        rm[8] = s;   rm[2] = -s;
        rm[1] = 0;   rm[4] = 0;
        rm[6] = 0;   rm[9] = 0;
        rm[5] = 1;
    } else if (0.0f == x && 0.0f == y && 1.0f == z) {
        rm[0] = c;   rm[5] = c;
        rm[1] = s;   rm[4] = -s;
        rm[2] = 0;   rm[6] = 0;
        rm[8] = 0;   rm[9] = 0;
        rm[10]= 1;
    } else {
        float len = vectorLength(x, y, z);
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
        rm[ 0] = x*x*nc +  c;
        rm[ 4] =  xy*nc - zs;
        rm[ 8] =  zx*nc + ys;
        rm[ 1] =  xy*nc + zs;
        rm[ 5] = y*y*nc +  c;
        rm[ 9] =  yz*nc - xs;
        rm[ 2] =  zx*nc - ys;
        rm[ 6] =  yz*nc + xs;
        rm[10] = z*z*nc +  c;
    }
}
void rotateMPre2(float* rm, float* m, float a, float x, float y, float z)
{
    setRotateM(sTemp,a,x,y,z);
    multiplyMM(rm,sTemp,m);
}
void rotateMPre(float* m, float a, float x, float y, float z)
{
    setRotateM(sTemp,a,x,y,z);
    multiplyMM(sTemp+16,sTemp,m);
    floatArrayCopy(sTemp+16,m,16);
}
void rotateM2(float* rm, float* m, float a, float x, float y, float z)
{
    setRotateM(sTemp,a,x,y,z);
    multiplyMM(rm,m,sTemp);
}
void rotateM(float* m, float a, float x, float y, float z)
{
    setRotateM(sTemp,a,x,y,z);
    multiplyMM(sTemp+16,m,sTemp);
    floatArrayCopy(sTemp+16,m,16);
}

void setLookAtM(float* rm,float eyeX, float eyeY, float eyeZ,
                float centerX, float centerY, float centerZ,
                float upX, float upY, float upZ)
{
    // See the OpenGL GLUT documentation for gluLookAt for a description
    // of the algorithm. We implement it in a straightforward way:

    float fx = centerX - eyeX;
    float fy = centerY - eyeY;
    float fz = centerZ - eyeZ;

    // Normalize f
    float rlf = 1.0f / vectorLength(fx, fy, fz);
    fx *= rlf;
    fy *= rlf;
    fz *= rlf;

    // compute s = f x up (x means "cross product")
    float sx = fy * upZ - fz * upY;
    float sy = fz * upX - fx * upZ;
    float sz = fx * upY - fy * upX;

    // and normalize s
    float rls = 1.0f / vectorLength(sx, sy, sz);
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
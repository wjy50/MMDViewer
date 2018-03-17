//
// Created by wjy50 on 2018/2/13.
//
#include <math.h>

float vectorLength3(float x, float y, float z)
{
    return (float) sqrt(x*x+y*y+z*z);
}

float vectorLength4(float x, float y, float z, float w)
{
    return (float) sqrt(x*x+y*y+z*z+w*w);
}

void normalize3Into(float* v)
{
    float l=(float) sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    if(l > 1e-6)
    {
        l=1.0f/l;
        v[0]*=l;
        v[1]*=l;
        v[2]*=l;
    }
}

void normalize3(float* rv, float* v)
{
    float l=(float) sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    if(l > 1e-6)
    {
        l=1.0f/l;
        rv[0]=v[0]*l;
        rv[1]=v[1]*l;
        rv[2]=v[2]*l;
    }
}

void normalize4Into(float* v)
{
    float l=(float) sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]+v[3]*v[3]);
    if(l > 1e-6)
    {
        l=1.0f/l;
        v[0]*=l;
        v[1]*=l;
        v[2]*=l;
        v[3]*=l;
    }
}

void normalize4(float* rv, float* v)
{
    float l=(float) sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]+v[3]*v[3]);
    if(l > 1e-6)
    {
        l=1.0f/l;
        rv[0]=v[0]*l;
        rv[1]=v[1]*l;
        rv[2]=v[2]*l;
        rv[3]=v[3]*l;
    }
}

void crossProduct(float* rv, float* lhs, float* rhs)
{
    float i=lhs[1]*rhs[2]-lhs[2]*rhs[1];
    float j=lhs[2]*rhs[0]-lhs[0]*rhs[2];
    float k=lhs[0]*rhs[1]-lhs[1]*rhs[0];
    rv[0]=i;
    rv[1]=j;
    rv[2]=k;
}

float dotProduct3(float* v1, float* v2)
{
    return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2];
}

float dotProduct4(float* v1, float* v2)
{
    return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2]+v1[3]*v2[3];
}

void subtractVector4(float* rv, float* r1, float* r2)
{
    rv[0]=r1[0]-r2[0];
    rv[1]=r1[1]-r2[1];
    rv[2]=r1[2]-r2[2];
    rv[3]=r1[3]-r2[3];
}

void subtractVector3(float* rv, float* v1, float* v2)
{
    rv[0]=v1[0]-v2[0];
    rv[1]=v1[1]-v2[1];
    rv[2]=v1[2]-v2[2];
}

float distance3(float* v1, float* v2)
{
    return vectorLength3(v1[0]-v2[0],v1[1]-v2[1],v1[2]-v2[2]);
}

float distance4(float* v1, float* v2)
{
    return vectorLength4(v1[0]-v2[0],v1[1]-v2[1],v1[2]-v2[2],v1[3]-v2[3]);
}
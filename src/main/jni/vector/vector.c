//
// Created by wjy50 on 2018/2/13.
//
#include <math.h>

float vectorLength(float x, float y, float z)
{
    return (float) sqrt(x*x+y*y+z*z);
}

void normalizeInto(float* v)
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

void normalize(float* rv, float* v)
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

void crossProduct(float* rv, float* lhs, float* rhs)
{
    float i=lhs[1]*rhs[2]-lhs[2]*rhs[1];
    float j=lhs[2]*rhs[0]-lhs[0]*rhs[2];
    float k=lhs[0]*rhs[1]-lhs[1]*rhs[0];
    rv[0]=i;
    rv[1]=j;
    rv[2]=k;
}

float dotProduct(float* v1, float* v2)
{
    return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2];
}
//
// Created by wjy50 on 18-3-16.
//
#include <math.h>
#include "quaternion.h"
#include "../vector/vector.h"

void matrixToQuaternion(float* q, const float* m)
{
    float w= (float) (sqrt(fabs(m[0]+m[5]+m[10]+m[15]))/2);
    float s=4*w;
    float x=(m[6]-m[9])/s;
    float y=(m[8]-m[2])/s;
    float z=(m[1]-m[4])/s;
    q[0]=w;
    q[1]=x;
    q[2]=y;
    q[3]=z;
    normalize4Into(q);
}

void multiplyQuaternionWXYZ(float* rq, float* lhs, float* rhs)
{
    float  w1=lhs[0],w2=rhs[0];
    float  x1=lhs[1],x2=rhs[1];
    float  y1=lhs[2],y2=rhs[2];
    float  z1=lhs[3],z2=rhs[3];
    rq[0]=w1*w2-x1*x2-y1*y2-z1*z2;
    rq[1]=w1*x2+x1*w2+z1*y2-y1*z2;
    rq[2]=w1*y2+y1*w2+x1*z2-z1*x2;
    rq[3]=w1*z2+z1*w2+y1*x2-x1*y2;
}

void quaternionToEuler(float *euler, float *q)
{
    float w=q[0];
    float x=q[1];
    float y=q[2];
    float z=q[3];
    euler[0]= (float) atan2(2*(w*x+y*z),1-2*(x*x+y*y));
    euler[1]= (float) asin(2*(w*y-z*x));
    euler[2]= (float) atan2(2*(w*z+x*y),1-2*(y*y+z*z));
}

void eulerToQuaternion(float* q, float* euler)
{
    float c1= (float) cos(euler[0]/2);
    float c2= (float) cos(euler[1]/2);
    float c3= (float) cos(euler[2]/2);
    float s1= (float) sin(euler[0]/2);
    float s2= (float) sin(euler[1]/2);
    float s3= (float) sin(euler[2]/2);
    q[0]=c1*c2*c3+s1*s2*s3;//w
    q[1]=s1*c2*c3-c1*s2*s3;//x
    q[2]=c1*s2*c3+s1*c2*s3;//y
    q[3]=c1*c2*s3-s1*s2*c3;//z
}
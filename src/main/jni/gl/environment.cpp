//
// Created by wjy50 on 18-3-15.
//
#include "environment.h"
#include "../matrix/matrix.h"

EnvironmentLight::EnvironmentLight() {
    isSunMatValid= false;
    sunMat=0;
    shadowMapTextureUnit=-1;
}
void EnvironmentLight::setSunLightStrength(float r, float g, float b) {
    sunLightStrength[0]=r;
    sunLightStrength[1]=g;
    sunLightStrength[2]=b;
}
const float* EnvironmentLight::getSunLightStrength() {
    return sunLightStrength;
}
void EnvironmentLight::setSunPosition(float x, float y, float z) {
    sunPosition[0]=x;
    sunPosition[1]=y;
    sunPosition[2]=z;
    isSunMatValid= false;
}
const float* EnvironmentLight::getSunPosition() {
    return sunPosition;
}
float* EnvironmentLight::getSunMat() {
    if(isSunMatValid)return sunMat;
    if(!sunMat)sunMat=new float[32];
    float temp[32];
    setLookAtM(temp,sunPosition[0],sunPosition[1]+0.6f,sunPosition[2],0,0.6f,0,0,1,0);
    orthoM(temp+16,-1.4f,1.4f,-1.1f,1.4f,0.2f,6);
    multiplyMM(sunMat,temp+16,temp);

    for (int i=0 ; i<16 ; i++) {
        temp[i] = 0;
    }
    for(int i = 0; i < 12; i += 5) {
        temp[i] = 0.5f;
    }
    temp[12]=temp[13]=temp[14]=0.5f;
    temp[15]=1;
    multiplyMM(sunMat+16,temp,sunMat);

    isSunMatValid= true;
    return sunMat;
}
float* EnvironmentLight::getSunMatForDraw() {
    return sunMat+16;
}
int EnvironmentLight::getShadowMapTextureUnit() {
    return shadowMapTextureUnit;
}
EnvironmentLight::~EnvironmentLight() {
    if(sunMat)delete [] sunMat;
}
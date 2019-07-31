//
// Created by wjy50 on 2018/2/7.
//

#ifndef MMDVIEWER_ENVIRONMENT_H
#define MMDVIEWER_ENVIRONMENT_H

class EnvironmentLight
{
private:
    float sunPosition[3];
    float sunLightStrength[3];
    float *sunMat;
    bool isSunMatValid;

    int shadowMapTextureUnit;

    friend class Renderer;

public:
    EnvironmentLight();

    void setSunPosition(float, float, float);

    const float *getSunPosition() const;

    void setSunLightStrength(float, float, float);

    const float *getSunLightStrength() const;

    float *getSunMat();

    float *getSunMatForDraw() const;

    int getShadowMapTextureUnit() const;

    ~EnvironmentLight();
};

#endif //MMDVIEWER_ENVIRONMENT_H

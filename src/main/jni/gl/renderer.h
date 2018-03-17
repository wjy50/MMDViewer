//
// Created by wjy50 on 2018/2/5.
//

#ifndef MMDVIEWER_RENDERER_H
#define MMDVIEWER_RENDERER_H

#include "scenecamera.h"
#include "objects/cube.h"
#include "../mmd/pmx/pmxreader.h"

class Renderer{
private:
    int surfaceWidth,surfaceHeight;
    AbsCamera* camera;
    PMXReader* pmx;
    EnvironmentLight* environmentLight;

    int shadowMapSize;
    GLuint shadowMapFBOHandle;
    GLuint shadowMapTextureHandle;
public:
    Renderer();
    void onSurfaceCreate();
    void onSurfaceChanged(int width,int height);
    void onDrawFrame();
    int getSurfaceWidth();
    int getSurfaceHeight();
    void setCamera(AbsCamera* camera);
    AbsCamera* getCamera();
    void setSunPosition(float, float, float);
    void setSunLightStrength(float, float, float);
    void addPMXModel(const char* path);
    PMXReader* getModel();

    /*if shadowMapSize is set to 0, shadow map will be disabled*/
    void setShadowMapSize(int);

    ~Renderer();
};

#endif //MMDVIEWER_RENDERER_H

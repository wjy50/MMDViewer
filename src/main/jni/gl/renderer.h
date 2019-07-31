//
// Created by wjy50 on 2018/2/5.
//

#ifndef MMDVIEWER_RENDERER_H
#define MMDVIEWER_RENDERER_H

#include "scenecamera.h"
#include "objects/cube.h"
#include "../mmd/pmx/pmxreader.h"
#include "../mmd/vmd/vmdreader.h"

class Renderer
{
private:
    int surfaceWidth, surfaceHeight;
    AbsCamera *camera;
    PMXReader *pmx;
    VMDReader *vmd;
    EnvironmentLight environmentLight;

    int shadowMapSize;
    GLuint shadowMapFBOHandle;
    GLuint shadowMapTextureHandle;
public:
    Renderer();

    void onSurfaceCreate();

    void onSurfaceChanged(int width, int height);

    void onDrawFrame();

    int getSurfaceWidth() const;

    int getSurfaceHeight() const;

    void setCamera(AbsCamera *camera);

    AbsCamera *getCamera() const;

    void setSunPosition(float x, float y, float z);

    void setSunLightStrength(float r, float g, float b);

    void addPMXModel(const char *path);

    void addVMDMotion(const char *path);

    PMXReader *getModel() const;

    /*if shadowMapSize is set to 0, shadow map will be disabled*/
    void setShadowMapSize(int size);

    ~Renderer();
};

#endif //MMDVIEWER_RENDERER_H

//
// Created by wjy50 on 2018/2/5.
//
#include <android/log.h>
#include "renderer.h"
Renderer::Renderer() {
    camera=0;
    surfaceWidth=surfaceHeight=0;
    pmx=0;
    shadowMapSize=0;
    shadowMapFBOHandle=0;
    shadowMapTextureHandle=0;
    environmentLight=new EnvironmentLight;
}
void Renderer::setSunPosition(float x, float y, float z) {
    environmentLight->setSunPosition(x,y,z);
}
void Renderer::setSunLightStrength(float r, float g, float b) {
    environmentLight->setSunLightStrength(r,g,b);
}
void Renderer::setShadowMapSize(int size) {
    if(size < 0)size=0;
    if(size != 0)
    {
        if(shadowMapFBOHandle == 0)
        {
            glGenFramebuffers(1,&shadowMapFBOHandle);
            if(!shadowMapFBOHandle)
            {
                __android_log_print(ANDROID_LOG_DEBUG,"em.ou","Failed to generate FBO");
                return;
            }
            glGenTextures(1,&shadowMapTextureHandle);
            if(!shadowMapTextureHandle)
            {
                __android_log_print(ANDROID_LOG_DEBUG,"em.ou","Failed to generate shadow map texture");
                glDeleteFramebuffers(GL_FRAMEBUFFER,&shadowMapFBOHandle);
                shadowMapFBOHandle=0;
                return;
            }
            glBindFramebuffer(GL_FRAMEBUFFER,shadowMapFBOHandle);
            glBindTexture(GL_TEXTURE_2D,shadowMapTextureHandle);
            glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT,size,size,0,GL_DEPTH_COMPONENT,GL_UNSIGNED_INT,0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,shadowMapTextureHandle,0);
            glBindTexture(GL_TEXTURE_2D,0);
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                __android_log_print(ANDROID_LOG_DEBUG,"em.ou","Failed to initialize FBO");
                glBindFramebuffer(GL_FRAMEBUFFER,0);
                glDeleteFramebuffers(1,&shadowMapFBOHandle);
                glDeleteTextures(1,&shadowMapTextureHandle);
                shadowMapFBOHandle=0;
                shadowMapTextureHandle=0;
                return;
            }
            glBindFramebuffer(GL_FRAMEBUFFER,0);
            environmentLight->shadowMapTextureUnit=2;
        }
        else if(shadowMapSize != size)
        {
            glBindTexture(GL_TEXTURE_2D,shadowMapTextureHandle);
            glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT,size,size,0,GL_DEPTH_COMPONENT,GL_UNSIGNED_INT,0);
            glBindTexture(GL_TEXTURE_2D,0);
        }
    }
    shadowMapSize=size;
}
void Renderer::onSurfaceCreate() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    setShadowMapSize(1024);
    if(pmx)pmx->initShadowMapShader();
}
void Renderer::onSurfaceChanged(int width, int height) {
    surfaceWidth=width;
    surfaceHeight=height;
    if(width != 0 && height != 0)
    {
        float ratio=(float)width/height;
        glViewport(0,0,width,height);
        if(camera)camera->setPerspectiveM(30,ratio,.2f,6000);
    }
}
void Renderer::onDrawFrame() {
    if(shadowMapSize != 0 && shadowMapFBOHandle != 0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER,shadowMapFBOHandle);
        glViewport(0,0,shadowMapSize,shadowMapSize);
        glClear(GL_DEPTH_BUFFER_BIT);
        if(pmx)pmx->drawShadowMap(environmentLight);
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D,shadowMapTextureHandle);
        glViewport(0,0,surfaceWidth,surfaceHeight);
    }

    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    if(pmx)pmx->draw(camera->getViewMat(),camera->getProjectionMat(),environmentLight);
}

int Renderer::getSurfaceWidth() {
    return surfaceWidth;
}
int Renderer::getSurfaceHeight() {
    return surfaceHeight;
}
void Renderer::setCamera(AbsCamera *camera) {
    this->camera=camera;
}
AbsCamera* Renderer::getCamera()
{
    return camera;
}

void Renderer::addPMXModel(const char *path) {
    if(pmx)delete pmx;
    pmx=new PMXReader(path);
    pmx->initShadowMapShader();
}
Renderer::~Renderer() {
    if(pmx)delete pmx;
    delete environmentLight;
}
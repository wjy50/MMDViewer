#include "application.h"
#include <android/log.h>
//
// Created by wjy50 on 2018/2/5.
//
Application::Application() {
    renderer=new Renderer;
    defaultCamera=new SceneCamera(0,1,5,0);
    renderer->setCamera(defaultCamera);
    renderer->setSunPosition(1,1,1);
    renderer->setSunLightStrength(.7,.7,.7);
}
Application::~Application() {
    delete renderer;
    delete defaultCamera;
}
Renderer* Application::getRenderer() {
    return renderer;
}
void Application::onTouchEvent(int action, float x, float y) {
    switch(action)
    {
        case ACTION_DOWN:
            this->x=dx=x;
            this->y=dy=y;
            break;
        case ACTION_MOVE:
            if(dx < renderer->getSurfaceWidth()>>1)//translate
            {
                float tx=(this->x-x)*50/renderer->getSurfaceHeight();
                float tz=(this->y-y)*50/renderer->getSurfaceHeight();
                renderer->getCamera()->translate(tx,0,tz);
                this->x=x;
                this->y=y;
            }
            else
            {
                float rx=(this->y-y)*100/renderer->getSurfaceHeight();
                float ry=(this->x-x)*100/renderer->getSurfaceHeight();
                renderer->getCamera()->rotate(rx,ry,0);
                this->y=y;
                this->x=x;
            }
            break;
        case ACTION_UP:
            break;
        default:
            break;
    }
}
//
// Created by wjy50 on 2018/2/5.
//

#ifndef MMDVIEWER_APPLICATION_H
#define MMDVIEWER_APPLICATION_H

#include "renderer.h"

#define ACTION_DOWN 0
#define ACTION_UP 1
#define ACTION_MOVE 2

class Application{
private:
    Renderer* renderer;
    AbsCamera* defaultCamera;
    float dx,dy,x,y;
public:
    Application();
    Renderer* getRenderer();
    void onTouchEvent(int action, float x, float y);
    ~Application();
};

#endif //MMDVIEWER_APPLICATION_H

//
// Created by wjy50 on 2018/2/5.
//

#ifndef MMDVIEWER_APPLICATION_H
#define MMDVIEWER_APPLICATION_H

#include "renderer.h"

#define ACTION_DOWN 0
#define ACTION_UP 1
#define ACTION_MOVE 2
#define ACTION_POINTER_DOWN 5
#define ACTION_POINTER_UP 6

class PointerInfo
{
private:
    int index;
    float x;
    float y;
public:
    PointerInfo();

    float deltaX(float x);

    float deltaY(float y);

    int getIndex() const;

    void down(int index, float x, float y);

    void up(int index);
};

class Application
{
private:
    Renderer renderer;
    AbsCamera *defaultCamera;
    int currentMorph;

    PointerInfo moveInfo, rotateInfo;
public:
    Application();

    Renderer &getRenderer();

    void onTouchEvent(int action, int actionIndex, float x, float y);

    ~Application();
};

#endif //MMDVIEWER_APPLICATION_H

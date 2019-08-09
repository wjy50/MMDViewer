#include "application.h"
#include "../utils/debugutils.h"

//
// Created by wjy50 on 2018/2/5.
//
Application::Application() : renderer(), defaultCamera(new SceneCamera(0, 1, 5, 0)),
    currentMorph(0), moveInfo(), rotateInfo()
{
    renderer.setCamera(defaultCamera);
    renderer.setSunPosition(1, 1, 1);
    renderer.setSunLightStrength(.7, .7, .7);
}

Application::~Application()
{
    delete defaultCamera;
}

Renderer &Application::getRenderer()
{
    return renderer;
}

void Application::onTouchEvent(int action, int actionIndex, float x, float y)
{
    switch (action) {
        case ACTION_DOWN:
            if (y + (renderer.getSurfaceHeight() / 16) > renderer.getSurfaceHeight()) {
                switch ((int) x * 3 / renderer.getSurfaceWidth()) {
                    case 0:
                        if (--currentMorph < 0)
                            currentMorph = renderer.getModel().getMorphCount() - 1;
                        break;
                    case 2:
                        if (++currentMorph >= renderer.getModel().getMorphCount())
                            currentMorph = 0;
                        break;
                    default:
                        if (renderer.getModel().getMorphFraction(currentMorph) == 0)
                            renderer.getModel().setMorphFraction(currentMorph, 1);
                        else renderer.getModel().setMorphFraction(currentMorph, 0);
                }
                LOG_PRINTF("name=%s", renderer.getModel().getMorphName(currentMorph));
            } else {
                if (x < renderer.getSurfaceWidth() / 2)
                    moveInfo.down(actionIndex, x, y);
                else
                    rotateInfo.down(actionIndex, x, y);
            }
            break;
        case ACTION_MOVE:
            if (actionIndex == moveInfo.getIndex())  // translate
            {
                float tx = moveInfo.deltaX(x) * 300 / renderer.getSurfaceHeight();
                float tz = moveInfo.deltaY(y) * 300 / renderer.getSurfaceHeight();
                dynamic_cast<SceneCamera*>(renderer.getCamera())->addMoveSpeed(tx, 0, tz);
            } else if (actionIndex == rotateInfo.getIndex()) {
                float rx = rotateInfo.deltaY(y) * 100 / renderer.getSurfaceHeight();
                float ry = rotateInfo.deltaX(x) * 100 / renderer.getSurfaceHeight();
                renderer.getCamera()->rotate(rx, ry, 0);
            } else if (x < renderer.getSurfaceWidth() / 2) {
                if (moveInfo.getIndex() < 0)
                    moveInfo.down(actionIndex, x, y);
            } else {
                if (rotateInfo.getIndex() < 0)
                    rotateInfo.down(actionIndex, x, y);
            }
            break;
        case ACTION_POINTER_DOWN:
            if (x < renderer.getSurfaceWidth() / 2)
                moveInfo.down(actionIndex, x, y);
            else
                rotateInfo.down(actionIndex, x, y);
            break;
        case ACTION_POINTER_UP:
        case ACTION_UP:
            if (actionIndex == moveInfo.getIndex())
                dynamic_cast<SceneCamera*>(renderer.getCamera())->setMoveSpeed(0, 0, 0);
            rotateInfo.up(actionIndex);
            break;
        default:
            break;
    }
}

PointerInfo::PointerInfo()
: index(-1), x(0), y(0)
{}

void PointerInfo::down(int index, float x, float y)
{
    this->index = index;
    this->x = x;
    this->y = y;
}

float PointerInfo::deltaX(float x)
{
    float delta = this->x - x;
    this->x = x;
    return delta;
}

float PointerInfo::deltaY(float y)
{
    float delta = this->y - y;
    this->y = y;
    return delta;
}

void PointerInfo::up(int index)
{
    if (index == this->index)
        this->index = -1;
}

int PointerInfo::getIndex() const
{
    return index;
}
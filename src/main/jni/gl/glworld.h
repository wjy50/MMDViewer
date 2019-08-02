//
// Created by wjy50 on 19-8-1.
//

#ifndef MMDVIEWER_GLWORLD_H
#define MMDVIEWER_GLWORLD_H

#include <vector>
#include "objects/absglobject.h"

class GLWorld
{
private:
    std::vector<AbsGLObject*> objects;
public:
    GLWorld();

    GLWorld(const GLWorld &other);

    GLWorld(GLWorld &&other) noexcept;


};

#endif //MMDVIEWER_GLWORLD_H

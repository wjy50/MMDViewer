//
// Created by wjy50 on 2018/2/7.
//

#include <GLES3/gl3.h>
#include "../environment.h"

#ifndef MMDVIEWER_ABSGLOBJECT_H
#define MMDVIEWER_ABSGLOBJECT_H

class AbsGLObject{
public:
    virtual void draw(const float*, const float*, EnvironmentLight*)=0;
    virtual void updateModelState()=0;
};

#endif //MMDVIEWER_ABSGLOBJECT_H

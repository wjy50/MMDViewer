//
// Created by wjy50 on 19-8-1.
//

#include "glworld.h"

GLWorld::GLWorld()
: objects()
{}

GLWorld::GLWorld(GLWorld &&other) noexcept
: objects(move(other.objects))
{}

GLWorld::GLWorld(const GLWorld &other)
: objects(other.objects)
{}
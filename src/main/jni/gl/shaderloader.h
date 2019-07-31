//
// Created by wjy50 on 2018/2/6.
//

#ifndef MMDVIEWER_SHADERLOADER_H
#define MMDVIEWER_SHADERLOADER_H

#include "../utils/UniquePointerExt.h"

int loadShader(const char *fileName, int *ptrLength, std::unique_ptr<char[]> &result);

#endif //MMDVIEWER_SHADERLOADER_H

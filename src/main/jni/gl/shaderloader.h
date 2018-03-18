//
// Created by wjy50 on 2018/2/6.
//

#ifndef MMDVIEWER_SHADERLOADER_H
#define MMDVIEWER_SHADERLOADER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int loadShader(const char* fileName, size_t * ptrLength, char** ptrResult);

#ifdef __cplusplus
};
#endif

#endif //MMDVIEWER_SHADERLOADER_H

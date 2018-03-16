//
// Created by wjy50 on 2018/2/6.
//

#ifndef MMDVIEWER_SHADERLOADER_H
#define MMDVIEWER_SHADERLOADER_H

#ifdef __cplusplus
extern "C" {
#endif

int loadShader(const char* fileName, int* ptrLength, char** ptrResult);

#ifdef __cplusplus
};
#endif

#endif //MMDVIEWER_SHADERLOADER_H

#include "shaderloader.h"
#include <stdio.h>
//
// Created by wjy50 on 2018/2/6.
//
int loadShader(const char* fileName, int* ptrLength, char** ptrResult)
{
    FILE* file=fopen(fileName,"rb");
    if(file)
    {
        fread(ptrLength, sizeof(int),1,file);
        if(*ptrLength > 0)
        {
            *ptrResult=new char[(*ptrLength)+1];
            (*ptrResult)[*ptrLength]=0;
            fread(*ptrResult, sizeof(char),*ptrLength,file);
        }
        fclose(file);
        return 1;
    }
    return 0;
}
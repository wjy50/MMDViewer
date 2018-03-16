//
// Created by wjy50 on 2018/2/10.
//

#ifndef MMDVIEWER_TEXTURE_H
#define MMDVIEWER_TEXTURE_H

#include <stdio.h>
#include <stdlib.h>
#include "../turbojpeg/turbojpeg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum TEXTURE_COLOR_TYPE{
    TEX_RGB,
    TEX_ARGB
}TextureColorType;

typedef enum TEXTURE_LOAD_RESULT{
    LOAD_SUCCESS=0,
    INVALID_JPEG,
    UNKNOWN_ERROR,
    INVALID_PNG,
    NOT_SUPPORTED_PNG_COLOR_TYPE,
    INVALID_BMP,
    NOT_SUPPORTED_BMP_COLOR_TYPE,
    NOT_SUPPORTED_BMP_COMPRESSION_TYPE,
    OPEN_FILE_FAILED,
    MALLOC_FAILED,
    NOT_SUPPORTED_TGA_COLOR_TYPE,
    UNKNOWN_TGA_TYPE
}TextureLoadResult;

typedef struct TEXTURE_IMAGE{
    int width;
    int height;
    TextureColorType colorType;
    unsigned char * data;
}TextureImage;

TextureLoadResult loadJpeg(const char* filePath,TextureImage* dest);

TextureLoadResult loadPng(const char* filePath,TextureImage* dest);

TextureLoadResult loadBmp(const char* filePath,TextureImage* dest);

TextureLoadResult loadTga(const char* filePath,TextureImage* dest);

int loadTexture(const char* filePath,TextureImage* dest);

#ifdef __cplusplus
};
#endif

#endif //MMDVIEWER_TEXTURE_H

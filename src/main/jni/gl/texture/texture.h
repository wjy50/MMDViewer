//
// Created by wjy50 on 2018/2/10.
//

#ifndef MMDVIEWER_TEXTURE_H
#define MMDVIEWER_TEXTURE_H

#include "../../utils/UniquePointerExt.h"

typedef enum TEXTURE_COLOR_TYPE
{
    TEX_RGB,
    TEX_ARGB
} TextureColorType;

class TextureImage
{
private:
    int width;
    int height;
    TextureColorType colorType;
    unsigned char *data;

    void tryJpeg(const std::string &filePath);

    void tryPng(const std::string &filePath);

    void tryBmp(const std::string &filePath);

    void tryTga(const std::string &filePath);
public:
    TextureImage(const std::string &filePath);

    const unsigned char *get() const;

    int getWidth() const;

    int getHeight() const;

    TextureColorType getColorType() const;

    ~TextureImage();
};

typedef enum TEXTURE_LOAD_ERROR
{
    INVALID_JPEG = 0,
    UNKNOWN_ERROR,
    INVALID_PNG,
    UNSUPPORTED_PNG_COLOR_TYPE,
    INVALID_BMP,
    UNSUPPORTED_BMP_COLOR_TYPE,
    UNSUPPORTED_BMP_COMPRESSION_TYPE,
    OPEN_FILE_FAILED,
    UNSUPPORTED_TGA_COLOR_TYPE,
    UNKNOWN_TGA_TYPE,
    UNSUPPORTED_FILE_TYPE
} TextureLoadError;

class TextureLoadException : public std::exception
{
private:
    TextureLoadError error;
public:
    TextureLoadException(TextureLoadError error);

    TextureLoadError getError() const;

    const char * what() const noexcept override;
};

#endif //MMDVIEWER_TEXTURE_H

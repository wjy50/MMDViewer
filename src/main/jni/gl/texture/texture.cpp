//
// Created by wjy50 on 2018/2/10.
//

#include <fstream>

#include "texture.h"
#include "../../png/png.h"
#include "../../turbojpeg/turbojpeg.h"
#include "../../utils/debugutils.h"

using namespace std;

TextureImage::TextureImage(const std::string &filePath) : data(nullptr)
{
    size_t pos = filePath.rfind('.');
    unsigned char tried = 0;
    if (pos != string::npos) {  // try according to postfix
        string postfix(filePath.substr(pos + 1));
        for (char &c : postfix) {
            if (c >= 'A' && c <= 'Z')
                c += 'a' - 'A';
        }
        if ("png" == postfix) {
            try {
                tryPng(filePath);
                return;
            } catch (const TextureLoadException &e) {
                if (e.getError() == OPEN_FILE_FAILED)
                    throw e;
                LOG_PRINTLN(e.what());
            }
            tried = 1;
        } else if ("jpg" == postfix || "jpeg" == postfix) {
            try {
                tryJpeg(filePath);
                return;
            } catch (const TextureLoadException &e) {
                LOG_PRINTLN(e.what());
            }
            tried = 2;
        } else if ("bmp" == postfix || "spa" == postfix || "sph" == postfix) {
            try {
                tryBmp(filePath);
                return;
            } catch (const TextureLoadException &e) {
                LOG_PRINTLN(e.what());
            }
            tried = 3;
        } else if ("tga" == postfix) {
            try {
                tryTga(filePath);
                return;
            } catch (const TextureLoadException &e) {
                LOG_PRINTLN(e.what());
            }
            tried = 4;
        }
    }
    if (tried != 1) {
        try {
            tryPng(filePath);
            return;
        } catch (const TextureLoadException &e) {
            LOG_PRINTLN(e.what());
        }
    }
    if (tried != 2) {
        try {
            tryJpeg(filePath);
            return;
        } catch (const TextureLoadException &e) {
            LOG_PRINTLN(e.what());
        }
    }
    if (tried != 3) {
        try {
            tryBmp(filePath);
            return;
        } catch (const TextureLoadException &e) {
            LOG_PRINTLN(e.what());
        }
    }
    if (tried != 4) {
        try {
            tryTga(filePath);
            return;
        } catch (const TextureLoadException &e) {
            LOG_PRINTLN(e.what());
        }
    }
    throw TextureLoadException(UNSUPPORTED_FILE_TYPE);
}

const unsigned char *TextureImage::get() const
{
    return data;
}

int TextureImage::getWidth() const
{
    return width;
}

int TextureImage::getHeight() const
{
    return height;
}

TextureColorType TextureImage::getColorType() const
{
    return colorType;
}

TextureImage::~TextureImage()
{
    delete[] data;
}
//
// Created by wjy50 on 19-7-31.
//

#include <fstream>

#include "texture.h"

using namespace std;

void TextureImage::tryTga(const std::string &filePath)
{
    ifstream file(filePath, ios::binary);
    if (file) {
        unsigned char picInfoLen;
        file.read(reinterpret_cast<char *>(&picInfoLen), sizeof(picInfoLen));
        file.seekg(1, ios::cur);
        unsigned char type;
        file.read(reinterpret_cast<char *>(&type), sizeof(type));
        file.seekg(5, ios::cur);
        // int picType = 2;
        unsigned short lbx, lby;
        file.read(reinterpret_cast<char *>(&lbx), sizeof(lbx));
        file.read(reinterpret_cast<char *>(&lby), sizeof(lby));
        unsigned short width, height;
        file.read(reinterpret_cast<char *>(&width), sizeof(width));
        file.read(reinterpret_cast<char *>(&height), sizeof(height));
        this->width = width;
        this->height = height;
        unsigned char bitsPerPix, screenStart;
        file.read(reinterpret_cast<char *>(&bitsPerPix), sizeof(bitsPerPix));
        file.read(reinterpret_cast<char *>(&screenStart), sizeof(screenStart));
        screenStart = (unsigned char) ((screenStart & 0x10) / 16);
        if (picInfoLen > 0)
            file.seekg(picInfoLen, ios::cur);
        int area = width * height;
        int byteCount = bitsPerPix / 8;
        int size = area * byteCount;
        auto color = make_unique_array<unsigned char[]>(size);
        if (type == 2) {
            if (bitsPerPix == 32) {
                colorType = TEX_ARGB;
            } else if (bitsPerPix == 24) {
                colorType = TEX_RGB;
            } else {
                throw TextureLoadException(UNSUPPORTED_TGA_COLOR_TYPE);
            }
            int pitch = width * byteCount;
            for (int i = 0; i < height; ++i) {
                int offset = (height - i - 1) * pitch;
                file.read(reinterpret_cast<char *>(color.get() + offset), sizeof(unsigned char) * pitch);
            }
        } else if (type == 10) {
            int hi = 0, wi = 0;
            int offset = (height - hi - 1) * width;
            if (bitsPerPix == 32) {
                colorType = TEX_ARGB;
                unsigned int *colorInt = reinterpret_cast<unsigned int *>(color.get());
                while (hi < height) {
                    unsigned char head;
                    file.read(reinterpret_cast<char *>(&head), sizeof(head));
                    unsigned char dataType = head >> 7;
                    unsigned char len = (unsigned char) ((head & 0x7f) + 1);
                    if (dataType == 0) {
                        while (wi + len >= width) {
                            int count = width - wi;
                            file.read(reinterpret_cast<char *>(colorInt + offset + wi), sizeof(unsigned int) * count);
                            wi = 0;
                            ++hi;
                            offset = (height - hi - 1) * width;
                            len -= count;
                        }
                        file.read(reinterpret_cast<char *>(colorInt + offset + wi), sizeof(unsigned int) * len);
                        wi += len;
                    } else {
                        unsigned int c;
                        file.read(reinterpret_cast<char *>(&c), sizeof(c));
                        while (wi + len >= width) {
                            int count = width - wi;
                            for (int i = 0; i < count; ++i) {
                                colorInt[offset + wi + i] = c;
                            }
                            wi = 0;
                            ++hi;
                            offset = (height - hi - 1) * width;
                            len -= count;
                        }
                        for (int i = 0; i < len; ++i) {
                            colorInt[offset + wi + i] = c;
                        }
                        wi += len;
                    }
                }
            } else if (bitsPerPix == 24) {
                colorType = TEX_RGB;
                while (hi < height) {
                    unsigned char head;
                    file.read(reinterpret_cast<char *>(&head), sizeof(head));
                    unsigned char dataType = head >> 7;
                    unsigned char len = (unsigned char) ((head & 0x7f) + 1);
                    if (dataType == 0) {
                        while (wi + len >= width) {
                            int count = width - wi;
                            file.read(reinterpret_cast<char *>(color.get() + (offset + wi) * 3),
                                      sizeof(unsigned char) * count * 3);
                            wi = 0;
                            hi++;
                            offset = (height - hi - 1) * width;
                            len -= count;
                        }
                        file.read(reinterpret_cast<char *>(color.get() + (offset + wi) * 3), sizeof(unsigned char) * len * 3);
                        wi += len;
                    } else {
                        unsigned int c;
                        file.read(reinterpret_cast<char *>(&c), sizeof(c) * 3);
                        while (wi + len >= width) {
                            int count = width - wi;
                            for (int i = 0; i < count; ++i) {
                                int o = (offset + wi + i) * 3;
                                color[o] = (unsigned char) ((c >> 16) & 0xff);
                                color[o + 1] = (unsigned char) ((c >> 8) & 0xff);
                                color[o + 2] = (unsigned char) (c & 0xff);
                            }
                            wi = 0;
                            hi++;
                            offset = (height - hi - 1) * width;
                            len -= count;
                        }
                        for (int i = 0; i < len; ++i) {
                            int o = (offset + wi + i) * 3;
                            color[o] = (unsigned char) (c & 0xff);
                            color[o + 1] = (unsigned char) ((c >> 8) & 0xff);
                            color[o + 2] = (unsigned char) ((c >> 16) & 0xff);
                        }
                        wi += len;
                    }
                }
            } else {
                throw TextureLoadException(UNSUPPORTED_TGA_COLOR_TYPE);
            }
        } else {
            throw TextureLoadException(UNKNOWN_TGA_TYPE);
        }
        for (int i = 0; i < area; ++i) {
            int offset = i * byteCount;
            color[offset + 2] ^= color[offset];
            color[offset] = color[offset + 2] ^ color[offset];
            color[offset + 2] ^= color[offset];
        }
        data = color.release();
    } else
        throw TextureLoadException(OPEN_FILE_FAILED);
}
//
// Created by wjy50 on 19-7-31.
//

#include <fstream>

#include "texture.h"

using namespace std;

void TextureImage::tryBmp(const std::string &filePath)
{
    ifstream file(filePath, ios::binary);
    if (file) {
        unsigned short magic;
        file.read(reinterpret_cast<char *>(&magic), sizeof(magic));
        if (magic == 0x4d42) {
            int size;
            int offset;
            file.read(reinterpret_cast<char *>(&size), sizeof(size));
            file.seekg(4, ios::cur);
            file.read(reinterpret_cast<char *>(&offset), sizeof(offset));
            int biSize;
            int width, height;
            short planes;
            short bitCount;
            int compression;
            int imageSize;
            int xPixPerMeter;
            int yPixPerMeter;
            int colorUsed;
            int importantColor;
            file.read(reinterpret_cast<char *>(&biSize), sizeof(biSize));
            file.read(reinterpret_cast<char *>(&width), sizeof(width));
            file.read(reinterpret_cast<char *>(&height), sizeof(height));
            file.read(reinterpret_cast<char *>(&planes), sizeof(planes));
            file.read(reinterpret_cast<char *>(&bitCount), sizeof(bitCount));
            if (bitCount != 24 && bitCount != 32) {
                throw TextureLoadException(UNSUPPORTED_BMP_COLOR_TYPE);
            }
            file.read(reinterpret_cast<char *>(&compression), sizeof(compression));
            if (compression != 0) {
                throw TextureLoadException(UNSUPPORTED_BMP_COMPRESSION_TYPE);
            }
            file.read(reinterpret_cast<char *>(&imageSize), sizeof(imageSize));
            file.read(reinterpret_cast<char *>(&xPixPerMeter), sizeof(xPixPerMeter));
            file.read(reinterpret_cast<char *>(&yPixPerMeter), sizeof(yPixPerMeter));
            file.read(reinterpret_cast<char *>(&colorUsed), sizeof(colorUsed));
            file.read(reinterpret_cast<char *>(&importantColor), sizeof(importantColor));
            if (imageSize == 0)
                imageSize = width * height * bitCount / 8;
            long curPos = file.tellg();
            if (curPos == offset) {
                auto color = make_unique_array<unsigned char[]>(imageSize);
                int byteCount = bitCount  / 8;
                int pitch = width * byteCount;
                for (int i = 0; i < height; ++i) {
                    int o = (height - i - 1) * pitch;
                    file.read(reinterpret_cast<char *>(color.get() + o), sizeof(unsigned char) * pitch);
                }
                file.read(reinterpret_cast<char *>(color.get()), sizeof(unsigned char) * imageSize);
                file.close();
                this->width = width;
                this->height = height;
                data = color.release();
                colorType = bitCount == 24 ? TEX_RGB : TEX_ARGB;
                for (int i = 0; i < imageSize; i += byteCount) {
                    data[i + 2] ^= data[i];
                    data[i] ^= data[i + 2];
                    data[i + 2] ^= data[i];
                }
                return;
            }
        } else {
            throw TextureLoadException(INVALID_BMP);
        }
        file.close();
    } else
        throw TextureLoadException(OPEN_FILE_FAILED);
    throw TextureLoadException(UNKNOWN_ERROR);
}
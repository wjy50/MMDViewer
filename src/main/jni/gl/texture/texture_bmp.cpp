//
// Created by wjy50 on 19-7-31.
//

#include <fstream>

#include "texture.h"
#include "../../utils/mathutils.h"

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
            long curPos = file.tellg();
            if (curPos == offset) {
                bool fromBottom = true;
                if (height < 0) {
                    height = -height;
                    fromBottom = false;
                }
                this->width = width;
                this->height = height;
                int byteCount = bitCount  / 8;
                int rowSize = 4 * ((width * byteCount + 3) / 4);
                if (imageSize == 0)
                    imageSize = rowSize * height;
                auto color = make_unique_array<unsigned char[]>(imageSize);
                for (int i = 0; i < height; ++i) {
                    int o;
                    if (fromBottom)
                        o = (height - i - 1) * rowSize;
                    else
                        o = i * rowSize;
                    file.read(reinterpret_cast<char *>(color.get() + o), sizeof(unsigned char) * rowSize);
                }
                // file.read(reinterpret_cast<char *>(color.get()), sizeof(unsigned char) * imageSize);
                file.close();
                colorType = bitCount == 24 ? TEX_RGB : TEX_ARGB;
                for (int i = 0; i < height; ++i) {
                    for (int j = 0; j < width; ++j) {
                        flipBytes(reinterpret_cast<char *>(color.get() + i * rowSize + j * byteCount), 3);
                    }
                }
                data = color.release();
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
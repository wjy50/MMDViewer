//
// Created by wjy50 on 19-7-31.
//

#include <fstream>

#include "texture.h"
#include "../../utils/mathutils.h"

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
        unsigned short srcWidth, srcHeight;
        file.read(reinterpret_cast<char *>(&srcWidth), sizeof(srcWidth));
        file.read(reinterpret_cast<char *>(&srcHeight), sizeof(srcHeight));
        unsigned char bitsPerPix, screenStart;
        file.read(reinterpret_cast<char *>(&bitsPerPix), sizeof(bitsPerPix));
        file.read(reinterpret_cast<char *>(&screenStart), sizeof(screenStart));
        screenStart = (unsigned char) ((screenStart & 0x10) / 16);
        if (picInfoLen > 0)
            file.seekg(picInfoLen, ios::cur);
        if (bitsPerPix == 32) {
            colorType = TEX_ARGB;
        } else if (bitsPerPix == 24) {
            colorType = TEX_RGB;
        } else {
            throw TextureLoadException(UNSUPPORTED_TGA_COLOR_TYPE);
        }
        width = srcWidth;
        height = srcHeight;
        int byteCount = bitsPerPix / 8;
        int rowSize = 4 * ((width * byteCount + 3) / 4);
        auto color = make_unique_array<unsigned char[]>(rowSize * height);
        if (type == 2) {
            for (int i = 0; i < height; ++i) {
                int offset = (height - i - 1) * rowSize;
                file.read(reinterpret_cast<char *>(color.get() + offset),
                        sizeof(unsigned char) * width * byteCount);
            }
        } else if (type == 10) {
            int hi = 0, wi = 0;
            int offset = (height - hi - 1) * rowSize;
            while (hi < height) {
                unsigned char head;
                file.read(reinterpret_cast<char *>(&head), sizeof(head));
                unsigned char dataType = head >> 7;
                unsigned char len = static_cast<unsigned char>((head & 0x7f) + 1);
                if (dataType == 0) {
                    while (wi + len >= width) {
                        int count = width - wi;
                        file.read(reinterpret_cast<char *>(color.get() + offset + wi * byteCount),
                                  sizeof(unsigned char) * count * byteCount);
                        wi = 0;
                        hi++;
                        offset = (height - hi - 1) * rowSize;
                        len -= count;
                    }
                    file.read(reinterpret_cast<char *>(color.get() + offset + wi * byteCount),
                            sizeof(unsigned char) * len * byteCount);
                    wi += len;
                } else {
                    unsigned int c;
                    file.read(reinterpret_cast<char *>(&c), sizeof(unsigned char) * byteCount);
                    while (wi + len >= width) {
                        int count = width - wi;
                        for (int i = 0; i < count; ++i) {
                            int o = offset + (wi + i) * byteCount;
                            color[o] = static_cast<unsigned char>(c & 0xff);
                            color[o + 1] = static_cast<unsigned char>((c >> 8) & 0xff);
                            color[o + 2] = static_cast<unsigned char>((c >> 16) & 0xff);
                            if (colorType == TEX_ARGB)
                                color[o + 3] = static_cast<unsigned char>((c >> 24) & 0xff);
                        }
                        wi = 0;
                        hi++;
                        offset = (height - hi - 1) * rowSize;
                        len -= count;
                    }
                    for (int i = 0; i < len; ++i) {
                        int o = offset + (wi + i) * byteCount;
                        color[o] = static_cast<unsigned char>(c & 0xff);
                        color[o + 1] = static_cast<unsigned char>((c >> 8) & 0xff);
                        color[o + 2] = static_cast<unsigned char>((c >> 16) & 0xff);
                        if (colorType == TEX_ARGB)
                            color[o + 3] = static_cast<unsigned char>((c >> 24) & 0xff);
                    }
                    wi += len;
                }
            }
        } else {
            throw TextureLoadException(UNKNOWN_TGA_TYPE);
        }
        for (int i = 0; i < height; ++i) {
            int offset = i * rowSize;
            for (int j = 0; j < width; ++j) {
                flipBytes(reinterpret_cast<char *>(color.get() + offset + j * byteCount), 3);
            }
        }
        data = color.release();
    } else
        throw TextureLoadException(OPEN_FILE_FAILED);
}
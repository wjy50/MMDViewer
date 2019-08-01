//
// Created by wjy50 on 19-7-31.
//

#include <string>

#include "texture.h"
#include "../../png/png.h"

using namespace std;

class FilePointerWrapper
{
private:
    FILE *file;
public:
    FilePointerWrapper(const char *filePath, const char *mode) : file(fopen(filePath, mode))
    {}

    FILE *get()
    {
        return file;
    }

    ~FilePointerWrapper()
    {
        if (file)
            fclose(file);
    }
};

class PngPtrWrapper
{
private:
    png_structp png_ptr;
    png_infop info_ptr;
public:
    PngPtrWrapper()
            : png_ptr(png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)),
              info_ptr(png_create_info_struct(png_ptr))
    {}

    png_structp getPngPtr()
    {
        return png_ptr;
    }

    png_infop getInfoPtr()
    {
        return info_ptr;
    }

    ~PngPtrWrapper()
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    }
};

void TextureImage::tryPng(const std::string &filePath)
{
    FilePointerWrapper file(filePath.data(), "rb");
    if (file.get()) {
        png_byte sig[8];
        fread(sig, sizeof(png_byte), 8, file.get());
        if (png_sig_cmp(sig, 0, 8) != 0) {
            throw TextureLoadException(INVALID_PNG);
        }
        fseek(file.get(), 0, SEEK_SET);
        PngPtrWrapper pngPtrWrapper;
        if (pngPtrWrapper.getPngPtr() && pngPtrWrapper.getInfoPtr()) {
            png_init_io(pngPtrWrapper.getPngPtr(), file.get());
            png_read_png(pngPtrWrapper.getPngPtr(), pngPtrWrapper.getInfoPtr(), PNG_TRANSFORM_EXPAND, 0);
            width = static_cast<int>(png_get_image_width(pngPtrWrapper.getPngPtr(), pngPtrWrapper.getInfoPtr()));
            height = static_cast<int>(png_get_image_height(pngPtrWrapper.getPngPtr(), pngPtrWrapper.getInfoPtr()));
            int color_type = png_get_color_type(pngPtrWrapper.getPngPtr(), pngPtrWrapper.getInfoPtr());
            png_bytep *row_pointers = png_get_rows(pngPtrWrapper.getPngPtr(), pngPtrWrapper.getInfoPtr());
            int byteCount;
            if (color_type == PNG_COLOR_TYPE_RGBA) {
                colorType = TEX_ARGB;
                byteCount = 4;
            } else if (color_type == PNG_COLOR_TYPE_RGB) {
                colorType = TEX_RGB;
                byteCount = 3;
            } else {
                throw TextureLoadException(UNSUPPORTED_PNG_COLOR_TYPE);
            }
            int rowSize = 4 * ((width * byteCount + 3) / 4);
            auto color = make_unique_array<unsigned char[]>(rowSize * height);
            for (int i = 0; i < height; ++i) {
                int o = i * rowSize;
                memcpy(color.get() + o, row_pointers[i], static_cast<size_t>(width * byteCount));
            }
            data = color.release();
            return;
        }
    } else
        throw TextureLoadException(OPEN_FILE_FAILED);
    throw TextureLoadException(UNKNOWN_ERROR);
}
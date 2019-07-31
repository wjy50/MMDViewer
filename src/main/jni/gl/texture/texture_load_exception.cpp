//
// Created by wjy50 on 19-7-31.
//

#include "texture.h"

TextureLoadException::TextureLoadException(TextureLoadError error)
: error(error)
{}

const char* TextureLoadException::what() const noexcept
{
    switch (error) {
        case INVALID_JPEG:
            return "Invalid jpeg";
        case INVALID_PNG:
            return "Invalid png";
        case UNSUPPORTED_PNG_COLOR_TYPE:
            return "Unsupported png color type";
        case INVALID_BMP:
            return "Invalid bmp";
        case UNSUPPORTED_BMP_COLOR_TYPE:
            return "Unsupported bmp color type";
        case UNSUPPORTED_BMP_COMPRESSION_TYPE:
            return "Unsupported bmp compression type";
        case OPEN_FILE_FAILED:
            return "Failed to open file";
        case UNSUPPORTED_TGA_COLOR_TYPE:
            return "Unsupported tga color type";
        case UNKNOWN_TGA_TYPE:
            return "Unknown tga type";
        case UNSUPPORTED_FILE_TYPE:
            return "Unsupported file type";
        default:
            return "Unknown error";
    }
}
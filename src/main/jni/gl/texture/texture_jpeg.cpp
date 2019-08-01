//
// Created by wjy50 on 19-7-31.
//

#include <fstream>

#include "texture.h"
#include "../../turbojpeg/turbojpeg.h"

using namespace std;

class TjHandleWrapper
{
private:
    tjhandle handle;
public:
    TjHandleWrapper() : handle(tjInitDecompress())
    {}

    const tjhandle &get()
    {
        return handle;
    }

    ~TjHandleWrapper()
    {
        tjDestroy(handle);
    }
};

void TextureImage::tryJpeg(const std::string &filePath)
{
    ifstream file(filePath, ios::binary);
    if (file) {
        TjHandleWrapper tjHandleWrapper;
        file.seekg(0, ios::end);
        unsigned long jpegSize = static_cast<unsigned long>(file.tellg());
        if (jpegSize > 0) {
            file.seekg(0, ios::beg);
            auto jpegBuffer = make_unique_array<char[]>(jpegSize);
            file.read(jpegBuffer.get(), jpegSize * sizeof(char));
            file.close();
            if (jpegBuffer[0] != 0xff || jpegBuffer[1] != 0xd8 || jpegBuffer[2] != 0xff) {
                throw TextureLoadException(INVALID_JPEG);
            }
            int jpegSubSample, jpegColorSpace;
            int r = tjDecompressHeader3(
                    tjHandleWrapper.get(),
                    reinterpret_cast<const unsigned char *>(jpegBuffer.get()),
                    jpegSize, &width, &height, &jpegSubSample, &jpegColorSpace);
            if (r == 0) {
                int pixelFormat = TJPF_RGB;
                int rowSize = 4 * ((width * tjPixelSize[pixelFormat] + 3) / 4);
                size_t destSize = static_cast<size_t>(rowSize * height);
                auto temp = make_unique_array<unsigned char[]>(destSize);
                r = tjDecompress(
                        tjHandleWrapper.get(), reinterpret_cast<unsigned char *>(jpegBuffer.get()),
                        jpegSize, temp.get(), width, rowSize, height, tjPixelSize[pixelFormat], 0);
                if (r == 0) {
                    colorType = TEX_RGB;
                    data = temp.release();
                    return;
                }
            }
        } else
            file.close();
    } else
        throw TextureLoadException(OPEN_FILE_FAILED);
    throw TextureLoadException(UNKNOWN_ERROR);
}
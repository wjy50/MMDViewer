//
// Created by wjy50 on 18-3-17.
//

#include "pmxtexture.h"
#include "../../gl/texture/texture.h"
#include "../../utils/debugutils.h"

PMXTexture::PMXTexture() : path(nullptr), textureId(0), mHasAlpha(false)
{}

GLuint PMXTexture::getTextureId()
{
    return textureId;
}

void PMXTexture::initGLTexture()
{
    if (textureId == 0) {
        try {
            TextureImage image(path);
            glGenTextures(1, &textureId);
            if (textureId == 0) {
                LOG_PRINTF("gen texture failed, path = %s", path);
                return;
            }
            LOG_PRINTF("%dx%d, path=%s", image.getWidth(), image.getHeight(), path);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            GLenum format = image.getColorType() == TEX_ARGB ? GL_RGBA : GL_RGB;
            LOG_PRINTF("gen tex err=%d", glGetError());
            glTexImage2D(GL_TEXTURE_2D, 0, format, image.getWidth(), image.getHeight(), 0, format,
                         GL_UNSIGNED_BYTE, image.get());
            LOG_PRINTF("image2d err=%d", glGetError());
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
            mHasAlpha = image.getColorType() == TEX_ARGB;
        } catch (const TextureLoadException &e) {
            LOG_PRINTF("Gen texture failed, path = %s, due to %s", path, e.what());
        }
    }
}

void PMXTexture::read(std::ifstream &file, MStringEncoding encoding, const char *pmxPath, int pathLength)
{
    MString textureName;
    textureName.readString(file, encoding, UTF_8);
    size_t l = textureName.length();
    char *absPath = new char[l + pathLength + 1];
    absPath[l + pathLength] = 0;
    for (int i = 0; i < pathLength; ++i) {
        absPath[i] = pmxPath[i];
    }
    for (size_t i = 0; i < l; ++i) {
        if (textureName[i] == '\\')
            absPath[pathLength + i] = '/';
        else
            absPath[pathLength + i] = textureName[i];
    }
    path = absPath;
}

bool PMXTexture::hasAlpha()
{
    return mHasAlpha;
}

PMXTexture::~PMXTexture()
{
    delete[] path;
    if (textureId != 0)
        glDeleteTextures(1, &textureId);
}